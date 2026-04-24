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

    // Returns index of spawned attack, or -1 if none
    int SpawnAttack();

    // if isHex is true, spawned attacks will be snapped to hex tile centers
    void SpawnAlteratingAttack(const float amount, const float gapSize, const Vector2f direction, const float mapWidth, const float mapHeight, bool isHex);
	void IncreaseAttackSpeed();

	const bool IsColliding(Circlef collider, const Vector2f direction);

private:
	std::vector<Attack> m_Attacks;
	std::vector<int> m_FreeSlots;


};

