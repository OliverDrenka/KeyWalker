#include "pch.h"
#include "Map.h"
#include <fstream>
#include <iostream>
#include "utils.h"
#include <algorithm>

Map::Map()
	:m_TileSize( 12 )
{
	m_Grid = new Grid(5, 5);
	m_Letters = new SpriteSheet(35, "Font.png");
	GenerateMapOrdered();
}

Map::~Map()
{
	delete m_Grid;
	delete m_Letters;
}

void Map::Draw( Vector2f position )
{
	const int
		numCols{ m_Grid->GetNumCols() },
		numRows{ m_Grid->GetNumRows() };
	Vector2f
		tilePosition{ position.x, position.y },
		letterPosition{ position.x + 1, position.y + 2 };
	for (int rowIdx{}; rowIdx < numRows; ++rowIdx)
	{
		for (int colIdx{}; colIdx < numCols; ++colIdx)
		{
			utils::SetColor(Color4f(1.f, 1.f, 1.f, 1.f));
			utils::FillRect(tilePosition, m_TileSize, m_TileSize);
			utils::SetColor(Color4f(0.f, 0.f, 0.f, 0.5f));
			utils::DrawRect(tilePosition, m_TileSize, m_TileSize);

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

void Map::GenerateMapKeyboard()
{
}

void Map::GenerateMapRandom()
{
}
