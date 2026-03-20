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
	m_Grid = new char[m_GridSize]();
}

Grid::~Grid()
{
	delete[] m_Grid;
}

const int Grid::GetTile( const int colIdx, const int rowIdx ) const
{
	return m_Grid[rowIdx * m_Cols + colIdx];
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

const void Grid::SetTile(const int colIdx, const int rowIdx, const char value)
{
	m_Grid[rowIdx * m_Cols + colIdx] = value;
}
