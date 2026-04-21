#pragma once
#include "Grid.h"
#include "SpriteSheet.h"
#include "Vector2i.h"
#include <vector>

class Map
{
public:
    Map();
    ~Map();

    // Switch between square (false) and hex (true) layouts at runtime
    void SetHexMode(bool hex);
    bool IsHexMode() const;

	void Draw( Vector2f position = Vector2f(0,0) );

	const float GetTileSize() const;
	const float GetWidth() const;
	const float GetHeight() const;
	const Vector2i GetAdjecentTileDirection( Vector2i position, int value );
	
	void RandomizeTile(const Vector2i& position);

	void GenerateMapOrdered();
	void GenerateMapKeyboard();
	void GenerateMapRandom();


private:
    Grid* m_Grid;
	SpriteSheet* m_Letters;
	Texture* m_TileTexture;
	float m_TileSize;

    bool m_IsHexMode;

    std::vector<Vector2i> m_PointTiles;

};

