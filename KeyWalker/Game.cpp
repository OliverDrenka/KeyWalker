#include "pch.h"
#include "Game.h"
#include <iostream>
#include "utils.h"
#include <fstream>

Game::Game( const Window& window ) 
	:BaseGame{ window }
{
	Initialize();
}

Game::~Game( )
{
	Cleanup( );
}

void Game::Initialize( )
{
	
	m_pFont = TTF_OpenFont("Resources/BoldPixels.ttf", 16);
	m_pRestartText = new Texture("Press R to Restart", m_pFont, Color4f(0.f,0.f,0.f,1.f));
	m_pStartText = new Texture("Press S to Start", m_pFont, Color4f(0.f,0.f,0.f,1.f));
	m_pPauseText = new Texture("Press ESC to Unpause", m_pFont, Color4f(0.f,0.f,0.f,1.f));
	m_pScoreText = new Texture("Score:", m_pFont, Color4f(0.f,0.f,0.f,1.f));
	m_pTimeText = new Texture("Time:", m_pFont, Color4f(0.f, 0.f, 0.f, 1.f));
	m_pHpText = new Texture("Hp:", m_pFont, Color4f(0.f, 0.f, 0.f, 1.f));
	m_pBestText = new Texture("Best", m_pFont, Color4f(0.f, 0.f, 0.f, 1.f));
	m_pInfoText = new Texture("Press I for Info", m_pFont, Color4f(0.f, 0.f, 0.f, 1.f));

    // debug indicators - initialize to empty, actual textures created after map setup
    m_LastRangeText.clear();
    m_LastMultiplierText.clear();
    m_pRangeText = nullptr;
    m_pMultiplierText = nullptr;

	m_pTitleScreen = new Texture("Resources/TitleScreen.png");
    m_pMap = new Map();
    m_pPlayer = new Player();
    m_pOverlay = new SpriteSheet(1, "Resources/Overlay.png", 1);
	m_pAttackManager = new AttackManager();
	m_pLetters = new SpriteSheet(36, "Resources/UIFont.png", 5);

	m_pSoundButtonPress = new SoundEffect("Resources/ButtonPress.wav");
	m_pSoundHit = new SoundEffect("Resources/Hit.wav");
	m_pSoundDebuff = new SoundEffect("Resources/Debuff.wav");
	m_pSoundBuff = new SoundEffect("Resources/Buff.wav");
	m_pSoundPointCollected = new SoundEffect("Resources/PointCollected.wav");
	m_pSoundPreparedTile = new SoundEffect("Resources/PreparedTile.wav");
	m_pSoundHeal = new SoundEffect("Resources/Heal.wav");

    m_pSoundButtonPress->SetVolume(50);
    m_pSoundPreparedTile->SetVolume(50);
    m_pSoundHit->SetVolume(40);
	m_pSoundDebuff->SetVolume(40);
	m_pSoundBuff->SetVolume(40);
    m_pSoundPointCollected->SetVolume(25);
	m_pSoundHeal->SetVolume(40);

    m_pMap->SetHexMode(false);
	m_pMap->SetBlindMode(false);
    m_pMap->SetWrapMode(false);
    m_pMap->GenerateMapRandom();

	m_MapIncreaseTimer = 0.f;
	m_AttackSpawnTime = 10.f;
	m_AttackTimer = 0;
	m_TotalTime = 0;
	m_Score = 0;
	m_PointsSpawned = false;

	m_MultiplierTimer = 0;
	m_Multiplier = 1.f;
	m_TimerStarted = false;
    m_vecDangerTiles.reserve(m_pMap->GetMaxDangerTiles());
    m_vecDebuffTiles.reserve(m_pMap->GetMaxDebuffTiles());
    m_vecBuffTiles.reserve(m_pMap->GetMaxBuffTiles());
    m_LastMaxPointTiles = m_pMap->GetMaxPointTiles();
    LoadBest();

    // create initial debug textures now that map exists
    m_LastRangeText = "Range:" + std::to_string(m_pMap->GetMinValue()) + "-" + std::to_string(m_pMap->GetMaxValue());
    char bufInit[64];
    snprintf(bufInit, sizeof(bufInit), "Mult: %.2f", m_Multiplier);
    m_LastMultiplierText = bufInit;
    Texture* t1 = new Texture(m_LastRangeText, m_pFont, Color4f(0.f, 0.f, 0.f, 1.f));
    if (t1->IsCreationOk()) m_pRangeText = t1; else { delete t1; m_pRangeText = nullptr; }
    Texture* t2 = new Texture(m_LastMultiplierText, m_pFont, Color4f(0.f, 0.f, 0.f, 1.f));
    if (t2->IsCreationOk()) m_pMultiplierText = t2; else { delete t2; m_pMultiplierText = nullptr; }

// create initial combined UI textures (label + value)
{
    char tmp[128];
    snprintf(tmp, sizeof(tmp), "Hp: %d", m_pPlayer->GetHp());
    m_LastHpCombined = tmp;
    Texture* hc = new Texture(m_LastHpCombined, m_pFont, Color4f(0.f,0.f,0.f,1.f));
    if (hc->IsCreationOk()) m_pHpCombined = hc; else { delete hc; m_pHpCombined = nullptr; }

    snprintf(tmp, sizeof(tmp), "Score: %d", m_Score);
    m_LastScoreCombined = tmp;
    Texture* sc = new Texture(m_LastScoreCombined, m_pFont, Color4f(0.f,0.f,0.f,1.f));
    if (sc->IsCreationOk()) m_pScoreCombined = sc; else { delete sc; m_pScoreCombined = nullptr; }

    snprintf(tmp, sizeof(tmp), "Time: %d", static_cast<int>(m_TotalTime));
    m_LastTimeCombined = tmp;
    Texture* tc = new Texture(m_LastTimeCombined, m_pFont, Color4f(0.f,0.f,0.f,1.f));
    if (tc->IsCreationOk()) m_pTimeCombined = tc; else { delete tc; m_pTimeCombined = nullptr; }

}

	m_OverlayTimer = 0;
	m_OverlayTimerMax = 1;
	m_OverlayFrame = 0;
    m_IsConfused = false;
    m_ConfusionTimer = 0.f;
    m_BuffSpawnTimer = m_BuffSpawnTimerMax;
    m_HealSpawnTimer = m_HealSpawnTimerMax;

}

void Game::Cleanup( )
{
    delete m_pMap;
    delete m_pPlayer;
    delete m_pOverlay;
    delete m_pAttackManager;
    delete m_pLetters;
	delete m_pTitleScreen;

    delete m_pSoundButtonPress;
    delete m_pSoundHit;
    delete m_pSoundDebuff;
    delete m_pSoundPointCollected;
	delete m_pSoundPreparedTile;
	delete m_pSoundBuff;
	delete m_pSoundHeal;

	delete m_pRestartText;
	delete m_pPauseText;
	delete m_pStartText;
	delete m_pScoreText;
	delete m_pTimeText;
	delete m_pHpText;
    delete m_pBestText;
	delete m_pInfoText;
	delete m_pRangeText;
	delete m_pMultiplierText;
    delete m_pScoreCombined;
    delete m_pTimeCombined;
    delete m_pHpCombined;
	TTF_CloseFont( m_pFont );
	m_vecDangerTiles.clear();
	m_vecDangerTiles.shrink_to_fit();
	m_vecDebuffTiles.clear();
	m_vecDebuffTiles.shrink_to_fit();

    m_vecBuffTiles.clear();
    m_vecBuffTiles.shrink_to_fit();
}

void Game::Update( float elapsedSec )
{
	// Check keyboard state
	//const Uint8 *pStates = SDL_GetKeyboardState( nullptr );
	//if ( pStates[SDL_SCANCODE_RIGHT] )
	//{
	//	std::cout << "Right arrow key is down\n";
	//}
	//if ( pStates[SDL_SCANCODE_LEFT] && pStates[SDL_SCANCODE_UP])
	//{
	//	std::cout << "Left and up arrow keys are down\n";
	//}
	switch (m_GameState)
	{
		case GameState::start:
		{
			break;
		}
		case GameState::gameplay:
		{
			if (m_OverlayTimer >= 0)
			{
				m_OverlayTimer -= elapsedSec;
			}
			else
			{
				m_OverlayFrame = 0;
			}
			m_MapIncreaseTimer += elapsedSec;
			if(m_Score >= 1)
			{
				if (m_Multiplier < 2.f)
				{
					m_Multiplier += elapsedSec / 100.f / 1.5f;
				}
				else
				{
					m_Multiplier += elapsedSec / 100.f / 2.f;
				}
				m_TotalTime += elapsedSec;
			}
			if(m_MultiplierTimer > 0) 
			{
				if (m_Easy)
				{

					m_MultiplierTimer -= elapsedSec / 2;
				}
				else
				{
					m_MultiplierTimer -= elapsedSec;
				}
			}
			else
			{
				if (m_TimerStarted)
				{
					m_pPlayer->Hit(3);
				}
				/*
				if (m_Multiplier != 1)
				{
					m_Multiplier = 1;
				}
				*/
			}
		

		// update debug textures when range or multiplier change
		{
			std::string rangeText = "Range:" + std::to_string(m_pMap->GetMinValue()) + "-" + std::to_string(m_pMap->GetMaxValue());
			if (rangeText != m_LastRangeText)
			{
				if (m_pRangeText) { delete m_pRangeText; m_pRangeText = nullptr; }
				Texture* t = new Texture(rangeText, m_pFont, Color4f(0.f,0.f,0.f,1.f));
				if (t->IsCreationOk()) m_pRangeText = t; else delete t;
				m_LastRangeText = rangeText;
			}

			char mbuf[64];
			snprintf(mbuf, sizeof(mbuf), "Mult: %.2f", m_Multiplier);
			std::string multText(mbuf);
			if (multText != m_LastMultiplierText)
			{
				if (m_pMultiplierText) { delete m_pMultiplierText; m_pMultiplierText = nullptr; }
				Texture* t = new Texture(multText, m_pFont, Color4f(0.f,0.f,0.f,1.f));
				if (t->IsCreationOk()) m_pMultiplierText = t; else delete t;
				m_LastMultiplierText = multText;
			}
	
            char buf[128];
            snprintf(buf, sizeof(buf), "Hp: %d", m_pPlayer->GetHp());
            std::string hcombined(buf);
            if (hcombined != m_LastHpCombined)
            {
                if (m_pHpCombined) { delete m_pHpCombined; m_pHpCombined = nullptr; }
                Texture* t = new Texture(hcombined, m_pFont, Color4f(0.f,0.f,0.f,1.f));
                if (t->IsCreationOk()) m_pHpCombined = t; else delete t;
                m_LastHpCombined = hcombined;
            }

            snprintf(buf, sizeof(buf), "Score: %d", m_Score);
            std::string scombined(buf);
            if (scombined != m_LastScoreCombined)
            {
                if (m_pScoreCombined) { delete m_pScoreCombined; m_pScoreCombined = nullptr; }
                Texture* t = new Texture(scombined, m_pFont, Color4f(0.f,0.f,0.f,1.f));
                if (t->IsCreationOk()) m_pScoreCombined = t; else delete t;
                m_LastScoreCombined = scombined;
            }

            snprintf(buf, sizeof(buf), "Time: %d", static_cast<int>(m_TotalTime));
            std::string tcombined(buf);
            if (tcombined != m_LastTimeCombined)
            {
                if (m_pTimeCombined) { delete m_pTimeCombined; m_pTimeCombined = nullptr; }
                Texture* t = new Texture(tcombined, m_pFont, Color4f(0.f,0.f,0.f,1.f));
                if (t->IsCreationOk()) m_pTimeCombined = t; else delete t;
                m_LastTimeCombined = tcombined;
            }
        }
			int curMaxPoints = m_pMap->GetMaxPointTiles();
			if (curMaxPoints > m_LastMaxPointTiles)
			{
				int need = curMaxPoints - m_LastMaxPointTiles;
				for (int i = 0; i < need; ++i)
				{
					m_pMap->CreateRandomPointTile(m_pPlayer->GetPosition());

				}
				m_LastMaxPointTiles = curMaxPoints;
			}
			if (m_TotalTime < 160)
			{
				if (m_pMap->GetMaxValue() < 36)
				{
					m_pMap->SetMaxValue(std::min(16 + static_cast<int>(m_TotalTime / 5), 36));
				}
			} 
			if (m_MapIncreaseTimer >= m_MapIncreaseInterval)
			{
				m_MapIncreaseTimer -= m_MapIncreaseInterval;

                bool isHex = m_pMap->IsHexMode();
				if (m_TotalTime > 160)
				{
					const int
						range { 16 + rand() % (36 - 16 + 1)},
						offset{ rand() % (36 - range + 1) },
						rows{ m_pMap->GetNumRows() };
					m_pMap->SetMaxValue(range + offset);
					m_pMap->SetMinValue(offset);
					const float
						cols{ 1.f * m_pMap->GetNumCols() },
						ratio{ 1.67f };
					if (m_Multiplier > 2.f)
					{
						if (cols / rows <= ratio)
						{
							m_pMap->IncreaseCols();
						}
						else
						{
							m_pMap->IncreaseRows();
						}
					}
					
				}

			}
			if ( !m_PointsSpawned)
			{
				m_PointsSpawned = true;
                m_pMap->CreateRandomPointTile(m_pPlayer->GetPosition());
                m_pMap->CreateRandomPointTile(m_pPlayer->GetPosition());
			}
                m_pAttackManager->Update(elapsedSec);
            // Confusion status timer
            if (m_ConfusionTimer > 0.f)
            {
                m_ConfusionTimer -= elapsedSec;
                if (m_ConfusionTimer <= 0.f)
                {
                    m_ConfusionTimer = 0.f;
                    m_IsConfused = false;
                }
            }
            // Update buff/debuff state on player and map
            // Player owns unified buff timer and debuff timer
            // Query player buff and enable/disable map modes accordingly
            if (m_pPlayer->HasBuff(Player::BuffType::hex))
            {
                m_pMap->SetHexMode(true);
            }
            else
            {
                m_pMap->SetHexMode(false);
            }

            if (m_pPlayer->HasBuff(Player::BuffType::wrap))
            {
                m_pMap->SetWrapMode(true);
            }
            else
            {
                m_pMap->SetWrapMode(false);
            }

            if (m_pPlayer->HasBuff(Player::BuffType::reveal))
            {
                m_pMap->SetRevealedMode(true);
            }
            else
            {
                m_pMap->SetRevealedMode(false);
            }

            // apply debuff map effects (blindness) while player debuffed
            if (m_pPlayer->IsDebuffed())
            {
                m_pMap->SetBlindMode(true);
            }
            else
            {
                m_pMap->SetBlindMode(false);
            }



            m_pPlayer->Update(elapsedSec);
            // Buff/Debuff spawn timers (replace existing ones if needed)
            // helper: find first tile with given state (used by buff and heal logic)
            auto FindStatePos = [&](Tile::State want, Vector2i& outPos) -> bool
            {
                const int cols = static_cast<int>(m_pMap->GetWidth() / m_pMap->GetTileSize());
                const int rows = static_cast<int>(m_pMap->GetHeight() / m_pMap->GetTileSize());
                if (cols <= 0 || rows <= 0) return false;
                for (int y = 0; y < rows; ++y)
                {
                    for (int x = 0; x < cols; ++x)
                    {
                        Vector2i p(x,y);
                        if (m_pMap->GetTileState(p) == want)
                        {
                            outPos = p;
                            return true;
                        }
                    }
                }
                return false;
            };

            if (m_TimerStarted)
            {
                m_BuffSpawnTimer -= elapsedSec;

                auto PlaceReplacement = [&](Tile::State kind)
                {
                    // If an existing tile of this kind exists, clear it first
                    Vector2i existing;
                    if (FindStatePos(kind, existing))
                    {
                        m_pMap->SetTileState(existing, Tile::State::normal);
                    }

                    // Try to pick a tile at distance using a function that does NOT
                    // create transient point tiles for non-point modifiers
                    Vector2i spawned = m_pMap->FindRandomNormalTile(m_pPlayer->GetPosition());
                    const Vector2i playerPos = m_pPlayer->GetPosition();
                    if (spawned == playerPos)
                    {
                        // fallback: choose any normal/preparing tile not under player
                        const int cols = static_cast<int>(m_pMap->GetWidth() / m_pMap->GetTileSize());
                        const int rows = static_cast<int>(m_pMap->GetHeight() / m_pMap->GetTileSize());
                        std::vector<Vector2i> candidates;
                        for (int y = 0; y < rows; ++y)
                        {
                            for (int x = 0; x < cols; ++x)
                            {
                                Vector2i p(x,y);
                                if (p == playerPos) continue;
                                Tile::State st = m_pMap->GetTileState(p);
                                if (st == Tile::State::normal || st == Tile::State::preparing)
                                    candidates.push_back(p);
                            }
                        }
                        if (!candidates.empty())
                        {
                            Vector2i pick = candidates[rand() % static_cast<int>(candidates.size())];
                            m_pMap->SetTileState(pick, kind);
                        }
                    }
                    else
                    {
                        m_pMap->SetTileState(spawned, kind);
                    }
                };

                if (m_BuffSpawnTimer <= 0.f)
                {
                    m_BuffSpawnTimer = m_BuffSpawnTimerMax;
                    // spawn up to max buff tiles, replacing old ones if necessary
                    int want = m_pMap->GetMaxBuffTiles();
                    // clear existing buff tiles on map
				for (const Vector2i &p : m_vecBuffTiles) m_pMap->RemoveTileModifier(p);
				m_vecBuffTiles.clear();
                    // Place up to 'want' buff tiles
                    for (int i = 0; i < want; ++i)
                    {
                        Vector2i spawned = m_pMap->FindRandomNormalTile(m_pPlayer->GetPosition());
                        if (spawned == m_pPlayer->GetPosition())
                        {
                            // fallback to any available tile
                            const int cols = static_cast<int>(m_pMap->GetWidth() / m_pMap->GetTileSize());
                            const int rows = static_cast<int>(m_pMap->GetHeight() / m_pMap->GetTileSize());
                            std::vector<Vector2i> candidates;
                            for (int y = 0; y < rows; ++y)
                            {
                                for (int x = 0; x < cols; ++x)
                                {
                                    Vector2i p(x,y);
                                    if (p == m_pPlayer->GetPosition()) continue;
                                    Tile::State st = m_pMap->GetTileState(p);
                                    if (st == Tile::State::normal || st == Tile::State::preparing)
                                        candidates.push_back(p);
                                }
                            }
                            if (!candidates.empty()) spawned = candidates[rand() % static_cast<int>(candidates.size())];
                        }
                        m_pMap->SetTileState(spawned, Tile::State::buff);
                        m_vecBuffTiles.push_back(spawned);
                    }
                }
            }
            else
            {
                // ensure timers are reset until gameplay starts
                m_BuffSpawnTimer = m_BuffSpawnTimerMax;
                
            }
            // Heal spawn logic: after first point picked up spawn a heal on the edge every interval
            if (m_TimerStarted)
            {
                m_HealSpawnTimer -= elapsedSec;
                if (m_HealSpawnTimer <= 0.f)
                {
                    m_HealSpawnTimer = m_HealSpawnTimerMax;
                    // Do not spawn if a heal tile already exists
                    Vector2i existingHeal;
                    if (!FindStatePos(Tile::State::heal, existingHeal))
                    {
                        const int cols = static_cast<int>(m_pMap->GetWidth() / m_pMap->GetTileSize());
                        const int rows = static_cast<int>(m_pMap->GetHeight() / m_pMap->GetTileSize());
                        const Vector2i playerPos = m_pPlayer->GetPosition();
                        std::vector<Vector2i> edgeCandidates;
                        if (cols > 0 && rows > 0)
                        {
                            // top and bottom rows
                            for (int x = 0; x < cols; ++x)
                            {
                                Vector2i top(x, 0);
                                Vector2i bottom(x, rows - 1);
                                if (!(top == playerPos))
                                {
                                    Tile::State st = m_pMap->GetTileState(top);
                                    if (st == Tile::State::normal || st == Tile::State::preparing) edgeCandidates.push_back(top);
                                }
                                if (!(bottom == playerPos) && rows > 1)
                                {
                                    Tile::State st = m_pMap->GetTileState(bottom);
                                    if (st == Tile::State::normal || st == Tile::State::preparing) edgeCandidates.push_back(bottom);
                                }
                            }
                            // left and right columns (skip corners already added)
                            for (int y = 1; y < rows - 1; ++y)
                            {
                                Vector2i left(0, y);
                                Vector2i right(cols - 1, y);
                                if (!(left == playerPos))
                                {
                                    Tile::State st = m_pMap->GetTileState(left);
                                    if (st == Tile::State::normal || st == Tile::State::preparing) edgeCandidates.push_back(left);
                                }
                                if (!(right == playerPos) && cols > 1)
                                {
                                    Tile::State st = m_pMap->GetTileState(right);
                                    if (st == Tile::State::normal || st == Tile::State::preparing) edgeCandidates.push_back(right);
                                }
                            }
                        }
                        if (!edgeCandidates.empty())
                        {
                            Vector2i pick = edgeCandidates[rand() % static_cast<int>(edgeCandidates.size())];
                            m_pMap->SetTileState(pick, Tile::State::heal);
                        }
                    }
                }
            }
            if (m_pAttackManager->IsColliding(m_pPlayer->GetBounds(m_pMap->GetTileSize(), m_pMap->IsHexMode()), m_pPlayer->GetDirection()))
			{
                m_pPlayer->Hit(1);
                m_pSoundHit->Play(0);
			}
			m_LastRangeText = "Range:" + std::to_string(m_pMap->GetMinValue()) + "-" + std::to_string(m_pMap->GetMaxValue());
			delete m_pMultiplierText;
			delete m_pRangeText;
			char bufInit[64];
			snprintf(bufInit, sizeof(bufInit), "Mult: %.2f", m_Multiplier);
			m_LastMultiplierText = bufInit;
			Texture* t1 = new Texture(m_LastRangeText, m_pFont, Color4f(0.f, 0.f, 0.f, 1.f));
			if (t1->IsCreationOk()) m_pRangeText = t1; else { delete t1; m_pRangeText = nullptr; }
			Texture* t2 = new Texture(m_LastMultiplierText, m_pFont, Color4f(0.f, 0.f, 0.f, 1.f));
			if (t2->IsCreationOk()) m_pMultiplierText = t2; else { delete t2; m_pMultiplierText = nullptr; }
			if (m_pPlayer->GetHp() <= 0)
			{
				m_GameState = GameState::end;
			}

			// Clear player's transient move direction so it only affects collisions in the frame the player moved.
			m_pPlayer->SetDirection(Vector2i(0,0));
			break;
		}
		case GameState::paused:
		{
			
		}
			break;
		case GameState::end:
		{
			// Do not advance gameplay timers in end state. Keep player static.
			break;
		}
		
	}


	
}

void Game::Draw() const
{
	ClearBackground();
	Rectf viewPort = GetViewPort();

	// UI anchored to a fixed logical map size so UI doesn't move when map scales
	static const float s_BaseMapWidth = 16.f * 10.f; // base cols=10, tile=16
	static const float s_BaseMapHeight = 16.f * 6.f; // base rows=6, tile=16

	glPushMatrix();
	{
		glTranslatef(viewPort.width / 2, viewPort.height / 2, 0.f);
		glScalef(8.f, 8.f, 1.f);

		glPushMatrix();
		{
            glTranslatef(-m_pMap->GetWidth() / 2, -m_pMap->GetHeight() / 2, 0.f);
            Vector2i playerTile = m_pPlayer->GetPosition();
            m_pMap->Draw(Vector2f(0.f, 0.f), &playerTile);
			const float
				tileSize{m_pMap->GetTileSize()};
			if (!m_pMap->IsBlindMode())
			{
				for (int i{ 0 }; i < m_vecDangerTiles.size(); i++)
				{
					const Vector2i position{ m_vecDangerTiles [i]};
					utils::SetColor(Color4f(1.f, 0.f, 0.f, 1.f));
					for (int j{ 0 }; j < i; j++)
					{
					const float
						x{ 1.f / (10.f / (10.f / m_pMap->GetScale() / m_pMap->GetScale()))},
						w{ x * j * 3 / 2 + x },
						mw{ tileSize / 2 - (x * i * 3 / 2 + x)/2 + (position.y%2 * tileSize/2 * m_pMap->IsHexMode()) },
						y{ 1.f };
						utils::FillRect( position.x * tileSize + w + mw, position.y*tileSize + y, x, y);

					}
				}
				for (int i{ 0 }; i < m_vecDebuffTiles.size(); i++)
				{
					const Vector2i position{ m_vecDebuffTiles[i] };
					utils::SetColor(Color4f(0.3f, 0.f, 0.6f, 1.f));
					for (int j{ 0 }; j < i; j++)
					{
						const float
							x{ 1.f / (10.f / (10.f / m_pMap->GetScale() / m_pMap->GetScale())) },
							w{ x * j * 3 / 2 + x },
							mw{ tileSize / 2 - (x * i * 3 / 2 + x) / 2 + (position.y % 2 * tileSize / 2 * m_pMap->IsHexMode()) },
							y{ 1.f };
						utils::FillRect(position.x * tileSize + w + mw, position.y * tileSize + y, x, y);

					}
				}
			}
            m_pPlayer->Draw(m_pMap->GetTileSize(), m_pMap->IsHexMode());

            m_pAttackManager->Draw();
		}
		glPopMatrix();
		glPushMatrix();
		{
			glScalef(0.5f, 0.5f, 1.f);
			glTranslatef(-m_pOverlay->GetSpriteWidth()/2, -m_pOverlay->GetSpriteHeight()/2,0.f);
			m_pOverlay->DrawSprite(Vector2f(0,0), 0, m_OverlayFrame);
		}
		glPopMatrix();
		// draw debug indicators top-left
		{
			// compute positions relative to map
			float left = -s_BaseMapWidth / 2 - 42.f;
			float top = -s_BaseMapHeight / 2 + 40.f;
		glPushMatrix();
		{
			glScalef(0.4f, 0.4f, 1.f);
			glTranslatef(-170.f, 100.f,0.f);
			// range text
			if (m_pRangeText)
			{
				m_pRangeText->Draw(Vector2f(left, top - 66.f));
			}
			// multiplier under it
			if (m_pMultiplierText)
			{
				m_pMultiplierText->Draw(Vector2f(left, top - 12.f));
			}

            // anchored UI labels and values: draw label first then value to ensure labels show
            // HP
            if (m_pHpCombined)
            {
				m_pHpCombined->Draw(Vector2f(left , top - 30.f));
            }

            // Score
            if (m_pScoreCombined)
            {
                m_pScoreCombined->Draw(Vector2f(left, top + 6));
            }

            // Time
            if (m_pTimeCombined)
            {
                m_pTimeCombined->Draw(Vector2f(left, top - 48.f));
            }

		}
		glPopMatrix();
		}
        const int playerHp{ m_pPlayer->GetHp() };
        const float y{ s_BaseMapHeight / 2 + m_pHpText->GetHeight() + 5.f };

        // UI drawing for HP/Score/Time moved to the debug UI block so position/size match Range/Multiplier

		if (m_Easy)
		{
			glPushMatrix();
			{
				glScalef(0.8f, 0.8f, 1.f);
				utils::SetColor(Color4f(0.f, 1.f, 0.f, 1.f));
				utils::FillEllipse(Vector2f(120, 73.5), 8, 8);
			}
			glPopMatrix();
		}


		switch (m_GameState)
		{
			case GameState::start:
			{
				utils::SetColor(Color4f(0.3f, 0.3f, 0.3f, 0.6f));
				utils::FillRect(-250, -250, 500, 500);
				m_pStartText->Draw(Vector2f(-m_pStartText->GetWidth() / 2, -m_pStartText->GetHeight() / 2 + 60.f));
				glPushMatrix();
				{
					glScalef(0.63f/2.f, 0.63f/2.f, 1.f);
					m_pTitleScreen->Draw(Vector2f(-m_pTitleScreen->GetWidth() / 2, -m_pTitleScreen->GetHeight() / 2));
				}
				glPopMatrix();
				break;
			}
			case GameState::gameplay:
			{
				glPushMatrix();
				{
					glScalef(0.8f, 0.8f, 1.f);
					utils::SetColor(Color4f(1.f, 2.f/m_Multiplier, 0.f, 1.f));
					const float
						x{0},
						yPos{-GetViewPort().height/16 - 10.f},
						width{m_MultiplierTimer * 30 / 2 - 5.f},
						height{8};
					utils::FillRect(x-width / 2 , yPos, width, height);
				}
				glPopMatrix();
				break;
			}
			case GameState::paused:
			{
				utils::SetColor(Color4f(0.5f, 0.5f, 0.5f, 0.6f));
				utils::FillRect(-250, -250, 500, 500);
				m_pPauseText->Draw(Vector2f(-m_pPauseText->GetWidth()/2,0));
				glPushMatrix();
				{

					glScalef(0.5f, 0.5f, 1.f);
					m_pInfoText->Draw(Vector2f(-m_pPauseText->GetWidth() / 2, -m_pInfoText->GetHeight()));

					//BestScore
					m_pBestText->Draw(Vector2f(-m_pScoreText->GetWidth() / 2 - 30.f - m_pScoreText->GetWidth() / 2 - m_pBestText->GetWidth() / 2 - 23.f, -50 - y - 13.f + 5.f));
					m_pScoreText->Draw(Vector2f(-m_pScoreText->GetWidth() / 2 - 30.f - 23.f, -50 - y - 13.f + 5.f));
					m_pLetters->DrawSprite(Vector2f(-m_pScoreText->GetWidth() / 2 + 30.f - 23.f, -50 - 1 * y - 10.f + 5.f), 26 + static_cast<int>(m_BestScore) % 10);
					m_pLetters->DrawSprite(Vector2f(-m_pScoreText->GetWidth() / 2 + 20.f - 23.f, -50 - 1 * y - 10.f + 5.f), 26 + (static_cast<int>(m_BestScore) / 10 % 10));
					m_pLetters->DrawSprite(Vector2f(-m_pScoreText->GetWidth() / 2 + 10.f - 23.f, -50 - 1 * y - 10.f + 5.f), 26 + (static_cast<int>(m_BestScore) / 100 % 10));


					//BestTime
					m_pBestText->Draw(Vector2f(10.f + m_pMap->GetWidth() / 2 - m_pTimeText->GetWidth() / 2 - 27.f - m_pTimeText->GetWidth(), -50 - y - 13.f + 5.f));
					m_pTimeText->Draw(Vector2f(10.f + m_pMap->GetWidth() / 2 - m_pTimeText->GetWidth() / 2 - 27.f, -50 - y - 13.f + 5.f));
					m_pLetters->DrawSprite(Vector2f(10.f + m_pMap->GetWidth() / 2 + 10.f, -50 - 1 * y - 10.f + 5.f), 26 + static_cast<int>(m_BestTime) % 10);
					m_pLetters->DrawSprite(Vector2f(10.f + m_pMap->GetWidth() / 2 + 0.f, -50 - 1 * y - 10.f + 5.f), 26 + (static_cast<int>(m_BestTime) / 10 % 10));
					m_pLetters->DrawSprite(Vector2f(10.f + m_pMap->GetWidth() / 2 - 10.f, -50 - 1 * y - 10.f + 5.f), 26 + (static_cast<int>(m_BestTime) / 100 % 10));

				}
				glPopMatrix();
				break;
			}
			case GameState::info:
			{
				glPushMatrix();
				{
					glScalef(0.5f,0.5f, 1.f);
					m_pTitleScreen->Draw(Vector2f(-m_pTitleScreen->GetWidth()/2, -m_pTitleScreen->GetHeight() / 2));
				}
				glPopMatrix();
				break;
			}
			case GameState::end:
			{
				utils::SetColor(Color4f(0.8f, 0.8f, 0.8f, 0.6f));
				utils::FillRect(-250, -250, 500, 500);
				m_pRestartText->Draw(Vector2f(-m_pRestartText->GetWidth()/2, -m_pRestartText->GetHeight() / 2));
				break;
			}
		}
	}
	glPopMatrix();

}

void Game::ProcessKeyDownEvent(const SDL_KeyboardEvent& e)
{

    switch (m_GameState)
    {
    case GameState::start:
    {
        // Use layout-aware keycode so AZERTY 's' works
        if (e.keysym.sym == SDLK_s)
        {
            m_GameState = GameState::gameplay;
            break;
        }
    }
    case GameState::gameplay:
    {
        // Prefer layout-aware keycode, fallback to scancode when needed
        SDL_Keycode key = e.keysym.sym;
        SDL_Scancode sc = e.keysym.scancode;

        if (key == SDLK_ESCAPE)
        {
            m_GameState = GameState::paused;
            // Hide player vision while paused
            m_pMap->SetZeroVisionDuringPause(true);
            break;
        }
      


        int value = -1;
        if (key >= SDLK_a && key <= SDLK_z)
        {
            value = static_cast<int>(key - SDLK_a); // a..z -> 0..25
        }
        else if (key >= SDLK_0 && key <= SDLK_9)
        {
            value = 26 + static_cast<int>(key - SDLK_0); // 0..9 -> 26..35
        }
        else if (key >= SDLK_KP_0 && key <= SDLK_KP_9)
        {
            value = 26 + static_cast<int>(key - SDLK_KP_0);
        }

        // Fallback: use physical scancode mapping
        if (value < 0)
        {
            if (sc >= SDL_SCANCODE_A && sc <= SDL_SCANCODE_Z)
            {
                value = static_cast<int>(sc - SDL_SCANCODE_A);
            }
            else if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_9)
            {
                value = 26 + static_cast<int>(sc - SDL_SCANCODE_1); // 1..9 -> 26..34
            }
            else if (sc == SDL_SCANCODE_0)
            {
                value = 26 + 9; // '0' -> 35
            }
            else if (sc >= SDL_SCANCODE_KP_1 && sc <= SDL_SCANCODE_KP_9)
            {
                value = 26 + static_cast<int>(sc - SDL_SCANCODE_KP_1 + 1);
            }
            else if (sc == SDL_SCANCODE_KP_0)
            {
                value = 26 + 0; // map keypad 0 to '0' mapping base
            }
        }

        if (value < 0) break; // unmapped key

        Vector2i movement{ m_pMap->GetAdjecentTileDirection(m_pPlayer->GetPosition(), value) };
        if (movement != Vector2i(0, 0))
        {
            // If confused, invert intended movement
            if (m_IsConfused)
            {
                movement.x = -movement.x;
                movement.y = -movement.y;
            }
            m_pPlayer->Move(movement);
            // Wrap player around map edges (if enabled)
            if (m_pMap->IsWrapMode())
            {
                int cols = static_cast<int>(m_pMap->GetWidth() / m_pMap->GetTileSize());
                int rows = static_cast<int>(m_pMap->GetHeight() / m_pMap->GetTileSize());
                Vector2i pos = m_pPlayer->GetPosition();
                if (pos.x < 0) pos.x = cols - 1;
                else if (pos.x >= cols) pos.x = 0;
                if (pos.y < 0) pos.y = rows - 1;
                else if (pos.y >= rows) pos.y = 0;
                m_pPlayer->SetPosition(pos);
            }
            else
            {
                // When wrapping is disabled, clamp player inside map bounds
                int cols = static_cast<int>(m_pMap->GetWidth() / m_pMap->GetTileSize());
                int rows = static_cast<int>(m_pMap->GetHeight() / m_pMap->GetTileSize());
                if (cols <= 0) cols = 1;
                if (rows <= 0) rows = 1;
                Vector2i pos = m_pPlayer->GetPosition();
                if (pos.x < 0) pos.x = 0;
                else if (pos.x >= cols) pos.x = cols - 1;
                if (pos.y < 0) pos.y = 0;
                else if (pos.y >= rows) pos.y = rows - 1;
                m_pPlayer->SetPosition(pos);
            }
			switch (m_pMap->GetTileState(m_pPlayer->GetPosition()))
			{
				case(Tile::State::point):
				{
					m_OverlayFrame = 1;
					m_OverlayTimer = m_OverlayTimerMax;
					m_pSoundPointCollected->Play(0);
					if (!m_TimerStarted)
					{
						m_TimerStarted = true;
					}
					m_Score += m_Multiplier;
					Vector2i length = m_pMap->CreateRandomPointTile(m_pPlayer->GetPosition());

					float dist = Vector2f(m_pPlayer->GetPosition().x - length.x, m_pPlayer->GetPosition().y - length.y).Length();
					m_MultiplierTimer += dist / m_Multiplier;
					if (m_MultiplierTimer > 10)
					{
						m_MultiplierTimer = 10;
					}
				// Clear point explicitly via ClearPointAt so it cannot be removed by other cleanup paths
				m_pMap->ClearPointAt(m_pPlayer->GetPosition());
					break;
				}
				case(Tile::State::heal):
				{
					// consume heal
					m_pSoundHeal->Play(0);
					m_pPlayer->Heal(1);
					m_pMap->SetTileState(m_pPlayer->GetPosition(), Tile::State::normal);
					break;
				}
				case(Tile::State::debuff):
				{
					m_pSoundDebuff->Play(0);

				// apply debuff to player (player owns debuff timer)
				if (rand() % 2 == 0)
				{
					m_pPlayer->ApplyDebuff(m_StatusTimerMax);
					//m_pAttackManager->SpawnAlteratingAttack(1, m_pMap->GetTileSize() * 2, Vector2f(1, 0).Normalized(), m_pMap->GetWidth(), m_pMap->GetHeight(), false);
				}
				else
				{
					m_pPlayer->ApplyDebuff(m_StatusTimerMax);
					//m_pAttackManager->SpawnAlteratingAttack(1, m_pMap->GetTileSize() * 2, Vector2f(1, 0).Normalized(), m_pMap->GetWidth(), m_pMap->GetHeight(), false);

				}


					/*else
					{
						m_IsConfused = true;
						m_ConfusionTimer = m_StatusTimerMax;
					}*/
					// remove from debuff list if present
					for (int i{ 0 }; i < m_vecDebuffTiles.size(); ++i)
					{
						if (m_pPlayer->GetPosition() == m_vecDebuffTiles[i])
						{
							m_vecDebuffTiles.erase(m_vecDebuffTiles.begin() + i);
							break;
						}
					}
					m_pMap->SetTileState(m_pPlayer->GetPosition(), Tile::State::normal);
					break;
				}
				case(Tile::State::buff):
				{
					m_pSoundBuff->Play(0);
					switch (rand() % 3)
					{
						case(0):
						{
						m_pPlayer->ApplyBuff(Player::BuffType::hex, m_StatusTimerMax);
							break;
						}
						case(1):
						{
						m_pPlayer->ApplyBuff(Player::BuffType::wrap, m_StatusTimerMax);
							break;
						}
						case(2):
						{
						m_pPlayer->ApplyBuff(Player::BuffType::reveal, m_StatusTimerMax);
							break;
						}
					}
					m_pMap->SetTileState(m_pPlayer->GetPosition(), Tile::State::normal);
					break;
				}
				case(Tile::State::normal):
				{
					 m_pSoundButtonPress->Play(0);
					m_pMap->SetTileState(m_pPlayer->GetPosition(), Tile::State::preparing);
					break;
				}
				case(Tile::State::preparing):
				{
				m_pSoundPreparedTile->Play(0);
				// 1 in 10 chance this preparing tile becomes a debuff instead of danger
				{
					Vector2i pos = m_pPlayer->GetPosition();
                    if ((rand() % 7) == 0)
                    {
                        m_pMap->SetTileState(pos, Tile::State::debuff);
                        m_vecDebuffTiles.push_back(pos);
           
                    }
                    else
                    {
                        m_pMap->SetTileState(pos, Tile::State::danger);
                        m_vecDangerTiles.push_back(pos);
                    }
				}
					break;
				}
				case(Tile::State::danger):
				{
					m_OverlayFrame = 2;
					m_OverlayTimer = m_OverlayTimerMax;
					m_pSoundHit->Play(0);
					m_pPlayer->Hit(1);
					for(int i{ 0 }; i < m_vecDangerTiles.size(); i++ )
					{
						if (m_pPlayer->GetPosition() == m_vecDangerTiles[i])
						{
							m_vecDangerTiles.erase(m_vecDangerTiles.begin() + i);
							break;
						}
					}
					m_pMap->SetTileState(m_pPlayer->GetPosition(), Tile::State::normal);
					break;
				}
				default:
				{
					 m_pSoundButtonPress->Play(0);
				}
			}
			if (m_vecDangerTiles.size() >= static_cast<int>(11 * m_pMap->GetScale()))
			{
				m_pMap->RemoveTileModifier(m_vecDangerTiles[0]);
				m_vecDangerTiles.erase(m_vecDangerTiles.begin());
			}
			if (m_vecDebuffTiles.size() >= static_cast<int>(4 * m_pMap->GetScale()))
			{
				m_pMap->RemoveTileModifier(m_vecDebuffTiles[0]);
				m_vecDebuffTiles.erase(m_vecDebuffTiles.begin());
			}
        }
        break;
    }
    case GameState::paused:
    {

		// Toggle hex mode (H) and wrapping (Z) at runtime
		switch (e.keysym.sym)
		{
			case(SDLK_F1):
			{
				m_pPlayer->ApplyBuff(Player::BuffType::hex, m_StatusTimerMax);
				break;
			}
			case(SDLK_F2):
			{
				m_pPlayer->ApplyBuff(Player::BuffType::wrap, m_StatusTimerMax);
				break;
			}
			case(SDLK_F3):
			{
				m_pPlayer->ApplyBuff(Player::BuffType::reveal, m_StatusTimerMax);
				break;
			}
			case(SDLK_F4):
			{
				m_pPlayer->ClearBuff();
				break;
			}
			case(SDLK_F5):
			{
				m_IsConfused = !m_IsConfused;
				break;
			}
			case(SDLK_F6):
			{
				m_pPlayer->ApplyDebuff(m_StatusTimerMax);
				break;
			}
			case(SDLK_F7):
			{
				m_pPlayer->ApplyDebuff(0.f);
				break;
			}
			case(SDLK_i):
			{
				m_GameState = GameState::info;
				break;
			}
			case(SDLK_f):
			{
				if (m_Easy)
				{
					m_Easy = false;
				}
				else
				{
					m_Easy = true;
				}
				break;
			}
			case(SDLK_ESCAPE):
			{
                m_GameState = GameState::gameplay;
                // restore vision when unpausing
                m_pMap->SetZeroVisionDuringPause(false);
				break;
			}
		}
		break;
    }
	case GameState::info:
	{
		switch (e.keysym.sym)
		{
			case(SDLK_i):
			{
				m_GameState = GameState::paused;
				break;
			}
		}
		break;
	}
    case GameState::end:
    {
        if (e.keysym.sym == SDLK_r)
        {
            SaveBest();
            Cleanup();
            Initialize();
            m_GameState = GameState::gameplay;
            break;
        }
        break;
    }
    }

}


void Game::ProcessKeyUpEvent( const SDL_KeyboardEvent& e )
{
	//std::cout << "KEYUP event: " << e.keysym.sym << std::endl;
	//switch ( e.keysym.sym )
	//{
	//case SDLK_LEFT:
	//	//std::cout << "Left arrow key released\n";
	//	break;
	//case SDLK_RIGHT:
	//	//std::cout << "`Right arrow key released\n";
	//	break;
	//case SDLK_1:
	//case SDLK_KP_1:
	//	//std::cout << "Key 1 released\n";
	//	break;
	//}
 }

void Game::LoadBest()
{
    std::string filePath = "save.txt";
    std::ifstream inFile(filePath);
    if (inFile.is_open())
    {
        int bestScoreFile = 0;
        int bestTimeFile = 0;
        if (inFile >> bestScoreFile >> bestTimeFile)
        {
            m_BestScore = static_cast<float>(bestScoreFile);
            m_BestTime = static_cast<float>(bestTimeFile);
        }
    }

}

void Game::ProcessMouseMotionEvent( const SDL_MouseMotionEvent& e )
{
	//std::cout << "MOUSEMOTION event: " << e.x << ", " << e.y << std::endl;
}

void Game::ProcessMouseDownEvent( const SDL_MouseButtonEvent& e )
{
	switch ( e.button )
	{
	case SDL_BUTTON_LEFT:
		//m_pMap->IncreaseCols(1);
		break;
	case SDL_BUTTON_RIGHT:
		//m_pMap->IncreaseRows(1);
		break;
	case SDL_BUTTON_MIDDLE:
		break;
	}


}

void Game::ProcessMouseUpEvent( const SDL_MouseButtonEvent& e )
{
	//std::cout << "MOUSEBUTTONUP event: ";
	//switch ( e.button )
	//{
	//case SDL_BUTTON_LEFT:
	//	std::cout << " left button " << std::endl;
	//	break;
	//case SDL_BUTTON_RIGHT:
	//	std::cout << " right button " << std::endl;
	//	break;
	//case SDL_BUTTON_MIDDLE:
	//	std::cout << " middle button " << std::endl;
	//	break;
	//}
}

void Game::ClearBackground( ) const
{
	glClearColor( 116.f/255.f, 116.f/255.f, 116.f/255.f, 1.0f );
	/*const float
		r{ (m_Multiplier - static_cast<int>(m_Multiplier)) },
		g{ (m_Multiplier - static_cast<int>(m_Multiplier))/2.f },
		b{ (m_Multiplier - static_cast<int>(m_Multiplier))/3.f };
	glClearColor( r, g, b, 1.0f );*/
	glClear( GL_COLOR_BUFFER_BIT );
}


void Game::SaveBest()
{
    std::string filePath = "save.txt";
    std::ofstream outFile(filePath);
    if (outFile.is_open())
    {
        if (m_Score > m_BestScore) m_BestScore = static_cast<float>(m_Score);
        if (m_TotalTime > m_BestTime) m_BestTime = m_TotalTime;
        outFile << m_BestScore << " " << m_BestTime;
        outFile.close();
    }
}
