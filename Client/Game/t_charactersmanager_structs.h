#ifndef __T_CHARACTERSMANAGER_STRUCTS_H__
#define __T_CHARACTERSMANAGER_STRUCTS_H__

#pragma once

enum class CharacterBodyPart
{
	Head,
	Chest,
	Lower,
	Arms,
	Legs,
	Max,
};
constexpr mu_uint32 MaxCharacterBodyParts = static_cast<mu_uint32>(CharacterBodyPart::Max);

class NModel;

struct NCharacterAttachment
{
	NTextureAttachmentType Type;
	NGraphicsTexture *Texture;
};

struct NCharacterConfiguration
{
	NCharacterSex::Type Sex;
	std::vector<NCharacterAttachment> Attachments;
	NModel *Parts[MaxCharacterBodyParts];
};

struct NCharacterClass
{
	mu_uint32 Id;
	std::vector<NCharacterConfiguration> SubClasses;
};

#endif