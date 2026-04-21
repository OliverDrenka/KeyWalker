#include "pch.h"
#include "Map.h"
#include <fstream>
#include <iostream>
#include "utils.h"
#include <algorithm>
#include "utils.h"

Map::Map()
{
    m_Grid = new Grid(10, 6);
    m_Letters = new SpriteSheet(36, "Font.png");
    m_TileTexture = new Texture("Tile.png");
    m_TileSize = m_TileTexture->GetWidth();
    m_IsHexMode = false;
    GenerateMapRandom();
}

Map::~Map()
{
	delete m_Grid;
	delete m_Letters;
	delete m_TileTexture;
	m_PointTiles.clear();
}

void Map::SetHexMode(bool hex)
{
	m_IsHexMode = hex;
}

bool Map::IsHexMode() const
{
	return m_IsHexMode;
}

void Map::Draw( Vector2f position )
{
	const int
		numCols{ m_Grid->GetNumCols() },
		numRows{ m_Grid->GetNumRows() };
    // default positions for square layout
    Vector2f tilePosition{ position.x, position.y };
    Vector2f letterPosition{ position.x + (m_Letters->GetSpriteWidth() / 2.f) - 1.5f,
                             position.y + (m_Letters->GetSpriteHeight() / 2.f) };

    if (!m_IsHexMode)
    {
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
    else
    {
		const float xOffset = m_TileSize * 0.5f;

		for (int rowIdx{}; rowIdx < numRows; ++rowIdx)
		{
			for (int colIdx{}; colIdx < numCols; ++colIdx)
			{
				float x = position.x + colIdx * m_TileSize + ((rowIdx & 1) ? xOffset : 0.0f);
				float y = position.y + rowIdx * m_TileSize; // <-- IMPORTANT: no compression

				Vector2f tp{ x, y };
				Vector2f lp{ x + (m_Letters->GetSpriteWidth() / 2.f) - 1.5f,
							 y + (m_Letters->GetSpriteHeight() / 2.f) };

				m_TileTexture->Draw(tp);
				m_Letters->DrawSprite(lp, m_Grid->GetTile(colIdx, rowIdx));
			}
		}
    }

}

const float Map::GetTileSize() const
{
	return m_TileSize;
}

const Vector2i Map::GetAdjecentTileDirection( Vector2i position, int value )
{
    const int totalCols{ m_Grid->GetNumCols() };
    const int totalRows{ m_Grid->GetNumRows() };

    if (!m_IsHexMode)
    {
        int tile = m_Grid->GetTile(std::min(position.x + 1, totalCols - 1), position.y);
        if (position.x + 1 < totalCols && tile == value) return Vector2i(1, 0);
        tile = m_Grid->GetTile(position.x, std::min(position.y + 1, totalRows - 1));
        if (position.y + 1 < totalRows && tile == value) return Vector2i(0, 1);
        tile = m_Grid->GetTile(std::max(position.x - 1, 0), position.y);
        if (position.x - 1 >= 0 && tile == value) return Vector2i(-1, 0);
        tile = m_Grid->GetTile(position.x, std::max(position.y - 1, 0));
        if (position.y - 1 >= 0 && tile == value) return Vector2i(0, -1);
        return Vector2i(0, 0);
    }

    // Hex (odd-r horizontal layout) neighbors
    // neighbors: E, W, NE, NW, SE, SW depending on row parity
    const int col = position.x;
    const int row = position.y;

    // East
    if (col + 1 < totalCols && m_Grid->GetTile(col + 1, row) == value) return Vector2i(1, 0);
    // West
    if (col - 1 >= 0 && m_Grid->GetTile(col - 1, row) == value) return Vector2i(-1, 0);

    bool odd = (row & 1) != 0;
    // NE
    if (row - 1 >= 0)
    {
        int nc = odd ? col + 1 : col;
        if (nc >= 0 && nc < totalCols && m_Grid->GetTile(nc, row - 1) == value) return Vector2i(nc - col, -1);
    }
    // NW
    if (row - 1 >= 0)
    {
        int nc = odd ? col : col - 1;
        if (nc >= 0 && nc < totalCols && m_Grid->GetTile(nc, row - 1) == value) return Vector2i(nc - col, -1);
    }
    // SE
    if (row + 1 < totalRows)
    {
        int nc = odd ? col + 1 : col;
        if (nc >= 0 && nc < totalCols && m_Grid->GetTile(nc, row + 1) == value) return Vector2i(nc - col, 1);
    }
    // SW
    if (row + 1 < totalRows)
    {
        int nc = odd ? col : col - 1;
        if (nc >= 0 && nc < totalCols && m_Grid->GetTile(nc, row + 1) == value) return Vector2i(nc - col, 1);
    }

    return Vector2i(0, 0);
}

void Map::RandomizeTile(const Vector2i& position)
{
	int value{ rand() % 36 };
	const int cols{ m_Grid->GetNumCols() };
	const int rows{ m_Grid->GetNumRows() };

	// offsets depend on grid type
	auto IsValidForNeighbors = [&](int v) -> bool
		{
			if (!m_IsHexMode)
			{
				// orthogonal offsets
				const int dx[4] = { 1, -1, 0, 0 };
				const int dy[4] = { 0, 0, 1, -1 };

				// 1) immediate neighbors
				for (int i = 0; i < 4; ++i)
				{
					int nx = position.x + dx[i];
					int ny = position.y + dy[i];
					if (nx >= 0 && nx < cols && ny >= 0 && ny < rows)
					{
						if (m_Grid->GetTile(nx, ny) == v) return false;
					}
				}

				// 2) neighbors of neighbors
				for (int i = 0; i < 4; ++i)
				{
					int nx = position.x + dx[i];
					int ny = position.y + dy[i];
					if (nx < 0 || nx >= cols || ny < 0 || ny >= rows) continue;

					for (int j = 0; j < 4; ++j)
					{
						int nnx = nx + dx[j];
						int nny = ny + dy[j];

						if (nnx == position.x && nny == position.y) continue;

						if (nnx >= 0 && nnx < cols && nny >= 0 && nny < rows)
						{
							if (m_Grid->GetTile(nnx, nny) == v) return false;
						}
					}
				}

				return true;
			}

			const int col = position.x;
			const int row = position.y;

			auto IsInside = [&](int x, int y)
				{
					return (x >= 0 && x < cols && y >= 0 && y < rows);
				};

			auto Check = [&](int x, int y)
				{
					return IsInside(x, y) && m_Grid->GetTile(x, y) == v;
				};

			bool odd = (row & 1) != 0;

			//  Immediate neighbors (correct odd-r layout)
			int nx[6], ny[6];

			// E, W
			nx[0] = col + 1; ny[0] = row;
			nx[1] = col - 1; ny[1] = row;

			if (odd)
			{
				nx[2] = col + 1; ny[2] = row - 1; // NE
				nx[3] = col;     ny[3] = row - 1; // NW
				nx[4] = col + 1; ny[4] = row + 1; // SE
				nx[5] = col;     ny[5] = row + 1; // SW
			}
			else
			{
				nx[2] = col;     ny[2] = row - 1; // NE
				nx[3] = col - 1; ny[3] = row - 1; // NW
				nx[4] = col;     ny[4] = row + 1; // SE
				nx[5] = col - 1; ny[5] = row + 1; // SW
			}

			for (int i = 0; i < 6; ++i)
			{
				if (Check(nx[i], ny[i])) return false;
			}

			//  Radius-2 area (robust replacement for "neighbors of neighbors")
			for (int dy = -2; dy <= 2; ++dy)
			{
				for (int dx = -2; dx <= 2; ++dx)
				{
					if (dx == 0 && dy == 0) continue;

					int cx = col + dx;
					int cy = row + dy;

					if (!IsInside(cx, cy)) continue;

					// simple distance filter (prevents square corners being included)
					int dist = abs(dx) + abs(dy);
					if (dist > 3) continue;

					if (m_Grid->GetTile(cx, cy) == v) return false;
				}
			}

			return true;
		};

	int attempts = 0;
	while (!IsValidForNeighbors(value) && attempts < 36)
	{
		value = (value + 1) % 36;
		++attempts;
	}

	// Set the chosen value (if attempts exhausted, picks last candidate)
	m_Grid->SetTile(position.x, position.y, value);
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
			const int
				value{ (rowIdx * numCols + colIdx) % 36 };
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
	const int
		numCols{ m_Grid->GetNumCols() },
		numRows{ m_Grid->GetNumRows() };
	for (int rowIdx{}; rowIdx < numRows; ++rowIdx)
	{
		for (int colIdx{}; colIdx < numCols; ++colIdx)
		{
			const Vector2i
				position{ colIdx, rowIdx };
			RandomizeTile(position);
		}
	}
}
