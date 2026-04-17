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
	void Hit(const float damage);

	const int GetHp() const;
	const Vector2i GetPosition();
	const Circlef GetBounds(const float tileSize);

private:
	const float m_Radius;
	Texture* m_Texture;
	Vector2i m_Position;
	int m_Hp;


};

