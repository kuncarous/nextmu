#ifndef __RES_RENDER_H__
#define __RES_RENDER_H__

#pragma once

class NModel;

enum class NPartType : mu_uint32
{
	Helm,
	Armor,
	Pants,
	Gloves,
	Boots,
	ItemLeft,
	ItemRight,
	Wings,
	Helper,
	Max,
};
constexpr mu_uint32 MaxPartType = static_cast<mu_uint32>(NPartType::Max);

extern std::map<mu_utf8string, NPartType> PartTypeIds;
NEXTMU_INLINE const NPartType GetPartTypeById(const mu_utf8string id)
{
	auto iter = PartTypeIds.find(id);
	if (iter == PartTypeIds.end()) return NPartType::Max;
	return iter->second;
}

struct NRenderAttachment
{
	mu_utf8string Bone;
	glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 Angle = glm::vec3(0.0f, 0.0f, 0.0f);
	mu_float Scale = 1.0f;
};

struct NRenderAttachments
{
	NRenderAttachment Default;
	std::map<NPartType, NRenderAttachment> Customs;
};

struct NRenderAnimation
{
	mu_boolean Visible = true;
	NRenderAttachments Attachments;

	NEXTMU_INLINE const NRenderAttachment *GetAttachmentByPartType(const NPartType id) const
	{
		if (id == NPartType::Max) return &Attachments.Default;
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