#include "pch.h"
#include "Player.h"
#include "utils.h"
#include <iostream>

Player::Player()
	:Player(Vector2i (0, 0))
{
}

Player::Player(Vector2i position)
	: m_Position{ position }
	, m_Hp{ 3 }
	, m_Radius{ 4.f }
	, m_SpriteIdx{ 0 }
	, m_FrameTimer{ 0.f }
	, m_TimePerFrame{ 0.1f }
	, m_Direction{Vector2i(0,0)}
{
	m_SpriteSheet = new SpriteSheet(4, "Player.png", 3);
}

Player::~Player()
{
	delete m_SpriteSheet;
}

void Player::Draw(const float tileSize, bool hexMode) const
{	
    if (!hexMode)
    {
        // For square grid, m_Position represents tile coords; compute center and draw sprite centered
        Vector2f center{ m_Position.x * tileSize + tileSize * 0.5f, m_Position.y * tileSize + tileSize * 0.5f };
        Vector2f drawPos{ center.x - m_SpriteSheet->GetSpriteWidth() * 0.5f, center.y - m_SpriteSheet->GetSpriteHeight() * 0.5f };
        m_SpriteSheet->DrawSprite(drawPos, m_SpriteIdx, std::max(m_Hp - 1, 0));
    }
    else
    {
        // odd-r horizontal hex layout: use same offsets as Map::Draw
		const float xOffset = tileSize * 0.5f;

		float x = m_Position.x * tileSize + ((m_Position.y & 1) ? xOffset : 0.0f);
		float y = m_Position.y * tileSize; // <-- keep square spacing

		const Vector2f position{
			x + (tileSize - m_SpriteSheet->GetSpriteWidth()) / 2,
			y + (tileSize - m_SpriteSheet->GetSpriteHeight()) / 2
		};

		m_SpriteSheet->DrawSprite(position, m_SpriteIdx, std::max(m_Hp - 1,0));
    }
}
void Player::Update(const float deltaTime)
{
	if ((m_SpriteIdx != 0 && m_Hp > 0) || (m_Hp <= 0 && m_SpriteIdx != 3))
	{
		m_FrameTimer += deltaTime;
		if (m_FrameTimer >= m_TimePerFrame)
		{
			m_FrameTimer -= m_TimePerFrame;
			m_SpriteIdx += 1;
		}
		if (m_SpriteIdx > 3)
		{
			m_SpriteIdx = 0;
			m_FrameTimer = 0.f;
		}
	}
}

void Player::Move(Vector2i direction)
{

	m_Direction = direction;
	m_Position.x += direction.x;
	m_Position.y += direction.y;
}

void Player::Hit(const float damage)
{
	m_Hp -= damage;
	m_SpriteIdx = 1;
}

const int Player::GetHp() const
{
	return m_Hp;
}

const Vector2i Player::GetPosition()
{
	return m_Position;
}

const Circlef Player::GetBounds(const float tileSize, bool hexMode)
{
    if (!hexMode)
    {
        // For square grid use tile center as collision center (matches draw position)
        Circlef bounds
        {
            static_cast<float>(m_Position.x) * tileSize + tileSize * 0.5f,
            static_cast<float>(m_Position.y) * tileSize + tileSize * 0.5f,
            m_Radius
        };
        return bounds;
    }

	const float xOffset = tileSize * 0.5f;

	float x = m_Position.x * tileSize
		+ ((m_Position.y & 1) ? xOffset : 0.0f)
		+ tileSize / 2.f;

	float y = m_Position.y * tileSize   // <-- no 0.866 factor
		+ tileSize / 2.f;

	Circlef bounds{ x, y, m_Radius };
	return bounds;
}
void Player::SetDirection(Vector2i direction)
{
	m_Direction = direction;
}

const Vector2f Player::GetDirection() const
{
	return Vector2f(m_Direction.x, m_Direction.y);
}
