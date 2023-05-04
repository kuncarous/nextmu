#include "stdafx.h"
#include "mu_environment_characters.h"
#include "mu_environment.h"
#include "mu_state.h"

mu_float CalculateAngle(const glm::vec2 &vec1, const glm::vec2 &vec2)
{
	// Calculate the direction vector from vec1 to vec2
	glm::vec2 dir = glm::normalize(vec2 - vec1);

	// Calculate the angle in radians using the inverse tangent function
	mu_float angle = glm::atan(dir.y, dir.x);

	// Convert the angle from radians to degrees
	angle = glm::degrees(angle);

	// Adjust the angle to be in the range [0, 360)
	if (angle < 0.0f) {
		angle += 360.0f;
	}

	angle = glm::mod((angle + 90.0f), 360.0f);

	return angle;
}

void NCharacters::MoveCharacter(const entt::entity entity)
{
	auto [position, action, movement, moveSpeed, modifiers] = Registry.get<NEntity::NPosition, NEntity::NAction, NEntity::NMovement, NEntity::NMoveSpeed, NEntity::NModifiers>(entity);
	
	mu_boolean canMove = true;
	if (action.Index != NInvalidUInt16)
	{

	}

	if (canMove && movement.Moving && MovePath(position, movement, moveSpeed, modifiers) == true)
	{
		movement.Moving = false;
		SetCharacterAction(entity, NAnimationType::Stop);
	}
}

NEXTMU_INLINE const mu_float CalculateMoveSpeed(const NTerrain *terrain, const glm::vec3 &position, const NEntity::NMoveSpeed &moveSpeed, const NEntity::NModifiers &modifiers)
{
	constexpr mu_float MaxWalkSpeed = 12.0f;
	const auto attribute = terrain->GetAttribute(GetPositionFromFloat(position.x), GetPositionFromFloat(position.y));
	if ((attribute & TerrainAttribute::SafeZone) != 0) return glm::min(moveSpeed.Walk * modifiers.Normalized.MoveSpeed, MaxWalkSpeed);
	if (terrain->IsSwimming()) return moveSpeed.Swim * modifiers.Normalized.MoveSpeed;
	return moveSpeed.Run * modifiers.Normalized.MoveSpeed;
}

mu_boolean NCharacters::MovePath(NEntity::NPosition &position, NEntity::NMovement &movement, NEntity::NMoveSpeed &moveSpeed, const NEntity::NModifiers &modifiers)
{
	const mu_float updateTime = MUState::GetUpdateTime();
	mu_float distanceToMove = CalculateMoveSpeed(Environment->GetTerrain(), position.Position, moveSpeed, modifiers) * updateTime;
	if (distanceToMove <= 0.0f) return true;
	auto &path = movement.Path;
	if (path.CurrentPoint >= path.PointsCount) return true;

	while (true)
	{
		const glm::vec2 targetPosition = path.Points[path.CurrentPoint];
		const glm::vec2 sourcePosition = position.Position;
		const mu_float distance = glm::distance(sourcePosition, targetPosition);
		const mu_float moveDistance = glm::min(distance, distanceToMove);
		distanceToMove -= moveDistance;
		const glm::vec2 direction = glm::normalize(targetPosition - sourcePosition);

		const glm::vec2 newPosition = sourcePosition + direction * moveDistance;
		if (newPosition == targetPosition || moveDistance >= distance)
		{
			++path.CurrentPoint;
		}

		position.Position.x = newPosition.x;
		position.Position.y = newPosition.y;

		if (distanceToMove <= 0.0f || path.CurrentPoint >= path.PointsCount)
		{
			position.Angle.z = CalculateAngle(sourcePosition, targetPosition);
			return path.CurrentPoint >= path.PointsCount;
		}
	}

	return false;
}