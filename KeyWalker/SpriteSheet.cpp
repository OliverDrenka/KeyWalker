#include "pch.h"
#include "SpriteSheet.h"

SpriteSheet::SpriteSheet()
	:SpriteSheet( 1, "" )
{
}

SpriteSheet::SpriteSheet(const int totalSprites, std::string spriteSheetPath, const int numRows )
	:m_TotalSprites{ totalSprites }
	, m_Rows{ numRows }
{
	m_SpriteSheet = new Texture( spriteSheetPath );
	m_SpriteWidth = m_SpriteSheet->GetWidth() / m_TotalSprites;
	m_SpriteHeight = m_SpriteSheet->GetHeight() / m_Rows;

}

SpriteSheet::~SpriteSheet()
{
	delete m_SpriteSheet;
}

void SpriteSheet::DrawSprite( Vector2f position, int spriteIdx, int row )
{
	Rectf
		srcRect {
			spriteIdx * m_SpriteWidth,
			row * m_SpriteHeight,
			m_SpriteWidth,
			GetSpriteHeight()
			};

	m_SpriteSheet->Draw(position, srcRect);
}

const float SpriteSheet::GetSpriteHeight()
{
	return m_SpriteHeight;
}

const float SpriteSheet::GetSpriteWidth()
{
	return m_SpriteWidth;
}
