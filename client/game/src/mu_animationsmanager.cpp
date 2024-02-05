#include "mu_precompiled.h"
#include "mu_animationsmanager.h"
#include "mu_charactersmanager.h"

namespace MUAnimationsManager
{
	typedef std::unique_ptr<NAnimationsRoot> AnimationRootPtr;
	std::map<mu_utf8string, AnimationRootPtr> Animations;

	const NAnimationCondition GetConditionFromString(const mu_utf8string value)
	{
		if (value.compare("action") == 0) return NAnimationCondition::Action;
		if (value.compare("safezone") == 0) return NAnimationCondition::Safezone;
		if (value.compare("swimming") == 0) return NAnimationCondition::Swimming;
		if (value.compare("sex") == 0) return NAnimationCondition::Sex;
		if (value.compare("class") == 0) return NAnimationCondition::Class;
		if (value.compare("mount") == 0) return NAnimationCondition::Mount;
		if (value.compare("pet") == 0) return NAnimationCondition::Pet;
		if (value.compare("wings") == 0) return NAnimationCondition::Wings;
		if (value.compare("weapons") == 0) return NAnimationCondition::Weapons;
		return NAnimationCondition::Unknown;
	}

	const mu_uint32 GetCharacterSexFromString(const mu_utf8string value)
	{
		return static_cast<mu_uint32>(value.compare("male") == 0 ? NCharacterSex::Male : NCharacterSex::Female);
	}

	const NCharacterType GetCharacterClassFromString(const mu_utf8string value)
	{
		return MUCharactersManager::GetTypeFromString(value);
	}

	void GetRouteValue(const NAnimationCondition condition, const nlohmann::json &jroute, NAnimationValue &value)
	{
		if (jroute.contains("value") == false) return;
		const auto &jvalue = jroute["value"];

		switch (condition)
		{
		case NAnimationCondition::Safezone:
		case NAnimationCondition::Swimming:
		case NAnimationCondition::Wings: value.SetBool(jvalue.get<mu_boolean>()); return;
		case NAnimationCondition::Sex: value.SetUInteger(GetCharacterSexFromString(jvalue.get<mu_utf8string>())); return;
		case NAnimationCondition::Class:
		case NAnimationCondition::SubClass: value.SetCharacterType(GetCharacterClassFromString(jvalue.get<mu_utf8string>())); return;
		}

		if (jvalue.is_boolean()) value.SetBool(jvalue.get<mu_boolean>());
		else if (jvalue.is_number_integer()) value.SetUInteger(jvalue.get<mu_uint32>());
		else if (jvalue.is_string()) value.SetString(jvalue.get<mu_utf8string>());
	}

	void ParseAnimationNode(const nlohmann::json &jnode, NAnimationNode &node);
	void ParseAnimationRoute(const NAnimationCondition condition, const nlohmann::json &jroute, NAnimationRoute &route)
	{
		GetRouteValue(condition, jroute, route.Value);
		if (jroute.contains("animation"))
			route.Animation = jroute["animation"].get<mu_utf8string>();
		if (jroute.contains("nodes"))
		{
			const auto &jnodes = jroute["nodes"];
			route.Nodes.resize(jnodes.size());

			for (mu_uint32 n = 0; n < route.Nodes.size(); ++n)
			{
				const auto &jnode = jnodes[n];
				NAnimationNode &node = route.Nodes[n];
				ParseAnimationNode(jnode, node);
			}
		}
	}

	void ParseAnimationNode(const nlohmann::json &jnode, NAnimationNode &node)
	{
		node.Condition = GetConditionFromString(jnode["condition"]);
		const auto &jroutes = jnode["routes"];
		node.Routes.resize(jroutes.size());
		for (mu_uint32 n = 0; n < node.Routes.size(); ++n)
		{
			const auto &jroute = jroutes[n];
			NAnimationRoute &route = node.Routes[n];
			ParseAnimationRoute(node.Condition, jroute, route);
		}
	}

	const mu_boolean Load()
	{
		const mu_utf8string filename = "data/animations.json";

		SDL_RWops *fp = nullptr;
		if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, filename, "rb") == false)
		{
			mu_error("animations.json missing ({})", filename);
			return false;
		}

		mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));
		std::unique_ptr<mu_char[]> jsonBuffer(new_nothrow mu_char[fileLength]);
		SDL_RWread(fp, jsonBuffer.get(), fileLength, 1);
		SDL_RWclose(fp);

		const mu_utf8string inputBuffer = JsonStripComments(jsonBuffer.get(), static_cast<mu_uint32>(fileLength));
		jsonBuffer.reset();
		auto document = nlohmann::json::parse(inputBuffer.c_str());
		if (document.is_discarded() == true || document.is_array() == false)
		{
			mu_error("animations malformed ({})", filename);
			return false;
		}

		for (const auto &jroot : document)
		{
			const auto id = jroot["id"].get<mu_utf8string>();
			const auto &jnodes = jroot["nodes"];

			AnimationRootPtr root(new_nothrow NAnimationsRoot());
			root->Id = id;
			root->Nodes.resize(jnodes.size());

			for (mu_uint32 n = 0; n < root->Nodes.size(); ++n)
			{
				const auto &jnode = jnodes[n];
				NAnimationNode &node = root->Nodes[n];
				ParseAnimationNode(jnode, node);
			}

			Animations.insert(std::make_pair(id, std::move(root)));
		}

		return true;
	}

	void Destroy()
	{
		Animations.clear();
	}

	void GetAnimationFromNodes(const NAnimationInput &input, const std::vector<NAnimationNode> &nodes, mu_utf8string &animation)
	{
		for (auto iter = nodes.begin(); iter != nodes.end(); ++iter)
		{
			const NAnimationNode &node = *iter;
			switch (node.Condition)
			{
			case NAnimationCondition::Unknown: continue;
			case NAnimationCondition::Action:
				{
					for (auto riter = node.Routes.begin(); riter != node.Routes.end(); ++riter)
					{
						const auto &route = *riter;
						if (route.Value.Type != NAnimationRouteType::String && route.Value.Type != NAnimationRouteType::Always) continue;
						if (route.Value.Type == NAnimationRouteType::String && input.Action.compare(route.Value.String.Value) != 0) continue;
						if (route.Animation.empty() == false) animation = route.Animation;
						GetAnimationFromNodes(input, route.Nodes, animation);
						return;
					}
				}
				break;
			case NAnimationCondition::Safezone:
				{
					for (auto riter = node.Routes.begin(); riter != node.Routes.end(); ++riter)
					{
						const auto &route = *riter;
						if (route.Value.Type != NAnimationRouteType::Boolean && route.Value.Type != NAnimationRouteType::Always) continue;
						if (route.Value.Type == NAnimationRouteType::Boolean && route.Value.Bool != input.SafeZone) continue;
						if (route.Animation.empty() == false) animation = route.Animation;
						GetAnimationFromNodes(input, route.Nodes, animation);
						return;
					}
				}
				break;
			case NAnimationCondition::Swimming:
				{
					for (auto riter = node.Routes.begin(); riter != node.Routes.end(); ++riter)
					{
						const auto &route = *riter;
						if (route.Value.Type != NAnimationRouteType::Boolean && route.Value.Type != NAnimationRouteType::Always) continue;
						if (route.Value.Type == NAnimationRouteType::Boolean && route.Value.Bool != input.Swimming) continue;
						if (route.Animation.empty() == false) animation = route.Animation;
						GetAnimationFromNodes(input, route.Nodes, animation);
						return;
					}
				}
				break;
			case NAnimationCondition::Sex:
				{
					for (auto riter = node.Routes.begin(); riter != node.Routes.end(); ++riter)
					{
						const auto &route = *riter;
						if (route.Value.Type != NAnimationRouteType::UInteger && route.Value.Type != NAnimationRouteType::Always) continue;
						if (route.Value.Type == NAnimationRouteType::UInteger && route.Value.UInteger != input.Sex) continue;
						if (route.Animation.empty() == false) animation = route.Animation;
						GetAnimationFromNodes(input, route.Nodes, animation);
						return;
					}
				}
				break;
			case NAnimationCondition::Class:
				{
					for (auto riter = node.Routes.begin(); riter != node.Routes.end(); ++riter)
					{
						const auto &route = *riter;
						if (route.Value.Type != NAnimationRouteType::CharacterType && route.Value.Type != NAnimationRouteType::Always) continue;
						if (route.Value.Type == NAnimationRouteType::CharacterType && route.Value.CharacterType.Class != input.CharacterType.Class) continue;
						if (route.Animation.empty() == false) animation = route.Animation;
						GetAnimationFromNodes(input, route.Nodes, animation);
						return;
					}
				}
				break;
			case NAnimationCondition::SubClass:
				{
					for (auto riter = node.Routes.begin(); riter != node.Routes.end(); ++riter)
					{
						const auto &route = *riter;
						if (route.Value.Type != NAnimationRouteType::CharacterType && route.Value.Type != NAnimationRouteType::Always) continue;
						if (route.Value.Type == NAnimationRouteType::CharacterType && route.Value.CharacterType.Class != input.CharacterType.Class && route.Value.CharacterType.SubClass != input.CharacterType.SubClass) continue;
						if (route.Animation.empty() == false) animation = route.Animation;
						GetAnimationFromNodes(input, route.Nodes, animation);
						return;
					}
				}
				break;
			case NAnimationCondition::Wings:
				{
					for (auto riter = node.Routes.begin(); riter != node.Routes.end(); ++riter)
					{
						const auto &route = *riter;
						if (route.Value.Type != NAnimationRouteType::Boolean && route.Value.Type != NAnimationRouteType::Always) continue;
						if (route.Value.Type == NAnimationRouteType::Boolean && route.Value.Bool != input.HasWings) continue;
						if (route.Animation.empty() == false) animation = route.Animation;
						GetAnimationFromNodes(input, route.Nodes, animation);
						return;
					}
				}
				break;
			}
		}
	}

	const NAnimationsRoot *GetAnimationsRoot(const mu_utf8string id)
	{
		const auto iter = Animations.find(id);
		if (iter == Animations.end()) return nullptr;
		return iter->second.get();
	}

	const mu_utf8string GetAnimation(const NAnimationsRoot *root, const NAnimationInput &input)
	{
		mu_utf8string animation = "";
		GetAnimationFromNodes(input, root->Nodes, animation);
		return animation;
	}
};