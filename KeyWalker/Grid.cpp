#include "pch.h"
#include "Grid.h"

Grid::Grid()
	: Grid(1,1)
{
}

Grid::Grid(int numCols, int numRows)
	: m_Cols{ numCols }
	, m_Rows{ numRows }
	, m_GridSize{ numRows * numCols }
{
	m_Grid = new Tile[m_GridSize]();
}

Grid::~Grid()
{
	delete[] m_Grid;
}

const int Grid::GetTileValue( const int colIdx, const int rowIdx ) const
{
	return m_Grid[rowIdx * m_Cols + colIdx].GetValue();
}

const int Grid::GetNumCols() const
{
	return m_Cols;
}

const int Grid::GetNumRows() const
{
	return m_Rows;
}

const int Grid::GetSize() const
{
	return m_GridSize;
}

const void Grid::SetTile(const int colIdx, const int rowIdx, const int value)
{
	m_Grid[rowIdx * m_Cols + colIdx].SetValue(value);
}

void Grid::SetTileState(const int colIdx, const int rowIdx, const Tile::State state)
{
	m_Grid[rowIdx * m_Cols + colIdx].SetState(state);

}

const Tile::State Grid::GetTileState(const int colIdx, const int rowIdx) const
{
	return m_Grid[rowIdx * m_Cols + colIdx].GetState();
}


