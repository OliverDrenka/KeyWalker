#pragma once
#include "Grid.h"
#include "SpriteSheet.h"
#include "Vector2f.h"
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
    // Toggle wrapping (toroidal) behavior for movement/vision/generation
    void SetWrapMode(bool wrap);
    bool IsWrapMode() const;

    void SetBlindMode(bool wrap);
    bool IsBlindMode() const;

    // Reveal entire map (ignore blind/visibility); when enabled Draw will show all tiles
    void SetRevealedMode(bool revealed);
    bool IsRevealed() const;

    // If pPlayerPosition is non-null, restrict letter drawing to tiles visible from that position
    void Draw( Vector2f position = Vector2f(0,0), const Vector2i* pPlayerPosition = nullptr );

    // Draw the letter for a single tile at grid coordinates 'position' (col,row).
    void DrawLetter(const Vector2i position, const int colorOffest = 0);

	const float GetTileSize() const;
	const float GetWidth() const;
	const float GetHeight() const;
	const Vector2i GetAdjecentTileDirection( Vector2i position, int value );
	
	const Vector2i CreateRandomPointTile(const Vector2i playerpos);
	void SetTileState(const Vector2i playerpos, const Tile::State state);
	void RemoveTileModifier(const Vector2i position);
	const Tile::State GetTileState(Vector2i position) const;

	const float GetScale();

	const int GetMaxValue();
	void SetMaxValue(const int maxValue);
	const int GetMinValue();
	void SetMinValue(const int minValue);


	void RandomizeTile(const Vector2i& position);

	const int GetNumRows() const;
	const int GetNumCols() const;

	void GenerateMapOrdered();
	void GenerateMapKeyboard();
	void GenerateMapRandom();

	// Increase grid size: add columns on the left or rows at the bottom without modifying existing content
	void IncreaseCols(int count = 1);
	void IncreaseRows(int count = 1);

private:
    // Origin passed to Draw() used by DrawLetter to compute world positions
    Vector2f m_DrawOrigin{0.f, 0.f};

    Grid* m_Grid;
	SpriteSheet* m_Letters;
	SpriteSheet* m_TileTexture;
	float m_TileSize;
	float m_Scale;

	int m_MaxValue;
	int m_MinValue;
    // Multipliers used to compute max counts based on current map scale.
    int m_MaxDangerMultiplier;
    int m_MaxDebuffMultiplier;
    int m_MaxPointMultiplier;
    int m_MaxBuffMultiplier;
    bool m_IsHexMode;
    bool m_IsWrapped;
	bool m_IsBlind;
    bool m_IsRevealed{ false };
    // persistent visibility mask from previous frame
    std::vector<char> m_PrevVisible;
    // Current visibility mask (set during Draw). If empty, treat all tiles as visible.
    std::vector<char> m_CurrentVisible;

public:
    // Get computed maxima (based on scale * multiplier). These update automatically when scale changes.
    int GetMaxDangerTiles() const;
    int GetMaxDebuffTiles() const;
    int GetMaxPointTiles() const;
    int GetMaxBuffTiles() const;

    // Optional: adjust multipliers used to compute maxima
    void SetMaxDangerMultiplier(int m);
    void SetMaxDebuffMultiplier(int m);
    void SetMaxPointMultiplier(int m);
    void SetMaxBuffMultiplier(int m);

};

