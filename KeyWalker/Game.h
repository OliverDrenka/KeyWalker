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
		info,
		end
	};

    Map* m_pMap;
    Player* m_pPlayer;
    AttackManager* m_pAttackManager;
    SpriteSheet* m_pOverlay;
    SpriteSheet* m_pLetters;
	Texture* m_pTitleScreen;

	std::vector<Vector2i> m_vecDangerTiles;
	std::vector<Vector2i> m_vecDebuffTiles;

	TTF_Font* m_pFont;
	Texture* m_pStartText;
	Texture* m_pBestText;
	Texture* m_pRestartText;
	Texture* m_pPauseText;
	Texture* m_pScoreText;
	Texture* m_pTimeText;
	Texture* m_pHpText;
	Texture* m_pInfoText;


	float m_AttackTimer;
	float m_AttackSpawnTime;
	float m_TotalTime;
	int m_Score;
	
	bool m_PointsSpawned;

	SoundEffect* m_pSoundPreparedTile;
    SoundEffect* m_pSoundButtonPress;
    SoundEffect* m_pSoundHit;
    SoundEffect* m_pSoundDebuff;
    SoundEffect* m_pSoundPointCollected;

	GameState m_GameState{ GameState::start };

	float m_BestTime;
	float m_BestScore;

	float m_MultiplierTimer;
	int m_Multiplier;

	bool m_TimerStarted;

	bool m_Easy{false};

	float m_BuffSpawnTimer;
	const float m_BuffSpawnTimerMax{10.f};

	bool m_IsConfused;

	float m_BlindnessTimer;
	float m_ConfusionTimer;
	float m_WrappingTimer;
	float m_HexTimer;
	const float m_StatusTimerMax{5.f};

	float m_OverlayTimer;
	float m_OverlayTimerMax;
	int m_OverlayFrame;

	// Save/load helpers
	void SaveBest();
	void LoadBest();

};