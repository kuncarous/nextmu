#include "mu_precompiled.h"
#include "mu_environment_controller.h"
#include "mu_environment_characters.h"
#include "mu_environment.h"
#include "mu_config.h"
#include "mu_graphics.h"
#include "mu_capabilities.h"
#include "mu_state.h"
#include "mu_renderstate.h"
#include "mu_input.h"
#include "mu_navigation.h"
#include <glm/gtx/vec_swizzle.hpp>

NController::NController(const NEnvironment *environment) : Environment(environment), Camera(new_nothrow NCamera()), Character(entt::null)
{
	/*auto &camera = *Camera;
	camera.SetMode(NCameraMode::Directional);
	camera.SetEye(glm::vec3(123.0f * TerrainScale, 123.0f * TerrainScale, 1400.0f));
	camera.SetAngle(glm::vec3(glm::radians(30.0f), glm::radians(107.0f), glm::radians(0.0f)));*/
}

NController::~NController()
{

}

void NController::Update()
{
	if (Character != entt::null)
	{
		// We update the camera based on the character
		auto characters = Environment->GetCharacters();
		auto &registry = characters->GetRegistry();
		const auto &position = registry.get<NEntity::NPosition>(Character);
		Camera->SetTarget(position.Position);
	}
	else
	{
		// We update the camera as free camera
	}

	Camera->Update();

	const auto windowWidth = MURenderState::GetRenderWidth();
	const auto windowHeight = MURenderState::GetRenderHeight();
	auto camera = GetCamera();
	glm::mat4 view = camera->GetView();
	glm::mat4 shadowView = camera->GetShadowView();
	constexpr mu_float HomogeneousFar = 15000.0f;
	constexpr mu_float NonHomogeneousFar = 50000.0f;
	const mu_float aspect = static_cast<mu_float>(windowWidth) / static_cast<mu_float>(windowHeight);
	const mu_boolean isGL = (
		MUGraphics::GetDeviceType() == Diligent::RENDER_DEVICE_TYPE_GL ||
		MUGraphics::GetDeviceType() == Diligent::RENDER_DEVICE_TYPE_GLES
	);
	const mu_float nearZ = 50.0f;
	const mu_float farZ = (
		MUCapabilities::IsHomogeneousDepth()
		? HomogeneousFar
		: NonHomogeneousFar
	);
	Diligent::float4x4 projection = Diligent::float4x4::Projection(
		glm::radians(35.0f),
		aspect,
		nearZ,
		farZ,
		MUGraphics::GetDeviceType() == Diligent::RENDER_DEVICE_TYPE_GL ||
		MUGraphics::GetDeviceType() == Diligent::RENDER_DEVICE_TYPE_GLES
	);
	Diligent::float4x4 shadowProjection = Diligent::float4x4::Projection(
		glm::pi<mu_float>() * 0.25f,
		aspect,
		nearZ,
		MUConfig::GetShadowFarZ(),
		MUGraphics::GetDeviceType() == Diligent::RENDER_DEVICE_TYPE_GL ||
		MUGraphics::GetDeviceType() == Diligent::RENDER_DEVICE_TYPE_GLES
	);

	camera->GenerateFrustum(view, GLMFromFloat4x4(projection), nearZ, farZ);

	MURenderState::SetViewTransform(view, GLMFromFloat4x4(projection), GLMFromFloat4x4(projection), shadowView, GLMFromFloat4x4(shadowProjection));
	MURenderState::AttachCamera(camera);
	
	auto centerNearPoint = (
		isGL
		? glm::unProjectNO(glm::vec3(windowWidth * 0.5f, windowHeight * 0.5f, 0.0f), view, MURenderState::GetProjection(), glm::vec4(0.0f, 0.0f, windowWidth, windowHeight))
		: glm::unProjectZO(glm::vec3(windowWidth * 0.5f, windowHeight * 0.5f, 0.0f), view, MURenderState::GetProjection(), glm::vec4(0.0f, 0.0f, windowWidth, windowHeight))
	);
	NearPoint = Diligent::float3(centerNearPoint.x, centerNearPoint.z, centerNearPoint.y); // glm::xzy

	if (Character != entt::null)
	{
		const auto mousePosition = MUInput::GetMousePosition();
		auto nearPoint = (
			isGL
			? glm::unProjectNO(glm::vec3(mousePosition.x, windowHeight - mousePosition.y, 0.0f), view, MURenderState::GetProjection(), glm::vec4(0.0f, 0.0f, windowWidth, windowHeight))
			: glm::unProjectZO(glm::vec3(mousePosition.x, windowHeight - mousePosition.y, 0.0f), view, MURenderState::GetProjection(), glm::vec4(0.0f, 0.0f, windowWidth, windowHeight))
		);
		auto farPoint = (
			isGL
			? glm::unProjectNO(glm::vec3(mousePosition.x, windowHeight - mousePosition.y, 1.0f), view, MURenderState::GetProjection(), glm::vec4(0.0f, 0.0f, windowWidth, windowHeight))
			: glm::unProjectZO(glm::vec3(mousePosition.x, windowHeight - mousePosition.y, 1.0f), view, MURenderState::GetProjection(), glm::vec4(0.0f, 0.0f, windowWidth, windowHeight))
		);
		nearPoint = glm::xzy(nearPoint);
		farPoint = glm::xzy(farPoint);

		const auto rayDirection = glm::normalize(farPoint - nearPoint);
		glm::vec3 intersection;
		auto terrain = Environment->GetTerrain();
		if (terrain->GetTriangleIntersection(nearPoint, farPoint, rayDirection, intersection) == true)
		{
			if (MUInput::IsShiftPressing() == false && MUInput::IsMousePressing(MOUSE_BUTTON_LEFT) == true)
			{
				static NNavPolys navPolys;

				auto characters = Environment->GetCharacters();
				auto &registry = characters->GetRegistry();
				auto [position, movement, action] = registry.get<NEntity::NPosition, NEntity::NMovement, NEntity::NAction>(Character);

				movement.Moving = MUNavigation::FindShortestPath(terrain->GetNavMesh(), terrain->GetNavMeshQuery(0), position.Position, intersection, &navPolys, &movement.Path, 0.0f);
				if (movement.Moving)
				{
					characters->SetCharacterAction(Character, NAnimationType::Walk);
				}
				else if (action.Type == NAnimationType::Walk)
				{
					characters->SetCharacterAction(Character, NAnimationType::Stop);
				}

				const auto updateCount = MUState::GetUpdateCount();
				if (updateCount > 0)
				{
					Environment->GetParticles()->Create(
						NParticleData{
							.Layer = 0,
							.Type = ParticleType::TrueFire_Red_V5,
							.Position = intersection + glm::vec3(0.0f, 0.0f, 30.0f),
							.Scale = 2.8f
						}
					);
				}
			}
		}
	}
}

void NController::PreRender()
{
	DistanceToCharacter = 0.0f;
	if (Character != entt::null)
	{
		auto characters = Environment->GetCharacters();
		auto &registry = characters->GetRegistry();
		auto [position, boundingBoxes] = registry.get<NEntity::NPosition, NEntity::NBoundingBoxes>(Character);
		auto &bbox = boundingBoxes.AABB.Calculated;

		Diligent::BoundBox b;
		b.Min = Diligent::float3(bbox.Min.x, bbox.Min.y, bbox.Min.z);
		b.Max = Diligent::float3(bbox.Max.x, bbox.Max.y, bbox.Max.z);
		DistanceToCharacter = Diligent::GetPointToBoxDistance(b, NearPoint);
	}
}

void NController::Configure()
{
	auto entity = Character;
	if (entity != entt::null)
	{
		Camera->SetMode(NCameraMode::Positional);
		Camera->SetMinDistance(200.0f);
		Camera->SetMaxDistance(50000.0f);
		Camera->SetDistance(1313.43750f);
		Camera->SetAngle(glm::vec3(glm::radians(135.0f), glm::radians(130.0f), glm::radians(0.0f)));
		Camera->SaveAsDefault();
	}
	else
	{
		Camera->SetMode(NCameraMode::Directional);
	}
}