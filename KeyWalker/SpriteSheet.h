#pragma once
#include "Texture.h"

class SpriteSheet
{
public:
	SpriteSheet();
	SpriteSheet(const int totalSprites, std::string spriteSheetPath, const int numRows = 1 );
	~SpriteSheet();

	void DrawSprite(Vector2f position, int spriteIdx, int row = 0);
    // Draw the sprite but scale it to the given destination size (width, height).
    void DrawSprite(Vector2f position, int spriteIdx, int row, float destWidth, float destHeight);
    // Convenience: square destination size
    void DrawSprite(Vector2f position, int spriteIdx, int row, float destSize);

	const float GetSpriteHeight();
	const float GetSpriteWidth();

private:
	Texture* m_SpriteSheet;
	const int m_TotalSprites;
	const int m_Rows;
	float m_SpriteWidth;
	float m_SpriteHeight;

};

