#pragma once
#include "Vector2i.h"
#include "SpriteSheet.h"

class Player
{
public:
	Player();
	Player(Vector2i position);
	~Player();

    void Draw(const float tileSize = 0, bool hexMode = false) const;
	void Move( Vector2i direction );
    void SetPosition(const Vector2i& position);
	void Update(const float deltaTime);
	void Hit(const float damage);
    // Buffs: hex layout, wrap mode, reveal map
    enum class BuffType { none, hex, wrap, reveal };
    void ApplyBuff(BuffType type, float duration);
    bool HasBuff(BuffType type) const;
    void ClearBuff();

    // Debuff (e.g. blindness) timer
    void ApplyDebuff(float duration);
    bool IsDebuffed() const;
    void Heal(int amount);
	const Vector2f GetDirection() const;
	void SetDirection(Vector2i direction);

	const int GetHp() const;
	const Vector2i GetPosition();
    const Circlef GetBounds(const float tileSize, bool hexMode = false);

private:
	const float m_Radius;
	SpriteSheet* m_SpriteSheet;
	Vector2i m_Position;
	int m_Hp;
	Vector2i m_Direction;

	int m_SpriteIdx;
	float m_FrameTimer;
	float m_TimePerFrame;

    // unified buff timer and independent buff flags
    bool m_HexBuff{ false };
    bool m_WrapBuff{ false };
    bool m_RevealBuff{ false };
    float m_BuffTimer{ 0.f };

    // debuff timer
    float m_DebuffTimer{ 0.f };


};

