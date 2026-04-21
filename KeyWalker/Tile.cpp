#include "pch.h"
#include "Tile.h"

Tile::Tile()
	:Tile(-1)
{
}

Tile::Tile(const int value)
	:m_Value{value}
	,m_State{State::normal}
{
}

void Tile::SetValue(const int value)
{
	m_Value = value;
}

void Tile::SetState(const State state)
{
	m_State = state;
}

const int Tile::GetValue() const
{
	return m_Value;
}

const Tile::State Tile::GetState() const
{
	return m_State;
}
