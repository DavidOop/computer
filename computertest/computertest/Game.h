#pragma once
#include "Utilities.h"
#include "Images.h"
#include <memory>
#include <map>
#include <unordered_map>

const float MOVE = 400;
const sf::Vector2f BOARD_SIZE{ 3000.f,3000.f };


class Game
{
public:
	Game(const Images &images, Uint32 image);
	void receive(const Images &images);
	unsigned play(sf::RenderWindow &w, const Images &images);
	
	void draw(sf::RenderWindow &w) const;
	void setView(sf::RenderWindow &w) const;

private:
	bool updateMove(float);
	bool receiveChanges( const Images &images);
	/*
	*/
	char Game::move(float);
	float Game::direction(const pair& ver);
	float Game::go(const pair& temp, float& max);
	//===========================
	Maps m_objectsOnBoard;
	std::unordered_map<Uint32, std::unique_ptr<OtherPlayers>> m_players;
	sf::Sprite m_background;
	std::unique_ptr<MyPlayer> m_me;
	//m_miniMap;
	//Score m_score;
	sf::TcpSocket m_socket;
	//sf::View m_view{ sf::FloatRect{ 0,0,float(SCREEN_WIDTH),float(SCREEN_HEIGHT) } };
};
//--------------------------------------------------------------------------
