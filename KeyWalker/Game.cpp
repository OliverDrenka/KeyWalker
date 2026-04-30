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
	
	m_pFont = TTF_OpenFont("BoldPixels.ttf", 16);
	m_pRestartText = new Texture("Press R to Restart", m_pFont, Color4f(0.f,0.f,0.f,1.f));
	m_pStartText = new Texture("Press S to Start", m_pFont, Color4f(0.f,0.f,0.f,1.f));
	m_pPauseText = new Texture("Press ESC to Unpause", m_pFont, Color4f(0.f,0.f,0.f,1.f));
	m_pScoreText = new Texture("Score:", m_pFont, Color4f(0.f,0.f,0.f,1.f));
	m_pTimeText = new Texture("Time:", m_pFont, Color4f(0.f, 0.f, 0.f, 1.f));
	m_pHpText = new Texture("Hp:", m_pFont, Color4f(0.f, 0.f, 0.f, 1.f));
	m_pBestText = new Texture("Best", m_pFont, Color4f(0.f, 0.f, 0.f, 1.f));

    m_pMap = new Map();
    m_pPlayer = new Player();
    m_pOverlay = new Texture("Overlay.png");
    m_pAttackManager = new AttackManager();
    m_pLetters = new SpriteSheet(36, "Font.png", 5);

    m_pSoundButtonPress = new SoundEffect("ButtonPress.wav");
    m_pSoundHit = new SoundEffect("Hit.wav");
    m_pSoundPointSpawn = new SoundEffect("PointSpawn.wav");
    m_pSoundPointCollected = new SoundEffect("PointCollected.wav");
	m_pSoundPreparedTile = new SoundEffect("PreparedTile.wav");

    m_pSoundButtonPress->SetVolume(50);
    m_pSoundPreparedTile->SetVolume(50);
    m_pSoundHit->SetVolume(40);
    m_pSoundPointCollected->SetVolume(25);

    m_pMap->SetHexMode(false);
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
    LoadBest();


}

void Game::Cleanup( )
{
    delete m_pMap;
    delete m_pPlayer;
    delete m_pOverlay;
    delete m_pAttackManager;
    delete m_pLetters;

    delete m_pSoundButtonPress;
    delete m_pSoundHit;
    delete m_pSoundPointSpawn;
    delete m_pSoundPointCollected;
	delete m_pSoundPreparedTile;

	delete m_pRestartText;
	delete m_pPauseText;
	delete m_pStartText;
	delete m_pScoreText;
	delete m_pTimeText;
	delete m_pHpText;
    delete m_pBestText;
	TTF_CloseFont( m_pFont );
	m_vecDangerTiles.clear();
	m_vecDangerTiles.shrink_to_fit();
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
			m_AttackTimer += elapsedSec;
			m_TotalTime += elapsedSec;
			if(m_MultiplierTimer > 0) 
			{
				m_MultiplierTimer -= elapsedSec;
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
				m_pMap->SetMaxValue(15 + static_cast<int>(m_TotalTime/5));
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
			if (m_TotalTime > 6.f && !m_PointsSpawned)
			{
				m_PointsSpawned = true;
                m_pMap->CreateRandomPointTile(m_pPlayer->GetPosition());
                m_pMap->CreateRandomPointTile(m_pPlayer->GetPosition());
			}
                m_pAttackManager->Update(elapsedSec);
            m_pPlayer->Update(elapsedSec);
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
			for (int i{ 0 }; i < m_vecDangerTiles.size(); i++)
			{
				const Vector2i position{ m_vecDangerTiles [i]};
				const float
					x{i + 1.f},
					y{1.f};
				utils::SetColor(Color4f(1.f, 0.f, 0.f, 1.f));
				utils::FillRect(position.x*tileSize- x/2 + tileSize/2, position.y*tileSize + y/2, x, y);
			}
            m_pPlayer->Draw(m_pMap->GetTileSize(), m_pMap->IsHexMode());
			m_pOverlay->Draw(Vector2f(-m_pOverlay->GetWidth() / 2 + m_pMap->GetWidth() / 2, - m_pOverlay->GetHeight() / 2 + m_pMap->GetHeight() / 2));
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



		switch (m_GameState)
		{
			case GameState::start:
			{
				utils::SetColor(Color4f(0.3f, 0.3f, 0.3f, 0.6f));
				utils::FillRect(-250, -250, 500, 500);
				m_pStartText->Draw(Vector2f(-m_pStartText->GetWidth() / 2, -m_pStartText->GetHeight() / 2));
				break;
			}
			case GameState::gameplay:
			{
				utils::SetColor(Color4f(1.f, 2.f/m_Multiplier, 0.f, 1.f));
				const float
					x{0},
					yPos{-GetViewPort().height/16 - 10.f},
					width{m_MultiplierTimer * 30},
					height{8};
				utils::FillRect(x-width / 2, yPos -height /2, width, height);
				break;
			}
			case GameState::paused:
			{
				utils::SetColor(Color4f(0.5f, 0.5f, 0.5f, 0.6f));
				utils::FillRect(-250, -250, 500, 500);
				m_pPauseText->Draw(Vector2f(-m_pPauseText->GetWidth()/2, -m_pPauseText->GetHeight() / 2));
				glPushMatrix();
				{

					glScalef(0.6f, 0.6f, 1.f);

					//BestScore
					m_pBestText->Draw(Vector2f(-m_pScoreText->GetWidth() / 2 - 30.f - m_pScoreText->GetWidth() / 2 - m_pBestText->GetWidth() / 2 - 23.f, -50 - y - 18.f));
					m_pScoreText->Draw(Vector2f(-m_pScoreText->GetWidth() / 2 - 30.f - 23.f, -50 - y - 18.f));
					m_pLetters->DrawSprite(Vector2f(-m_pScoreText->GetWidth() / 2 + 30.f - 23.f, -50 - 1 * y - 15.f), 26 + static_cast<int>(m_BestScore) % 10);
					m_pLetters->DrawSprite(Vector2f(-m_pScoreText->GetWidth() / 2 + 20.f - 23.f, -50 - 1 * y - 15.f), 26 + (static_cast<int>(m_BestScore) / 10 % 10));
					m_pLetters->DrawSprite(Vector2f(-m_pScoreText->GetWidth() / 2 + 10.f - 23.f, -50 - 1 * y - 15.f), 26 + (static_cast<int>(m_BestScore) / 100 % 10));


					//BestTime
					m_pBestText->Draw(Vector2f(10.f + m_pMap->GetWidth() / 2 - m_pTimeText->GetWidth() / 2 - 27.f - m_pTimeText->GetWidth(), -50 - y - 18.f));
					m_pTimeText->Draw(Vector2f(10.f + m_pMap->GetWidth() / 2 - m_pTimeText->GetWidth() / 2 - 27.f, -50 - y - 18.f));
					m_pLetters->DrawSprite(Vector2f(10.f + m_pMap->GetWidth() / 2 + 10.f, -50 - 1 * y - 15.f), 26 + static_cast<int>(m_BestTime) % 10);
					m_pLetters->DrawSprite(Vector2f(10.f + m_pMap->GetWidth() / 2 + 0.f, -50 - 1 * y - 15.f), 26 + (static_cast<int>(m_BestTime) / 10 % 10));
					m_pLetters->DrawSprite(Vector2f(10.f + m_pMap->GetWidth() / 2 - 10.f, -50 - 1 * y - 15.f), 26 + (static_cast<int>(m_BestTime) / 100 % 10));

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
            m_pPlayer->Move(movement);
			switch (m_pMap->GetTileState(m_pPlayer->GetPosition()))
			{
				case(Tile::State::point):
				{
					m_pSoundPointCollected->Play(0);
					if (!m_TimerStarted)
					{
						m_TimerStarted = true;
					}
					m_Score += m_Multiplier;
					Vector2i length = m_pMap->CreateRandomPointTile(m_pPlayer->GetPosition());

					float dist = Vector2f(m_pPlayer->GetPosition().x - length.x, m_pPlayer->GetPosition().y - length.y).Length();
					m_MultiplierTimer += dist / m_Multiplier;
					if (m_MultiplierTimer > 5)
					{
						m_MultiplierTimer = 5;
					}
					m_pMap->RemoveTileModifier(m_pPlayer->GetPosition());
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
					m_pMap->SetTileState(m_pPlayer->GetPosition(), Tile::State::danger);
					m_vecDangerTiles.push_back(m_pPlayer->GetPosition());
					break;
				}
				case(Tile::State::danger):
				{
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
				/*for (const Vector2i& tile : m_vecDangerTiles)
				{
					m_pMap->RemoveTileModifier(tile);
				}
				m_vecDangerTiles.clear();
				*/
				m_pMap->RemoveTileModifier(m_vecDangerTiles[0]);
				m_vecDangerTiles.erase(m_vecDangerTiles.begin());
			}
        }
        break;
    }
    case GameState::paused:
    {
        if (e.keysym.sym == SDLK_ESCAPE)
        {
            m_GameState = GameState::gameplay;
            break;
        }
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
	glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );
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
