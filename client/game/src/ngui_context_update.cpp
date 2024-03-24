#include "mu_precompiled.h"
#include "ngui_context_update.h"

mu_float NGUpdateContext::GetProgress() const
{
	return Progress;
}

void NGUpdateContext::SetProgress(mu_float value)
{
	if (Progress == value) return;
	Progress = value;
	OnPropertyChanged("Progress");
}

const char *NGUpdateContext::GetState() const
{
	return State.Str();
}

void NGUpdateContext::SetState(const char *value)
{
	if (State == value) return;
	State = value;
	OnPropertyChanged("State");
}

mu_uint32 NGUpdateContext::GetVerifyCount() const
{
	return VerifyCount;
}

void NGUpdateContext::SetVerifyCount(mu_uint32 value)
{
	if (VerifyCount == value) return;
	VerifyCount = value;
	OnPropertyChanged("VerifyCount");
}

mu_uint32 NGUpdateContext::GetVerifiedCount() const
{
	return VerifiedCount;
}

void NGUpdateContext::SetVerifiedCount(mu_uint32 value)
{
	if (VerifiedCount == value) return;
	VerifiedCount = value;
	OnPropertyChanged("VerifiedCount");
}

const char *NGUpdateContext::GetDownloadSizeType() const
{
	return DownloadSizeType.Str();
}

void NGUpdateContext::SetDownloadSizeType(const char *value)
{
	if (DownloadSizeType == value) return;
	DownloadSizeType = value;
	OnPropertyChanged("DownloadSizeType");
}

mu_float NGUpdateContext::GetDownloadSize() const
{
	return DownloadSize;
}

void NGUpdateContext::SetDownloadSize(mu_float value)
{
	if (DownloadSize == value) return;
	DownloadSize = value;
	OnPropertyChanged("DownloadSize");
}

mu_float NGUpdateContext::GetDownloadedSize() const
{
	return DownloadedSize;
}

void NGUpdateContext::SetDownloadedSize(mu_float value)
{
	if (DownloadedSize == value) return;
	DownloadedSize = value;
	OnPropertyChanged("DownloadedSize");
}

NS_BEGIN_COLD_REGION

NS_IMPLEMENT_REFLECTION(NGUpdateContext)
{
	NsImpl<Noesis::INotifyPropertyChanged>();
	NsProp("Progress", &NGUpdateContext::GetProgress);
	NsProp("State", &NGUpdateContext::GetState);
	NsProp("VerifyCount", &NGUpdateContext::GetVerifyCount);
	NsProp("VerifiedCount", &NGUpdateContext::GetVerifiedCount);
	NsProp("DownloadSizeType", &NGUpdateContext::GetDownloadSizeType);
	NsProp("DownloadSize", &NGUpdateContext::GetDownloadSize);
	NsProp("DownloadedSize", &NGUpdateContext::GetDownloadedSize);
}

NS_END_COLD_REGION