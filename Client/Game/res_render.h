#ifndef __RES_RENDER_H__
#define __RES_RENDER_H__

#pragma once

class NModel;

struct NRenderAttachment
{
	mu_utf8string Bone;
};

struct NRenderAttachments
{
	NRenderAttachment Default;
	std::map<mu_utf8string, NRenderAttachment> Customs;
};

struct NRenderAnimation
{
	mu_boolean Visible = true;
	glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 Angle = glm::vec3(0.0f, 0.0f, 0.0f);
	mu_float Scale = 1.0f;
	NRenderAttachments Attachments;

	NEXTMU_INLINE const NRenderAttachment *GetAttachmentById(const mu_utf8string id) const
	{
		if (id.empty()) return &Attachments.Default;
		auto iter = Attachments.Customs.find(id);
		if (iter == Attachments.Customs.end()) return &Attachments.Default;
		return &iter->second;
	}
};

struct NRenderAnimations
{
	NRenderAnimation Default;
	std::map<mu_utf8string, NRenderAnimation> Customs;
};

struct NRender
{
	mu_utf8string Id;
	NModel *Model;
	mu_boolean IsLinked;
	NRenderAnimations Animations;

	NEXTMU_INLINE const NRenderAnimation *GetAnimationById(const mu_utf8string id) const
	{
		if (id.empty()) return &Animations.Default;
		auto iter = Animations.Customs.find(id);
		if (iter == Animations.Customs.end()) return &Animations.Default;
		return &iter->second;
	}
};

#endif