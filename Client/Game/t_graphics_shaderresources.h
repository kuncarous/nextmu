#ifndef __T_GRAPHICS_SHADERRESOURCES_H__
#define __T_GRAPHICS_SHADERRESOURCES_H__

#pragma once

#include "t_graphics_resources.h"

typedef mu_uint32 NShaderParentId;
typedef mu_uint32 NShaderResourcesId;
typedef mu_uint32 NShaderResourcesComponents;

struct NShaderResourcesBinding
{
	NShaderParentId ParentId;
	NShaderResourcesId ShaderResourceId;
	mu_boolean Initialized;
	mu_boolean ShouldTransition;
	std::vector<NResourceId> Resources;
	Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> Binding;
};

typedef std::vector<std::unique_ptr<NShaderResourcesBinding>> NShaderResourcesVector;
typedef std::map<NShaderResourcesComponents, NShaderResourcesVector> NShaderResourcesMapByComponents;
struct NPipelineShaderResources
{
	NPipelineStateId Id;
	NShaderResourcesMapByComponents Resources;
};

struct NShaderResourceLink
{
	NShaderResourcesComponents Components;
	NShaderResourcesId ShaderResourceId;
};

struct NResourceLinkInformation
{
	std::map<NPipelineStateId, std::vector<NShaderResourceLink>> Bindings;
};

template<typename ParentType = Diligent::IPipelineState>
class NShaderResourcesBindingManager
{
public:
	void Destroy()
	{
		ShaderResourcesMap.clear();
		TemporaryShaderResourcesMap.clear();
		ResourceLinksMap.clear();
	}

	void MergeTemporaryShaderBindings()
	{
		for (auto &[pipelineId, tmpPipelineResources] : TemporaryShaderResourcesMap)
		{
			auto pipelineResourcesIter = ShaderResourcesMap.find(pipelineId);
			if (pipelineResourcesIter == ShaderResourcesMap.end())
			{
				pipelineResourcesIter = ShaderResourcesMap.insert(std::make_pair(pipelineId, NPipelineShaderResources())).first;
			}
			auto &pipelineResources = pipelineResourcesIter->second;

			for (auto &[numResources, tmpShaderResources] : tmpPipelineResources.Resources)
			{
				auto componentsIter = pipelineResources.Resources.find(numResources);
				if (componentsIter == pipelineResources.Resources.end())
				{
					componentsIter = pipelineResources.Resources.insert(std::make_pair(numResources, NShaderResourcesVector())).first;
				}
				auto &shaderResources = componentsIter->second;

				shaderResources.insert(shaderResources.end(), std::make_move_iterator(tmpShaderResources.begin()), std::make_move_iterator(tmpShaderResources.end()));
				std::sort(
					shaderResources.begin(),
					shaderResources.end(),
					[](const std::unique_ptr<NShaderResourcesBinding> &lhs, const std::unique_ptr<NShaderResourcesBinding> &rhs) -> bool { return lhs->ShaderResourceId < rhs->ShaderResourceId; }
				);
			}
		}

		TemporaryShaderResourcesMap.clear();
	}

private:
	NShaderResourcesBinding *CreateShaderBinding(NShaderParentId parentId, ParentType *parent, const mu_uint32 numResources, const NResourceId *resources)
	{
		std::unique_ptr<NShaderResourcesBinding> binding(new (std::nothrow) NShaderResourcesBinding());
		parent->CreateShaderResourceBinding(&binding->Binding, true);
		if (binding->Binding == nullptr)
		{
			return nullptr;
		}

		const auto shaderResourceId = IdGenerator++;
		binding->ParentId = parentId;
		binding->Initialized = false;
		binding->ShouldTransition = true;
		binding->ShaderResourceId = shaderResourceId;
		binding->Resources.insert(binding->Resources.begin(), resources, resources + numResources);

		auto pipelineResourcesIter = TemporaryShaderResourcesMap.find(parentId);
		if (pipelineResourcesIter == TemporaryShaderResourcesMap.end())
		{
			pipelineResourcesIter = TemporaryShaderResourcesMap.insert(std::make_pair(parentId, NPipelineShaderResources())).first;
		}
		auto &pipelineResources = pipelineResourcesIter->second;

		auto componentsIter = pipelineResources.Resources.find(numResources);
		if (componentsIter == pipelineResources.Resources.end())
		{
			componentsIter = pipelineResources.Resources.insert(std::make_pair(numResources, NShaderResourcesVector())).first;
		}
		auto &shaderResources = componentsIter->second;
		shaderResources.push_back(std::move(binding));

		for (mu_uint32 n = 0; n < numResources; ++n)
		{
			auto resourceId = resources[n];

			auto resourceLinksIter = ResourceLinksMap.find(resourceId);
			if (resourceLinksIter == ResourceLinksMap.end())
			{
				resourceLinksIter = ResourceLinksMap.insert(std::make_pair(resourceId, NResourceLinkInformation())).first;
			}
			auto &resourceLinks = resourceLinksIter->second;

			auto pipelineIter = resourceLinks.Bindings.find(parentId);
			if (pipelineIter == resourceLinks.Bindings.end())
			{
				pipelineIter = resourceLinks.Bindings.insert(std::make_pair(parentId, std::vector<NShaderResourceLink>())).first;
			}
			auto &bindings = pipelineIter->second;

			bindings.push_back(
				NShaderResourceLink{
					.Components = numResources,
					.ShaderResourceId = shaderResourceId,
				}
			);
		}

		return shaderResources[shaderResources.size() - 1].get();
	}

	NShaderResourcesBinding *GetShaderBindingFromTemporaryMap(NShaderParentId parentId, ParentType *parent, const mu_uint32 numResources, const NResourceId *resources)
	{
		std::lock_guard lock(Mutex);
		auto pipelineResourcesIter = TemporaryShaderResourcesMap.find(parentId);
		if (pipelineResourcesIter == TemporaryShaderResourcesMap.end()) return CreateShaderBinding(parentId, parent, numResources, resources);
		auto &pipelineResources = pipelineResourcesIter->second;

		auto componentsIter = pipelineResources.Resources.find(numResources);
		if (componentsIter == pipelineResources.Resources.end()) return CreateShaderBinding(parentId, parent, numResources, resources);
		auto &shaderResources = componentsIter->second;

		for (auto shaderResourceIter = shaderResources.begin(); shaderResourceIter != shaderResources.end(); ++shaderResourceIter)
		{
			auto &shaderResource = *shaderResourceIter;
			if (mu_memcmp(resources, shaderResource->Resources.data(), sizeof(NResourceId) * numResources) == 0) return shaderResource.get();
		}

		return CreateShaderBinding(parentId, parent, numResources, resources);
	}

public:
	NShaderResourcesBinding *GetShaderBinding(NShaderParentId parentId, ParentType *parent, const mu_uint32 numResources, NResourceId *resources)
	{
		// Sort Resources
		std::sort(resources, resources + numResources);

		auto pipelineResourcesIter = ShaderResourcesMap.find(parentId);
		if (pipelineResourcesIter == ShaderResourcesMap.end()) return GetShaderBindingFromTemporaryMap(parentId, parent, numResources, resources);
		auto &pipelineResources = pipelineResourcesIter->second;

		auto componentsIter = pipelineResources.Resources.find(numResources);
		if (componentsIter == pipelineResources.Resources.end()) return GetShaderBindingFromTemporaryMap(parentId, parent, numResources, resources);
		auto &shaderResources = componentsIter->second;

		for (auto shaderResourceIter = shaderResources.begin(); shaderResourceIter != shaderResources.end(); ++shaderResourceIter)
		{
			auto &shaderResource = *shaderResourceIter;
			if (mu_memcmp(resources, shaderResource->Resources.data(), sizeof(NResourceId) * numResources) == 0) return shaderResource.get();
		}

		return GetShaderBindingFromTemporaryMap(parentId, parent, numResources, resources);
	}

	void ReleaseShaderResourcesByResourceId(const NResourceId id)
	{
		std::lock_guard lock(Mutex);
		auto iter = ResourceLinksMap.find(id);
		if (iter == ResourceLinksMap.end()) return;
		auto &info = iter->second;

		for (auto &[pipelineId, resourcesLink] : info.Bindings)
		{
			auto pipelineIter = ShaderResourcesMap.find(pipelineId);
			if (pipelineIter == ShaderResourcesMap.end()) continue;
			auto &resourcesInfo = pipelineIter->second;

			NShaderResourcesComponents componentsCount = 0;
			NShaderResourcesMapByComponents::iterator componentsIter = resourcesInfo.Resources.end();

			for (auto resourceLinkIter = resourcesLink.begin(); resourceLinkIter != resourcesLink.end();)
			{
				auto &resourceLink = *resourceLinkIter;
				if (resourceLink.Components != componentsCount)
				{
					componentsIter = resourcesInfo.Resources.find(resourceLink.Components);
					if (componentsIter == resourcesInfo.Resources.end())
					{
						mu_assert(!"shader resources by components not found");
					}
				}
				if (componentsIter == resourcesInfo.Resources.end())
				{
					++resourceLinkIter;
					continue;
				}
				auto &resources = componentsIter->second;

				for (auto resourceIter = resources.begin(); resourceIter != resources.end(); ++resourceIter)
				{
					auto &resource = *resourceIter;
					if (resource->ShaderResourceId > resourceLink.ShaderResourceId)
					{
						mu_assert(!"shader resource is missing");
						++resourceLinkIter;
						break;
					}

					if (resource->ShaderResourceId == resourceLink.ShaderResourceId)
					{
						resources.erase(resourceIter);
						resourceLinkIter = resourcesLink.erase(resourceLinkIter);
						break;
					}
				}
			}

			if (resourcesLink.empty() == false)
			{
				mu_assert(!"some resources are missing");
			}
		}

		ResourceLinksMap.erase(iter);
	}

	void ReleaseShaderResources(const NShaderResourcesBinding *binding)
	{
		auto pipelineResourcesIter = ShaderResourcesMap.find(binding->ParentId);
		if (pipelineResourcesIter == ShaderResourcesMap.end()) return;
		auto &pipelineResources = pipelineResourcesIter->second;

		auto componentsIter = pipelineResources.Resources.find(static_cast<NShaderResourcesComponents>(binding->Resources.size()));
		if (componentsIter == pipelineResources.Resources.end()) return;
		auto &shaderResources = componentsIter->second;

		for (auto shaderResourceIter = shaderResources.begin(); shaderResourceIter != shaderResources.end(); ++shaderResourceIter)
		{
			auto &shaderResource = *shaderResourceIter;
			if (shaderResource->ShaderResourceId != binding->ShaderResourceId) continue;

			for (const auto resourceId : shaderResource->Resources)
			{
				auto resourceLinksIter = ResourceLinksMap.find(resourceId);
				if (resourceLinksIter == ResourceLinksMap.end()) continue;
				auto &resourceLinks = resourceLinksIter->second;

				auto pipelineIter = resourceLinks.Bindings.find(shaderResource->PipelineId);
				if (pipelineIter == resourceLinks.Bindings.end()) continue;
				auto &resources = pipelineIter->second;

				for (auto resourceIter = resources.begin(); resourceIter != resources.end(); ++resourceIter)
				{
					auto &resource = *resourceIter;
					if (resource.ShaderResourceId == shaderResource->ShaderResourceId)
					{
						resources.erase(resourceIter);
						break;
					}
				}
			}
			shaderResources.erase(shaderResourceIter);
			break;
		}
	}

private:
	std::mutex Mutex;
	mu_uint32 IdGenerator = 0u;
	std::map<NPipelineStateId, NPipelineShaderResources> ShaderResourcesMap;
	std::map<NPipelineStateId, NPipelineShaderResources> TemporaryShaderResourcesMap;
	std::map<NResourceId, NResourceLinkInformation> ResourceLinksMap;
};

extern NShaderResourcesBindingManager<Diligent::IPipelineState> ShaderResourcesBindingManager;

#endif