#include "stdafx.h"
#include "mu_camera.h"
#include "mu_input.h"
#include "mu_state.h"
#include "mu_capabilities.h"
#include "mu_math_frustum.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>

void NCamera::Update()
{
	const mu_float elapsedTime = MUState::GetElapsedTime();

	mu_boolean shiftPressed = MUInput::IsShiftPressing() == true;
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
		mu_float PercentOfNew = 0.5f;
		mu_float PercentOfOld = 1.0f - PercentOfNew;
		MouseDelta.x = MouseDelta.x * PercentOfOld + static_cast<mu_float>(mousePosition.x) * PercentOfNew;
		MouseDelta.y = MouseDelta.y * PercentOfOld + static_cast<mu_float>(mousePosition.y) * PercentOfNew;

		glm::vec2 rotVelocity(MouseDelta.x * 0.01f, MouseDelta.y * 0.01f);
		cglm::glm_vec3_add(Angle, cglm::vec3{ -rotVelocity.x, rotVelocity.y, 0.0f }, Angle);
	}

	switch (Mode)
	{
	case NCameraMode::Directional:
		{
			Angle[0] = glm::mod(Angle[0], glm::radians(360.0f));
			// We need to limit the angle to avoid direction become exactly |up*inf|, this way we avoid the nand view issue
			Angle[1] = glm::clamp(Angle[1], glm::radians(2.0f), glm::radians(178.0f));
		}
		break;

	case NCameraMode::Positional:
		{
			Angle[0] = glm::mod(Angle[0], glm::radians(360.0f) - glm::pi<float>() * 0.5f);
			// We need to limit the angle to avoid direction become exactly |up*inf|, this way we avoid the nand view issue
			Angle[1] = glm::clamp(Angle[1], glm::radians(90.0f), glm::radians(160.0f));
		}
		break;
	}

	const mu_float xSin = glm::sin(Angle[0]);
	const mu_float xCos = glm::cos(Angle[0]);
	const mu_float ySin = glm::sin(Angle[1]);
	const mu_float yCos = glm::cos(Angle[1]);

	cglm::vec3 direction = {
		xCos * ySin,
		xSin * ySin,
		yCos
	};

	if (shiftPressed && Mode == NCameraMode::Directional)
	{
		constexpr mu_float MoveSpeed = 2000.0f / 1000.0f;

		cglm::vec3 right{
			glm::cos(Angle[0] - glm::half_pi<mu_float>()) * -1.0f,
			glm::sin(Angle[0] - glm::half_pi<mu_float>()) * -1.0f,
			0.0f
		};

		if (MUInput::IsKeyPressing(SDL_SCANCODE_W))
		{
			cglm::glm_vec3_muladds(direction, MoveSpeed * elapsedTime, Eye);
		}

		if (MUInput::IsKeyPressing(SDL_SCANCODE_S))
		{
			cglm::glm_vec3_muladds(direction, -(MoveSpeed * elapsedTime), Eye);
		}

		if (MUInput::IsKeyPressing(SDL_SCANCODE_A))
		{
			cglm::glm_vec3_muladds(right, MoveSpeed * elapsedTime, Eye);
		}

		if (MUInput::IsKeyPressing(SDL_SCANCODE_D))
		{
			cglm::glm_vec3_muladds(right, -(MoveSpeed * elapsedTime), Eye);
		}
	}

	switch (Mode)
	{
	case NCameraMode::Directional:
		{
			cglm::glm_vec3_add(Eye, direction, Target);
		}
		break;

	case NCameraMode::Positional:
		{
			cglm::glm_vec3_copy(Target, Eye);
			cglm::glm_vec3_muladds(direction, Distance.Current, Eye);
		}
		break;
	}
}

void NCamera::GenerateFrustum(cglm::mat4 view, cglm::mat4 projection)
{
	Frustum.Update(view, projection);
}

void NCamera::GetView(cglm::mat4 out)
{
	cglm::glm_lookat_rh(Eye, Target, Up, out);
}

void NCamera::SetMode(NCameraMode mode)
{
	Mode = mode;
}

void NCamera::SetEye(glm::vec3 eye)
{
	cglm::glm_vec3_copy(glm::value_ptr(eye), Eye);
}

void NCamera::SetTarget(glm::vec3 target)
{
	cglm::glm_vec3_copy(glm::value_ptr(target), Target);
}

void NCamera::SetAngle(glm::vec3 angle)
{
	cglm::glm_vec3_copy(glm::value_ptr(angle), Angle);
}

void NCamera::SetUp(glm::vec3 up)
{
	cglm::glm_vec3_copy(glm::value_ptr(up), Up);
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

void NCamera::GetEye(cglm::vec3 out)
{
	cglm::glm_vec3_copy(Eye, out);
}

void NCamera::GetTarget(cglm::vec3 out)
{
	cglm::glm_vec3_copy(Target, out);
}

void NCamera::GetAngle(cglm::vec3 out)
{
	cglm::glm_vec3_copy(Angle, out);
}

void NCamera::GetUp(cglm::vec3 out)
{
	cglm::glm_vec3_copy(Up, out);
}

const mu_float NCamera::GetDistance()
{
	return Distance.Current;
}

const mu_float NCamera::GetMinDistance()
{
	return Distance.Minimum;
}

const mu_float NCamera::GetMaxDistance()
{
	return Distance.Maximum;
}