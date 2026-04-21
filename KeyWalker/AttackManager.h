#pragma once
#include "Attack.h"
#include <vector>

class AttackManager
{
public:
	AttackManager();
	~AttackManager();

	void Update(const float deltaTime);
	void Draw() const;

	Attack* SpawnAttack();

	void SpawnAlteratingAttack(const float amount, const float gapSize, const Vector2f direction, const float mapWidth, const float mapHeight , bool offSet);
	void IncreaseAttackSpeed();

	const bool isColliding(Circlef collider);

private:
	std::vector<Attack> m_Attacks;
	std::vector<int> m_FreeSlots;


};

