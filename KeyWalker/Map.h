#pragma once
#include "Grid.h"
#include "SpriteSheet.h"
#include "Vector2i.h"

class Map
{
public:
	Map();
	~Map();

	void Draw( Vector2f position = Vector2f(0,0) );

	const float GetTileSize();
	const float GetWidth() const;
	const float GetHeight() const;
	const Vector2i GetAdjecentTileDirection( Vector2i position, int value );


	void GenerateMapOrdered();
	void GenerateMapKeyboard();
	void GenerateMapRandom();


private:
	Grid* m_Grid;
	SpriteSheet* m_Letters;
	Texture* m_TileTexture;
	float m_TileSize;

};

