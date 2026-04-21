#pragma once
struct Vector2i
{
	Vector2i();
	Vector2i(int x, int y);

	bool operator!=(const Vector2i& rhs) const;

	int
		x,
		y;
};