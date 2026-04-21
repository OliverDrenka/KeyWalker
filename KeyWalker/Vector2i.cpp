#include "pch.h"
#include "Vector2i.h"

Vector2i::Vector2i()
	:Vector2i(0, 0)
{
}

Vector2i::Vector2i(int x, int y)
	:x{ x }
	,y{ y }
{
}

bool Vector2i::operator!=(const Vector2i& rhs) const
{
	return this->x != rhs.x || this->y != rhs.y;
}
