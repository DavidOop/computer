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
	Game(const Images &images, Uint32 image, sf::View&);
	void receive(const Images &images);
	unsigned play(sf::RenderWindow &w, const Images &images);
	void draw(sf::RenderWindow &w) const;
	void setView(sf::RenderWindow &w) const;
	const Maps& getObjectsOnBoard()const { return m_objectsOnBoard; }
	//Maps& getObjectsOnBoard() { return m_objectsOnBoard; }
	const auto& getOtherPlayers()const { return m_players; }
	void addPlayer(const std::pair<Uint32, sf::Vector2f> &temp, sf::Packet &packet, const Images &images);

	template<typename T>
	bool safeSquare(const std::set<sf::Uint32>&, T, const Square&)const;
	//------------------------------
	auto findInMap(sf::Uint32 key)const{ return m_objectsOnBoard.find(key); }
private:
	bool updateMove(float);
	bool receiveChanges(const Images &images);
	//-----------------------------------------
	void Game::move(float);
	float Game::direction(const pair& ver);
	
	Maps m_objectsOnBoard;
	std::unordered_map<Uint32, std::unique_ptr<OtherPlayers>> m_players;
	sf::Sprite m_background;
	std::unique_ptr<MyPlayer> m_me;

	bool m_receive{ true };
	sf::TcpSocket m_socket;
	sf::View& m_view;
	void Game::clear(const sq& root, sq& curr);
	void setSquare();
	std::vector<std::vector<sq>> m_squares;
	std::stack<sq> bfs(sq square);
};
//--------------------------------------------------------------------------
template <typename T>
bool Game::safeSquare(const std::set<sf::Uint32>& keys, T function, const Square& s)const {
	for (auto it = keys.begin(); it != keys.end(); ++it) {
		if (function(*it))
			if (s.collide(m_objectsOnBoard.find(*it)->second.get()))
				return true;
	}
	return false;
}