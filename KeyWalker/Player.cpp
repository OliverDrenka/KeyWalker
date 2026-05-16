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
	m_SpriteSheet = new SpriteSheet(4, "Resources/Player.png", 3);
}

void Player::ApplyBuff(BuffType type, float duration)
{
    // multiple buff types can be active while sharing a single timer
    switch (type)
    {
    case BuffType::hex: m_HexBuff = true; break;
    case BuffType::wrap: m_WrapBuff = true; break;
    case BuffType::reveal: m_RevealBuff = true; break;
    default: break;
    }
    m_BuffTimer = duration;
}

bool Player::HasBuff(BuffType type) const
{
    if (m_BuffTimer <= 0.f) return false;
    switch (type)
    {
    case BuffType::hex: return m_HexBuff;
    case BuffType::wrap: return m_WrapBuff;
    case BuffType::reveal: return m_RevealBuff;
    default: return false;
    }
}

void Player::ClearBuff()
{
    m_HexBuff = m_WrapBuff = m_RevealBuff = false;
    m_BuffTimer = 0.f;
}

void Player::ApplyDebuff(float duration)
{
    m_DebuffTimer = duration;
}

bool Player::IsDebuffed() const
{
    return m_DebuffTimer > 0.f;
}

Player::~Player()
{
	delete m_SpriteSheet;
}

void Player::Draw(const float tileSize, bool hexMode) const
{	
    // Draw player at native sprite size, centered inside the tile (original behavior)
    // scale player so base 16px -> tile scaling; keep centering offsets
    const float baseTile = 16.f;
    const float playerScale = tileSize / baseTile;
    const float playerW = 14;
    const float playerH = 14;
    const float playerDestW = playerW * playerScale;
    const float playerDestH = playerH * playerScale;

    if (!hexMode)
    {
        Vector2f center{ m_Position.x * tileSize + tileSize * 0.5f, m_Position.y * tileSize + tileSize * 0.5f };
        Vector2f drawPos{ center.x - (playerDestW * 0.5f), center.y - (playerDestH * 0.5f) };
        m_SpriteSheet->DrawSprite(drawPos, 2, 2, playerDestW, playerDestH);
		const float
			statusBarWidth{11.f / 10.f * m_BuffTimer },
			debuffBarWidth{11.f / 10.f * m_DebuffTimer };
	
		utils::SetColor(Color4f(10.f / 255.f, 222.f / 255.f, 241.f / 255.f, 1.f));
		utils::FillRect(drawPos.x + playerW/2 - statusBarWidth/2, drawPos.y + 9.2f, statusBarWidth, 2.4f);
		utils::SetColor(Color4f(118.f / 255.f, 45.f / 255.f, 255.f / 255.f, 1.f));
		utils::FillRect(drawPos.x + playerW/2 - debuffBarWidth /2, drawPos.y + 2.5f, debuffBarWidth, 2.4);
        m_SpriteSheet->DrawSprite(drawPos, m_SpriteIdx,m_Hp - 1, playerDestW, playerDestH);
    }
    else
    {
        const float xOffset = tileSize * 0.5f;

        float x = m_Position.x * tileSize + ((m_Position.y & 1) ? xOffset : 0.0f);
        float y = m_Position.y * tileSize; // <-- keep square spacing

        const Vector2f position{
            x + (tileSize - playerDestW) / 2,
            y + (tileSize - playerDestH) / 2
        };

		m_SpriteSheet->DrawSprite(position, 2, 2, playerDestW, playerDestH);
		const float
			statusBarWidth{ 11.f / 10.f * m_BuffTimer },
			debuffBarWidth{ 11.f / 10.f * m_DebuffTimer };

		utils::SetColor(Color4f(10.f / 255.f, 222.f / 255.f, 241.f / 255.f, 1.f));
		utils::FillRect(position.x + playerW / 2 - statusBarWidth / 2, position.y + 9.2f, statusBarWidth, 2.4f);
		utils::SetColor(Color4f(118.f / 255.f, 45.f / 255.f, 255.f / 255.f, 1.f));
		utils::FillRect(position.x + playerW / 2 - debuffBarWidth / 2, position.y + 2.5f, debuffBarWidth, 2.4);
		m_SpriteSheet->DrawSprite(position, m_SpriteIdx, m_Hp - 1, playerDestW, playerDestH);
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

    // update buff/debuff timers
    if (m_BuffTimer > 0.f)
    {
        m_BuffTimer -= deltaTime;
        if (m_BuffTimer <= 0.f)
        {
            m_BuffTimer = 0.f;
            // clear all buff flags when timer expires
            m_HexBuff = m_WrapBuff = m_RevealBuff = false;
        }
    }

    if (m_DebuffTimer > 0.f)
    {
        m_DebuffTimer -= deltaTime;
        if (m_DebuffTimer <= 0.f)
        {
            m_DebuffTimer = 0.f;
        }
    }
}

void Player::Move(Vector2i direction)
{

	m_Direction = direction;
    m_Position.x += direction.x;
    m_Position.y += direction.y;
}

void Player::SetPosition(const Vector2i& position)
{
    m_Position = position;
}

void Player::Hit(const float damage)
{
	if (m_Hp > 0)
	{
		m_Hp -= damage;
		if (m_Hp < 0)
		{
			m_Hp = 0;
		}
		m_SpriteIdx = 1;

	}
}

void Player::Heal(int amount)
{
    m_Hp += amount;
    if (m_Hp > 3) m_Hp = 3;
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
