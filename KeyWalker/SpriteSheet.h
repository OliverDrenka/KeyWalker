#pragma once
#include "Texture.h"

class SpriteSheet
{
public:
	SpriteSheet();
	SpriteSheet(const int totalSprites, std::string spriteSheetPath);
	~SpriteSheet();

	void DrawSprite( Vector2f position, int spriteIdx );

	const float GetSpriteHeight();
	const float GetSpriteWidth();

private:
	Texture* m_SpriteSheet;
	const int m_TotalSprites;
	float m_SpriteWidth;

};

