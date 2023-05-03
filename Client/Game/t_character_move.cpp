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
	const auto updateCount = MUState::GetUpdateCount();
	if (updateCount == 0u) return;

	auto [position, action, movement, moveSpeed] = Registry.get<NEntity::NPosition, NEntity::NAction, NEntity::NMovement, NEntity::NMoveSpeed>(entity);
	
	mu_boolean canMove = true;
	if (action.Index != NInvalidUInt16)
	{

	}

	if (canMove && movement.Moving && MovePath(position, movement, moveSpeed) == true)
	{
		movement.Moving = false;
	}
}

NEXTMU_INLINE const mu_float CalculateMoveSpeed(const NTerrain *terrain, const glm::vec3 &position, const NEntity::NMoveSpeed &moveSpeed)
{
	constexpr mu_float MaxWalkSpeed = 12.0f;
	const auto attribute = terrain->GetAttribute(GetPositionFromFloat(position.x), GetPositionFromFloat(position.y));
	if ((attribute & TerrainAttribute::SafeZone) != 0) return glm::min(moveSpeed.Walk * moveSpeed.Multiplier, MaxWalkSpeed);
	if (terrain->IsSwimming()) return moveSpeed.Swim * moveSpeed.Multiplier;
	return moveSpeed.Run * moveSpeed.Multiplier;
}

mu_boolean NCharacters::MovePath(NEntity::NPosition &position, NEntity::NMovement &movement, NEntity::NMoveSpeed &moveSpeed)
{
	const mu_float updateTime = MUState::GetUpdateTime();
	mu_float distanceToMove = CalculateMoveSpeed(Environment->GetTerrain(), position.Position, moveSpeed) * updateTime;
	if (distanceToMove == 0.0f) return true;
	auto &path = movement.Path;
	if (path.CurrentPoint >= path.PointsCount) return true;

	while (distanceToMove > 0.0f)
	{
		const glm::vec2 targetPosition = path.Points[path.CurrentPoint];
		const glm::vec2 sourcePosition = position.Position;
		const mu_float distance = glm::distance(sourcePosition, targetPosition);
		const mu_float moveDistance = glm::min(distance, distanceToMove);
		distanceToMove -= moveDistance;

		const glm::vec2 newPosition = glm::mix(sourcePosition, targetPosition, moveDistance / distance);
		if (newPosition == targetPosition || moveDistance >= distance)
		{
			++path.CurrentPoint;
		}

		position.Angle.z = CalculateAngle(sourcePosition * TerrainScaleInv, targetPosition * TerrainScaleInv);
		position.Position.x = newPosition.x;
		position.Position.y = newPosition.y;

		if (distanceToMove <= 0.0f || path.CurrentPoint >= path.PointsCount)
		{
			return path.CurrentPoint >= path.PointsCount;
		}
	}

	return false;
}