#include "mu_precompiled.h"
#include "mu_camera.h"
#include "mu_input.h"
#include "mu_state.h"
#include "mu_capabilities.h"
#include "mu_graphics.h"
#include "mu_math.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vec_swizzle.hpp>

void NCamera::Update()
{
	const mu_float elapsedTime = MUState::GetElapsedTime();

	const mu_boolean shiftPressed = MUInput::IsShiftPressing() == true;
	if (shiftPressed == true &&
		MUInput::IsMousePressing(MOUSE_BUTTON_LEFT) == true)
	{
		if (Dragging == false)
		{
			SDL_SetRelativeMouseMode(SDL_TRUE);
			SDL_GetRelativeMouseState(&BackupMousePosition.x, &BackupMousePosition.y);
			MUInput::ShowCursor(false);
			Dragging = true;
		}
	}
	else
	{
		if (Dragging == true)
		{
			SDL_SetRelativeMouseMode(SDL_FALSE);
			MUInput::ShowCursor(true);
			Dragging = false;
		}
	}

	// Update Mouse Delta
	if (Dragging)
	{
		// Get current position of mouse
		glm::ivec2 mousePosition;
		SDL_GetRelativeMouseState(&mousePosition.x, &mousePosition.y);

		// Smooth the relative mouse data with the old data so it isn't 
		// jerky when moving slowly at low frame rates.
		constexpr mu_float PercentOfNew = 0.5f;
		constexpr mu_float PercentOfOld = 1.0f - PercentOfNew;
		MouseDelta.x = MouseDelta.x * PercentOfOld + static_cast<mu_float>(mousePosition.x) * PercentOfNew;
		MouseDelta.y = MouseDelta.y * PercentOfOld + static_cast<mu_float>(mousePosition.y) * PercentOfNew;

		const glm::vec2 rotVelocity(MouseDelta.x * 0.01f, MouseDelta.y * 0.01f);
		if (Mode == NCameraMode::Directional)
			Angle += glm::vec3(-rotVelocity.x, rotVelocity.y, 0.0f);
		else
			Angle += glm::vec3(rotVelocity.x, -rotVelocity.y, 0.0f);
	}

	if (
		shiftPressed == true &&
		Dragging == false &&
		MUInput::IsMousePressing(MOUSE_BUTTON_RIGHT) == true
	)
	{
		RestoreDefault();
	}

	switch (Mode)
	{
	case NCameraMode::Directional:
		{
			Angle[0] = glm::mod(Angle[0], glm::radians(360.0f));
			// We need to limit the angle to avoid direction become exactly |up*inf|, this way we avoid the inf view issue
			Angle[1] = glm::clamp(Angle[1], glm::radians(2.0f), glm::radians(178.0f));
		}
		break;

	case NCameraMode::Positional:
		{
			Angle[0] = glm::mod(Angle[0], glm::radians(360.0f));
			// We need to limit the angle to avoid direction become exactly |up*inf|, this way we avoid the inf view issue
			Angle[1] = glm::clamp(Angle[1], glm::radians(90.0f), glm::radians(160.0f));
		}
		break;
	}

	const mu_float xSin = glm::sin(Angle[0]);
	const mu_float xCos = glm::cos(Angle[0]);
	const mu_float ySin = glm::sin(Angle[1]);
	const mu_float yCos = glm::cos(Angle[1]);

	const glm::vec3 direction(
		xCos * ySin,
		xSin * ySin,
		yCos
	);

	if (shiftPressed)
	{
		if (Mode == NCameraMode::Directional)
		{
			constexpr mu_float MoveSpeed = 2000.0f / 1000.0f;

			const glm::vec3 right(
				glm::cos(Angle[0] - glm::half_pi<mu_float>()) * -1.0f,
				glm::sin(Angle[0] - glm::half_pi<mu_float>()) * -1.0f,
				0.0f
			);

			if (MUInput::IsKeyPressing(SDL_SCANCODE_W))
			{
				Eye += direction * (MoveSpeed * elapsedTime);
			}

			if (MUInput::IsKeyPressing(SDL_SCANCODE_S))
			{
				Eye += direction * -(MoveSpeed * elapsedTime);
			}

			if (MUInput::IsKeyPressing(SDL_SCANCODE_A))
			{
				Eye += right * (MoveSpeed * elapsedTime);
			}

			if (MUInput::IsKeyPressing(SDL_SCANCODE_D))
			{
				Eye += right * -(MoveSpeed * elapsedTime);
			}
		}
		else if (Mode == NCameraMode::Positional)
		{
			const auto wheel = MUInput::GetMouseWheel() * 100.0f;
			if (glm::abs(wheel) > glm::epsilon<mu_float>())
			{
				Distance.Current -= wheel;
			}
		}
	}

	switch (Mode)
	{
	case NCameraMode::Directional:
		{
			Target = Eye + direction;
		}
		break;

	case NCameraMode::Positional:
		{
			Eye = Target - direction * Distance.Current;
		}
		break;
	}
}

void NCamera::GenerateFrustum(glm::mat4 view, glm::mat4 projection, const mu_float nearZ, const mu_float farZ)
{
	const auto deviceType = MUGraphics::GetDeviceType();
	const auto proj = Float4x4FromGLM(projection);
	glm::mat4 viewProj = projection * view;
	Diligent::ExtractViewFrustumPlanesFromMatrix(Float4x4FromGLM(viewProj), Frustum, deviceType == Diligent::RENDER_DEVICE_TYPE_GL || deviceType == Diligent::RENDER_DEVICE_TYPE_GLES);
	Diligent::BoundBox bbox {
		.Min = Diligent::float3(FLT_MAX, FLT_MAX, FLT_MAX),
		.Max = Diligent::float3(FLT_MIN, FLT_MIN, FLT_MIN),
	};

	for (mu_uint32 n = 0; n < 8; ++n)
	{
		auto &point = Frustum.FrustumCorners[n];
		bbox.Min.x = glm::min(bbox.Min.x, point.x);
		bbox.Min.y = glm::min(bbox.Min.y, point.y);
		bbox.Min.z = glm::min(bbox.Min.z, point.z);
		bbox.Max.x = glm::min(bbox.Max.x, point.x);
		bbox.Max.y = glm::min(bbox.Max.y, point.y);
		bbox.Max.z = glm::min(bbox.Max.z, point.z);
	}
	FrustumBBox = bbox;
}

glm::mat4 NCamera::GetView() const
{
	return glm::lookAtLH(glm::xzy(Eye), glm::xzy(Target), Up);
}

glm::mat4 NCamera::GetShadowView() const
{
	// We need a view that points in a direction XY direction but not from Up or Down.
	glm::vec3 eye = Eye;
	const glm::vec3 target = Target;
	eye.z = target.z;
	return glm::lookAtLH(glm::xzy(eye), glm::xzy(target), Up);
}

void NCamera::SetMode(const NCameraMode mode)
{
	Mode = mode;
}

void NCamera::SetEye(const glm::vec3 eye)
{
	Eye = eye;
}

void NCamera::SetTarget(const glm::vec3 target)
{
	Target = target;
}

void NCamera::SetAngle(const glm::vec3 angle)
{
	Angle = angle;
}

void NCamera::SetDistance(const mu_float distance)
{
	Distance.Current = distance;
}

void NCamera::SetMinDistance(const mu_float minDistance)
{
	Distance.Minimum = minDistance;
}

void NCamera::SetMaxDistance(const mu_float maxDistance)
{
	Distance.Maximum = maxDistance;
}

const NCameraMode NCamera::GetMode() const
{
	return Mode;
}

glm::vec3 NCamera::GetEye() const
{
	return Eye;
}

glm::vec3 NCamera::GetTarget() const
{
	return Target;
}

glm::vec3 NCamera::GetAngle() const
{
	return Angle;
}

glm::vec3 NCamera::GetUp() const
{
	return Up;
}

const mu_float NCamera::GetDistance() const
{
	return Distance.Current;
}

const mu_float NCamera::GetMinDistance() const
{
	return Distance.Minimum;
}

const mu_float NCamera::GetMaxDistance() const
{
	return Distance.Maximum;
}