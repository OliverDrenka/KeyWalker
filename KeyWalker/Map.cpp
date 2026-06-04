#include "pch.h"
#include "Map.h"
#include <fstream>
#include <iostream>
#include "utils.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <vector>
#include <deque>


Map::Map()
	: m_MaxValue{ 25 }
	, m_MinValue{ 0 }
	, m_MaxDangerMultiplier{ 11 }
	, m_MaxDebuffMultiplier{ 4 }
	, m_MaxPointMultiplier{ 2 }
	, m_MaxBuffMultiplier{ 1 }
{
	m_Letters = new SpriteSheet(36, "Resources/Font.png", 8);
	m_TileTexture = new SpriteSheet(8, "Resources/Tile.png", 6);
	m_TileSize = 16;
	m_IsHexMode = false;
	m_IsWrapped = false;
	// initialize previous visibility mask 
	m_Scale = 16.f / m_TileSize;
	m_Grid = new Grid(10 * m_Scale, 6 * m_Scale);
	m_PrevVisible.assign(m_Grid->GetNumCols() * m_Grid->GetNumRows(), 0);
}

const Vector2i Map::FindRandomNormalTile(const Vector2i playerpos)
{
    const int cols = m_Grid->GetNumCols();
    const int rows = m_Grid->GetNumRows();
    const int requiredDistance = 3;

    auto squareDist = [&](const Vector2i& a, const Vector2i& b) -> int
    {
        int dx = std::abs(a.x - b.x);
        int dy = std::abs(a.y - b.y);
        dx = std::min(dx, cols - dx);
        dy = std::min(dy, rows - dy);
        return std::max(dx, dy);
    };

    auto hexDist = [&](const Vector2i& a, const Vector2i& b) -> int
    {
        auto oddr_to_cube = [](int col, int row, int& cx, int& cy, int& cz)
        {
            int q = col - (row - (row & 1)) / 2;
            int r = row;
            cx = q; cz = r; cy = -cx - cz;
        };
        int best = std::numeric_limits<int>::max();
        for (int sx = -1; sx <= 1; ++sx)
        for (int sy = -1; sy <= 1; ++sy)
        {
            Vector2i bshift(b.x + sx * cols, b.y + sy * rows);
            int ax, ay, az, bx, by, bz;
            oddr_to_cube(a.x, a.y, ax, ay, az);
            oddr_to_cube(bshift.x, bshift.y, bx, by, bz);
            int d = (std::abs(ax - bx) + std::abs(ay - by) + std::abs(az - bz)) / 2;
            if (d < best) best = d;
        }
        return best == std::numeric_limits<int>::max() ? 0 : best;
    };

    // try random picks
    for (int i = 0; i < 200; ++i)
    {
        int x = rand() % cols;
        int y = rand() % rows;
        Tile::State st = m_Grid->GetTileState(x, y);
        if (st != Tile::State::normal && st != Tile::State::preparing) continue;
        Vector2i cand(x, y);
        int d = m_IsHexMode ? hexDist(playerpos, cand) : squareDist(playerpos, cand);
        if (d >= requiredDistance) return cand;
    }

    // fallback scan
    for (int ry = 0; ry < rows; ++ry)
    for (int rx = 0; rx < cols; ++rx)
    {
        Tile::State st = m_Grid->GetTileState(rx, ry);
        if (st != Tile::State::normal && st != Tile::State::preparing) continue;
        Vector2i cand(rx, ry);
        int d = m_IsHexMode ? hexDist(playerpos, cand) : squareDist(playerpos, cand);
        if (d >= requiredDistance) return cand;
    }

    return playerpos;
}

void Map::ClearPointAt(const Vector2i position)
{
    // Only clear if it is a point -- used when player picks one up
    if (m_Grid->GetTileState(position.x, position.y) == Tile::State::point)
    {
        m_Grid->SetTileState(position.x, position.y, Tile::State::normal);
    }
}

int Map::GetMaxDangerTiles() const
{
	const int cols = m_Grid->GetNumCols();
	const int rows = m_Grid->GetNumRows();
	const int area = cols * rows;
	const int baseArea = 10 * 6; // original reference area
	int result = static_cast<int>(std::max(1.0f, m_MaxDangerMultiplier * (static_cast<float>(area) / static_cast<float>(baseArea))));
	return result;
}

int Map::GetMaxDebuffTiles() const
{
	const int cols = m_Grid->GetNumCols();
	const int rows = m_Grid->GetNumRows();
	const int area = cols * rows;
	const int baseArea = 10 * 6;
	int result = static_cast<int>(std::max(1.0f, m_MaxDebuffMultiplier * (static_cast<float>(area) / static_cast<float>(baseArea))));
	return result;
}

int Map::GetMaxPointTiles() const
{
	const int cols = m_Grid->GetNumCols();
	const int rows = m_Grid->GetNumRows();
	const int area = cols * rows;
	const int baseArea = 10 * 6;
	int result = static_cast<int>(std::max(1.0f, m_MaxPointMultiplier * (static_cast<float>(area) / static_cast<float>(baseArea))));
	return result;
}

int Map::GetMaxBuffTiles() const
{
	const int cols = m_Grid->GetNumCols();
	const int rows = m_Grid->GetNumRows();
	const int area = cols * rows;
	const int baseArea = 10 * 6;
	int result = static_cast<int>(std::max(1.0f, m_MaxBuffMultiplier * (static_cast<float>(area) / static_cast<float>(baseArea))));
	return result;
}

void Map::SetMaxDangerMultiplier(int m) { m_MaxDangerMultiplier = m; }
void Map::SetMaxDebuffMultiplier(int m) { m_MaxDebuffMultiplier = m; }
void Map::SetMaxPointMultiplier(int m) { m_MaxPointMultiplier = m; }
void Map::SetMaxBuffMultiplier(int m) { m_MaxBuffMultiplier = m; }

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
					 y };
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
	bool prev = m_IsHexMode;
	if (prev == hex) return;
	m_IsHexMode = hex;
}


void Map::SetWrapMode(bool wrap)
{
	m_IsWrapped = wrap;
}

void Map::SetBlindMode(bool wrap)
{
	m_IsBlind = wrap;
}

void Map::SetRevealedMode(bool revealed)
{
	m_IsRevealed = revealed;
}

void Map::SetZeroVisionDuringPause(bool v)
{
    m_ZeroVisionDuringPause = v;
}

bool Map::IsRevealed() const
{
	return m_IsRevealed;
}

bool Map::IsWrapMode() const
{
	return m_IsWrapped;
}

bool Map::IsHexMode() const
{
	return m_IsHexMode;
}

bool Map::IsBlindMode() const
{
	return m_IsBlind;
}

void Map::Draw(Vector2f position, const Vector2i* pPlayerPosition)
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

    // Precompute visibility mask if player position given. If the map is in
    // zero-vision-during-pause mode, treat the provided player pointer as
    // absent so no player-anchored visibility is computed.
    const Vector2i* pPlayerPtr = (m_ZeroVisionDuringPause && pPlayerPosition) ? nullptr : pPlayerPosition;
    std::vector<char> visible;
    if (pPlayerPtr && !(m_IsRevealed && !m_IsBlind))
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

			if (m_IsBlind && !m_IsRevealed)
			{
				// Blind mode: only show the four orthogonal adjacent tiles (N, S, E, W)
				const int dx[4] = { 1, -1, 0, 0 };
				const int dy[4] = { 0, 0, 1, -1 };
				for (int i = 0; i < 4; ++i)
				{
					int cx = wrapX(pp.x + dx[i]);
					int cy = wrapY(pp.y + dy[i]);
					if (cx < 0 || cx >= numCols || cy < 0 || cy >= numRows) continue;
					visible[cy * numCols + cx] = 1;
				}
				// also mark the player's own tile visible so stepping on a vision tile still triggers expansion
				{
					int pcx = wrapX(pp.x);
					int pcy = wrapY(pp.y);
					if (!(pcx < 0 || pcx >= numCols || pcy < 0 || pcy >= numRows))
						visible[pcy * numCols + pcx] = 1;
				}
			}
            else
            {
                // If player is blind but the map is revealed, use the original radius-2 square
                // visibility (5x5 centered, excluding the 4 corner tiles at ±2). Otherwise use
                // the newer radius-3 intersected with a Manhattan radius-4 to produce
                // the 3,5,7,7,7,5,3 pattern.
                if (m_IsBlind && m_IsRevealed)
                {
                    for (int dy = -2; dy <= 2; ++dy)
                    {
                        for (int dx = -2; dx <= 2; ++dx)
                        {
                            // exclude the four corner tiles where both offsets are ±2
                            if (std::abs(dx) == 2 && std::abs(dy) == 2) continue;
                            int cx = wrapX(pp.x + dx);
                            int cy = wrapY(pp.y + dy);
                            if (cx < 0 || cx >= numCols || cy < 0 || cy >= numRows) continue;
                            visible[cy * numCols + cx] = 1;
                        }
                    }
                }
                else
                {
                    // increase player square vision from radius 2 (5x5) to radius 3 (7x7)
                    // but intersect (AND) it with a diamond (Manhattan) radius of 4 so the
                    // final visible pattern per row becomes 3,5,7,7,7,5,3 when centered.
                    for (int dy = -3; dy <= 3; ++dy)
                    {
                        for (int dx = -3; dx <= 3; ++dx)
                        {
                            // exclude the four corner tiles where both offsets are ±3 (keeps the square mask)
                            if (std::abs(dx) == 3 && std::abs(dy) == 3) continue;

                            // diamond (Manhattan) radius 4 check
                            if ((std::abs(dx) + std::abs(dy)) > 4) continue;

                            int cx = wrapX(pp.x + dx);
                            int cy = wrapY(pp.y + dy);
                            // when wrapping is disabled wrapX/wrapY return raw coords;
                            // skip out-of-bounds indices in that case to avoid invalid access
                            if (cx < 0 || cx >= numCols || cy < 0 || cy >= numRows) continue;
                            visible[cy * numCols + cx] = 1;
                        }
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
                for (int dy = -3; dy <= 3; ++dy)
                {
                    for (int dx = -3; dx <= 3; ++dx)
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
			std::deque<std::pair<Vector2i, int>> q0;
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
            if (m_IsBlind && !m_IsRevealed)
			{
				// Blind mode on hex: mark the six immediate hex neighbors (odd-r layout) and player
				int col = pp.x;
				int row = pp.y;
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
					try_mark(nx[i], ny[i]);
				}
			}
            else
            {
                // If player is blind but the map is revealed, use the original hex radius 2.
                // Otherwise use radius 3 (new behavior).
                const int maxDepth = (m_IsBlind && m_IsRevealed) ? 2 : 3;

                while (!q0.empty())
                {
                    auto cur = q0.front(); q0.pop_front();
                    Vector2i pos = cur.first;
                    int depth = cur.second;
                    if (depth >= maxDepth) continue;
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
				std::deque<std::pair<Vector2i, int>> qv;
				int sx = wrapX(seed.x);
				int sy = wrapY(seed.y);
				if (sx < 0 || sx >= numCols || sy < 0 || sy >= numRows) continue;
				qv.emplace_back(Vector2i(sx, sy), 0);
				visited[sy * numCols + sx] = 1;
				// seed is already marked visible by earlier code
                // Use same maxDepth logic as above: if blind+revealed use 2, else 3
                const int seedMaxDepth = (m_IsBlind && m_IsRevealed) ? 2 : 3;
                while (!qv.empty())
                {
                    auto cur = qv.front(); qv.pop_front();
                    Vector2i pos = cur.first;
                    int depth = cur.second;
                    if (depth >= seedMaxDepth) continue;
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
	else if (m_IsRevealed && !m_IsBlind)
	{
		// mark all tiles visible
		visible.assign(numCols * numRows, 1);
		// ensure previous-visible mask matches grid size
		if (m_PrevVisible.size() != visible.size())
			m_PrevVisible.assign(visible.size(), 0);
	}

	// set current visible mask for other systems (RandomizeTile may consult it)
	if (visible.empty())
		m_CurrentVisible.clear();
	else
		m_CurrentVisible = visible;

	// draw tiles; if visible mask exists, only draw letters for visible tiles
	if (!m_IsHexMode)
	{
		for (int rowIdx{}; rowIdx < numRows; ++rowIdx)
		{
			for (int colIdx{}; colIdx < numCols; ++colIdx)
			{
				// draw tile texture scaled to the current tile size
				int spriteRowHex;
                if (pPlayerPtr)
				{
					bool isVisible = (visible[rowIdx * numCols + colIdx] != 0);
					spriteRowHex = isVisible ? 0 : (m_IsBlind ? 2 : 1);
				}
				else
				{
					spriteRowHex = 1 + (m_IsBlind ? 1 : 0);
				}
				// if wrapping is enabled and tile is on edge, use the spritesheet rows 3,4,5
				if (m_IsWrapped && (rowIdx == 0 || rowIdx == numRows - 1 || colIdx == 0 || colIdx == numCols - 1))
				{
					spriteRowHex += 3;
				}
				m_TileTexture->DrawSprite(tilePosition, static_cast<int>(GetTileState(Vector2i(colIdx, rowIdx))), spriteRowHex, m_TileSize, m_TileSize);

                // Default to not showing letters when no player-provided visibility (e.g. paused)
                bool showLetter = false;
                if (pPlayerPtr) showLetter = (visible[rowIdx * numCols + colIdx] != 0);
                // newly visible? randomize tile (only when tile becomes visible)
                if (pPlayerPtr && visible[rowIdx * numCols + colIdx] && !m_PrevVisible[rowIdx * numCols + colIdx])
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

					//utils::FillEllipse(center, radius, radius);
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
				int spriteRowHex;
                if (pPlayerPtr)
				{
					bool isVisible = (visible[rowIdx * numCols + colIdx] != 0);
					spriteRowHex = isVisible ? 0 : (m_IsBlind ? 2 : 1);
				}
				else
				{
					spriteRowHex = 1 + (m_IsBlind ? 1 : 0);
				}
				// if wrapping is enabled and tile is on edge, use the spritesheet rows 3,4,5
				if (m_IsWrapped && (rowIdx == 0 || rowIdx == numRows - 1 || colIdx == 0 || colIdx == numCols - 1))
				{
					spriteRowHex += 3;
				}
				m_TileTexture->DrawSprite(Vector2f(x, y), static_cast<int>(GetTileState(Vector2i(colIdx, rowIdx))), spriteRowHex, m_TileSize, m_TileSize);
                // Default to not showing letters when no player-provided visibility (e.g. paused)
                bool showLetter = false;
                if (pPlayerPtr) showLetter = (visible[rowIdx * numCols + colIdx] != 0);
                // newly visible? randomize tile (only when tile becomes visible)
                if (pPlayerPtr && visible[rowIdx * numCols + colIdx] && !m_PrevVisible[rowIdx * numCols + colIdx])
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
    if (pPlayerPtr)
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

const Vector2i Map::GetAdjecentTileDirection(Vector2i position, int value)
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
			if (!m_IsWrapped && (nx < 0 || nx >= totalCols || ny < 0 || ny >= totalRows));
			else if (m_Grid->GetTileValue(nx, ny) == value) return Vector2i(1, 0);
		}
		// Check south
		{
			int nx = wrapX(position.x);
			int ny = wrapY(position.y + 1);
			if (!m_IsWrapped && (nx < 0 || nx >= totalCols || ny < 0 || ny >= totalRows));
			else if (m_Grid->GetTileValue(nx, ny) == value) return Vector2i(0, 1);
		}
		// Check west
		{
			int nx = wrapX(position.x - 1);
			int ny = wrapY(position.y);
			if (!m_IsWrapped && (nx < 0 || nx >= totalCols || ny < 0 || ny >= totalRows));
			else if (m_Grid->GetTileValue(nx, ny) == value) return Vector2i(-1, 0);
		}
		// Check north
		{
			int nx = wrapX(position.x);
			int ny = wrapY(position.y - 1);
			if (!m_IsWrapped && (nx < 0 || nx >= totalCols || ny < 0 || ny >= totalRows));
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
		if (!m_IsWrapped && (nc < 0 || nc >= totalCols || nr < 0 || nr >= totalRows));
		else if (m_Grid->GetTileValue(nc, nr) == value) return Vector2i(1, 0);
	}
	// West
	{
		int nc = wrapX(col - 1);
		int nr = wrapY(row);
		if (!m_IsWrapped && (nc < 0 || nc >= totalCols || nr < 0 || nr >= totalRows));
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
		if (m_Grid->GetTileState(x, y) != Tile::State::normal && m_Grid->GetTileState(x, y) != Tile::State::preparing) continue;

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
			if (m_Grid->GetTileState(rx, ry) != Tile::State::normal && m_Grid->GetTileState(rx, ry) != Tile::State::preparing) continue;
			Vector2i cand(rx, ry);
			int d = m_IsHexMode ? hexDist(playerpos, cand) : squareDist(playerpos, cand);
			if (d >= requiredDistance)
			{
				m_Grid->SetTileState(rx, ry, Tile::State::point);
				return Vector2i(rx, ry);
			}
		}
	}

    // Last-resort: pick any normal or preparing tile (no tile meets distance requirement)
    for (int ry = 0; ry < rows; ++ry)
    {
        for (int rx = 0; rx < cols; ++rx)
        {
            Tile::State st = m_Grid->GetTileState(rx, ry);
            if (st == Tile::State::normal || st == Tile::State::preparing)
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
    // Prevent accidental clearing of point tiles via generic setters: only allow
    // explicit ClearPointAt to remove points. Other callers may still change
    // point -> something else but normal clears are ignored here.
    Tile::State oldState = m_Grid->GetTileState(playerpos.x, playerpos.y);
    if (oldState == Tile::State::point && state == Tile::State::normal)
    {
        return;
    }

    // Ensure that certain special tiles only spawn over normal or preparing tiles
    // to avoid overwriting danger/debuff/etc accidentally (reported bug: buff
    // spawning over danger). Only enforce for point, buff, and heal spawns.
    if (state == Tile::State::point || state == Tile::State::buff || state == Tile::State::heal)
    {
        if (!(oldState == Tile::State::normal || oldState == Tile::State::preparing))
        {
            // do not overwrite non-normal/preparing tiles
            return;
        }
    }

    if (oldState != state)
    {
        m_Grid->SetTileState(playerpos.x, playerpos.y, state);
    }
}

void Map::RemoveTileModifier(const Vector2i position)
{
    // Do not clear point tiles here. Points should only be cleared when the player collects them.
    Tile::State old = m_Grid->GetTileState(position.x, position.y);
    if (old == Tile::State::point)
    {
        return;
    }
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
	// pick an initial candidate value in the configured [m_MinValue, m_MaxValue) range
	int value = 0;
	if (m_MaxValue > m_MinValue)
		value = (rand() % (m_MaxValue - m_MinValue)) + m_MinValue;
	const int cols{ m_Grid->GetNumCols() };
	const int rows{ m_Grid->GetNumRows() };
	// For safety against future toggles, always check both square and hex adjacency using wrapping.
	auto IsValidForNeighbors = [&](int v) -> bool
		{
			auto wrapXAlways = [&](int x) { int r = x % cols; if (r < 0) r += cols; return r; };
			auto wrapYAlways = [&](int y) { int r = y % rows; if (r < 0) r += rows; return r; };

			// 1) Square: check full Chebyshev radius-2 area (all dx,dy with max(|dx|,|dy|)<=2)
			for (int dy = -2; dy <= 2; ++dy)
			{
				for (int dx = -2; dx <= 2; ++dx)
				{
					if (dx == 0 && dy == 0) continue;
					int wx = wrapXAlways(position.x + dx);
					int wy = wrapYAlways(position.y + dy);
					if (m_Grid->GetTileValue(wx, wy) == v) return false;
				}
			}

			// 2) Hex immediate neighbors (odd-r) and radius-2 hex area
			const int col = position.x;
			const int row = position.y;
			bool odd = (row & 1) != 0;
			int hx[6], hy[6];
			hx[0] = col + 1; hy[0] = row; // E
			hx[1] = col - 1; hy[1] = row; // W
			if (odd)
			{
				hx[2] = col + 1; hy[2] = row - 1; // NE
				hx[3] = col;     hy[3] = row - 1; // NW
				hx[4] = col + 1; hy[4] = row + 1; // SE
				hx[5] = col;     hy[5] = row + 1; // SW
			}
			else
			{
				hx[2] = col;     hy[2] = row - 1; // NE
				hx[3] = col - 1; hy[3] = row - 1; // NW
				hx[4] = col;     hy[4] = row + 1; // SE
				hx[5] = col - 1; hy[5] = row + 1; // SW
			}
			for (int i = 0; i < 6; ++i)
			{
				int wx = wrapXAlways(hx[i]);
				int wy = wrapYAlways(hy[i]);
				if (wx == position.x && wy == position.y) continue;
				if (m_Grid->GetTileValue(wx, wy) == v) return false;
			}

			// radius-2 hex area: use cube conversion and wrapped offsets to find any hex within distance 2
			auto oddr_to_cube = [](int c, int r, int& cx, int& cy, int& cz)
				{
					int q = c - (r - (r & 1)) / 2;
					int rr = r;
					cx = q;
					cz = rr;
					cy = -cx - cz;
				};

			int ax, ay, az;
			oddr_to_cube(col, row, ax, ay, az);
			for (int dy = -2; dy <= 2; ++dy)
			{
				for (int dx = -2; dx <= 2; ++dx)
				{
					if (dx == 0 && dy == 0) continue;
					int cxpos = col + dx;
					int cypos = row + dy;
					// quick Manhattan filter
					if (std::abs(dx) + std::abs(dy) > 3) continue;
					// test wrapped shifts to compute shortest hex distance
					for (int sx = -1; sx <= 1; ++sx)
					{
						for (int sy = -1; sy <= 1; ++sy)
						{
							int bx = cxpos + sx * cols;
							int by = cypos + sy * rows;
							int bx2, by2, bz2;
							oddr_to_cube(bx, by, bx2, by2, bz2);
							int dist = (std::abs(ax - bx2) + std::abs(ay - by2) + std::abs(az - bz2)) / 2;
							if (dist <= 2)
							{
								int wx = wrapXAlways(cxpos);
								int wy = wrapYAlways(cypos);
								if (!(wx == position.x && wy == position.y))
								{
									if (m_Grid->GetTileValue(wx, wy) == v) return false;
								}
								// once matched as within radius-2 for some shift, stop checking shifts
								sx = 2; sy = 2; // break both loops
							}
						}
					}
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
		numCols{ m_Grid->GetNumCols() },
		numRows{ m_Grid->GetNumRows() };
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
	const int numCols{ m_Grid->GetNumCols() };
	return m_TileSize * m_Grid->GetNumCols();
}

const float Map::GetHeight() const
{
	const int numRows{ m_Grid->GetNumRows() };
	return m_TileSize * m_Grid->GetNumRows();
}

void Map::GenerateMapKeyboard()
{
    const int
        numCols{ m_Grid->GetNumCols() },
        numRows{ m_Grid->GetNumRows() };
    for (int rowIdx{}; rowIdx < numRows; ++rowIdx)
    {
        for (int colIdx{}; colIdx < numCols; ++colIdx)
        {
            const int value{ (rowIdx * numCols + colIdx) % 36 };
            m_Grid->SetTile(colIdx, rowIdx, value);
        }
    }
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

void Map::IncreaseCols(int count)
{
	if (count <= 0) return;

	// preserve prev-visible before changing grid
	const int oldCols = m_Grid->GetNumCols();
	const int oldRows = m_Grid->GetNumRows();
	std::vector<char> oldPrev = m_PrevVisible;

	// Add columns to the right so existing indices remain valid
	m_Grid->AddColsRight(count);

	// Recompute tile size and scale so the map keeps a reasonable world size
	{
		const float baseTile = 16.f;
		const float baseCols = 10.f;
		const float baseRows = 6.f;
		const int cols = m_Grid->GetNumCols();
		const int rows = m_Grid->GetNumRows();
		const float baseWidth = baseTile * baseCols;
		const float baseHeight = baseTile * baseRows;
		// choose tile size so the map fits inside the original world rectangle
		float tileSize = std::min(baseWidth / static_cast<float>(cols), baseHeight / static_cast<float>(rows));
		if (tileSize < 1.f) tileSize = 1.f;
		m_TileSize = tileSize;
		m_Scale = baseTile / m_TileSize;
	}

	// Resize visibility buffers preserving previous content in the left part of each row
	const int newCols = m_Grid->GetNumCols();
	const int newRows = m_Grid->GetNumRows();
	std::vector<char> newPrev(static_cast<size_t>(newCols * newRows), 0);
	for (int r = 0; r < oldRows; ++r)
	{
		for (int c = 0; c < oldCols; ++c)
		{
			newPrev[r * newCols + c] = ( (r * oldCols + c) < static_cast<int>(oldPrev.size()) ) ? oldPrev[r * oldCols + c] : 0;
		}
	}
	m_PrevVisible.swap(newPrev);
	m_CurrentVisible.clear();
}

void Map::IncreaseRows(int count)
{
    if (count <= 0) return;

    // preserve prev-visible before changing grid
    const int oldCols = m_Grid->GetNumCols();
    const int oldRows = m_Grid->GetNumRows();
    std::vector<char> oldPrev = m_PrevVisible;

    m_Grid->AddRowsBottom(count);
    // Recompute tile size and scale so the map keeps a reasonable world size
    {
        const float baseTile = 16.f;
        const float baseCols = 10.f;
        const float baseRows = 6.f;
        const int cols = m_Grid->GetNumCols();
        const int rows = m_Grid->GetNumRows();
        const float baseWidth = baseTile * baseCols;
        const float baseHeight = baseTile * baseRows;
        // choose tile size so the map fits inside the original world rectangle
        float tileSize = std::min(baseWidth / static_cast<float>(cols), baseHeight / static_cast<float>(rows));
        if (tileSize < 1.f) tileSize = 1.f;
        m_TileSize = tileSize;
        m_Scale = baseTile / m_TileSize;
    }

    // Resize visibility buffers preserving previous content in the top rows
    const int newCols = m_Grid->GetNumCols();
    const int newRows = m_Grid->GetNumRows();
    std::vector<char> newPrev(static_cast<size_t>(newCols * newRows), 0);
    for (int r = 0; r < oldRows; ++r)
    {
        for (int c = 0; c < oldCols; ++c)
        {
            newPrev[r * newCols + c] = ( (r * oldCols + c) < static_cast<int>(oldPrev.size()) ) ? oldPrev[r * oldCols + c] : 0;
        }
    }
    m_PrevVisible.swap(newPrev);
    m_CurrentVisible.clear();
}

const int Map::GetNumRows() const
{
	return m_Grid->GetNumRows();
}
const int Map::GetNumCols() const
{
	return m_Grid->GetNumCols();
}

