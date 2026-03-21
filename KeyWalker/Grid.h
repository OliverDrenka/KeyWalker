#pragma once
class Grid
{
public:
	Grid();
	Grid(int numCols = 1, int numRows = 1);
	~Grid();

	const int GetTile(const int colIdx, const int rowIdx ) const;
	const int GetNumCols() const;
	const int GetNumRows() const;
	const int GetSize() const;

	const void SetTile(const int colIdx, const int rowIdx, const int value);


private:
	int* m_Grid;
	int m_GridSize;
	int m_Cols;
	int m_Rows;
};

