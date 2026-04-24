#include "pch.h"
#include "AttackManager.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include "utils.h"

AttackManager::AttackManager()
{
	m_Attacks.resize(300);
	m_FreeSlots.reserve(300);
	for (int idx{ 0 }; idx < m_Attacks.size(); ++idx) {
		m_FreeSlots.push_back(idx);
		m_Attacks[idx].SetSpeed(rand()%5 + 14);
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

		if (attack.GetLifeTime() >= 25)
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

void AttackManager::IncreaseAttackSpeed()
{
	for (int idx{ 0 }; idx < m_Attacks.size(); ++idx) {
		m_Attacks[idx].SetSpeed(m_Attacks[idx].GetSpeed() + 1);
	}
}

int AttackManager::SpawnAttack()
{
    if (m_FreeSlots.empty())
    {
        return -1;
    }

    int index = m_FreeSlots.back();
    m_FreeSlots.pop_back();

    Attack& attack = m_Attacks[index];
    attack.Reset();

    return index;
}

void AttackManager::SpawnAlteratingAttack(const float amount, const float gapSize, const Vector2f direction, const float mapWidth, const float mapHeight, bool isHex)
{
    // gapSize is expected to be tileSize * 2 by callers; derive tileSize
    const float tileSize = gapSize * 0.5f;
    const int numCols = std::max(1, static_cast<int>(std::round(mapWidth / tileSize)));
    const int numRows = std::max(1, static_cast<int>(std::round(mapHeight / tileSize)));

    // Spawn distance outside the map (randomized to avoid perfect alignment on screen)
    // localSpawnDist is computed per-spawn below

    // Helper: spawn for one direction with parity offset (0 or 1)
    auto spawnDir = [&](const Vector2f& dir, int parity)
    {
        Vector2f nd = dir.Normalized();
        // If approaching more horizontally, iterate rows and anchor to tile centers on the edge
        if (std::abs(dir.x) > std::abs(dir.y))
        {
            for (int row = parity; row < numRows; row += 2)
            {
                int idx = SpawnAttack();
                if (idx < 0) return;
                Attack* attack = &m_Attacks[idx];

                const float y = row * tileSize + tileSize * 0.5f;
                const float rowXOffset = (isHex && (row & 1)) ? tileSize * 0.5f : 0.0f;
                const float spawnDist = tileSize * static_cast<float>(3 + (rand() % 5));

                // exact tile center at map edge (col 0 or last)
                const int edgeCol = (dir.x > 0) ? 0 : (numCols - 1);
                const float centerX = edgeCol * tileSize + rowXOffset + tileSize * 0.5f;
                Vector2f baseCenter(centerX, y);

                // move center back along the incoming direction to place off-map
                Vector2f spawnCenter = baseCenter - nd * spawnDist;

                // separation check
                const float minSeparation = tileSize * 0.9f;
                bool tooClose = false;
                for (int ai = 0; ai < static_cast<int>(m_Attacks.size()); ++ai)
                {
                    if (ai == idx) continue;
                    const Attack& a = m_Attacks[ai];
                    if (!a.IsActive()) continue;
                    if ((a.GetPosition() - spawnCenter).Length() < minSeparation)
                    {
                        tooClose = true; break;
                    }
                }
                if (tooClose) { attack->Deactivate(); m_FreeSlots.push_back(idx); continue; }

                attack->SetCenterPosition(spawnCenter);
                attack->SetDirection(-nd);
            }
        }
        else
        {
            for (int col = parity; col < numCols; col += 2)
            {
                int idx = SpawnAttack();
                if (idx < 0) return;
                Attack* attack = &m_Attacks[idx];

                const int edgeRow = (dir.y > 0) ? 0 : (numRows - 1);
                const float rowXOffset = (isHex && (edgeRow & 1)) ? tileSize * 0.5f : 0.0f;
                const float centerX = col * tileSize + rowXOffset + tileSize * 0.5f;
                const float centerY = edgeRow * tileSize + tileSize * 0.5f;
                const float spawnDist = tileSize * static_cast<float>(3 + (rand() % 5));

                Vector2f baseCenter(centerX, centerY);
                Vector2f spawnCenter = baseCenter - nd * spawnDist;

                const float minSeparation = tileSize * 0.9f;
                bool tooCloseCol = false;
                for (int ai = 0; ai < static_cast<int>(m_Attacks.size()); ++ai)
                {
                    if (ai == idx) continue;
                    const Attack& a = m_Attacks[ai];
                    if (!a.IsActive()) continue;
                    if ((a.GetPosition() - spawnCenter).Length() < minSeparation)
                    {
                        tooCloseCol = true; break;
                    }
                }
                if (tooCloseCol) { attack->Deactivate(); m_FreeSlots.push_back(idx); continue; }

                attack->SetCenterPosition(spawnCenter);
                attack->SetDirection(-nd);
            }
        }
    };

    // spawn for direction and its opposite (opposite uses parity=1 to fill gaps)
    spawnDir(direction, 0);
    spawnDir(Vector2f(-direction.x, -direction.y), 1);
}

const bool AttackManager::IsColliding(Circlef collider, const Vector2f direction)
{
	for (int idx{ 0 }; idx < m_Attacks.size(); ++idx)
	{
		Attack& attack = m_Attacks[idx];


		if (attack.IsActive())
		{
			const Vector2f
				centerDistance(attack.GetPosition() - collider.center);
			const float
				radiusTotal(attack.GetRadius() + collider.radius);
			if (centerDistance.Length() <= radiusTotal)
			{
				
				attack.SetDirection(attack.GetDirection() * -1);
				const float overlap{ radiusTotal - centerDistance.Length() };
				std::cout << centerDistance.Normalized() << ", " << attack.GetDirection() << std::endl;
				if (direction.x == attack.GetDirection().x && direction.y == attack.GetDirection().y)
				{
					attack.Deactivate();
					m_FreeSlots.push_back(idx);
					return false;
				}
				Vector2f newCenter = attack.GetPosition() - overlap * attack.GetDirection();
				attack.SetCenterPosition(newCenter);
				return true;
			}

		}
	}
	return false;
}
