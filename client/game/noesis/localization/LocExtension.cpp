////////////////////////////////////////////////////////////////////////////////////////////////////
// NoesisGUI - http://www.noesisengine.com
// Copyright (c) 2013 Noesis Technologies S.L. All Rights Reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////


#include "LocExtension.h"
#include <NsCore/ReflectionImplement.h>
#include <NsGui/ValueTargetProvider.h>
#include <NsGui/ContentPropertyMetaData.h>
#include <NsGui/DependencyObject.h>
#include <NsGui/FrameworkPropertyMetadata.h>
#include <NsGui/ResourceDictionary.h>
#include <NsGui/UIElementData.h>
#include <NsGui/Uri.h>


using namespace Noesis;
using namespace NoesisApp;


////////////////////////////////////////////////////////////////////////////////////////////////////
static Ptr<BaseComponent> Evaluate(const DependencyProperty *targetProperty,
	const char *resourceKey, const ResourceDictionary *resourceDictionary)
{
	if (resourceDictionary != nullptr)
	{
		Ptr<BaseComponent> resource(resourceDictionary->Get(resourceKey));
		if (resource != nullptr && resource != DependencyProperty::GetUnsetValue())
		{
			return resource;
		}

		NS_ERROR("Resource key '%s' not found in Loc Resources", resourceKey);
	}

	if (targetProperty->GetType() == TypeOf<String>()
		|| targetProperty->GetType() == TypeOf<BaseComponent>())
	{
		return Boxing::Box<String>(String(String::VarArgs(),
			"<Loc !%s>", resourceKey));
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace
{
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Expression created from LocExtension to look for localized resources.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class LocMonitor : public BaseComponent
	{
	public:
		LocMonitor(DependencyObject *targetObject) : mTargetObject(targetObject)
		{
			NS_ASSERT(targetObject != nullptr);
			NS_ASSERT(targetObject->GetNumReferences() > 0);
		}

		~LocMonitor()
		{
			mTargetObject = nullptr;
		}

		DependencyObject *GetTargetObject() const
		{
			return mTargetObject;
		}

		Ptr<BaseComponent> AddDependencyProperty(const DependencyProperty *targetProperty,
			const char *resourceKey)
		{
			NS_ASSERT(mTargetObject != nullptr);

			const ResourceDictionary *resources = LocExtension::GetResources(mTargetObject);

			Symbol resourceKeySymbol = Symbol(resourceKey);
			for (MonitorPair &element : mMonitoredDependencyProperties)
			{
				if (element.first == targetProperty)
				{
					element.second = resourceKeySymbol;
					return Evaluate(targetProperty, resourceKey, resources);
				}
			}

			mMonitoredDependencyProperties.EmplaceBack(targetProperty, resourceKeySymbol);

			return Evaluate(targetProperty, resourceKey, resources);
		}

		void InvalidateResources(const ResourceDictionary *resourceDictionary) const
		{
			for (const MonitorPair &element : mMonitoredDependencyProperties)
			{
				mTargetObject->SetValueObject(element.first, Evaluate(element.first,
					element.second.Str(), resourceDictionary));
			}
		}

		Ptr<LocMonitor> Clone(DependencyObject *targetObject) const
		{
			Ptr<LocMonitor> clone = MakePtr<LocMonitor>(targetObject);
			clone->mMonitoredDependencyProperties.Resize(mMonitoredDependencyProperties.Size());
			for (uint32_t i = 0; i < mMonitoredDependencyProperties.Size(); i++)
			{
				clone->mMonitoredDependencyProperties[i] = mMonitoredDependencyProperties[i];
			}
			return clone;
		}

		NS_IMPLEMENT_INLINE_REFLECTION_(LocMonitor, BaseComponent)

	private:
		typedef Pair<const DependencyProperty *, Symbol> MonitorPair;

		DependencyObject *mTargetObject;
		Vector<MonitorPair, 1> mMonitoredDependencyProperties;
	};

}

////////////////////////////////////////////////////////////////////////////////////////////////////
static const DependencyProperty *ResourcesProperty;
static const DependencyProperty *MonitorProperty;

////////////////////////////////////////////////////////////////////////////////////////////////////
LocExtension::LocExtension(const char *key) : mResourceKey(key)
{
	if (mResourceKey.Empty())
	{
		NS_ERROR("LocExtension requires a valid string ResourceKey");
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ResourceDictionary *LocExtension::GetResources(const DependencyObject *element)
{
	NS_CHECK(element != nullptr, "Element is null");
	return element->GetValue<Ptr<ResourceDictionary>>(ResourcesProperty);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool LocExtension::IsResourcesProperty(const DependencyProperty *dp)
{
	return dp == ResourcesProperty;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
const Uri &LocExtension::GetSource(const DependencyObject *element)
{
	NS_CHECK(element != nullptr, "Element is null");
	return element->GetValue<Uri>(SourceProperty);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void LocExtension::SetSource(DependencyObject *element, const Uri &source)
{
	NS_CHECK(element != nullptr, "Element is null");
	element->SetValue<Uri>(SourceProperty, source);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
const char *LocExtension::GetResourceKey() const
{
	return mResourceKey.Str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void LocExtension::SetResourceKey(const char *key)
{
	mResourceKey = key;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
Ptr<BaseComponent> LocExtension::ProvideValue(const ValueTargetProvider *provider)
{
	NS_ASSERT(provider != nullptr);
	const DependencyProperty *targetProperty = provider->GetTargetProperty();
	NS_ASSERT(targetProperty != nullptr);

	DependencyObject *target = DynamicCast<DependencyObject *>(provider->GetTargetObject());

	Ptr<LocMonitor> monitor = target->GetValue<Ptr<LocMonitor>>(MonitorProperty);

	if (monitor == nullptr)
	{
		monitor = MakePtr<LocMonitor>(target);
		target->SetValue<Ptr<LocMonitor>>(MonitorProperty, monitor);
	}

	return monitor->AddDependencyProperty(targetProperty, mResourceKey.Str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void OnSourceChanged(DependencyObject *obj, const DependencyPropertyChangedEventArgs &)
{
	const Uri sourceUri = obj->GetValue<Uri>(LocExtension::SourceProperty);

	const Ptr<ResourceDictionary> resourceDictionary = MakePtr<ResourceDictionary>();
	resourceDictionary->SetSource(sourceUri);

	obj->SetValue<Ptr<ResourceDictionary>>(ResourcesProperty, resourceDictionary);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
static void OnResourcesChanged(DependencyObject *d, const DependencyPropertyChangedEventArgs &args)
{
	Ptr<LocMonitor> monitor = d->GetValue<Ptr<LocMonitor>>(MonitorProperty);

	if (monitor != nullptr)
	{
		if (monitor->GetTargetObject() != d)
		{
			monitor.Reset(monitor->Clone(d));
			d->SetValue<Ptr<LocMonitor>>(MonitorProperty, monitor);
		}
		monitor->InvalidateResources(args.NewValue<Ptr<ResourceDictionary>>());
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
NS_BEGIN_COLD_REGION

NS_IMPLEMENT_REFLECTION(LocExtension, "NoesisGUIExtensions.Loc")
{
	NsMeta<ContentPropertyMetaData>("ResourceKey");
	NsProp("ResourceKey", &LocExtension::GetResourceKey,
		&LocExtension::SetResourceKey);

	DependencyData *data = NsMeta<DependencyData>(TypeOf<SelfClass>());
	data->RegisterProperty<Uri>(SourceProperty, "Source",
		PropertyMetadata::Create(Uri(), OnSourceChanged));
	data->RegisterProperty<Ptr<ResourceDictionary>>(ResourcesProperty, ".Resources",
		FrameworkPropertyMetadata::Create(Ptr<ResourceDictionary>(),
			FrameworkPropertyMetadataOptions_Inherits, OnResourcesChanged));
	data->RegisterProperty<Ptr<LocMonitor>>(MonitorProperty, ".Monitor",
		PropertyMetadata::Create(Ptr<LocMonitor>()));
}

NS_END_COLD_REGION

////////////////////////////////////////////////////////////////////////////////////////////////////
const DependencyProperty *LocExtension::SourceProperty;
