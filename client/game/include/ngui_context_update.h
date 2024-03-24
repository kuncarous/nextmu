#ifndef __NGUI_CONTEXT_UPDATE_H__
#define __NGUI_CONTEXT_UPDATE_H__

#pragma once

#include <NsGui/Enums.h>
#include "ngui_notifier.h"
#include "upd_enum.h"

class NGUpdateContext : public NoesisGUI::NotifierBase
{
public:
	mu_float GetProgress() const;
	void SetProgress(mu_float value);

	const char *GetState() const;
	void SetState(const char *value);

	mu_uint32 GetVerifyCount() const;
	void SetVerifyCount(mu_uint32 value);

	mu_uint32 GetVerifiedCount() const;
	void SetVerifiedCount(mu_uint32 value);

	const char *GetDownloadSizeType() const;
	void SetDownloadSizeType(const char *value);

	mu_float GetDownloadSize() const;
	void SetDownloadSize(mu_float value);

	mu_float GetDownloadedSize() const;
	void SetDownloadedSize(mu_float value);

private:
	mu_float Progress = 0.0f;
	Noesis::String State = "Initializing";
	mu_uint32 VerifyCount = 0u;
	mu_uint32 VerifiedCount = 0u;
	Noesis::String DownloadSizeType;
	mu_float DownloadSize = 0.0f;
	mu_float DownloadedSize = 0.0f;

private:
    NS_DECLARE_REFLECTION(NGUpdateContext, NoesisGUI::NotifierBase)
};

#endif