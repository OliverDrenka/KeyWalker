#pragma once
#include "Vector2i.h"
#include "SpriteSheet.h"

class Player
{
public:
	Player();
	Player(Vector2i position);
	~Player();

    void Draw(const float tileSize = 0, bool hexMode = false) const;
	void Move( Vector2i direction );
	void Update(const float deltaTime);
	void Hit(const float damage);

	const int GetHp() const;
	const Vector2i GetPosition();
    const Circlef GetBounds(const float tileSize, bool hexMode = false);

private:
	const float m_Radius;
	SpriteSheet* m_SpriteSheet;
	Vector2i m_Position;
	int m_Hp;

	int m_SpriteIdx;
	float m_FrameTimer;
	float m_TimePerFrame;


};

