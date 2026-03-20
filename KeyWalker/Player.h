#pragma once
#include "Vector2i.h"

class Player
{
public:
	Player();
	Player(Vector2i position);
	~Player();

	void Draw();
	void Move( Vector2i direction );

	const Vector2i GetPosition();
	const Rectf GetBounds();

private:
	Vector2i m_Position;
	int m_Hp;


};

