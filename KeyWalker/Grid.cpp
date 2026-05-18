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

void Grid::AddColsLeft(int count)
{
	if (count <= 0) return;
	int newCols = m_Cols + count;
	int newSize = newCols * m_Rows;
	Tile* newGrid = new Tile[newSize]();

	// copy each row, shifting columns to the right by 'count'
	for (int row = 0; row < m_Rows; ++row)
	{
		for (int col = 0; col < m_Cols; ++col)
		{
			newGrid[row * newCols + (col + count)] = m_Grid[row * m_Cols + col];
		}
	}

	delete[] m_Grid;
	m_Grid = newGrid;
	m_Cols = newCols;
	m_GridSize = newSize;
}

void Grid::AddColsRight(int count)
{
    if (count <= 0) return;
    int newCols = m_Cols + count;
    int newSize = newCols * m_Rows;
    Tile* newGrid = new Tile[newSize]();

    // copy each row, preserving columns at same indices (existing stay left)
    for (int row = 0; row < m_Rows; ++row)
    {
        for (int col = 0; col < m_Cols; ++col)
        {
            newGrid[row * newCols + col] = m_Grid[row * m_Cols + col];
        }
    }

    delete[] m_Grid;
    m_Grid = newGrid;
    m_Cols = newCols;
    m_GridSize = newSize;
}

void Grid::AddRowsBottom(int count)
{
	if (count <= 0) return;
	int newRows = m_Rows + count;
	int newSize = m_Cols * newRows;
	Tile* newGrid = new Tile[newSize]();

	// copy existing rows into the top portion (rows keep same indices)
	for (int row = 0; row < m_Rows; ++row)
	{
		for (int col = 0; col < m_Cols; ++col)
		{
			newGrid[row * m_Cols + col] = m_Grid[row * m_Cols + col];
		}
	}

	delete[] m_Grid;
	m_Grid = newGrid;
	m_Rows = newRows;
	m_GridSize = newSize;
}


