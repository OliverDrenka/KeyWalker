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
{
	m_Texture = new Texture("Player.png");
}

Player::~Player()
{
	delete m_Texture;
}

void Player::Draw(const float tileSize) const
{	
	const Vector2f position
	{
		m_Position.x * tileSize + (tileSize - m_Texture->GetWidth() )/2,
		m_Position.y * tileSize + (tileSize - m_Texture->GetWidth() ) / 2
	};
	m_Texture->Draw(position);
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
