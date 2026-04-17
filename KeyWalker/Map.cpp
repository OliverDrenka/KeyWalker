#include "pch.h"
#include "Map.h"
#include <fstream>
#include <iostream>
#include "utils.h"
#include <algorithm>
#include "utils.h"

Map::Map()
{
	m_Grid = new Grid(12, 8);
	m_Letters = new SpriteSheet(35, "Font.png");
	m_TileTexture = new Texture("Tile.png");
	m_TileSize = m_TileTexture->GetWidth();
	GenerateMapOrdered();
}

Map::~Map()
{
	delete m_Grid;
	delete m_Letters;
	delete m_TileTexture;
}

void Map::Draw( Vector2f position )
{
	const int
		numCols{ m_Grid->GetNumCols() },
		numRows{ m_Grid->GetNumRows() };

	Vector2f
	//	- m_TileSize / 2
		tilePosition{ position.x, position.y },
		letterPosition{ 
		position.x + (m_Letters->GetSpriteWidth() / 2.f) -1.5f,
		position.y + (m_Letters->GetSpriteHeight() / 2.f)
	};

	for (int rowIdx{}; rowIdx < numRows; ++rowIdx)
	{
		for (int colIdx{}; colIdx < numCols; ++colIdx)
		{
			m_TileTexture->Draw(tilePosition);
			m_Letters->DrawSprite(letterPosition, m_Grid->GetTile(colIdx, rowIdx));
			tilePosition.x += m_TileSize;
			letterPosition.x += m_TileSize;
		}
		tilePosition.y += m_TileSize;
		tilePosition.x -= m_TileSize * numCols;
		letterPosition.y += m_TileSize;
		letterPosition.x -= m_TileSize * numCols;

	}

}

const float Map::GetTileSize()
{
	return m_TileSize;
}

const Vector2i Map::GetAdjecentTileDirection( Vector2i position, int value )
{
	const int
		totalCols{ m_Grid->GetNumCols() },
		totalRows{ m_Grid->GetNumRows() };
	int tile = m_Grid->GetTile(std::min(position.x + 1, totalCols - 1 ), position.y);
	if (position.x + 1 < totalCols && tile == value)
	{
		return Vector2i(1, 0);
	} 
	tile = m_Grid->GetTile(position.x, std::min(position.y + 1, totalRows - 1));
	if(position.y + 1 < totalRows && tile == value)
	{
		return Vector2i(0, 1);
	}
	tile = m_Grid->GetTile(std::max(position.x - 1, 0), position.y);
	if(position.x - 1 >= 0 && tile == value)
	{
		return Vector2i(-1, 0);
	}
	tile = m_Grid->GetTile(position.x, std::max(position.y - 1, 0));
	if(position.y - 1 >= 0 && tile == value)
	{
		return Vector2i(0, -1);
	}
	return Vector2i(0, 0);
}

void Map::GenerateMapOrdered()
{
	const int
		numCols { m_Grid->GetNumCols() },
		numRows { m_Grid->GetNumRows() };
	for (int rowIdx{}; rowIdx < numRows; ++rowIdx)
	{
		for (int colIdx{}; colIdx < numCols; ++colIdx)
		{
			const char
				value{ static_cast<char>( (rowIdx * numCols + colIdx) %35) };
			m_Grid->SetTile(colIdx, rowIdx, value);
		}
	}
}

const float Map::GetWidth() const
{
	const int numCols { m_Grid->GetNumCols() };
	return m_TileSize * m_Grid->GetNumCols();
}

const float Map::GetHeight() const
{
	const int numRows{ m_Grid->GetNumRows() };
	return m_TileSize * m_Grid->GetNumRows();
}

void Map::GenerateMapKeyboard()
{
}

void Map::GenerateMapRandom()
{
}
