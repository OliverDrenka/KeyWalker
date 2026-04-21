#pragma once
#include "Tile.h"

class Grid
{
public:
	Grid();
	Grid(int numCols = 1, int numRows = 1);
	~Grid();

	const int GetTileValue(const int colIdx, const int rowIdx ) const;
	const int GetNumCols() const;
	const int GetNumRows() const;
	const int GetSize() const;

	void SetTileState(const int colIdx, const int rowIdx, const Tile::State state);
	const Tile::State GetTileState(const int colIdx, const int rowIdx) const;

	const void SetTile(const int colIdx, const int rowIdx, const int value);


private:
	Tile* m_Grid;
	int m_GridSize;
	int m_Cols;
	int m_Rows;
};

