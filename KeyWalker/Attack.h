#pragma once
#include "Texture.h"

class Attack
{
public:
	Attack();
	Attack( Vector2f position, float speed, float scale );
	~Attack();

	void Update( float deltaTime );

	void Draw() const;

	const float GetLifeTime() const;
	const float GetRadius() const;
	const Vector2f GetPosition() const;
	const Vector2f GetDirection() const;

	void Reset();

	void SetPosition( Vector2f position );
	// Set internal position directly (position is the logical center)
	void SetCenterPosition( Vector2f position );
	void SetDirection( Vector2f direction );
	void SetRadius( float radius );
	void SetSpeed( float speed );
	void Deactivate();

	const bool IsActive() const;
	const Circlef GetBounds() const;


private:
	void Move( float deltaTime );

	Texture* m_Texture;
	Vector2f m_Position;
	Vector2f m_Direction;
	float m_Speed;
	const float m_Radius;
	float m_Scale;
	float m_LifeTime;
	bool m_Active;
};

