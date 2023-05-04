#ifndef __ANI_NODE_H__
#define __ANI_NODE_H__

#pragma once

enum class NAnimationCondition : mu_uint32
{
	Unknown,
	Action,
	Safezone,
	Swimming,
	Sex,
	Class,
	Mount,
	Pet,
	Wings,
	Weapons,
};

enum class NAnimationRouteType : mu_uint32
{
	Always,
	Boolean,
	UInteger,
	String,
};

struct NAnimationString
{
	mu_uint32 Length = 0;
	mu_char *Value = nullptr;
};

NEXTMU_INLINE NAnimationString AllocateString(const mu_utf8string value)
{
	NAnimationString string;
	string.Length = static_cast<mu_uint32>(value.size());
	string.Value = new (std::nothrow) mu_char[string.Length + 1];
	if (string.Value == nullptr) return NAnimationString();
	mu_memset(string.Value, 0, string.Length + 1);
	mu_memcpy(string.Value, value.c_str(), string.Length);
	return string;
}

struct NAnimationValue
{
	NAnimationValue() : Type(NAnimationRouteType::Always), Bool(false) {}
	~NAnimationValue()
	{
		if (Type == NAnimationRouteType::String && String.Value != nullptr)
		{
			mu_free(String.Value);
			String.Length = 0;
			String.Value = nullptr;
		}
	}

	void SetBool(const mu_boolean value)
	{
		Type = NAnimationRouteType::Boolean;
		Bool = value;
	}

	void SetUInteger(const mu_uint32 value)
	{
		Type = NAnimationRouteType::UInteger;
		UInteger = value;
	}

	void SetString(const mu_utf8string value)
	{
		Type = NAnimationRouteType::String;
		String = AllocateString(value);
	}

	NAnimationRouteType Type;
	union
	{
		mu_boolean Bool;
		mu_uint32 UInteger;
		NAnimationString String;
	};
};

struct NAnimationNode;
struct NAnimationRoute
{
	NAnimationValue Value;
	mu_utf8string Animation;
	std::vector<NAnimationNode> Nodes;
};

struct NAnimationNode
{
	NAnimationCondition Condition;
	std::vector<NAnimationRoute> Routes;
};

struct NAnimationsRoot
{
	mu_utf8string Id;
	std::vector<NAnimationNode> Nodes;
};

#endif