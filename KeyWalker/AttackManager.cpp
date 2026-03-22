#include "pch.h"
#include "AttackManager.h"
#include <iostream>

AttackManager::AttackManager()
{
	m_Attacks.resize(300);
	m_FreeSlots.reserve(300);
	for (int idx{ 0 }; idx < m_Attacks.size(); ++idx) {
		m_FreeSlots.push_back(idx);
		m_Attacks[idx].SetSpeed(rand()%5 + 15);
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

		if (attack.GetLifeTime() >= 15)
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

void AttackManager::SpawnAlteratingAttack(const float amount, const float gapSize, const Vector2f direction, const float mapWidth, const float mapHeight, bool offSet)
{
	const Vector2f
		orthogonal{ direction.Orthogonal() };
	const float
		maxWidth{ mapWidth - 16 },
		maxHeight{ mapHeight - 16 };

	for (int idx{}; idx < amount+3; ++idx)
	{
		Attack* attack = SpawnAttack();
		if (!attack)
		{
			return;
		}


		Vector2f offset
		{
			-16 * (1 + rand() % 5) * direction.x + orthogonal.x * (idx * gapSize),
			-16 * (1 + rand() % 5) * direction.y + orthogonal.y * (idx * gapSize)
		};
		Vector2f center 
		{
			maxWidth / 2.f,
			maxHeight / 2.f
		};

		Vector2f dir{ (direction + orthogonal).Normalized() };
		float angle = atan2(dir.y, dir.x);
		float x = (maxWidth) / sqrt(2.0f) * cos(angle);
		float y = (maxHeight) / sqrt(2.0f) * sin(angle);
		center.x += x - (16 * (rand() % 2));
		center.y += y - (16 * (rand()%2));

		Vector2f position{ center - offset };

		attack->SetPosition(position);
		attack->SetDirection(direction);
	}
}
