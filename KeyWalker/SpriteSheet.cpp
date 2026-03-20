#include "pch.h"
#include "SpriteSheet.h"

SpriteSheet::SpriteSheet()
	:SpriteSheet( 1, "" )
{
}

SpriteSheet::SpriteSheet(const int totalSprites, std::string spriteSheetPath)
	:m_TotalSprites( totalSprites )
{
	m_SpriteSheet = new Texture( spriteSheetPath );
	m_SpriteWidth = m_SpriteSheet->GetWidth() / m_TotalSprites;

}

SpriteSheet::~SpriteSheet()
{
	delete m_SpriteSheet;
}

void SpriteSheet::DrawSprite( Vector2f position, int spriteIdx )
{
	Rectf
		srcRect {
			spriteIdx * m_SpriteWidth,
			0,
			m_SpriteWidth,
			GetSpriteHeight()
			};

	m_SpriteSheet->Draw(position, srcRect);
}

const float SpriteSheet::GetSpriteHeight()
{
	return m_SpriteSheet->GetHeight();
}

const float SpriteSheet::GetSpriteWidth()
{
	return m_SpriteWidth;
}
