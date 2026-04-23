#pragma once
class Tile
{
public:

	enum class State
	{
		normal,
		point,
		danger,
		vision
	};
	Tile();
	Tile(const int value);

	void SetValue(const int value);
	void SetState(const State state);
	const int GetValue() const;
	const State GetState() const;


private:
	int m_Value;
	State m_State;
};

