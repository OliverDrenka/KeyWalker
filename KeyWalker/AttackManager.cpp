#include "pch.h"
#include "AttackManager.h"

AttackManager::AttackManager()
{
	m_Attacks.resize(100);
	m_FreeSlots.reserve(100);
	for (int idx{ 0 }; idx < m_Attacks.size(); ++idx) {
		m_FreeSlots.push_back(idx);
		m_Attacks[idx].SetSpeed(45);
	}
}

AttackManager::~AttackManager()
{
}

void AttackManager::Update(const float deltaTime)
{
	for (int idx{ 0 }; idx < m_Attacks.size(); ++idx)
	{
		Attack& attack = m_Attacks[idx];

		if (!attack.IsActive())
		{
			continue;
		}

		attack.Update(deltaTime);

		if (attack.GetLifeTime() >= 5)
		{
			attack.Deactivate();
			m_FreeSlots.push_back(idx);
		}
	}
}

void AttackManager::Draw() const
{
	for (const Attack& attack : m_Attacks)
	{
		if (!attack.IsActive())
		{
			continue;
		}
		attack.Draw();
	}
}

Attack* AttackManager::SpawnAttack()
{
	if (m_FreeSlots.empty())
	{
		return nullptr;
	}

	int index = m_FreeSlots.back();
	m_FreeSlots.pop_back();

	Attack& attack = m_Attacks[index];
	attack.Reset();

	return &attack;
}

void AttackManager::SpawnAlteratingAttack(const float amount, const float gapSize, const Vector2f direction, bool offSet)
{
	const Vector2f
		attackDirection{ direction.Orthogonal() };
	for (int idx{}; idx < amount; ++idx)
	{
		Attack* attack = SpawnAttack();
		if (!attack)
		{
			return;
		}
		
		const Vector2f
			position{ attackDirection.x * idx * gapSize, attackDirection.y * idx * gapSize };


		attack->SetPosition(position);
		attack->SetDirection(direction);
	}
}
