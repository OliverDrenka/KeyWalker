#include "pch.h"
#include "Map.h"
#include <fstream>
#include <iostream>
#include "utils.h"
#include <algorithm>
#include <vector>
#include <deque>


Map::Map()
{
    m_Grid = new Grid(10, 6);
    m_Letters = new SpriteSheet(36, "Font.png", 4);
    m_TileTexture = new Texture("Tile.png");
    m_TileSize = m_TileTexture->GetWidth();
    m_IsHexMode = false;
    // initialize previous visibility mask
    m_PrevVisible.assign(m_Grid->GetNumCols() * m_Grid->GetNumRows(), 0);
}

Map::~Map()
{
    delete m_Grid;
    delete m_Letters;
    delete m_TileTexture;
}

void Map::SetHexMode(bool hex)
{
	m_IsHexMode = hex;
}

bool Map::IsHexMode() const
{
	return m_IsHexMode;
}

void Map::Draw( Vector2f position, const Vector2i* pPlayerPosition )
{
	const int
		numCols{ m_Grid->GetNumCols() },
		numRows{ m_Grid->GetNumRows() };
    // default positions for square layout
    Vector2f tilePosition{ position.x, position.y };
    Vector2f letterPosition{ position.x + (m_Letters->GetSpriteWidth() / 2.f) - 1.5f,
                             position.y + (m_Letters->GetSpriteHeight() / 2.f) };

    // Precompute visibility mask if player position given
    std::vector<char> visible;
    if (pPlayerPosition)
    {
        visible.assign(numCols * numRows, 0);
        // ensure previous-visible mask matches grid size
        if (m_PrevVisible.size() != visible.size())
            m_PrevVisible.assign(visible.size(), 0);
        const Vector2i pp = *pPlayerPosition;

        if (!m_IsHexMode)
        {
            // square: initial 5x5 box centered on player, but exclude the 4 far corners
            for (int dy = -2; dy <= 2; ++dy)
            {
                for (int dx = -2; dx <= 2; ++dx)
                {
                    // exclude the four corner tiles where both offsets are ±2
                    if (std::abs(dx) == 2 && std::abs(dy) == 2) continue;
                    int cx = pp.x + dx;
                    int cy = pp.y + dy;
                    if (cx >= 0 && cx < numCols && cy >= 0 && cy < numRows)
                    {
                        visible[cy * numCols + cx] = 1;
                    }
                }
            }

            // Expand visibility for any visible 'vision' tiles.
            // Use a queue so vision tiles can cascade (vision tiles revealed by other vision tiles also expand).
            std::deque<Vector2i> q;
            for (int ry = 0; ry < numRows; ++ry)
            {
                for (int rx = 0; rx < numCols; ++rx)
                {
                    if (visible[ry * numCols + rx] && m_Grid->GetTileState(rx, ry) == Tile::State::vision)
                        q.emplace_back(rx, ry);
                }
            }

            while (!q.empty())
            {
                Vector2i v = q.front(); q.pop_front();
                for (int dy = -2; dy <= 2; ++dy)
                {
                    for (int dx = -2; dx <= 2; ++dx)
                    {
                        int cx = v.x + dx;
                        int cy = v.y + dy;
                        if (cx >= 0 && cx < numCols && cy >= 0 && cy < numRows)
                        {
                            if (!visible[cy * numCols + cx])
                            {
                                visible[cy * numCols + cx] = 1;
                                if (m_Grid->GetTileState(cx, cy) == Tile::State::vision)
                                    q.emplace_back(cx, cy);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            // hex: BFS to depth 2 on odd-r layout from the player
            std::deque<std::pair<Vector2i,int>> q0;
            q0.emplace_back(pp, 0);
            auto try_mark = [&](int x, int y)
            {
                if (x >= 0 && x < numCols && y >= 0 && y < numRows && !visible[y * numCols + x])
                {
                    visible[y * numCols + x] = 1;
                    return true;
                }
                return false;
            };

            try_mark(pp.x, pp.y);
            while (!q0.empty())
            {
                auto cur = q0.front(); q0.pop_front();
                Vector2i pos = cur.first;
                int depth = cur.second;
                if (depth >= 2) continue;
                int col = pos.x;
                int row = pos.y;
                bool odd = (row & 1) != 0;
                int nx[6], ny[6];
                nx[0] = col + 1; ny[0] = row; // E
                nx[1] = col - 1; ny[1] = row; // W
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
                    int cx = nx[i];
                    int cy = ny[i];
                    if (cx >= 0 && cx < numCols && cy >= 0 && cy < numRows)
                    {
                        if (try_mark(cx, cy))
                            q0.emplace_back(Vector2i(cx, cy), depth + 1);
                    }
                }
            }

            // Now expand from any 'vision' tiles that are within the player's initial depth-2 visibility.
            // Do NOT expand from vision tiles that were revealed only by this expansion (no cascading).
            std::vector<Vector2i> visionSeeds;
            for (int ry = 0; ry < numRows; ++ry)
            {
                for (int rx = 0; rx < numCols; ++rx)
                {
                    if (visible[ry * numCols + rx] && m_Grid->GetTileState(rx, ry) == Tile::State::vision)
                        visionSeeds.emplace_back(rx, ry);
                }
            }

            for (const Vector2i& seed : visionSeeds)
            {
                // local visited ensures we explore full depth-2 around the seed even if
                // some tiles are already globally visible from the player's BFS.
                std::vector<char> visited(numCols * numRows, 0);
                std::deque<std::pair<Vector2i,int>> qv;
                qv.emplace_back(seed, 0);
                visited[seed.y * numCols + seed.x] = 1;
                // seed is already marked visible by earlier code
                while (!qv.empty())
                {
                    auto cur = qv.front(); qv.pop_front();
                    Vector2i pos = cur.first;
                    int depth = cur.second;
                    if (depth >= 2) continue;
                    int col = pos.x;
                    int row = pos.y;
                    bool odd = (row & 1) != 0;
                    int nx[6], ny[6];
                    nx[0] = col + 1; ny[0] = row; // E
                    nx[1] = col - 1; ny[1] = row; // W
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
                        int cx = nx[i];
                        int cy = ny[i];
                        if (cx >= 0 && cx < numCols && cy >= 0 && cy < numRows)
                        {
                            int idx = cy * numCols + cx;
                            if (visited[idx]) continue;
                            visited[idx] = 1;
                            // mark globally visible
                            if (!visible[idx]) visible[idx] = 1;
                            // enqueue neighbor for further expansion only if it's not a vision tile
                            if (m_Grid->GetTileState(cx, cy) != Tile::State::vision)
                                qv.emplace_back(Vector2i(cx, cy), depth + 1);
                        }
                    }
                }
            }
        }
    }

    // draw tiles; if visible mask exists, only draw letters for visible tiles
    if (!m_IsHexMode)
    {
        for (int rowIdx{}; rowIdx < numRows; ++rowIdx)
        {
            for (int colIdx{}; colIdx < numCols; ++colIdx)
            {
                m_TileTexture->Draw(tilePosition);
                bool showLetter = true;
                if (pPlayerPosition) showLetter = (visible[rowIdx * numCols + colIdx] != 0);
                // newly visible? randomize tile
                if (pPlayerPosition && visible[rowIdx * numCols + colIdx] && !m_PrevVisible[rowIdx * numCols + colIdx])
                {
                    RandomizeTile(Vector2i(colIdx, rowIdx));
                }
                if (showLetter)
                {
                    m_Letters->DrawSprite(letterPosition, m_Grid->GetTileValue(colIdx, rowIdx), static_cast<int>(m_Grid->GetTileState(colIdx, rowIdx)));
                }
                else if (m_Grid->GetTileState(colIdx, rowIdx) == Tile::State::point)
                {
                    // draw a small golden marker at tile center to indicate point location
                    Vector2f center = tilePosition + Vector2f(m_TileSize * 0.5f, m_TileSize * 0.5f);
                    const float radius = m_TileSize * 0.25f;
                    utils::SetColor(Color4f(1.0f, 0.84f, 0.0f, 1.0f));
                    utils::FillEllipse(center, radius, radius);
                    utils::SetColor(Color4f(1.f, 1.f, 1.f, 1.f));
                }
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
                bool showLetter = true;
                if (pPlayerPosition) showLetter = (visible[rowIdx * numCols + colIdx] != 0);
                // newly visible? randomize tile
                if (pPlayerPosition && visible[rowIdx * numCols + colIdx] && !m_PrevVisible[rowIdx * numCols + colIdx])
                {
                    RandomizeTile(Vector2i(colIdx, rowIdx));
                }
                if (showLetter)
                {
                    m_Letters->DrawSprite(lp,
                        m_Grid->GetTileValue(colIdx, rowIdx),
                        static_cast<int>(m_Grid->GetTileState(colIdx, rowIdx)));
                }
                else if (m_Grid->GetTileState(colIdx, rowIdx) == Tile::State::point)
                {
                    // hex tile: draw golden marker at tile center
                    Vector2f center = tp + Vector2f(m_TileSize * 0.5f, m_TileSize * 0.5f);
                    const float radius = m_TileSize * 0.25f;
                    utils::SetColor(Color4f(1.0f, 0.84f, 0.0f, 1.0f));
                    utils::FillEllipse(center, radius, radius);
                    utils::SetColor(Color4f(1.f, 1.f, 1.f, 1.f));
                }
            }
        }

    }

    // update previous visibility mask for both square and hex modes
    if (pPlayerPosition)
    {
        if (m_PrevVisible.size() != visible.size()) m_PrevVisible.assign(visible.size(), 0);
        m_PrevVisible = visible;
    }
    else
    {
        // No player -> clear previous visibility to avoid stale "newly visible" detections
        if (m_PrevVisible.size() != static_cast<size_t>(numCols * numRows))
            m_PrevVisible.assign(numCols * numRows, 0);
        else
            std::fill(m_PrevVisible.begin(), m_PrevVisible.end(), 0);
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
        int tile = m_Grid->GetTileValue(std::min(position.x + 1, totalCols - 1), position.y);
        if (position.x + 1 < totalCols && tile == value) return Vector2i(1, 0);
        tile = m_Grid->GetTileValue(position.x, std::min(position.y + 1, totalRows - 1));
        if (position.y + 1 < totalRows && tile == value) return Vector2i(0, 1);
        tile = m_Grid->GetTileValue(std::max(position.x - 1, 0), position.y);
        if (position.x - 1 >= 0 && tile == value) return Vector2i(-1, 0);
        tile = m_Grid->GetTileValue(position.x, std::max(position.y - 1, 0));
        if (position.y - 1 >= 0 && tile == value) return Vector2i(0, -1);
        return Vector2i(0, 0);
    }

    // Hex (odd-r horizontal layout) neighbors
    // neighbors: E, W, NE, NW, SE, SW depending on row parity
    const int col = position.x;
    const int row = position.y;

    // East
    if (col + 1 < totalCols && m_Grid->GetTileValue(col + 1, row) == value) return Vector2i(1, 0);
    // West
    if (col - 1 >= 0 && m_Grid->GetTileValue(col - 1, row) == value) return Vector2i(-1, 0);

    bool odd = (row & 1) != 0;
    // NE
    if (row - 1 >= 0)
    {
        int nc = odd ? col + 1 : col;
        if (nc >= 0 && nc < totalCols && m_Grid->GetTileValue(nc, row - 1) == value) return Vector2i(nc - col, -1);
    }
    // NW
    if (row - 1 >= 0)
    {
        int nc = odd ? col : col - 1;
        if (nc >= 0 && nc < totalCols && m_Grid->GetTileValue(nc, row - 1) == value) return Vector2i(nc - col, -1);
    }
    // SE
    if (row + 1 < totalRows)
    {
        int nc = odd ? col + 1 : col;
        if (nc >= 0 && nc < totalCols && m_Grid->GetTileValue(nc, row + 1) == value) return Vector2i(nc - col, 1);
    }
    // SW
    if (row + 1 < totalRows)
    {
        int nc = odd ? col : col - 1;
        if (nc >= 0 && nc < totalCols && m_Grid->GetTileValue(nc, row + 1) == value) return Vector2i(nc - col, 1);
    }

    return Vector2i(0, 0);
}

void Map::CreateRandomPointTile()
{
	int
		x {rand() % m_Grid->GetNumCols()},
		y {rand() % m_Grid->GetNumRows()};
	if (m_Grid->GetTileState(x, y) == Tile::State::normal)
	{
		m_Grid->SetTileState(x, y, Tile::State::point);
	} 
	else
	{
		CreateRandomPointTile();
	}

}

void Map::RemoveTileModifier(const Vector2i position)
{
	m_Grid->SetTileState(position.x, position.y, Tile::State::normal);
}

const Tile::State Map::GetTileState(Vector2i position) const
{
	return m_Grid->GetTileState(position.x, position.y);
}

void Map::RandomizeTile(const Vector2i& position)
{
	const int maxValue{ 36 };
	int value{ rand() % maxValue };
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
						if (m_Grid->GetTileValue(nx, ny) == v) return false;
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
							if (m_Grid->GetTileValue(nnx, nny) == v) return false;
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
					return IsInside(x, y) && m_Grid->GetTileValue(x, y) == v;
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

					if (m_Grid->GetTileValue(cx, cy) == v) return false;
				}
			}

			return true;
		};

    // Build list of all valid candidates and pick one uniformly at random.
    std::vector<int> candidates;
    candidates.reserve(maxValue);
    for (int v = 0; v < maxValue; ++v)
    {
        if (IsValidForNeighbors(v)) candidates.push_back(v);
    }

    if (!candidates.empty())
    {
        int pick = candidates[rand() % static_cast<int>(candidates.size())];
        m_Grid->SetTile(position.x, position.y, pick);
    }
    else
    {
        // Fallback to original incremental scan if no candidate found (should be rare)
        int attempts = 0;
        while (!IsValidForNeighbors(value) && attempts < maxValue)
        {
            value = (value + 1) % maxValue;
            ++attempts;
        }
        m_Grid->SetTile(position.x, position.y, value);
    }
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

    // Place vision tiles at: 3rd and 8th tile in the 2nd and 5th row
    // (using 0-based indices: columns 2 and 7; rows 1 and 4)
    if (m_Grid->GetNumCols() > 7 && m_Grid->GetNumRows() > 4)
    {
        m_Grid->SetTileState(2, 1, Tile::State::vision);
        m_Grid->SetTileState(7, 1, Tile::State::vision);
        m_Grid->SetTileState(2, 4, Tile::State::vision);
        m_Grid->SetTileState(7, 4, Tile::State::vision);
    }
}
