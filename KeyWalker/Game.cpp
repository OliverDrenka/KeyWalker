#include "pch.h"
#include "Game.h"
#include <iostream>
#include "utils.h"

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
	m_pPauseText = new Texture("Press ESC to Unpause", m_pFont, Color4f(0.f,0.f,0.f,1.f));
	m_pScoreText = new Texture("Score", m_pFont, Color4f(0.f,0.f,0.f,1.f));
	m_pTimeText = new Texture("Time", m_pFont, Color4f(0.f, 0.f, 0.f, 1.f));
	m_pHpText = new Texture("Hp", m_pFont, Color4f(0.f, 0.f, 0.f, 1.f));
	
    m_pMap = new Map();
    m_pPlayer = new Player();
    m_pOverlay = new Texture("Overlay.png");
    m_pAttackManager = new AttackManager();
    m_pLetters = new SpriteSheet(36, "Font.png", 3);

    m_pSoundButtonPress = new SoundEffect("ButtonPress.wav");
    m_pSoundHit = new SoundEffect("Hit.wav");
    m_pSoundPointSpawn = new SoundEffect("PointSpawn.wav");
    m_pSoundPointCollected = new SoundEffect("PointCollected.wav");

    m_pSoundButtonPress->SetVolume(50);
    m_pSoundHit->SetVolume(40);
    m_pSoundPointCollected->SetVolume(25);

    m_pMap->SetHexMode(false);
    m_pMap->GenerateMapRandom();

	m_AttackTimer = 0;
	m_TotalTime = 0;
	m_Score = 0;
	m_PointsSpawned = false;

	
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
	
	delete m_pRestartText;
	delete m_pPauseText;
	delete m_pScoreText;
	delete m_pTimeText;
	delete m_pHpText;
	TTF_CloseFont( m_pFont );
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
			if (m_AttackTimer >= 6.f) 
			{
                m_AttackTimer -= 10;
                m_pAttackManager->SpawnAlteratingAttack(1, m_pMap->GetTileSize() * 2, Vector2f(0, 1).Normalized(), m_pMap->GetWidth(), m_pMap->GetHeight(), false);
                m_pAttackManager->SpawnAlteratingAttack(1, m_pMap->GetTileSize() * 2, Vector2f(1, 0).Normalized(), m_pMap->GetWidth(), m_pMap->GetHeight(), false);
                m_pAttackManager->SpawnAlteratingAttack(1, m_pMap->GetTileSize() * 2, Vector2f(0, -1).Normalized(), m_pMap->GetWidth(), m_pMap->GetHeight(), false);
                m_pAttackManager->SpawnAlteratingAttack(1, m_pMap->GetTileSize() * 2, Vector2f(-1, 0).Normalized(), m_pMap->GetWidth(), m_pMap->GetHeight(), false);
			
			}
			if (m_TotalTime > 6.f && !m_PointsSpawned)
			{
				m_PointsSpawned = true;
                m_pMap->CreateRandomPointTile();
                m_pMap->CreateRandomPointTile();
			}
                m_pAttackManager->Update(elapsedSec);
            m_pPlayer->Update(elapsedSec);
            if (m_pAttackManager->isColliding(m_pPlayer->GetBounds(m_pMap->GetTileSize(), m_pMap->IsHexMode())))
			{
                m_pPlayer->Hit(1);
                m_pSoundHit->Play(0);
				if (m_pPlayer->GetHp() <= 0)
				{
					m_GameState = GameState::end;
				}
			}
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
            m_pMap->Draw();
            m_pPlayer->Draw(m_pMap->GetTileSize(), m_pMap->IsHexMode());
            m_pAttackManager->Draw();
		}
		glPopMatrix();
		//m_Overlay->Draw(Vector2f(-m_Overlay->GetWidth() / 2, - m_Overlay->GetHeight() / 2));
        const int
            playerHp{ m_pPlayer->GetHp() };
		const float y{ m_pMap->GetHeight() / 2 };

		m_pHpText->Draw(Vector2f(-m_pMap->GetWidth() / 2, y));
        m_pLetters->DrawSprite(Vector2f(-m_pMap->GetWidth() / 2 + 17.f, y + 3.f), 26 + playerHp);

		m_pScoreText->Draw(Vector2f(-m_pScoreText->GetWidth()/2 - 30.f, y));
		m_pLetters->DrawSprite(Vector2f(-m_pScoreText->GetWidth() / 2 + 27.f, y + 3.f), 26 + static_cast<int>(m_Score) % 10);
        m_pLetters->DrawSprite(Vector2f(-m_pScoreText->GetWidth() / 2 + 17.f, y + 3.f), 26 + (static_cast<int>(m_Score) - static_cast<int>(m_Score) % 10)/10);
        m_pLetters->DrawSprite(Vector2f(-m_pScoreText->GetWidth() / 2 + 7.f, y + 3.f), 26 + (static_cast<int>(m_Score) - static_cast<int>(m_Score) % 100)/100);
		
       
		m_pTimeText->Draw(Vector2f(m_pMap->GetWidth() / 2 - m_pTimeText->GetWidth() / 2 - 45.f, y));
		m_pLetters->DrawSprite(Vector2f( m_pMap->GetWidth()/2 - 9.f, y + 3.f), 26 + static_cast<int>(m_TotalTime) % 10);
        m_pLetters->DrawSprite(Vector2f( m_pMap->GetWidth()/2 - 19.f, y + 3.f), 26 + (static_cast<int>(m_TotalTime) - static_cast<int>(m_TotalTime) % 10)/10);
        m_pLetters->DrawSprite(Vector2f( m_pMap->GetWidth()/2 - 29.f, y + 3.f), 26 + (static_cast<int>(m_TotalTime) - static_cast<int>(m_TotalTime) % 100)/100);
		

	}
	switch (m_GameState)
	{
		case GameState::start:
		{
			break;
		}
		case GameState::gameplay:
		{
			break;
		}
		case GameState::paused:
		{
			utils::SetColor(Color4f(0.5f, 0.5f, 0.5f, 0.6f));
			utils::FillRect(-250, -250, 500, 500);
			m_pPauseText->Draw(Vector2f(-m_pPauseText->GetWidth()/2, -m_pPauseText->GetHeight() / 2));
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
	glPopMatrix();

}

void Game::ProcessKeyDownEvent( const SDL_KeyboardEvent & e )
{
	switch (m_GameState)
	{
		case GameState::start:
		{
			break;
		}
		case GameState::gameplay:
		{
			int value{ e.keysym.sym };
			if (value == 27)
			{
				m_GameState = GameState::paused;
				break;
			}
			if (value >= 48 && value <= 57)
			{
				value -= 22;
			}
			else
			{
				value -= 97;
			}
			//m_Map->RandomizeTile(m_Player->GetPosition());
            Vector2i movement{ m_pMap->GetAdjecentTileDirection(m_pPlayer->GetPosition(), value) };
            if (movement != Vector2i(0, 0))
            {
                m_pSoundButtonPress->Play(0);
                m_pPlayer->Move(movement);
            }
            if (m_pMap->GetTileState(m_pPlayer->GetPosition()) == Tile::State::point)
            {
                m_Score += 1;
                m_pSoundPointCollected->Play(0);
                m_pMap->RemoveTileModifier(m_pPlayer->GetPosition());
                m_pMap->CreateRandomPointTile();
            }
			break;
		}
		case GameState::paused:
		{
			if (e.keysym.sym == 27)
			{
				m_GameState = GameState::gameplay;
				break;
			}
		}
		case GameState::end:
		{
			if (e.keysym.sym == 114)
			{
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
