#include "mu_precompiled.h"
#include "mu_model.h"
#include "mu_crypt.h"
#include "shared_binaryreader.h"
#include "mu_textures.h"
#include "mu_resourcesmanager.h"
#include "mu_graphics.h"
#include "mu_angelscript.h"
#include <glm/gtc/type_ptr.hpp>

std::map<mu_utf8string, Diligent::COMPARISON_FUNCTION> DepthTestMap = {
	{ "none", Diligent::COMPARISON_FUNC_UNKNOWN },
	{ "equal", Diligent::COMPARISON_FUNC_EQUAL },
	{ "not_equal", Diligent::COMPARISON_FUNC_NOT_EQUAL },
	{ "less", Diligent::COMPARISON_FUNC_LESS },
	{ "less_equal", Diligent::COMPARISON_FUNC_LESS_EQUAL },
	{ "greater", Diligent::COMPARISON_FUNC_GREATER },
	{ "greater_equal", Diligent::COMPARISON_FUNC_GREATER_EQUAL },
	{ "never", Diligent::COMPARISON_FUNC_NEVER },
	{ "always", Diligent::COMPARISON_FUNC_ALWAYS },
};

std::map<mu_utf8string, Diligent::BLEND_FACTOR> BlendFunctionMap = {
	{ "zero", Diligent::BLEND_FACTOR_ZERO },
	{ "one", Diligent::BLEND_FACTOR_ONE },
	{ "src_alpha", Diligent::BLEND_FACTOR_SRC_ALPHA },
	{ "src_color", Diligent::BLEND_FACTOR_SRC_COLOR },
	{ "inv_src_alpha", Diligent::BLEND_FACTOR_INV_SRC_ALPHA },
	{ "inv_src_color", Diligent::BLEND_FACTOR_INV_SRC_COLOR },
	{ "dst_alpha", Diligent::BLEND_FACTOR_DEST_ALPHA },
	{ "dst_color", Diligent::BLEND_FACTOR_DEST_COLOR },
	{ "inv_dst_alpha", Diligent::BLEND_FACTOR_INV_DEST_ALPHA },
	{ "inv_dst_color", Diligent::BLEND_FACTOR_INV_DEST_COLOR },
	{ "src_alpha_sat", Diligent::BLEND_FACTOR_SRC_ALPHA_SAT },
};

std::map<mu_utf8string, Diligent::BLEND_OPERATION> BlendEquationMap = {
	{ "add", Diligent::BLEND_OPERATION_ADD },
	{ "sub", Diligent::BLEND_OPERATION_SUBTRACT },
	{ "revsub", Diligent::BLEND_OPERATION_REV_SUBTRACT },
	{ "min", Diligent::BLEND_OPERATION_MIN },
	{ "max", Diligent::BLEND_OPERATION_MAX },
};

std::map<mu_utf8string, Diligent::CULL_MODE> CullMap = {
	{ "none", Diligent::CULL_MODE_NONE },
	{ "cw", Diligent::CULL_MODE_FRONT },
	{ "ccw", Diligent::CULL_MODE_BACK },
};

std::map<mu_utf8string, NRenderClassify> ClassifyMap = {
	{ "auto", NRenderClassify::None },
	{ "opaque", NRenderClassify::Opaque },
	{ "pre_alpha", NRenderClassify::PreAlpha },
	{ "post_alpha", NRenderClassify::PostAlpha },
};

std::map<mu_utf8string, NAnimationModifierType> AnimationModifierMap = {
	{ "move_speed", NAnimationModifierType::MoveSpeed },
	{ "attack_speed", NAnimationModifierType::AttackSpeed },
};

std::map<mu_utf8string, EMeshRenderConditionType> RenderConditionTypeMap = {
	{ "item_level", EMeshRenderConditionType::ItemLevel },
	{ "item_level_by_formula", EMeshRenderConditionType::ItemLevelByFormula },
	{ "item_rank", EMeshRenderConditionType::ItemRank },
	{ "options_count", EMeshRenderConditionType::ItemOptionsCount },
	{ "option_type", EMeshRenderConditionType::ItemOption },
	{ "option_min_rank", EMeshRenderConditionType::ItemOptionsMinRank },
	{ "option_max_rank", EMeshRenderConditionType::ItemOptionsMaxRank },
	{ "option_avg_rank", EMeshRenderConditionType::ItemOptionsAvgRank },
};

std::map<mu_utf8string, EMeshRenderConditionOperator> RenderConditionOperatorMap = {
	{ "equal", EMeshRenderConditionOperator::Equal },
	{ "not_equal", EMeshRenderConditionOperator::NotEqual },
	{ "less", EMeshRenderConditionOperator::Less },
	{ "less_equal", EMeshRenderConditionOperator::LessOrEqual },
	{ "greater", EMeshRenderConditionOperator::Greater },
	{ "greater_equal", EMeshRenderConditionOperator::GreaterOrEqual }
};

std::map<mu_utf8string, EMeshRenderLightType> RenderLightTypeMap = {
	{ "add", EMeshRenderLightType::BlendAdd },
	{ "subtract", EMeshRenderLightType::BlendSubtract },
	{ "multiply", EMeshRenderLightType::BlendMultiply },
	{ "divide", EMeshRenderLightType::BlendDivide },
	{ "inv_divide", EMeshRenderLightType::BlendInverseDivide },
	{ "source_set", EMeshRenderLightType::SourceSet },
	{ "target_set", EMeshRenderLightType::TargetSet },
};

std::map<mu_utf8string, EMeshRenderLightSource> RenderLightSourceMap = {
	{ "none", EMeshRenderLightSource::None },
	{ "light", EMeshRenderLightSource::Light },
	{ "luminosity", EMeshRenderLightSource::Luminosity },
};

#define MAPPING_VALUE_FROM_STRING(name, map, default_value) \
constexpr auto name##Default = default_value; \
NEXTMU_INLINE const auto name##FromString(const mu_utf8string value) \
{ \
	auto iter = map.find(value); \
	if (iter == map.end()) return default_value; \
	return iter->second; \
}

MAPPING_VALUE_FROM_STRING(DepthTest, DepthTestMap, Diligent::COMPARISON_FUNC_LESS_EQUAL)
MAPPING_VALUE_FROM_STRING(BlendFunction, BlendFunctionMap, Diligent::BLEND_FACTOR_UNDEFINED)
MAPPING_VALUE_FROM_STRING(BlendEquation, BlendEquationMap, Diligent::BLEND_OPERATION_ADD)
MAPPING_VALUE_FROM_STRING(Cull, CullMap, Diligent::CULL_MODE_NONE)
MAPPING_VALUE_FROM_STRING(Classify, ClassifyMap, NRenderClassify::None)
MAPPING_VALUE_FROM_STRING(AnimationModifier, AnimationModifierMap, NAnimationModifierType::None)
MAPPING_VALUE_FROM_STRING(RenderConditionType, RenderConditionTypeMap, EMeshRenderConditionType::ItemLevel)
MAPPING_VALUE_FROM_STRING(RenderConditionOperator, RenderConditionOperatorMap, EMeshRenderConditionOperator::Equal)
MAPPING_VALUE_FROM_STRING(RenderLightType, RenderLightTypeMap, EMeshRenderLightType::SourceSet)
MAPPING_VALUE_FROM_STRING(RenderLightSource, RenderLightSourceMap, EMeshRenderLightSource::None)

const mu_utf8string ProgramDefault = "mesh_normal";

NModel::~NModel()
{
}

const NDynamicPipelineState CalculateStateFromObject(const nlohmann::json &object)
{
	NDynamicPipelineState state;

	// Write
	if (object.contains("write"))
	{
		const auto &write = object["write"];
		if (write.contains("rgb"))
			state.ColorWrite = !!write["rgb"].get<mu_boolean>();

		if (write.contains("alpha"))
			state.AlphaWrite = !!write["alpha"].get<mu_boolean>();

		if (write.contains("depth"))
			state.DepthWrite = !!write["depth"].get<mu_boolean>();
	}

	// Culling
	if (object.contains("cull"))
		state.CullMode = CullFromString(object["cull"].get<mu_utf8string>());
	else
		state.CullMode = CullDefault;

	// Depth Test
	if (object.contains("depth_test"))
		state.DepthFunc = DepthTestFromString(object["depth_test"].get<mu_utf8string>());
	else
		state.DepthFunc = DepthTestDefault;

	// Blend
	{
		state.SrcBlend = BlendFunctionDefault;
		state.DestBlend = BlendFunctionDefault;
		state.SrcBlendAlpha = BlendFunctionDefault;
		state.DestBlendAlpha = BlendFunctionDefault;
		state.BlendOp = BlendEquationDefault;
		state.BlendOpAlpha = BlendEquationDefault;

		if (object.contains("blend"))
		{
			const auto &blend = object["blend"];

			if (blend.contains("src"))
				state.SrcBlend = state.SrcBlendAlpha = BlendFunctionFromString(blend["src"].get<mu_utf8string>());

			if (blend.contains("dst"))
				state.DestBlend = state.DestBlendAlpha = BlendFunctionFromString(blend["dst"].get<mu_utf8string>());

			if (blend.contains("src_alpha"))
				state.SrcBlendAlpha = BlendFunctionFromString(blend["src_alpha"].get<mu_utf8string>());

			if (blend.contains("dst_alpha"))
				state.DestBlendAlpha = BlendFunctionFromString(blend["dst_alpha"].get<mu_utf8string>());

			if (blend.contains("equation"))
				state.BlendOp = state.BlendOpAlpha = BlendEquationFromString(blend["equation"].get<mu_utf8string>());

			if (blend.contains("equation_alpha"))
				state.BlendOpAlpha = BlendEquationFromString(blend["equation_alpha"].get<mu_utf8string>());
		}
	}

	return state;
}

const mu_boolean NModel::Load(const mu_utf8string id, mu_utf8string path)
{
	Id = id;
	NormalizePath<true>(path);

	const mu_utf8string filename = path + "model.json";
	SDL_RWops *fp = nullptr;
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, filename, "rb") == false)
	{
		mu_error("model json missing ({})", filename);
		return false;
	}

	mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));
	std::unique_ptr<mu_char[]> jsonBuffer(new_nothrow mu_char[fileLength]);
	SDL_RWread(fp, jsonBuffer.get(), fileLength, 1);
	SDL_RWclose(fp);

	const mu_utf8string inputBuffer = JsonStripComments(jsonBuffer.get(), static_cast<mu_uint32>(fileLength));
	jsonBuffer.reset();
	auto document = nlohmann::json::parse(inputBuffer.c_str());
	if (document.is_discarded() == true)
	{
		mu_error("model json malformed ({})", filename);
		return false;
	}

	const auto model = document["model"].get<mu_utf8string>();
	if (LoadModel(path + model) == false)
	{
		mu_error("failed to load model ({})", filename);
		return false;
	}

	LoadBoundingBoxes(path + model);

	if (LoadTextures(path, document) == false)
	{
		mu_error("failed to load model textures ({})", filename);
		return false;
	}

	if (document.contains("scripts"))
	{
		const auto &jscripts = document["scripts"];

		if (jscripts.contains("config"))
		{
			ConfigScript = MUAngelScript::CompileScript(path + jscripts["config"].get<mu_utf8string>());
			if (ConfigScript != nullptr)
			{
				ConfigScriptFunction = ConfigScript->GetFunctionByName("main");
				if (ConfigScriptFunction == nullptr)
				{
					ConfigScript = nullptr;
				}
			}
		}
	}

	if (document.contains("hide_body"))
	{
		HideBody = document["hide_body"].get<mu_boolean>();
	}

	if (document.contains("bone_head"))
	{
		BoneHead = document["bone_head"].get<mu_int16>();
	}

	if (document.contains("body_height"))
	{
		BodyHeight = document["body_height"].get<mu_float>();
	}

	if (document.contains("settings"))
	{
		const auto &settings = document["settings"];
		if (settings.contains("virtual_meshes"))
		{
			const auto program = MUResourcesManager::GetResourcesManager()->GetProgram(ProgramDefault);
			const auto shadowProgram = MUResourcesManager::GetResourcesManager()->GetProgram(ProgramDefault + "_shadow");

			const auto &jvirtualMeshes = settings["virtual_meshes"];
			for (const auto &jmesh : jvirtualMeshes)
			{
				const auto id = jmesh["id"].get<mu_uint32>();
				if (id >= Meshes.size()) continue;

				NVirtualMesh virtualMesh;
				virtualMesh.Mesh = id;
				auto &settings = virtualMesh.Settings;

				if (jmesh.contains("conditions"))
				{
					const auto &jconditionsGroup = jmesh["conditions"];
					for (const auto &jconditions : jconditionsGroup)
					{
						NMeshRenderConditions conditions;

						for (const auto &jcondition : jconditions)
						{
							NMeshRenderCondition condition = {};
							condition.Type = RenderConditionTypeFromString(jcondition["type"].get<mu_utf8string>());
							condition.Operator = RenderConditionOperatorFromString(jcondition["operator"].get<mu_utf8string>());

							switch (condition.Type)
							{
							case EMeshRenderConditionType::ItemLevel:
							case EMeshRenderConditionType::ItemLevelByFormula:
							case EMeshRenderConditionType::ItemRank:
							case EMeshRenderConditionType::ItemOptionsCount:
							case EMeshRenderConditionType::ItemOption:
							case EMeshRenderConditionType::ItemOptionsMinRank:
							case EMeshRenderConditionType::ItemOptionsMaxRank:
							case EMeshRenderConditionType::ItemOptionsAvgRank:
								condition.UValue = jcondition["value"].get<mu_uint32>(); break;
							default:
								goto continue_virtualmesh_conditions;
							}

							conditions.push_back(condition);
						}

						if (conditions.empty()) continue;
						virtualMesh.Conditions.push_back(std::move(conditions));
continue_virtualmesh_conditions:
						continue;
					}
				}

				if (jmesh.contains("program"))
				{
					const auto shaderId = jmesh["program"].get<mu_utf8string>();

					const auto program = MUResourcesManager::GetResourcesManager()->GetProgram(shaderId);
					if (program != NInvalidShader)
						settings.Program = program;

					const auto shadowProgram = MUResourcesManager::GetResourcesManager()->GetProgram(shaderId + "_shadow");
					if (shadowProgram != NInvalidShader)
						settings.ShadowProgram = shadowProgram;
				}

				if (settings.Program == NInvalidShader)
					settings.Program = program;

				if (settings.ShadowProgram == NInvalidShader)
					settings.ShadowProgram = shadowProgram;

				if (jmesh.contains("scripts"))
				{
					const auto &jscripts = jmesh["scripts"];
					
					if (jscripts.contains("settings"))
					{
						settings.Script = MUAngelScript::CompileScript(path + jscripts["settings"].get<mu_utf8string>());
						if (settings.Script != nullptr)
						{
							settings.ScriptFunction = settings.Script->GetFunctionByName("main");
							if (settings.ScriptFunction == nullptr)
							{
								settings.Script = nullptr;
							}
						}
					}
				}

				if (jmesh.contains("inherits"))
				{
					const auto &jinherits = jmesh["inherits"];
					if (jinherits.contains("blend_light"))
					{
						settings.Inherit.BlendMeshLight = jinherits["blend_light"].get<mu_boolean>();
					}
				}

				if (jmesh.contains("vertex_texture"))
					settings.VertexTexture = MUResourcesManager::GetResourcesManager()->GetTexture(jmesh["vertex_texture"].get<mu_utf8string>());

				if (jmesh.contains("texture"))
					settings.Texture = MUResourcesManager::GetResourcesManager()->GetTexture(jmesh["texture"].get<mu_utf8string>());

				if (jmesh.contains("classify"))
				{
					const auto &classify = jmesh["classify"];
					if (classify.contains("mode"))
						settings.ClassifyMode = ClassifyFromString(classify["mode"].get<mu_utf8string>());
					if (classify.contains("index"))
						settings.ClassifyIndex = classify["index"].get<mu_uint32>();
				}

				if (jmesh.contains("normal"))
				{
					auto &renderState = settings.RenderState[ModelRenderMode::Normal];
					auto &shadowRenderState = settings.ShadowRenderState[ModelRenderMode::Normal];
					renderState = CalculateStateFromObject(jmesh["normal"]);
					shadowRenderState = NormalizeShadowRenderState(renderState);
				}

				if (jmesh.contains("alpha"))
				{
					auto &renderState = settings.RenderState[ModelRenderMode::Alpha];
					auto &shadowRenderState = settings.ShadowRenderState[ModelRenderMode::Alpha];
					renderState = CalculateStateFromObject(jmesh["alpha"]);
					shadowRenderState = NormalizeShadowRenderState(renderState);
				}

				if (jmesh.contains("lights"))
				{
					const auto &jlights = jmesh["lights"];
					for (const auto &jlight : jlights)
					{
						NRenderVirtualMeshLight light = {};

						light.ShouldClamp = jlight["clamp"].get<mu_boolean>();

						light.PreType = RenderLightTypeFromString(jlight["pre_type"].get<mu_utf8string>());
						light.PreSource = RenderLightSourceFromString(jlight["pre_source"].get<mu_utf8string>());
						
						const auto &jpreValue = jlight["pre_value"];
						light.PreValue[0] = jpreValue[0].get<mu_float>();
						light.PreValue[1] = jpreValue[1].get<mu_float>();
						light.PreValue[2] = jpreValue[2].get<mu_float>();

						light.PreType = RenderLightTypeFromString(jlight["post_type"].get<mu_utf8string>());
						light.PreSource = RenderLightSourceFromString(jlight["post_source"].get<mu_utf8string>());

						const auto &jpostValue = jlight["post_value"];
						light.PostValue[0] = jpostValue[0].get<mu_float>();
						light.PostValue[1] = jpostValue[1].get<mu_float>();
						light.PostValue[2] = jpostValue[2].get<mu_float>();

						if (jlight.contains("conditions"))
						{
							const auto &jconditionsGroup = jlight["conditions"];
							for (const auto &jconditions : jconditionsGroup)
							{
								NMeshRenderConditions conditions;

								for (const auto &jcondition : jconditions)
								{
									NMeshRenderCondition condition = {};
									condition.Type = RenderConditionTypeFromString(jcondition["type"].get<mu_utf8string>());
									condition.Operator = RenderConditionOperatorFromString(jcondition["operator"].get<mu_utf8string>());

									switch (condition.Type)
									{
									case EMeshRenderConditionType::ItemLevel:
									case EMeshRenderConditionType::ItemLevelByFormula:
									case EMeshRenderConditionType::ItemRank:
									case EMeshRenderConditionType::ItemOptionsCount:
									case EMeshRenderConditionType::ItemOption:
									case EMeshRenderConditionType::ItemOptionsMinRank:
									case EMeshRenderConditionType::ItemOptionsMaxRank:
									case EMeshRenderConditionType::ItemOptionsAvgRank:
										condition.UValue = jcondition["value"].get<mu_uint32>(); break;
									default:
										goto continue_virtualmesh_light_conditions;
									}

									conditions.push_back(condition);
								}

								if (conditions.empty()) continue;
								light.Conditions.push_back(std::move(conditions));
continue_virtualmesh_light_conditions:
								continue;
							}
						}

						virtualMesh.Settings.Lights.push_back((light));
					}
				}

				if (jmesh.contains("alpha_test"))
					settings.AlphaTest = jmesh["alpha_test"].get<mu_float>();

				if (jmesh.contains("premultiply_light"))
					settings.PremultiplyLight = jmesh["premultiply_light"].get<mu_boolean>();

				if (jmesh.contains("premultiply_alpha"))
					settings.PremultiplyAlpha = jmesh["premultiply_alpha"].get<mu_boolean>();

				VirtualMeshes.push_back(virtualMesh);
			}
		}
		else if (settings.contains("meshes"))
		{
			const auto program = MUResourcesManager::GetResourcesManager()->GetProgram(ProgramDefault);
			const auto shadowProgram = MUResourcesManager::GetResourcesManager()->GetProgram(ProgramDefault + "_shadow");

			const auto &meshes = settings["meshes"];
			for (const auto &jmesh : meshes)
			{
				const auto id = jmesh["id"].get<mu_uint32>();
				if (id >= Meshes.size()) continue;

				auto &settings = Meshes[id].Settings;

				if (jmesh.contains("program"))
				{
					const auto shaderId = jmesh["program"].get<mu_utf8string>();

					const auto program = MUResourcesManager::GetResourcesManager()->GetProgram(shaderId);
					if (program != NInvalidShader)
						settings.Program = program;

					const auto shadowProgram = MUResourcesManager::GetResourcesManager()->GetProgram(shaderId + "_shadow");
					if (shadowProgram != NInvalidShader)
						settings.ShadowProgram = shadowProgram;
				}

				if (settings.Program == NInvalidShader)
					settings.Program = program;

				if (settings.ShadowProgram == NInvalidShader)
					settings.ShadowProgram = shadowProgram;

				if (jmesh.contains("scripts"))
				{
					const auto &jscripts = jmesh["scripts"];

					if (jscripts.contains("settings"))
					{
						settings.Script = MUAngelScript::CompileScript(path + jscripts["settings"].get<mu_utf8string>());
						if (settings.Script != nullptr)
						{
							settings.ScriptFunction = settings.Script->GetFunctionByName("main");
							if (settings.ScriptFunction == nullptr)
							{
								settings.Script = nullptr;
							}
						}
					}
				}

				if (jmesh.contains("vertex_texture"))
					settings.VertexTexture = MUResourcesManager::GetResourcesManager()->GetTexture(jmesh["vertex_texture"].get<mu_utf8string>());

				if (jmesh.contains("texture"))
					settings.Texture = MUResourcesManager::GetResourcesManager()->GetTexture(jmesh["texture"].get<mu_utf8string>());

				if (jmesh.contains("classify"))
				{
					const auto &classify = jmesh["classify"];
					if (classify.contains("mode"))
						settings.ClassifyMode = ClassifyFromString(classify["mode"].get<mu_utf8string>());
					if (classify.contains("index"))
						settings.ClassifyIndex = classify["index"].get<mu_uint32>();
				}

				if (jmesh.contains("normal")) {
					auto &renderState = settings.RenderState[ModelRenderMode::Normal];
					auto &shadowRenderState = settings.ShadowRenderState[ModelRenderMode::Normal];
					renderState = CalculateStateFromObject(jmesh["normal"]);
					shadowRenderState = NormalizeShadowRenderState(renderState);
				}

				if (jmesh.contains("alpha")) {
					auto &renderState = settings.RenderState[ModelRenderMode::Alpha];
					auto &shadowRenderState = settings.ShadowRenderState[ModelRenderMode::Alpha];
					renderState = CalculateStateFromObject(jmesh["alpha"]);
					shadowRenderState = NormalizeShadowRenderState(renderState);
				}

				if (jmesh.contains("lights")) {
					const auto &jlights = jmesh["lights"];
					for (const auto &jlight : jlights)
					{
						NRenderVirtualMeshLight light = {};

						light.ShouldClamp = jlight["clamp"].get<mu_boolean>();

						light.PreType = RenderLightTypeFromString(jlight["pre_type"].get<mu_utf8string>());
						light.PreSource = RenderLightSourceFromString(jlight["pre_source"].get<mu_utf8string>());

						light.PreType = RenderLightTypeFromString(jlight["post_type"].get<mu_utf8string>());
						light.PreSource = RenderLightSourceFromString(jlight["post_source"].get<mu_utf8string>());

						if (jlight.contains("conditions"))
						{
							const auto &jconditionsGroup = jlight["conditions"];
							for (const auto &jconditions : jconditionsGroup)
							{
								NMeshRenderConditions conditions;

								for (const auto &jcondition : jconditions)
								{
									NMeshRenderCondition condition = {};
									condition.Type = RenderConditionTypeFromString(jcondition["type"].get<mu_utf8string>());
									condition.Operator = RenderConditionOperatorFromString(jcondition["operator"].get<mu_utf8string>());

									switch (condition.Type)
									{
									case EMeshRenderConditionType::ItemLevel:
									case EMeshRenderConditionType::ItemLevelByFormula:
									case EMeshRenderConditionType::ItemRank:
									case EMeshRenderConditionType::ItemOptionsCount:
									case EMeshRenderConditionType::ItemOption:
									case EMeshRenderConditionType::ItemOptionsMinRank:
									case EMeshRenderConditionType::ItemOptionsMaxRank:
									case EMeshRenderConditionType::ItemOptionsAvgRank:
										condition.UValue = jcondition["value"].get<mu_uint32>(); break;
									default:
										goto continue_mesh_light_conditions;
									}

									conditions.push_back(condition);
								}

								if (conditions.empty()) continue;
								light.Conditions.push_back(std::move(conditions));
continue_mesh_light_conditions:
								continue;
							}
						}
					}
				}

				if (jmesh.contains("alpha_test"))
					settings.AlphaTest = jmesh["alpha_test"].get<mu_float>();

				if (jmesh.contains("premultiply_light"))
					settings.PremultiplyLight = jmesh["premultiply_light"].get<mu_boolean>();

				if (jmesh.contains("premultiply_alpha"))
					settings.PremultiplyAlpha = jmesh["premultiply_alpha"].get<mu_boolean>();
			}
		}
	}

	if (document.contains("bones"))
	{
		const auto &jbones = document["bones"];
		for (const auto &jbone : jbones)
		{
			const auto id = jbone["id"].get<mu_utf8string>();
			const auto bone = jbone["bone"].get<mu_uint32>();

			BonesById.insert(std::make_pair(id, bone));
		}
	}

	if (document.contains("animations"))
	{
		const auto &janimations = document["animations"];
		for (const auto &janimation : janimations)
		{
			const auto id = janimation["id"].get<mu_utf8string>();
			const auto index = janimation["index"].get<mu_uint32>();

			auto &animation = Animations[index];
			animation.Id = id;

			if (janimation.contains("loop"))
			{
				animation.Loop = janimation["loop"].get<mu_boolean>();
			}

			if (janimation.contains("play_speed"))
			{
				animation.PlaySpeed = janimation["play_speed"].get<mu_float>();
			}

			if (janimation.contains("modifier"))
			{
				animation.Modifier = AnimationModifierFromString(janimation["modifier"].get<mu_utf8string>());
			}

			AnimationsById.insert(std::make_pair(id, index));
		}
	}

	if (GenerateBuffers() == false)
	{
		mu_error("failed to generate model buffers ({})", filename);
		return false;
	}

	return true;
}

const mu_boolean NModel::LoadModel(mu_utf8string path)
{
	NormalizePath(path);

	auto ext = path.substr(path.find_last_of('.') + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), mu_utf8tolower);

	SDL_RWops *fp = nullptr;
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, path, "rb") == false)
	{
		mu_error("model not found ({})", path);
		return false;
	}

	mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));
	std::unique_ptr<mu_uint8[]> buffer(new_nothrow mu_uint8[fileLength]);
	SDL_RWread(fp, buffer.get(), fileLength, 1);
	SDL_RWclose(fp);

	NBinaryReader reader(buffer.get(), static_cast<mu_uint32>(fileLength));

	reader.Skip(3); // BMD bytes
	const mu_uint8 version = reader.Read<mu_uint8>();
	if (version == 12)
	{
		mu_int32 encryptedSize = reader.Read<mu_int32>();
		std::unique_ptr<mu_uint8[]> decryptedBuffer(new_nothrow mu_uint8[encryptedSize]);
		XorDecrypt(decryptedBuffer.get(), reader.GetPointer(), encryptedSize);
		buffer.swap(decryptedBuffer);
		reader.Replace(buffer.get(), encryptedSize);
	}
	else if (version == 14)
	{
		mu_int32 encryptedSize = reader.Read<mu_int32>();
		mu_uint32 decryptedSize = CryptoModulusDecrypt(reader.GetPointer(), encryptedSize, nullptr);
		std::unique_ptr<mu_uint8[]> decryptedBuffer(new_nothrow mu_uint8[decryptedSize]);
		CryptoModulusDecrypt(reader.GetPointer(), encryptedSize, decryptedBuffer.get());
		buffer.swap(decryptedBuffer);
		reader.Replace(buffer.get(), static_cast<mu_uint32>(decryptedSize));
	}

	reader.Skip(32); // Name bytes

	const mu_uint32 numMeshes = static_cast<mu_uint32>(reader.Read<mu_int16>());
	const mu_uint32 numBones = static_cast<mu_uint32>(reader.Read<mu_int16>());
	const mu_uint32 numActions = static_cast<mu_uint32>(reader.Read<mu_int16>());

	if (numBones >= static_cast<mu_int16>(MaxBones))
	{
		mu_error("model exceed max bones limit ({})", path);
		return false;
	}

	Meshes.resize(numMeshes);
	BoneName.resize(numBones);
	BoneInfo.resize(numBones);
	Animations.resize(numActions);
	BoundingBoxes.resize(numBones);

	const auto program = MUResourcesManager::GetResourcesManager()->GetProgram(ProgramDefault);
	const auto shadowProgram = MUResourcesManager::GetResourcesManager()->GetProgram(ProgramDefault + "_shadow");

	mu_char filename[32 + 1] = {};
	for (mu_uint32 m = 0; m < numMeshes; ++m)
	{
		auto &mesh = Meshes[m];

		mesh.Settings.Program = program;
		mesh.Settings.ShadowProgram = shadowProgram;

		const mu_uint32 numVertices = static_cast<mu_uint32>(reader.Read<mu_int16>());
		const mu_uint32 numNormals = static_cast<mu_uint32>(reader.Read<mu_int16>());
		const mu_uint32 numTexCoords = static_cast<mu_uint32>(reader.Read<mu_int16>());
		const mu_uint32 numTriangles = static_cast<mu_uint32>(reader.Read<mu_int16>());

		const mu_uint32 texture = static_cast<mu_uint32>(reader.Read<mu_int16>());
		mu_assert(texture == m); // Why would this variable even exists?

		mesh.Vertices.resize(numVertices);
		mesh.Normals.resize(numNormals);
		mesh.TexCoords.resize(numTexCoords);
		mesh.Triangles.resize(numTriangles);

		for (mu_uint32 v = 0; v < numVertices; ++v)
		{
			auto &vertex = mesh.Vertices[v];
			vertex.Node = reader.Read<mu_int16>();
			reader.Skip(2);
			vertex.Position[0] = reader.Read<mu_float>();
			vertex.Position[1] = reader.Read<mu_float>();
			vertex.Position[2] = reader.Read<mu_float>();
		}

		for (mu_uint32 n = 0; n < numNormals; ++n)
		{
			auto &normal = mesh.Normals[n];
			normal.Node = reader.Read<mu_int16>();
			reader.Skip(2);
			normal.Normal[0] = reader.Read<mu_float>();
			normal.Normal[1] = reader.Read<mu_float>();
			normal.Normal[2] = reader.Read<mu_float>();
			reader.Skip(4);
		}

		for (mu_uint32 t = 0; t < numTexCoords; ++t)
		{
			auto &texCoord = mesh.TexCoords[t];
			texCoord.UV[0] = reader.Read<mu_float>();
			texCoord.UV[1] = reader.Read<mu_float>();
		}

		for (mu_uint32 t = 0; t < numTriangles; ++t)
		{
			auto &triangle = mesh.Triangles[t];

			const mu_uint8 polygon = reader.Read<mu_uint8>();
			reader.Skip(1);
			mu_assert(polygon == 3);

			for (mu_uint32 n = 0; n < 3; ++n)
			{
				triangle.Vertices[n] = reader.Read<mu_int16>();
			}
			reader.Skip(2);

			for (mu_uint32 n = 0; n < 3; ++n)
			{
				triangle.Normals[n] = reader.Read<mu_int16>();
			}
			reader.Skip(2);

			for (mu_uint32 n = 0; n < 3; ++n)
			{
				triangle.TexCoords[n] = reader.Read<mu_int16>();
			}
			reader.Skip(40);
		}

		reader.ReadLine(filename, 32);
		mesh.Texture.Filename = filename;

		// TODO: Texture Script Parsing, really, didn't have a better idea?
	}

	std::vector<mu_uint32> AnimationKeys(numActions, 0);
	for (mu_uint32 a = 0; a < numActions; ++a)
	{
		auto &animation = Animations[a];
		animation.Loop = false;
		const mu_uint32 numKeys = static_cast<mu_uint32>(reader.Read<mu_int16>());
		AnimationKeys[a] = numKeys;
		animation.LockPositions = reader.Read<mu_boolean>();

		if (animation.LockPositions)
		{
			reader.Skip(sizeof(glm::vec3) * numKeys); // Useless data
		}
	}

	for (mu_uint32 b = 0; b < numBones; ++b)
	{
		auto &info = BoneInfo[b];
		info.Dummy = reader.Read<mu_boolean>();
		if (info.Dummy) continue;

		reader.ReadLine(filename, 32);
		BoneName[b] = filename;

		info.Parent = reader.Read<mu_int16>();

		for (mu_uint32 a = 0; a < numActions; ++a)
		{
			auto &animation = Animations[a];

			const mu_uint32 numKeys = AnimationKeys[a];
			animation.Keys.resize(numKeys);

			/*
				Why I decided to structure it this way?
				to benefit from cache when processing animations, slower loading but faster performance later.
				Hopefully the bones are sorted.
			*/
			for (mu_uint32 k = 0; k < numKeys; ++k)
			{
				auto &animationKey = animation.Keys[k];
				animationKey.Bones.resize(numBones);
			}

			for (mu_uint32 k = 0; k < numKeys; ++k)
			{
				auto &animationKey = animation.Keys[k];
				reader.ReadLine(&animationKey.Bones[b].Position, sizeof(glm::vec3));
			}

			for (mu_uint32 k = 0; k < numKeys; ++k)
			{
				auto &animationKey = animation.Keys[k];
				glm::vec3 rotation;
				reader.ReadLine(&rotation, sizeof(glm::vec3));
				animationKey.Bones[b].Rotation = glm::quat(rotation);
			}
		}
	}

	CalculateBoundingBoxes();

	return true;
}

void NModel::LoadBoundingBoxes(mu_utf8string path)
{
	const auto dotPos = path.find_last_of('.');
	if (dotPos != mu_utf8string::npos)
	{
		path = path.substr(0, dotPos);
	}
	path += ".bbox.json";

	SDL_RWops *fp = nullptr;
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, path, "rb") == false)
	{
		return;
	}

	mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));
	std::unique_ptr<mu_char[]> jsonBuffer(new_nothrow mu_char[fileLength]);
	SDL_RWread(fp, jsonBuffer.get(), fileLength, 1);
	SDL_RWclose(fp);

	const mu_utf8string inputBuffer = JsonStripComments(jsonBuffer.get(), static_cast<mu_uint32>(fileLength));
	jsonBuffer.reset();
	auto document = nlohmann::json::parse(inputBuffer.c_str());
	if (document.is_discarded() == true)
	{
		return;
	}

	// Global
	{
		const auto &bbox = document["bbox"];
		const auto &min = bbox["min"];
		const auto &max = bbox["max"];

		NBoundingBox b;
		b.Min[0] = min[0].get<mu_float>();
		b.Min[1] = min[1].get<mu_float>();
		b.Min[2] = min[2].get<mu_float>();
		b.Max[0] = max[0].get<mu_float>();
		b.Max[1] = max[1].get<mu_float>();
		b.Max[2] = max[2].get<mu_float>();

		BBoxes.Global = NOrientedBoundingBox(b);
	}

	if (document.contains("animations"))
	{
		NBoundingBox box = {};
		const auto &animations = document["animations"];
		for (const auto &bbox : animations)
		{
			const auto &min = bbox["min"];
			const auto &max = bbox["max"];

			NBoundingBox b;
			b.Min[0] = min[0].get<mu_float>();
			b.Min[1] = min[1].get<mu_float>();
			b.Min[2] = min[2].get<mu_float>();
			b.Max[0] = max[0].get<mu_float>();
			b.Max[1] = max[1].get<mu_float>();
			b.Max[2] = max[2].get<mu_float>();

			BBoxes.PerAnimation.push_back(NOrientedBoundingBox(b));
		}
	}

	BBoxes.Valid = true;
}

const mu_boolean NModel::LoadTextures(const mu_utf8string path, const nlohmann::json &document)
{
	mu_utf8string subPath = (
		document.contains("textures_dir")
		? document["textures_dir"].get<mu_utf8string>()
		: "textures/"
		);

	if (document.contains("textures"))
	{
		const auto &textures = document["textures"];

		mu_utf8string filter = textures.contains("filter") ? textures["filter"].get<mu_utf8string>() : "";
		mu_utf8string wrap = textures.contains("wrap") ? textures["wrap"].get<mu_utf8string>() : "";
		const mu_boolean isValidFilter = MUTextures::IsValidFilter(filter);
		const mu_boolean isValidWrap = MUTextures::IsValidWrap(wrap);

		if (isValidFilter || isValidWrap)
		{
			for (auto &mesh : Meshes)
			{
				if (isValidFilter) mesh.Texture.Filter = filter;
				if (isValidWrap) mesh.Texture.Wrap = wrap;
			}
		}

		if (textures.contains("meshes"))
		{
			const auto &meshes = textures["meshes"];
			for (const auto &texture : meshes)
			{
				const mu_uint32 index = texture["mesh"].get<mu_uint32>();
				if (index >= Meshes.size()) continue;

				auto &mesh = Meshes[index];

				mu_utf8string filter = texture.contains("filter") ? texture["filter"].get<mu_utf8string>() : "";
				mu_utf8string wrap = texture.contains("wrap") ? texture["wrap"].get<mu_utf8string>() : "";

				MUTextures::NormalizeFilter(filter);
				MUTextures::NormalizeFilter(wrap);

				if (MUTextures::IsValidFilter(filter))
				{
					mesh.Texture.ForceFilter = true;
					mesh.Texture.Filter = filter;
				}

				if (MUTextures::IsValidWrap(wrap))
				{
					mesh.Texture.ForceWrap = true;
					mesh.Texture.Wrap = wrap;
				}
			}
		}
	}

	auto *resourcesManager = MUResourcesManager::GetResourcesManager();
	const mu_uint32 numMeshes = static_cast<mu_uint32>(Meshes.size());
	Textures.resize(numMeshes);
	for (mu_uint32 m = 0; m < numMeshes; ++m)
	{
		const auto &mesh = Meshes[m];

		mu_utf8string filename = mesh.Texture.Filename;
		mu_utf8string filenameLC = filename;
		std::transform(filenameLC.begin(), filenameLC.end(), filenameLC.begin(), mu_utf8tolower);
		/* Hard-coded attachments, in future these will be removed and configured directly in the jsons of models */
		if (filename.starts_with("ski") || filename.starts_with("level_"))
		{
			Textures[m] = {
				.Type = resourcesManager->GetAttachmentTypeFromCRC32(GenerateAttachmentTypeAtCompileTime("skin")),
			};
		}
		else if (filenameLC.starts_with("hqskin3"))
		{
			Textures[m] = {
				.Type = resourcesManager->GetAttachmentTypeFromCRC32(GenerateAttachmentTypeAtCompileTime("skin_hq_alt2")),
			};
		}
		else if (filenameLC.starts_with("hqskin2"))
		{
			Textures[m] = {
				.Type = resourcesManager->GetAttachmentTypeFromCRC32(GenerateAttachmentTypeAtCompileTime("skin_hq_alt")),
			};
		}
		else if (filenameLC.starts_with("hqskin") || filenameLC.starts_with("hqlevel_"))
		{
			Textures[m] = {
				.Type = resourcesManager->GetAttachmentTypeFromCRC32(GenerateAttachmentTypeAtCompileTime("skin_hq")),
			};
		}
		else if (filename.starts_with("hid"))
		{
			Textures[m] = {
				.Type = resourcesManager->GetAttachmentTypeFromCRC32(GenerateAttachmentTypeAtCompileTime("hide")),
			};
		}
		else if (filename.starts_with("hair"))
		{
			Textures[m] = {
				.Type = resourcesManager->GetAttachmentTypeFromCRC32(GenerateAttachmentTypeAtCompileTime("hair")),
			};
		}
		else if (filenameLC.starts_with("hqhair_"))
		{
			Textures[m] = {
				.Type = resourcesManager->GetAttachmentTypeFromCRC32(GenerateAttachmentTypeAtCompileTime("hair_hq")),
			};
		}
		else if (filename.starts_with("face_"))
		{
			Textures[m] = {
				.Type = resourcesManager->GetAttachmentTypeFromCRC32(GenerateAttachmentTypeAtCompileTime("face")),
			};
		}
		else
		{
			const mu_size extAt = filename.find_last_of('.');
			mu_utf8string ext = extAt != mu_utf8string::npos ? filename.substr(extAt) : "";
			std::transform(ext.begin(), ext.end(), ext.begin(), mu_utf8tolower);

			mu_utf8string filter = mesh.Texture.Filter;
			mu_utf8string wrap = mesh.Texture.Wrap;

			// Force nearest filter in TGA files
			if (
				mesh.Texture.ForceFilter == false &&
				ext.starts_with(".t")
				)
			{
				filter = "nearest";
			}

			mu_boolean hasAlpha = false;
			if (ext.starts_with(".t"))
			{
				filename = filename.substr(0, extAt) + ".ozt";
				hasAlpha = true;
			}
			else if (ext.starts_with(".j"))
			{
				filename = filename.substr(0, extAt) + ".ozj";
			}

			filename = path + subPath + filename;
			NGraphicsTexturePtr texture = MUTextures::Load(
				filename,
				MUTextures::CalculateSamplerFlags(filter, wrap)
			);
			if (!texture)
			{
				mu_error("texture not found ({})", filename);
				return false;
			}

			Textures[m] = {
				.Type = resourcesManager->GetAttachmentTypeFromCRC32(GenerateAttachmentTypeAtCompileTime("normal")),
				.Texture = std::move(texture),
			};
		}
	}

	return true;
}

const mu_boolean NModel::GenerateBuffers()
{
	mu_uint32 verticesCount = 0;
	for (auto &mesh : Meshes)
	{
		mesh.VertexBuffer.Offset = verticesCount;
		mesh.VertexBuffer.Count = static_cast<mu_uint32>(mesh.Triangles.size()) * 3u;
		verticesCount += mesh.VertexBuffer.Count;
	}

	if (verticesCount == 0) return true;

	const mu_size memorySize = verticesCount * sizeof(NMeshVertex);
	std::unique_ptr<mu_uint8[]> memory(new_nothrow mu_uint8[memorySize]);
	NMeshVertex *dest = reinterpret_cast<NMeshVertex *>(memory.get());

	for (const auto &mesh : Meshes)
	{
		for (const auto &triangle : mesh.Triangles)
		{
			for (mu_uint32 n = 0; n < 3; ++n)
			{
				const auto &vertex = mesh.Vertices[triangle.Vertices[n]];
				const auto &normal = mesh.Normals[triangle.Normals[n]];
				const auto &texCoord = mesh.TexCoords[triangle.TexCoords[n]];

				dest->Position = vertex.Position;

#if NEXTMU_COMPRESSED_MESHS
				glm::vec4 Normal(normal.Normal, 0.0f);
				dest->Normal[0] = glm::packSnorm2x16(glm::vec2(normal.Normal[0], normal.Normal[1]));
				dest->Normal[1] = glm::packSnorm2x16(glm::vec2(normal.Normal[2], normal.Normal[3]));
				glm::vec2 UV(texCoord.UV.x, 1.0f - texCoord.UV.y);
				dest->UV = glm::packSnorm2x16(UV);
#else
				dest->Normal = normal.Normal;
				dest->TexCoords = glm::vec2(texCoord.UV.x, 1.0f - texCoord.UV.y);
#endif

				dest->Bone[0] = static_cast<mu_uint8>(vertex.Node);
				dest->Bone[1] = static_cast<mu_uint8>(normal.Node);
				dest->Vertex = static_cast<mu_uint16>(triangle.Vertices[n]);

				++dest;
			}
		}
	}

	const auto device = MUGraphics::GetDevice();

	Diligent::BufferDesc bufferDesc;
	bufferDesc.Usage = Diligent::USAGE_IMMUTABLE;
	bufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
	bufferDesc.Size = memorySize;

	Diligent::BufferData bufferData;
	bufferData.pData = memory.get();
	bufferData.DataSize = memorySize;

	Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
	device->CreateBuffer(bufferDesc, &bufferData, &buffer);
	if (buffer == nullptr)
	{
		return false;
	}

	const auto immediateContext = MUGraphics::GetImmediateContext();
	Diligent::StateTransitionDesc barrier(buffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
	immediateContext->TransitionResourceStates(1, &barrier);
	VertexBuffer = buffer;

	MUGraphics::IncreaseTransactions();
	MUGraphics::CheckIfRequireFlushContext();

	return true;
}

void NModel::CalculateBoundingBoxes()
{
	const mu_uint32 numMeshes = static_cast<mu_uint32>(Meshes.size());
	for (mu_uint32 m = 0; m < numMeshes; ++m)
	{
		auto &mesh = Meshes[m];

		const mu_uint32 numVertices = static_cast<mu_uint32>(mesh.Vertices.size());
		for (mu_uint32 v = 0; v < numVertices; ++v)
		{
			auto &vertex = mesh.Vertices[v];
			if (vertex.Node == NInvalidInt16) continue;

			auto &boundingBox = BoundingBoxes[vertex.Node];
			for (mu_uint32 n = 0; n < 3; ++n)
			{
				if (vertex.Position[n] < boundingBox.Min[n]) boundingBox.Min[n] = vertex.Position[n];
				if (vertex.Position[n] > boundingBox.Max[n]) boundingBox.Max[n] = vertex.Position[n];
			}

			boundingBox.Valid = true;
		}
	}
}

const mu_boolean NModel::PlayAnimation(
	mu_uint16 &CurrentAction,
	mu_uint16 &PriorAction,
	mu_float &CurrentFrame,
	mu_float &PriorFrame,
	const mu_float PlaySpeed
) const
{
	if (glm::abs(PlaySpeed) < glm::epsilon<mu_float>()) return true;
	const mu_uint32 numAnimations = static_cast<mu_uint32>(this->Animations.size());

	// Return true to keep original logic
	if (CurrentAction >= numAnimations) return true;

	const auto &currentAnimation = this->Animations[CurrentAction];
	const auto &currentFramesCount = static_cast<mu_uint32>(currentAnimation.Keys.size());
	if (currentFramesCount <= 1) return true;

	const mu_float tmpFrame = CurrentFrame;
	const mu_uint32 lastFrame = static_cast<mu_uint32>(glm::floor(CurrentFrame));
	CurrentFrame += PlaySpeed;
	const mu_uint32 newFrame = static_cast<mu_uint32>(glm::floor(CurrentFrame));

	if (CurrentAction == PriorAction || lastFrame != newFrame)
	{
		PriorAction = CurrentAction;
		PriorFrame = tmpFrame;
	}

	mu_boolean loop = true;
	if (currentAnimation.Loop)
	{
		if (newFrame >= currentFramesCount)
		{
			CurrentFrame = static_cast<mu_float>(currentFramesCount) - 0.01f;
			loop = false;
		}
	}
	else
	{
		const auto maxFrames = currentFramesCount - static_cast<mu_uint32>(!!currentAnimation.LockPositions);
		if (newFrame >= maxFrames)
		{
			CurrentFrame = glm::mod(CurrentFrame, static_cast<mu_float>(maxFrames));
			loop = false;
		}
	}

	return loop;
}

template<typename Type, typename InputType>
mu_boolean CheckRenderCondition(const EMeshRenderConditionOperator operatorType, const Type compareValue, const InputType value)
{
	switch (operatorType)
	{
	case EMeshRenderConditionOperator::Equal: return compareValue == static_cast<Type>(value);
	case EMeshRenderConditionOperator::NotEqual: return compareValue != static_cast<Type>(value);
	case EMeshRenderConditionOperator::Less: return compareValue < static_cast<Type>(value);
	case EMeshRenderConditionOperator::LessOrEqual: return compareValue <= static_cast<Type>(value);
	case EMeshRenderConditionOperator::Greater: return compareValue > static_cast<Type>(value);
	case EMeshRenderConditionOperator::GreaterOrEqual: return compareValue >= static_cast<Type>(value);
	default: return false;
	}
}

template<typename Type, typename InputType>
mu_boolean CheckRenderConditionWithVector(const Type findValue, const std::vector<InputType> values)
{
	return std::find(values.cbegin(), values.cend(), static_cast<InputType>(findValue)) != values.cend();
}

NRenderVirtualMeshToggle NModel::GenerateVirtualMeshToggle(const NMeshRenderConditionInput &input)
{
	const auto virtualMeshCount = VirtualMeshes.size();
	if (virtualMeshCount == 0u) return NRenderVirtualMeshToggle();

	NRenderVirtualMeshToggle toggles(virtualMeshCount, false);
	for (auto index = 0u; index < virtualMeshCount; ++index)
	{
		const auto &mesh = VirtualMeshes[index];

		if (mesh.Conditions.empty())
		{
			toggles[index] = true;
			continue;
		}

		for (const auto &conditions : mesh.Conditions)
		{
			mu_boolean isSelected = true;
			for (const auto &condition : conditions)
			{
				switch (condition.Type) {
				case EMeshRenderConditionType::ItemLevel: isSelected = CheckRenderCondition(condition.Operator, condition.UValue, input.Level); break;
				case EMeshRenderConditionType::ItemLevelByFormula: isSelected = CheckRenderCondition(condition.Operator, condition.UValue, input.LevelByFormula); break;
				case EMeshRenderConditionType::ItemRank: isSelected = CheckRenderCondition(condition.Operator, condition.UValue, input.Rank); break;
				case EMeshRenderConditionType::ItemOptionsCount: isSelected = CheckRenderCondition(condition.Operator, condition.UValue, input.OptionsCount); break;
				case EMeshRenderConditionType::ItemOption: isSelected = CheckRenderConditionWithVector(condition.UValue, input.OptionsType); break;
				case EMeshRenderConditionType::ItemOptionsMinRank: isSelected = CheckRenderCondition(condition.Operator, condition.UValue, input.OptionsMinRank); break;
				case EMeshRenderConditionType::ItemOptionsMaxRank: isSelected = CheckRenderCondition(condition.Operator, condition.UValue, input.OptionsMaxRank); break;
				case EMeshRenderConditionType::ItemOptionsAvgRank: isSelected = CheckRenderCondition(condition.Operator, condition.UValue, input.OptionsAvgRank); break;
				}
				if (isSelected) break;
			}
			if (!isSelected) continue;
			toggles[index] = true;
			break;
		}
	}

	return toggles;
}

NRenderVirtualMeshLightIndex NModel::GenerateVirtualMeshLightIndex(const NMeshRenderConditionInput &input)
{
	const auto virtualMeshCount = VirtualMeshes.size();
	if (virtualMeshCount == 0u) return NRenderVirtualMeshLightIndex();

	NRenderVirtualMeshLightIndex selectedLights(virtualMeshCount, NInvalidUInt32);
	for (auto index = 0u; index < virtualMeshCount; ++index)
	{
		const auto &mesh = VirtualMeshes[index];
		const auto &lights = mesh.Settings.Lights;
		const auto lightsCount = lights.size();

		for (auto lindex = 0u; lindex < lightsCount; ++lindex)
		{
			const auto &light = lights[lindex];

			if (light.Conditions.empty())
			{
				selectedLights[index] = lindex;
				goto continue_meshes;
			}

			for (const auto &conditions : mesh.Conditions)
			{
				mu_boolean isSelected = true;
				for (const auto &condition : conditions)
				{
					switch (condition.Type) {
					case EMeshRenderConditionType::ItemLevel: isSelected = CheckRenderCondition(condition.Operator, condition.UValue, input.Level); break;
					case EMeshRenderConditionType::ItemLevelByFormula: isSelected = CheckRenderCondition(condition.Operator, condition.UValue, input.LevelByFormula); break;
					case EMeshRenderConditionType::ItemRank: isSelected = CheckRenderCondition(condition.Operator, condition.UValue, input.Rank); break;
					case EMeshRenderConditionType::ItemOptionsCount: isSelected = CheckRenderCondition(condition.Operator, condition.UValue, input.OptionsCount); break;
					case EMeshRenderConditionType::ItemOption: isSelected = CheckRenderConditionWithVector(condition.UValue, input.OptionsType); break;
					case EMeshRenderConditionType::ItemOptionsMinRank: isSelected = CheckRenderCondition(condition.Operator, condition.UValue, input.OptionsMinRank); break;
					case EMeshRenderConditionType::ItemOptionsMaxRank: isSelected = CheckRenderCondition(condition.Operator, condition.UValue, input.OptionsMaxRank); break;
					case EMeshRenderConditionType::ItemOptionsAvgRank: isSelected = CheckRenderCondition(condition.Operator, condition.UValue, input.OptionsAvgRank); break;
					}
					if (isSelected) break;
				}
				if (!isSelected) continue;
				selectedLights[index] = lindex;
				goto continue_meshes;
			}
		}

continue_meshes:
		continue;
	}

	return selectedLights;
}