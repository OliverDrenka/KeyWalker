#include "pch.h"
#include "Player.h"
#include "utils.h"

Player::Player()
	:Player(Vector2i (0, 0))
{
}

Player::Player(Vector2i position)
	: m_Position{ position }
	, m_Hp{ 3 }
{
}

Player::~Player()
{
}

void Player::Draw()
{
	utils::SetColor(Color4f(0.5f, 0.2f, 0.5f, 1.f));

	utils::FillRect(m_Position.y * 12 + 1, m_Position.x * 12 + 1, 10, 10);
}

void Player::Move(Vector2i direction)
{
	m_Position.x += direction.x;
	m_Position.y += direction.y;
}

const Vector2i Player::GetPosition()
{
	return m_Position;
}

const Rectf Player::GetBounds()
{
	Rectf bounds
	{
		static_cast<float>(m_Position.x),
		static_cast<float>(m_Position.y),
		10.f,
		10.f
	};
	return bounds;
}
