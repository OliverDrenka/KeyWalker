#pragma once
#include "Vector2i.h"
#include "Texture.h"

class Player
{
public:
	Player();
	Player(Vector2i position);
	~Player();

	void Draw(const float tileSize = 0) const;
	void Move( Vector2i direction );
	void Update(const float deltaTime);


	const Vector2i GetPosition();
	const Rectf GetBounds();

private:
	Vector2i m_Position;
	Texture* m_Texture;
	int m_Hp;


};

