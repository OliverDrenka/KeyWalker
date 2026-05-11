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

	m_pTitleScreen = new Texture("Resources/TitleScreen.png");
    m_pMap = new Map();
    m_pPlayer = new Player();
    m_pOverlay = new SpriteSheet(1, "Resources/Overlay.png", 1);
	m_pAttackManager = new AttackManager();
	m_pLetters = new SpriteSheet(36, "Resources/UIFont.png", 5);

	m_pSoundButtonPress = new SoundEffect("Resources/ButtonPress.wav");
	m_pSoundHit = new SoundEffect("Resources/Hit.wav");
	m_pSoundDebuff = new SoundEffect("Resources/Debuff.wav");
	m_pSoundPointCollected = new SoundEffect("Resources/PointCollected.wav");
	m_pSoundPreparedTile = new SoundEffect("Resources/PreparedTile.wav");

    m_pSoundButtonPress->SetVolume(50);
    m_pSoundPreparedTile->SetVolume(50);
    m_pSoundHit->SetVolume(40);
	m_pSoundDebuff->SetVolume(40);
    m_pSoundPointCollected->SetVolume(25);

    m_pMap->SetHexMode(false);
	m_pMap->SetBlindMode(false);
    m_pMap->SetWrapMode(false);
    m_pMap->GenerateMapRandom();

	m_AttackSpawnTime = 10.f;
	m_AttackTimer = 0;
	m_TotalTime = 0;
	m_Score = 0;
	m_PointsSpawned = false;

	m_MultiplierTimer = 0;
	m_Multiplier = 1.f;
	m_TimerStarted = false;
	m_vecDangerTiles.reserve(static_cast<int>(11 * m_pMap->GetScale()));
    m_vecDebuffTiles.reserve(3);
    LoadBest();

	m_OverlayTimer = 0;
	m_OverlayTimerMax = 1;
	m_OverlayFrame = 0;
    m_IsConfused = false;
    m_ConfusionTimer = 0.f;
    m_BuffSpawnTimer = m_BuffSpawnTimerMax;

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

	delete m_pRestartText;
	delete m_pPauseText;
	delete m_pStartText;
	delete m_pScoreText;
	delete m_pTimeText;
	delete m_pHpText;
    delete m_pBestText;
	delete m_pInfoText;
	TTF_CloseFont( m_pFont );
	m_vecDangerTiles.clear();
	m_vecDangerTiles.shrink_to_fit();
	m_vecDebuffTiles.clear();
	m_vecDebuffTiles.shrink_to_fit();
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
			m_AttackTimer += elapsedSec;
			if(m_Score >= 1)
			{
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
			if (m_pMap->GetMaxValue() < 36 )
			{
				m_pMap->SetMaxValue(20 + static_cast<int>(m_TotalTime/5));
			}
			if (m_AttackTimer >= 4.f) 
			{
                m_AttackTimer -= m_AttackSpawnTime;
				if (m_AttackSpawnTime > 5.f)
				{
					m_AttackSpawnTime -= 0.5f;
				}
				m_Multiplier += 0.1f;
                bool isHex = m_pMap->IsHexMode();
				/*
               // m_pAttackManager->SpawnAlteratingAttack(1, m_pMap->GetTileSize() * 2, Vector2f(0.5f, 1).Normalized(), m_pMap->GetWidth(), m_pMap->GetHeight(), isHex);
               // m_pAttackManager->SpawnAlteratingAttack(1, m_pMap->GetTileSize() * 2, Vector2f(-0.5f, -1).Normalized(), m_pMap->GetWidth(), m_pMap->GetHeight(), isHex);
                // Spawn attacks for two principal axes; AttackManager will also spawn from the opposite
                m_pAttackManager->SpawnAlteratingAttack(1, m_pMap->GetTileSize() * 2, Vector2f(1, 0).Normalized(), m_pMap->GetWidth(), m_pMap->GetHeight(), isHex);
				if (isHex)
				{

					m_pAttackManager->SpawnAlteratingAttack(1, m_pMap->GetTileSize() * 2, Vector2f(0.5f, 1).Normalized(), m_pMap->GetWidth(), m_pMap->GetHeight(), isHex);
					m_pAttackManager->SpawnAlteratingAttack(1, m_pMap->GetTileSize() * 2, Vector2f(-0.5f, 1).Normalized(), m_pMap->GetWidth(), m_pMap->GetHeight(), isHex);
				} 
				else
				{
					m_pAttackManager->SpawnAlteratingAttack(1, m_pMap->GetTileSize() * 2, Vector2f(0, 1).Normalized(), m_pMap->GetWidth(), m_pMap->GetHeight(), isHex);

				}
                //m_pAttackManager->SpawnAlteratingAttack(1, m_pMap->GetTileSize() * 2, Vector2f(cos(0), sin(0)).Normalized(), m_pMap->GetWidth(), m_pMap->GetHeight(), false);
                //m_pAttackManager->SpawnAlteratingAttack(1, m_pMap->GetTileSize() * 2, Vector2f(cos(90), sin(90)).Normalized(), m_pMap->GetWidth(), m_pMap->GetHeight(), false);
				//std::cout << cos(0.f/180*M_PI) << ", " << sin(0.f/180*M_PI) << std::endl;
				//std::cout << cos(90.f/180*M_PI) << ", " << sin(90.f/180*M_PI) << std::endl;
				//   m_pAttackManager->SpawnAlteratingAttack(1, m_pMap->GetTileSize() * 2, Vector2f(1, 0).Normalized(), m_pMap->GetWidth(), m_pMap->GetHeight(), false);
              //  m_pAttackManager->SpawnAlteratingAttack(1, m_pMap->GetTileSize() * 2, Vector2f(-1, 0).Normalized(), m_pMap->GetWidth(), m_pMap->GetHeight(), false);
				m_pAttackManager->IncreaseAttackSpeed();
				*/
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
			if (m_HexTimer > 0.f)
			{
				m_HexTimer -= elapsedSec;
				if (m_HexTimer <= 0.f)
				{
					m_HexTimer = 0.f;
					m_pMap->SetHexMode(false);
				}
			}
			if (m_BlindnessTimer > 0.f)
			{
				m_BlindnessTimer -= elapsedSec;
				if (m_BlindnessTimer <= 0.f)
				{
					m_BlindnessTimer = 0.f;
					m_pMap->SetBlindMode(false);
				}
			}
			if (m_WrappingTimer > 0.f)
			{
				m_WrappingTimer -= elapsedSec;
				if (m_WrappingTimer <= 0.f)
				{
					m_WrappingTimer = 0.f;
					m_pMap->SetWrapMode(false);
				}
			}



            m_pPlayer->Update(elapsedSec);
            // Buff/Debuff spawn timers (replace existing ones if needed)
            if (m_TimerStarted)
            {
                m_BuffSpawnTimer -= elapsedSec;

                // helper: find first tile with given state
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

            auto PlaceReplacement = [&](Tile::State kind)
            {
                // If an existing tile of this kind exists, clear it first
                Vector2i existing;
                if (FindStatePos(kind, existing))
                {
                    m_pMap->SetTileState(existing, Tile::State::normal);
                }

                // Try to pick a tile at distance using CreateRandomPointTile
                Vector2i spawned = m_pMap->CreateRandomPointTile(m_pPlayer->GetPosition());
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
                    PlaceReplacement(Tile::State::buff);
                }
            }
            else
            {
                // ensure timers are reset until gameplay starts
                m_BuffSpawnTimer = m_BuffSpawnTimerMax;
                
            }
            if (m_pAttackManager->IsColliding(m_pPlayer->GetBounds(m_pMap->GetTileSize(), m_pMap->IsHexMode()), m_pPlayer->GetDirection()))
			{
                m_pPlayer->Hit(1);
                m_pSoundHit->Play(0);
			}
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
		case GameState::end:
		{
			m_pPlayer->Update(elapsedSec);
			break;
		}
		
	}


	
}

void Game::Draw() const
{
	ClearBackground();
	Rectf viewPort = GetViewPort();
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
			glPushMatrix();
			{
				glScalef(0.5f, 0.5f, 1.f);
				m_pOverlay->DrawSprite(Vector2f(-m_pOverlay->GetSpriteWidth() / 3 + m_pMap->GetWidth() / 2, - m_pOverlay->GetSpriteHeight() / 3.1 + m_pMap->GetHeight() / 2), 0, m_OverlayFrame);
			}
			glPopMatrix();
            m_pAttackManager->Draw();
		}
		glPopMatrix();
        const int
            playerHp{ m_pPlayer->GetHp() };
		const float y{ m_pMap->GetHeight() / 2 + m_pHpText->GetHeight() + 5.f};

		glScalef(0.8f, 0.8f, 1.f);
		//Hp
		m_pHpText->Draw(Vector2f(-m_pMap->GetWidth() / 2 - m_pHpText->GetWidth(), y));
        m_pLetters->DrawSprite(Vector2f(-m_pMap->GetWidth() / 2, y + 3.f), 26 + playerHp);

		//Score
		m_pScoreText->Draw(Vector2f(-m_pScoreText->GetWidth()/2 - 30,y ));
		m_pLetters->DrawSprite(Vector2f(-m_pScoreText->GetWidth() / 2 + 30.f, y + 3.f), 26 + static_cast<int>(m_Score) % 10);
        m_pLetters->DrawSprite(Vector2f(-m_pScoreText->GetWidth() / 2 + 20.f, y + 3.f), 26 + (static_cast<int>(m_Score) / 10 % 10));
        m_pLetters->DrawSprite(Vector2f(-m_pScoreText->GetWidth() / 2 + 10.f, y + 3.f), 26 + (static_cast<int>(m_Score) / 100 % 10));
		
       //Time
		m_pTimeText->Draw(Vector2f(m_pMap->GetWidth() / 2 - m_pTimeText->GetWidth() / 2 - 27.f, y));
		m_pLetters->DrawSprite(Vector2f( m_pMap->GetWidth()/2 + 10.f, y + 3.f), 26 + static_cast<int>(m_TotalTime) % 10);
        m_pLetters->DrawSprite(Vector2f( m_pMap->GetWidth()/2 + 0.f, y + 3.f), 26 + (static_cast<int>(m_TotalTime) / 10 % 10));
        m_pLetters->DrawSprite(Vector2f( m_pMap->GetWidth()/2 - 10.f, y + 3.f), 26 + (static_cast<int>(m_TotalTime) / 100 % 10));

		if (m_Easy)
		{
			utils::SetColor(Color4f(0.f, 1.f, 0.f, 1.f));
			utils::FillEllipse(Vector2f(120, 73.5), 8, 8);
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
				utils::SetColor(Color4f(1.f, 2.f/m_Multiplier, 0.f, 1.f));
				const float
					x{0},
					yPos{-GetViewPort().height/16 - 10.f},
					width{m_MultiplierTimer * 30 / 2 - 5.f},
					height{8};
				utils::FillRect(x-width / 2 , yPos, width, height);
				break;
			}
			case GameState::paused:
			{
				utils::SetColor(Color4f(0.5f, 0.5f, 0.5f, 0.6f));
				utils::FillRect(-250, -250, 500, 500);
				m_pPauseText->Draw(Vector2f(-m_pPauseText->GetWidth()/2,0));
				glPushMatrix();
				{

					glScalef(0.6f, 0.6f, 1.f);
					m_pInfoText->Draw(Vector2f(-m_pPauseText->GetWidth() / 2, -m_pInfoText->GetHeight()));

					//BestScore
					m_pBestText->Draw(Vector2f(-m_pScoreText->GetWidth() / 2 - 30.f - m_pScoreText->GetWidth() / 2 - m_pBestText->GetWidth() / 2 - 23.f, -50 - y - 13.f));
					m_pScoreText->Draw(Vector2f(-m_pScoreText->GetWidth() / 2 - 30.f - 23.f, -50 - y - 13.f));
					m_pLetters->DrawSprite(Vector2f(-m_pScoreText->GetWidth() / 2 + 30.f - 23.f, -50 - 1 * y - 10.f), 26 + static_cast<int>(m_BestScore) % 10);
					m_pLetters->DrawSprite(Vector2f(-m_pScoreText->GetWidth() / 2 + 20.f - 23.f, -50 - 1 * y - 10.f), 26 + (static_cast<int>(m_BestScore) / 10 % 10));
					m_pLetters->DrawSprite(Vector2f(-m_pScoreText->GetWidth() / 2 + 10.f - 23.f, -50 - 1 * y - 10.f), 26 + (static_cast<int>(m_BestScore) / 100 % 10));


					//BestTime
					m_pBestText->Draw(Vector2f(10.f + m_pMap->GetWidth() / 2 - m_pTimeText->GetWidth() / 2 - 27.f - m_pTimeText->GetWidth(), -50 - y - 13.f));
					m_pTimeText->Draw(Vector2f(10.f + m_pMap->GetWidth() / 2 - m_pTimeText->GetWidth() / 2 - 27.f, -50 - y - 13.f));
					m_pLetters->DrawSprite(Vector2f(10.f + m_pMap->GetWidth() / 2 + 10.f, -50 - 1 * y - 10.f), 26 + static_cast<int>(m_BestTime) % 10);
					m_pLetters->DrawSprite(Vector2f(10.f + m_pMap->GetWidth() / 2 + 0.f, -50 - 1 * y - 10.f), 26 + (static_cast<int>(m_BestTime) / 10 % 10));
					m_pLetters->DrawSprite(Vector2f(10.f + m_pMap->GetWidth() / 2 - 10.f, -50 - 1 * y - 10.f), 26 + (static_cast<int>(m_BestTime) / 100 % 10));

				}
				glPopMatrix();
				break;
			}
			case GameState::info:
			{
				glPushMatrix();
				{
					glScalef(0.63f,0.63f, 1.f);
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
					if (m_MultiplierTimer > 7.5)
					{
						m_MultiplierTimer = 7.5;
					}
					m_pMap->RemoveTileModifier(m_pPlayer->GetPosition());
					m_pMap->SetTileState(m_pPlayer->GetPosition(), Tile::State::normal);
					break;
				}
				case(Tile::State::debuff):
				{
					m_pSoundDebuff->Play(0);

					m_pMap->SetBlindMode(true);
					m_BlindnessTimer = m_StatusTimerMax;

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
					switch (rand() % 2)
					{
						case(0):
						{
							m_pMap->SetHexMode(true);
							m_HexTimer = m_StatusTimerMax;
							break;
						}
						case(1):
						{
							m_pMap->SetWrapMode(true);
							m_WrappingTimer = m_StatusTimerMax;
							break;
						}
						case(2):
						{
							m_pMap->SetHexMode(true);
							break;
						}
					}
					m_pMap->SetTileState(m_pPlayer->GetPosition(), Tile::State::normal);
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
                    if ((rand() % 2) == 0)
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
				m_pMap->SetHexMode(!m_pMap->IsHexMode());
				break;
			}
			case(SDLK_F2):
			{
				m_pMap->SetWrapMode(!m_pMap->IsWrapMode());
				break;
			}
			case(SDLK_F3):
			{
				m_IsConfused = !m_IsConfused;
				break;
			}
			case(SDLK_F4):
			{
				m_pMap->SetBlindMode(!m_pMap->IsBlindMode());
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
	//std::cout << "MOUSEBUTTONDOWN event: ";
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
