#include "stdafx.h"
#include "mu_entity.h"

std::array<const mu_utf8string, AnimationTypeMax> AnimationTypeStrings = {
	"appear",
	"stop",
	"walk",
	"run",
	"attack",
	"shock",
	"die"
};

namespace NEntity
{
	std::array<mu_utf8string, MaxPartType> PartTypeIds = {
		"body",
		"head",
		"helm",
		"armor",
		"pants",
		"gloves",
		"boots",
		"item_left",
		"item_right",
		"wings",
		"helper",
	};
}