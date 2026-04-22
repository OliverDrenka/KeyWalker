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
	, m_Radius { 6.f }
{
	m_Texture = new Texture("BallAttack.png");
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
    if (m_Texture)
    {
        // Draw texture centered at m_Position (m_Position is logical center used for collisions)
        Vector2f drawPos = m_Position - Vector2f(m_Texture->GetWidth() * 0.5f, m_Texture->GetHeight() * 0.5f);
        m_Texture->Draw(drawPos);
    }
    else
    {
        m_Texture->Draw(m_Position);
    }
}

const float Attack::GetLifeTime() const
{
	return m_LifeTime;
}

const float Attack::GetRadius() const
{
	return m_Radius;
}

const Vector2f Attack::GetPosition() const
{
    return m_Position;
}

const Vector2f Attack::GetDirection() const
{
	return m_Direction;
}

const float Attack::GetSpeed() const
{
	return m_Speed;
}

void Attack::Reset()
{
	m_LifeTime = 0.f;
	m_Active = true;
}


void Attack::SetPosition(Vector2f position)
{
    // External code historically provided bottom-left texture coordinates.
    // Internally we store the logical center to simplify collision math and drawing.
    if (m_Texture)
    {
        m_Position = position + Vector2f(m_Texture->GetWidth() * 0.5f, m_Texture->GetHeight() * 0.5f);
    }
    else
    {
        m_Position = position;
    }
}

void Attack::SetCenterPosition(Vector2f position)
{
    m_Position = position;
}

void Attack::SetDirection(Vector2f direction)
{
	m_Direction = direction;
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
