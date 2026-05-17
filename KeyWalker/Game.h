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
    std::vector<Vector2i> m_vecBuffTiles;

	TTF_Font* m_pFont;
	Texture* m_pStartText;
	Texture* m_pBestText;
	Texture* m_pRestartText;
	Texture* m_pPauseText;
	Texture* m_pScoreText;
	Texture* m_pTimeText;
	Texture* m_pHpText;
	Texture* m_pInfoText;

	// on-screen debug indicators
	Texture* m_pRangeText;
	Texture* m_pMultiplierText;
	std::string m_LastRangeText;
	std::string m_LastMultiplierText;


	float m_AttackTimer;
	float m_AttackSpawnTime;
	float m_TotalTime;
	int m_Score;
	
	bool m_PointsSpawned;

    // track last known maxima to react to increases
    int m_LastMaxPointTiles{0};

	SoundEffect* m_pSoundPreparedTile;
    SoundEffect* m_pSoundButtonPress;
    SoundEffect* m_pSoundHit;
    SoundEffect* m_pSoundDebuff;
    SoundEffect* m_pSoundBuff;
	SoundEffect* m_pSoundHeal;
    SoundEffect* m_pSoundPointCollected;

	GameState m_GameState{ GameState::start };

	float m_BestTime;
	float m_BestScore;

	float m_MultiplierTimer;
	float m_Multiplier;

	bool m_TimerStarted;

	bool m_Easy{false};

	float m_BuffSpawnTimer;
	const float m_BuffSpawnTimerMax{9.5f};

    // Heal spawn: after first point pickup spawn a heal on the edge every interval
    float m_HealSpawnTimer;
    const float m_HealSpawnTimerMax{100.f};

	bool m_IsConfused;
	float m_ConfusionTimer;
	// buff timers moved to Player: hex/wrap/reveal unified
	// debuff timer moved to Player
	const float m_StatusTimerMax{10.f};

	float m_OverlayTimer;
	float m_OverlayTimerMax;
	int m_OverlayFrame;

	// Save/load helpers
	void SaveBest();
	void LoadBest();

};