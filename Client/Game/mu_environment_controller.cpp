#include "stdafx.h"
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

NController::NController(const NEnvironment *environment) : Environment(environment), Camera(new (std::nothrow) NCamera()), Character(entt::null)
{
	auto &camera = *Camera;
	camera.SetMode(NCameraMode::Directional);
	camera.SetEye(glm::vec3(123.0f * TerrainScale, 123.0f * TerrainScale, 1400.0f));
	camera.SetAngle(glm::vec3(glm::radians(30.0f), glm::radians(107.0f), glm::radians(0.0f)));
	camera.SetUp(glm::vec3(0.0f, 0.0f, 1.0f));
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

	const auto windowWidth = MUConfig::GetWindowWidth();
	const auto windowHeight = MUConfig::GetWindowHeight();
	auto camera = GetCamera();
	glm::mat4 view = camera->GetView();
	constexpr mu_float HomogeneousFar = 5000.0f;
	constexpr mu_float NonHomogeneousFar = 50000.0f;
	const mu_float aspect = static_cast<mu_float>(windowWidth) / static_cast<mu_float>(windowHeight);
	const mu_boolean isGL = (
		MUGraphics::GetDeviceType() == Diligent::RENDER_DEVICE_TYPE_GL ||
		MUGraphics::GetDeviceType() == Diligent::RENDER_DEVICE_TYPE_GLES
		);
	Diligent::float4x4 projection = Diligent::float4x4::Projection(
		glm::radians(35.0f),
		aspect,
		50.0f,
		MUCapabilities::IsHomogeneousDepth()
		? HomogeneousFar
		: NonHomogeneousFar,
		MUGraphics::GetDeviceType() == Diligent::RENDER_DEVICE_TYPE_GL ||
		MUGraphics::GetDeviceType() == Diligent::RENDER_DEVICE_TYPE_GLES
	);

	camera->GenerateFrustum(view, GLMFromFloat4x4(projection));

	MURenderState::SetViewTransform(view, GLMFromFloat4x4(projection), GLMFromFloat4x4(projection));
	MURenderState::AttachCamera(camera);

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

		// Since we use a left handed view we need to change sign in Y coord
		nearPoint.y *= -1.0f;
		farPoint.y *= -1.0f;

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
							.Type = ParticleType::TrueFire_V5,
							.Position = intersection + glm::vec3(0.0f, 0.0f, 30.0f),
							.Scale = 2.8f
						}
					);
				}
			}
		}
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