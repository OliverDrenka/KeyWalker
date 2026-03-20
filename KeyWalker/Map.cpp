#include "pch.h"
#include "Map.h"
#include <fstream>
#include <iostream>
#include "utils.h"
#include <algorithm>

Map::Map()
{
	m_Grid = new Grid(5, 5);
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
		tilePosition{ position.x - m_TileSize / 2, position.y - m_TileSize / 2 },
		letterPosition{ 
		position.x - (m_TileSize / 2 + 1) + (m_Letters->GetSpriteWidth() / 2 - 0.5f),
		position.y - (m_TileSize / 2) + (m_Letters->GetSpriteHeight()/2)
	};
	for (int rowIdx{}; rowIdx < numRows; ++rowIdx)
	{
		for (int colIdx{}; colIdx < numCols; ++colIdx)
		{
			m_TileTexture->Draw(tilePosition);
			m_Letters->DrawSprite(letterPosition, m_Grid->GetTile(rowIdx, colIdx));
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
	int tile = m_Grid->GetTile(std::min(position.x + 1, totalCols), position.y);
	if (position.x + 1 <= totalCols && tile == value)
	{
		return Vector2i(1, 0);
	} 
	tile = m_Grid->GetTile(position.x, std::min(position.y + 1, totalRows));
	if(position.y + 1 <= totalRows && tile == value)
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
				value{ static_cast<char>( rowIdx * numCols + colIdx) };
			m_Grid->SetTile(rowIdx, colIdx, value);
		}
	}
}

const float Map::GetWidth() const
{
	return m_TileSize * m_Grid->GetNumCols();
}

const float Map::GetHeight() const
{
	return m_TileSize * m_Grid->GetNumRows();
}

void Map::GenerateMapKeyboard()
{
}

void Map::GenerateMapRandom()
{
}
