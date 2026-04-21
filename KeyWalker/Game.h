#pragma once
#include "SoundEffect.h"
#include "BaseGame.h"
#include "Map.h"
#include "Player.h"
#include "AttackManager.h"

class Game : public BaseGame
{
public:
	explicit Game(const Window& window);
	Game(const Game& other) = delete;
	Game& operator=(const Game& other) = delete;
	Game(Game&& other) = delete;
	Game& operator=(Game&& other) = delete;
	// http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rh-override
	~Game();

	void Update(float elapsedSec) override;
	void Draw() const override;

	// Event handling
	void ProcessKeyDownEvent(const SDL_KeyboardEvent& e) override;
	void ProcessKeyUpEvent(const SDL_KeyboardEvent& e) override;
	void ProcessMouseMotionEvent(const SDL_MouseMotionEvent& e) override;
	void ProcessMouseDownEvent(const SDL_MouseButtonEvent& e) override;
	void ProcessMouseUpEvent(const SDL_MouseButtonEvent& e) override;

private:

	// FUNCTIONS
	void Initialize();
	void Cleanup();
	void ClearBackground() const;

	enum class GameState
	{
		start,
		gameplay,
		paused,
		end
	};

	Map* m_Map;
	Player* m_Player;
	Texture* m_Overlay;
	AttackManager* m_AttackManager;
	float m_AttackTimer{};
	float m_TotalTime{};
	SpriteSheet* m_Letters;

	int m_Score{};
	
	bool m_PointsSpawned{ false };

	SoundEffect* m_SoundButtonPress;
	SoundEffect* m_SoundHit;
	SoundEffect* m_SoundPointSpawn;
	SoundEffect* m_SoundPointCollected;

	GameState m_GameState{ GameState::gameplay };



};