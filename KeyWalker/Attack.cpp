#include "pch.h"
#include "Attack.h"

Attack::Attack()
	: Attack( Vector2f( 0, 0 ), 0, 0 )
{
}

Attack::Attack(Vector2f position, float speed, float scale)
	: m_Position { position }
	, m_Speed { speed }
	, m_Scale { scale }
	, m_LifeTime { 0 }
	, m_Active { false }
{
	m_Texture = new Texture("BallAttack.png");
	m_Radius = m_Texture->GetWidth() / 2;
}

Attack::~Attack()
{
	delete m_Texture;
}

void Attack::Move( float deltaTime )
{
	m_Position -= m_Direction * deltaTime * m_Speed;
}

void Attack::Update(float deltaTime)
{
	Move(deltaTime);
	m_LifeTime += deltaTime;
}

void Attack::Draw() const
{
	m_Texture->Draw(m_Position);
}

const float Attack::GetLifeTime() const
{
	return m_LifeTime;
}

const float Attack::GetRadius() const
{
	return m_Radius;
}

void Attack::Reset()
{
	m_LifeTime = 0.f;
	m_Active = true;
}


void Attack::SetPosition(Vector2f position)
{
	m_Position = position;
}

void Attack::SetDirection(Vector2f direction)
{
	m_Direction = direction;
}

void Attack::SetRadius(float radius)
{
	m_Radius = radius;
}

void Attack::SetSpeed(float speed)
{
	m_Speed = speed;
}

void Attack::Deactivate()
{
	m_Active = false;
}

const bool Attack::IsActive() const
{
	return m_Active;
}

const Circlef Attack::GetBounds() const
{
	return Circlef(m_Position, m_Radius);
}
