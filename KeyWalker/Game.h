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

    Map* m_pMap;
    Player* m_pPlayer;
    Texture* m_pOverlay;
    AttackManager* m_pAttackManager;
    SpriteSheet* m_pLetters;

	TTF_Font* m_pFont;
	Texture* m_pRestartText;
	Texture* m_pPauseText;
	Texture* m_pScoreText;
	Texture* m_pTimeText;
	Texture* m_pHpText;


	float m_AttackTimer;
	float m_TotalTime;
	int m_Score;
	
	bool m_PointsSpawned;

    SoundEffect* m_pSoundButtonPress;
    SoundEffect* m_pSoundHit;
    SoundEffect* m_pSoundPointSpawn;
    SoundEffect* m_pSoundPointCollected;

	GameState m_GameState{ GameState::gameplay };



};