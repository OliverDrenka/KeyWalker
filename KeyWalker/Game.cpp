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
	m_Map = new Map();
	m_Player = new Player();
	m_Overlay = new Texture("Overlay.png");
	m_AttackManager = new AttackManager();
	m_Letters = new SpriteSheet(36, "Font.png");
	
}

void Game::Cleanup( )
{
	delete m_Map;
	delete m_Player;
	delete m_Overlay;
	delete m_AttackManager;
	delete m_Letters;
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
	if (m_Player->GetHp() > 0)
	{

		m_AttackTimer += elapsedSec;
		m_TotalTime += elapsedSec;
		if (m_AttackTimer >= 6.f) 
		{
			m_AttackTimer -= 6;
			m_AttackManager->SpawnAlteratingAttack(1, m_Map->GetTileSize() * 2, Vector2f(0, 1).Normalized(), m_Map->GetWidth(), m_Map->GetHeight(), false);
			m_AttackManager->SpawnAlteratingAttack(1, m_Map->GetTileSize() * 2, Vector2f(1, 0).Normalized(), m_Map->GetWidth(), m_Map->GetHeight(), false);
			m_AttackManager->SpawnAlteratingAttack(1, m_Map->GetTileSize() * 2, Vector2f(0, -1).Normalized(), m_Map->GetWidth(), m_Map->GetHeight(), false);
			m_AttackManager->SpawnAlteratingAttack(1, m_Map->GetTileSize() * 2, Vector2f(-1, 0).Normalized(), m_Map->GetWidth(), m_Map->GetHeight(), false);
		}
		m_AttackManager->Update(elapsedSec);
		if (m_AttackManager->isColliding(m_Player->GetBounds(m_Map->GetTileSize())))
		{
			m_Player->Hit(1);
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
			glTranslatef(-m_Map->GetWidth() / 2, -m_Map->GetHeight() / 2, 0.f);
			m_Map->Draw();
			m_Player->Draw(m_Map->GetTileSize());
			m_AttackManager->Draw();
		}
		glPopMatrix();
		//m_Overlay->Draw(Vector2f(-m_Overlay->GetWidth() / 2, - m_Overlay->GetHeight() / 2));
		const int
			playerHp{ m_Player->GetHp() };
		m_Letters->DrawSprite(Vector2f(- m_Map->GetWidth()/2 - 13.f, m_Map->GetHeight() / 2 - 10.f), 25 + playerHp);

		m_Letters->DrawSprite(Vector2f(- m_Map->GetWidth()/2 - 9.f, m_Map->GetHeight() / 2 - 26.f), 25 + static_cast<int>(m_TotalTime) % 10);
		m_Letters->DrawSprite(Vector2f(- m_Map->GetWidth()/2 - 19.f, m_Map->GetHeight() / 2 - 26.f), 25 + (static_cast<int>(m_TotalTime) - static_cast<int>(m_TotalTime) % 10)/10);
		m_Letters->DrawSprite(Vector2f(- m_Map->GetWidth()/2 - 29.f, m_Map->GetHeight() / 2 - 26.f), 25 + (static_cast<int>(m_TotalTime) - static_cast<int>(m_TotalTime) % 100)/100);
	}
	glPopMatrix();
}

void Game::ProcessKeyDownEvent( const SDL_KeyboardEvent & e )
{
	int value{ e.keysym.sym };
	std::cout << value << ", " << char(value) << std::endl;
	if (value >= 48 && value <= 57)
	{
		value -= 22;
	}
	else
	{
		value -= 97;
	}
	m_Player->Move( m_Map->GetAdjecentTileDirection( m_Player->GetPosition(), value ));
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
