#ifndef __NPOPUP_BASE_H__
#define __NPOPUP_BASE_H__

#pragma once

namespace MUPopup
{

	// OLD
	class NPopupBase
	{
	public:
		NPopupBase(const EPopupID id,
			const EPopupType type) : Id(id),
			Type(type)
		{}
		virtual ~NPopupBase() {}

	public:
		const EPopupID Id;
		const EPopupType Type;
	};

	class NCommonPopup : public NPopupBase
	{
	public:
		NCommonPopup(const EPopupID id,
			const mu_utf8string message) :
			NPopupBase(id, EPopupType::eOk),
			Popup(message)
		{}
		virtual ~NCommonPopup() {}

	public:
		const mu_utf8string Popup;
	};

	class NAcceptCancelPopup : public NPopupBase
	{
	public:
		NAcceptCancelPopup(const EPopupID id,
			const mu_utf8string message) :
			NPopupBase(id, EPopupType::eOk),
			Popup(message)
		{}
		virtual ~NAcceptCancelPopup() override {}

	public:
		const mu_utf8string Popup;
	};

	class NStaticPopup : public NPopupBase
	{
	public:
		NStaticPopup(const EPopupID id,
			const mu_utf8string message) :
			NPopupBase(id, EPopupType::eNone),
			Popup(message)
		{}
		virtual ~NStaticPopup() override {}

	public:
		const mu_utf8string Popup;
	};

	class NYesNoPopup : public NPopupBase
	{
	public:
		NYesNoPopup(
			const EPopupID id,
			const mu_utf8string message
		) :
			NPopupBase(id, EPopupType::eYesNo),
			Popup(message)
		{}
		virtual ~NYesNoPopup() override {}

	public:
		const mu_utf8string Popup;
	};

	class NYesNoKeyPopup : public NPopupBase
	{
	public:
		NYesNoKeyPopup(
			const EPopupID id,
			const mu_utf8string message,
			const mu_uint32 key
		) :
			NPopupBase(id, EPopupType::eYesNoKey),
			Popup(message),
			Key(key)
		{}
		virtual ~NYesNoKeyPopup() override {}

	public:
		const mu_utf8string Popup;
		const mu_uint32 Key;
	};

	class NCancelPopup : public NPopupBase
	{
	public:
		NCancelPopup(
			const EPopupID id,
			const mu_utf8string message
		) :
			NPopupBase(id, EPopupType::eCancel),
			Popup(message)
		{}
		virtual ~NCancelPopup() override {}

	public:
		const mu_utf8string Popup;
	};
};

#endif