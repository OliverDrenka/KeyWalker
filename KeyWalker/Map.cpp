#include "pch.h"
#include "Map.h"
#include <fstream>
#include <iostream>
#include "utils.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <deque>


Map::Map() 
	: m_MaxValue{25}
	, m_MinValue{ 0 }
{
    m_Letters = new SpriteSheet(36, "Font.png", 5);
    m_TileTexture = new Texture("Tile.png");
	m_TileSize = 16;
    m_IsHexMode = false;
    m_IsWrapped = false;
    // initialize previous visibility mask 
	m_Scale = 16.f / m_TileSize;
    m_Grid = new Grid(10 * m_Scale, 6 * m_Scale);
    m_PrevVisible.assign(m_Grid->GetNumCols() * m_Grid->GetNumRows(), 0);
}

// Draw the letter for a single tile at grid coordinates 'position' (col,row).
void Map::DrawLetter(const Vector2i position, const int colorOffest)
{
    const int cols = m_Grid->GetNumCols();
    const int rows = m_Grid->GetNumRows();
    if (position.x < 0 || position.x >= cols || position.y < 0 || position.y >= rows) return;

    // compute scaled letter drawing metrics (match Draw())
    const float baseTile = 16.f;
    const float letterScale = m_TileSize / baseTile;
    const float letterW = 16;
    const float letterH = 16;
    const float letterDestW = letterW * letterScale;
    const float letterDestH = letterH * letterScale;

    // compute tile top-left in world coordinates for square layout
    if (!m_IsHexMode)
    {
        Vector2f tilePos{ m_DrawOrigin.x + position.x * m_TileSize, m_DrawOrigin.y + position.y * m_TileSize };
		Vector2f letterPos = tilePos;
        m_Letters->DrawSprite(letterPos,
            m_Grid->GetTileValue(position.x, position.y),
            static_cast<int>(m_Grid->GetTileState(position.x, position.y)) + colorOffest,
            letterDestW, letterDestH);
    }
    else
    {
        // hex layout (odd-r horizontal) positioning
        const float xOffset = m_TileSize * 0.5f;
        float x = m_DrawOrigin.x + position.x * m_TileSize + ((position.y & 1) ? xOffset : 0.0f);
        float y = m_DrawOrigin.y + position.y * m_TileSize; // no compression
        Vector2f lp{ x ,
                     y  };
        m_Letters->DrawSprite(lp,
            m_Grid->GetTileValue(position.x, position.y),
            static_cast<int>(m_Grid->GetTileState(position.x, position.y)),
            letterDestW, letterDestH);
    }
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

void Map::SetWrapMode(bool wrap)
{
    m_IsWrapped = wrap;
}

bool Map::IsWrapMode() const
{
    return m_IsWrapped;
}

bool Map::IsHexMode() const
{
	return m_IsHexMode;
}

void Map::Draw( Vector2f position, const Vector2i* pPlayerPosition )
{
    // remember origin for DrawLetter
    m_DrawOrigin = position;

	const int
		numCols{ m_Grid->GetNumCols() },
		numRows{ m_Grid->GetNumRows() };
    // default positions for square layout
    Vector2f tilePosition{ position.x, position.y };
    // scale letters so their base 10px size scales with tile (base tile = 16px)
    const float baseTile = 16.f;
    const float letterScale = m_TileSize / baseTile;
    const float letterW = m_Letters->GetSpriteWidth();
    const float letterH = m_Letters->GetSpriteHeight();
    const float letterDestW = letterW * letterScale;
    const float letterDestH = letterH * letterScale;
    // keep the existing offsets but scale them so the relative placement remains
    Vector2f letterPosition{ position.x + ((letterW / 2.f) - 1.5f) * letterScale,
                             position.y + (letterH / 2.f) * letterScale };

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
            // Wrap coordinates so visibility loops around the map edges (if enabled)
            auto wrapX = [&](int x) {
                if (!m_IsWrapped) return x;
                int r = x % numCols; if (r < 0) r += numCols; return r; };
            auto wrapY = [&](int y) {
                if (!m_IsWrapped) return y;
                int r = y % numRows; if (r < 0) r += numRows; return r; };

            for (int dy = -2; dy <= 2; ++dy)
            {
                for (int dx = -2; dx <= 2; ++dx)
                {
                    // exclude the four corner tiles where both offsets are ±2
                    if (std::abs(dx) == 2 && std::abs(dy) == 2) continue;
                    int cx = wrapX(pp.x + dx);
                    int cy = wrapY(pp.y + dy);
                    // when wrapping is disabled wrapX/wrapY return raw coords;
                    // skip out-of-bounds indices in that case to avoid invalid access
                    if (cx < 0 || cx >= numCols || cy < 0 || cy >= numRows) continue;
                    visible[cy * numCols + cx] = 1;
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
                        int tx = v.x + dx;
                        int ty = v.y + dy;
                        int cx = wrapX(tx);
                        int cy = wrapY(ty);
                        if (cx < 0 || cx >= numCols || cy < 0 || cy >= numRows) continue; // out of bounds when wrapping disabled
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
        else
        {
            // hex: BFS to depth 2 on odd-r layout from the player
            std::deque<std::pair<Vector2i,int>> q0;
            q0.emplace_back(pp, 0);

            auto wrapX = [&](int x) { if (!m_IsWrapped) return x; int r = x % numCols; if (r < 0) r += numCols; return r; };
            auto wrapY = [&](int y) { if (!m_IsWrapped) return y; int r = y % numRows; if (r < 0) r += numRows; return r; };
            auto try_mark = [&](int x, int y)
            {
                int wx = wrapX(x);
                int wy = wrapY(y);
                if (wx < 0 || wx >= numCols || wy < 0 || wy >= numRows) return false;
                if (!visible[wy * numCols + wx])
                {
                    visible[wy * numCols + wx] = 1;
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
                    int tx = nx[i];
                    int ty = ny[i];
                    int cx = wrapX(tx);
                    int cy = wrapY(ty);
                    if (cx < 0 || cx >= numCols || cy < 0 || cy >= numRows) continue;
                    if (try_mark(cx, cy))
                        q0.emplace_back(Vector2i(cx, cy), depth + 1);
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
                int sx = wrapX(seed.x);
                int sy = wrapY(seed.y);
                if (sx < 0 || sx >= numCols || sy < 0 || sy >= numRows) continue;
                qv.emplace_back(Vector2i(sx, sy), 0);
                visited[sy * numCols + sx] = 1;
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
                    int tx = nx[i];
                    int ty = ny[i];
                    int cx = wrapX(tx);
                    int cy = wrapY(ty);
                    if (cx < 0 || cx >= numCols || cy < 0 || cy >= numRows) continue;
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

    // draw tiles; if visible mask exists, only draw letters for visible tiles
    if (!m_IsHexMode)
    {
        for (int rowIdx{}; rowIdx < numRows; ++rowIdx)
        {
            for (int colIdx{}; colIdx < numCols; ++colIdx)
            {
                // draw tile texture scaled to the current tile size
                m_TileTexture->Draw(Rectf{ tilePosition.x, tilePosition.y, m_TileSize, m_TileSize }, Rectf{0,0,34,34});

                bool showLetter = true;
                if (pPlayerPosition) showLetter = (visible[rowIdx * numCols + colIdx] != 0);
                // newly visible? randomize tile
                if (pPlayerPosition && visible[rowIdx * numCols + colIdx] && !m_PrevVisible[rowIdx * numCols + colIdx])
                {
                    RandomizeTile(Vector2i(colIdx, rowIdx));
                }
                if (showLetter)
                {
                    // draw letters for this tile
                    DrawLetter(Vector2i(colIdx, rowIdx));
                }
				else
				{
					Vector2f center = tilePosition + Vector2f(m_TileSize * 0.5f, m_TileSize * 0.5f);
					const float radius = m_TileSize * 0.25f;
					utils::SetColor(Color4f(1.0f, 1.f, 1.0f, 0.0f));
					switch (m_Grid->GetTileState(colIdx, rowIdx))
					{
						case(Tile::State::point):
						{
							utils::SetColor(Color4f(1.0f, 0.84f, 0.0f, 1.0f));
							break;
						}
						case(Tile::State::danger):
						{
							utils::SetColor(Color4f(1.0f, 0.0f, 0.0f, 1.0f));
							break;
						}
						case(Tile::State::preparing):
						{
							utils::SetColor(Color4f(142.f / 255.f, 91.f / 255.f, 91.f / 255.f, 1.0f));
							break;
						}
						case(Tile::State::normal):
						{
							utils::SetColor(Color4f(0.0f, 0.0f, 0.0f, 0.0f));
							break;
						}
						case(Tile::State::vision):
						{
							utils::SetColor(Color4f(10.f/255.f, 222.f/255.f, 241.f/255.f, 1.0f));
							break;
						}
					}
					utils::FillEllipse(center, radius, radius);
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
                Vector2f lp{ x + ((16 / 2.f) - 1.5f) * letterScale,
                             y + (16 / 2.f) * letterScale };

                // draw tile texture scaled to tile size
                m_TileTexture->Draw(Rectf{ tp.x, tp.y, m_TileSize, m_TileSize }, Rectf{ 0,0,34,34 });
                bool showLetter = true;
                if (pPlayerPosition) showLetter = (visible[rowIdx * numCols + colIdx] != 0);
                // newly visible? randomize tile
                if (pPlayerPosition && visible[rowIdx * numCols + colIdx] && !m_PrevVisible[rowIdx * numCols + colIdx])
                {
                    RandomizeTile(Vector2i(colIdx, rowIdx));
                }
                if (showLetter)
                {
                    DrawLetter(Vector2i(colIdx, rowIdx));
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
    auto wrapX = [&](int x) { if (!m_IsWrapped) return x; int r = x % totalCols; if (r < 0) r += totalCols; return r; };
    auto wrapY = [&](int y) { if (!m_IsWrapped) return y; int r = y % totalRows; if (r < 0) r += totalRows; return r; };

    if (!m_IsHexMode)
    {
        // Check east
        {
            int nx = wrapX(position.x + 1);
            int ny = wrapY(position.y);
            if (!m_IsWrapped && (nx < 0 || nx >= totalCols || ny < 0 || ny >= totalRows)) ;
            else if (m_Grid->GetTileValue(nx, ny) == value) return Vector2i(1, 0);
        }
        // Check south
        {
            int nx = wrapX(position.x);
            int ny = wrapY(position.y + 1);
            if (!m_IsWrapped && (nx < 0 || nx >= totalCols || ny < 0 || ny >= totalRows)) ;
            else if (m_Grid->GetTileValue(nx, ny) == value) return Vector2i(0, 1);
        }
        // Check west
        {
            int nx = wrapX(position.x - 1);
            int ny = wrapY(position.y);
            if (!m_IsWrapped && (nx < 0 || nx >= totalCols || ny < 0 || ny >= totalRows)) ;
            else if (m_Grid->GetTileValue(nx, ny) == value) return Vector2i(-1, 0);
        }
        // Check north
        {
            int nx = wrapX(position.x);
            int ny = wrapY(position.y - 1);
            if (!m_IsWrapped && (nx < 0 || nx >= totalCols || ny < 0 || ny >= totalRows)) ;
            else if (m_Grid->GetTileValue(nx, ny) == value) return Vector2i(0, -1);
        }
        return Vector2i(0, 0);
    }

    // Hex (odd-r horizontal layout) neighbors with wrapping
    const int col = position.x;
    const int row = position.y;
    bool odd = (row & 1) != 0;

    // East
    {
        int nc = wrapX(col + 1);
        int nr = wrapY(row);
        if (!m_IsWrapped && (nc < 0 || nc >= totalCols || nr < 0 || nr >= totalRows)) ;
        else if (m_Grid->GetTileValue(nc, nr) == value) return Vector2i(1, 0);
    }
    // West
    {
        int nc = wrapX(col - 1);
        int nr = wrapY(row);
        if (!m_IsWrapped && (nc < 0 || nc >= totalCols || nr < 0 || nr >= totalRows)) ;
        else if (m_Grid->GetTileValue(nc, nr) == value) return Vector2i(-1, 0);
    }

    // NE
    {
        int nc = odd ? col + 1 : col;
        int nr = row - 1;
        nc = wrapX(nc);
        nr = wrapY(nr);
        if (m_IsWrapped || (nc >= 0 && nc < totalCols && nr >= 0 && nr < totalRows))
            if (m_Grid->GetTileValue(nc, nr) == value) return odd ? Vector2i(1, -1) : Vector2i(0, -1);
    }
    // NW
    {
        int nc = odd ? col : col - 1;
        int nr = row - 1;
        nc = wrapX(nc);
        nr = wrapY(nr);
        if (m_IsWrapped || (nc >= 0 && nc < totalCols && nr >= 0 && nr < totalRows))
            if (m_Grid->GetTileValue(nc, nr) == value) return odd ? Vector2i(0, -1) : Vector2i(-1, -1);
    }
    // SE
    {
        int nc = odd ? col + 1 : col;
        int nr = row + 1;
        nc = wrapX(nc);
        nr = wrapY(nr);
        if (m_IsWrapped || (nc >= 0 && nc < totalCols && nr >= 0 && nr < totalRows))
            if (m_Grid->GetTileValue(nc, nr) == value) return odd ? Vector2i(1, 1) : Vector2i(0, 1);
    }
    // SW
    {
        int nc = odd ? col : col - 1;
        int nr = row + 1;
        nc = wrapX(nc);
        nr = wrapY(nr);
        if (m_IsWrapped || (nc >= 0 && nc < totalCols && nr >= 0 && nr < totalRows))
            if (m_Grid->GetTileValue(nc, nr) == value) return odd ? Vector2i(0, 1) : Vector2i(-1, 1);
    }

    return Vector2i(0, 0);
}

const Vector2i Map::CreateRandomPointTile(const Vector2i playerpos)
{
	const int cols = m_Grid->GetNumCols();
	const int rows = m_Grid->GetNumRows();

	// square distance (Chebyshev) - number of king moves between tiles
    auto squareDist = [&](const Vector2i& a, const Vector2i& b) -> int
    {
        int dx = std::abs(a.x - b.x);
        int dy = std::abs(a.y - b.y);
        // wrap distances (toroidal)
        dx = std::min(dx, cols - dx);
        dy = std::min(dy, rows - dy);
        return std::max(dx, dy);
    };

	// hex distance for odd-r offset coordinates:
	// convert odd-r (col,row) to cube coords then compute cube distance
    auto hexDist = [&](const Vector2i& a, const Vector2i& b) -> int
    {
        auto oddr_to_cube = [](int col, int row, int& cx, int& cy, int& cz)
        {
            int q = col - (row - (row & 1)) / 2;
            int r = row;
            cx = q;
            cz = r;
            cy = -cx - cz;
        };

        int best = std::numeric_limits<int>::max();
        // consider wrapping by shifting b by multiples of cols/rows to find shortest toroidal hex distance
        for (int sx = -1; sx <= 1; ++sx)
        {
            for (int sy = -1; sy <= 1; ++sy)
            {
                Vector2i bshift(b.x + sx * cols, b.y + sy * rows);
                int ax, ay, az, bx, by, bz;
                oddr_to_cube(a.x, a.y, ax, ay, az);
                oddr_to_cube(bshift.x, bshift.y, bx, by, bz);
                int d = (std::abs(ax - bx) + std::abs(ay - by) + std::abs(az - bz)) / 2;
                if (d < best) best = d;
            }
        }
        return best == std::numeric_limits<int>::max() ? 0 : best;
    };

	const int requiredDistance = 2;
	int attempts = 0;
	const int maxAttempts = 200;

	// Try random picks first (bounded attempts)
	while (attempts++ < maxAttempts)
	{
		int x = rand() % cols;
		int y = rand() % rows;
		if (m_Grid->GetTileState(x, y) != Tile::State::normal) continue;

		Vector2i cand(x, y);
		int d = m_IsHexMode ? hexDist(playerpos, cand) : squareDist(playerpos, cand);
		if (d >= requiredDistance)
		{
			m_Grid->SetTileState(x, y, Tile::State::point);
			return Vector2i(x, y);
		}
	}

	// Fallback: scan for any normal tile satisfying distance constraint
	for (int ry = 0; ry < rows; ++ry)
	{
		for (int rx = 0; rx < cols; ++rx)
		{
			if (m_Grid->GetTileState(rx, ry) != Tile::State::normal) continue;
			Vector2i cand(rx, ry);
			int d = m_IsHexMode ? hexDist(playerpos, cand) : squareDist(playerpos, cand);
			if (d >= requiredDistance)
			{
				m_Grid->SetTileState(rx, ry, Tile::State::point);
				return Vector2i(rx, ry);
			}
		}
	}

    // Last-resort: pick any normal tile (no tile meets distance requirement)
    for (int ry = 0; ry < rows; ++ry)
    {
        for (int rx = 0; rx < cols; ++rx)
        {
            if (m_Grid->GetTileState(rx, ry) == Tile::State::normal)
            {
                m_Grid->SetTileState(rx, ry, Tile::State::point);
                return Vector2i(rx, ry);
            }
        }
    }

    // No normal tiles exist; return player position as fallback
    return playerpos;
}

void Map::SetTileState(const Vector2i playerpos, const Tile::State state)
{
	m_Grid->SetTileState(playerpos.x, playerpos.y, state);
}

void Map::RemoveTileModifier(const Vector2i position)
{
	m_Grid->SetTileState(position.x, position.y, Tile::State::normal);
}

const Tile::State Map::GetTileState(Vector2i position) const
{
	return m_Grid->GetTileState(position.x, position.y);
}

const float Map::GetScale()
{
	return m_Scale;
}

const int Map::GetMaxValue()
{
	return m_MaxValue;
}

void Map::SetMaxValue(const int maxValue)
{
	m_MaxValue = maxValue;
}

const int Map::GetMinValue()
{
	return m_MinValue;
}

void Map::SetMinValue(const int minValue)
{
	m_MinValue = minValue;
}

void Map::RandomizeTile(const Vector2i& position)
{
	int value{ (rand() % (m_MaxValue - m_MinValue) + m_MinValue) % m_MaxValue };
	const int cols{ m_Grid->GetNumCols() };
	const int rows{ m_Grid->GetNumRows() };

	// offsets depend on grid type
    auto IsValidForNeighbors = [&](int v) -> bool
        {
                auto wrapX = [&](int x) { if (!m_IsWrapped) return x; int r = x % cols; if (r < 0) r += cols; return r; };
                auto wrapY = [&](int y) { if (!m_IsWrapped) return y; int r = y % rows; if (r < 0) r += rows; return r; };

            if (!m_IsHexMode)
            {
                // orthogonal offsets
                const int dx[4] = { 1, -1, 0, 0 };
                const int dy[4] = { 0, 0, 1, -1 };

                // 1) immediate neighbors (with wrapping)
                for (int i = 0; i < 4; ++i)
                {
                    int nx = wrapX(position.x + dx[i]);
                    int ny = wrapY(position.y + dy[i]);
                    if (m_Grid->GetTileValue(nx, ny) == v) return false;
                }

                // 2) neighbors of neighbors (with wrapping)
                for (int i = 0; i < 4; ++i)
                {
                    int nx = position.x + dx[i];
                    int ny = position.y + dy[i];
                    // iterate neighbors of this neighbor
                    for (int j = 0; j < 4; ++j)
                    {
                        int nnx = nx + dx[j];
                        int nny = ny + dy[j];
                        int wx = wrapX(nnx);
                        int wy = wrapY(nny);
                        if (wx < 0 || wx >= cols || wy < 0 || wy >= rows) continue;
                        if (wx == position.x && wy == position.y) continue;
                        if (m_Grid->GetTileValue(wx, wy) == v) return false;
                    }
                }

                return true;
            }

            const int col = position.x;
            const int row = position.y;

            auto Check = [&](int x, int y)
                {
                    int wx = wrapX(x);
                    int wy = wrapY(y);
                    return m_Grid->GetTileValue(wx, wy) == v;
                };

            bool odd = (row & 1) != 0;

            //  Immediate neighbors (correct odd-r layout, wrapped)
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

            //  Radius-2 area (robust replacement for "neighbors of neighbors") with wrapping
            for (int dy = -2; dy <= 2; ++dy)
            {
                for (int dx = -2; dx <= 2; ++dx)
                {
                    if (dx == 0 && dy == 0) continue;

                    int cx = col + dx;
                    int cy = row + dy;

                    // simple distance filter (prevents square corners being included)
                    int dist = abs(dx) + abs(dy);
                    if (dist > 3) continue;

                    if (Check(cx, cy)) return false;
                }
            }

            return true;
        };

    // Build list of all valid candidates and pick one uniformly at random.
    std::vector<int> candidates;
	const int
		max{ m_MaxValue - m_MinValue };
    candidates.reserve(max);
    for (int v = m_MinValue; v < m_MaxValue; ++v)
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
        while (!IsValidForNeighbors(value) && attempts < m_MaxValue)
        {
            value = (value + 1) % m_MaxValue;
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

    // Place vision tiles by repeating the original 2x2 pattern across the grid
    // using the original tile gaps so the amount of vision markers scales with grid size.
    // Original pattern: columns starting at 2 with gap 5 -> 2,7,... ; rows starting at 1 with gap 3 -> 1,4,...
    const int numColsV = m_Grid->GetNumCols();
    const int numRowsV = m_Grid->GetNumRows();
    if (numColsV > 0 && numRowsV > 0)
    {
        const int colGap = 5; // original column repetition gap
        const int rowGap = 3; // original row repetition gap

        // Choose column offset in [0,colGap-1] that centers the pattern best
        std::vector<int> bestCols;
        float bestColScore = std::numeric_limits<float>::infinity();
        const float gridColCenter = (numColsV - 1) * 0.5f;
        for (int offset = 0; offset < colGap; ++offset)
        {
            std::vector<int> cols;
            for (int c = offset; c < numColsV; c += colGap) cols.push_back(c);
            if (cols.empty()) continue;
            float center = (cols.front() + cols.back()) * 0.5f;
            float score = std::abs(center - gridColCenter);
            if (score < bestColScore)
            {
                bestColScore = score;
                bestCols = cols;
            }
        }

        // Choose row offset in [0,rowGap-1] that centers the pattern best
        std::vector<int> bestRows;
        float bestRowScore = std::numeric_limits<float>::infinity();
        const float gridRowCenter = (numRowsV - 1) * 0.5f;
        for (int offset = 0; offset < rowGap; ++offset)
        {
            std::vector<int> rows;
            for (int r = offset; r < numRowsV; r += rowGap) rows.push_back(r);
            if (rows.empty()) continue;
            float center = (rows.front() + rows.back()) * 0.5f;
            float score = std::abs(center - gridRowCenter);
            if (score < bestRowScore)
            {
                bestRowScore = score;
                bestRows = rows;
            }
        }

        // If we didn't find any (shouldn't happen), fallback to simple starts
        if (bestCols.empty())
        {
            for (int c = 0; c < numColsV; c += colGap) bestCols.push_back(c);
            if (bestCols.empty()) bestCols.push_back(0);
        }
        if (bestRows.empty())
        {
            for (int r = 0; r < numRowsV; r += rowGap) bestRows.push_back(r);
            if (bestRows.empty()) bestRows.push_back(0);
        }

        // Set vision tiles for all combinations of chosen columns and rows
        for (int c : bestCols)
        {
            for (int r : bestRows)
            {
                m_Grid->SetTileState(c, r, Tile::State::vision);
            }
        }
    }
}
