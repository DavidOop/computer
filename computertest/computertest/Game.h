#pragma once
#include "Utilities.h"
#include "Images.h"
#include <memory>
#include <map>
#include <unordered_map>
#include <thread>
const float MOVE = 400;
const sf::Vector2f BOARD_SIZE{ 3000.f,3000.f };


class Game
{
public:
	Game(const Images &images, const Fonts &fonts, Uint32 image_id, sf::View& view, const sf::String &name);
	void receive(const Images &images, const Fonts &fonts);
	unsigned play(sf::RenderWindow &w, const Images &images, const Fonts &fonts);
	void draw(sf::RenderWindow &w) const;
	void setView(sf::RenderWindow &w) const;
	const Maps& getObjectsOnBoard()const { return m_objectsOnBoard; }
	const auto& getOtherPlayers()const { return m_players; }
	void addPlayer(const std::pair<Uint32, sf::Vector2f> &temp, sf::Packet &packet, const Images &images, const Fonts &fonts);

	template<typename T>
	bool safeSquare(const std::set<sf::Uint32>&, T, const Square&)const;
	//------------------------------
	auto findInMap(sf::Uint32 key)const { return m_objectsOnBoard.find(key); }
private:
	void updateMove(float);
	void receiveChanges(const Images &images, const Fonts &fonts);
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
	bool safe(sf::Vector2f m);
	//void actMove(const sq&);
	void Game::checkDir(std::stack<sq>& curr, const sq&);
	std::thread makeThread(std::stack<sq>& curr, const sq& squ) { return std::thread([&] {checkDir(std::ref(curr),squ); }); }
	//void Game::checkDir();
	std::mutex _mu_stack_begin;
	std::mutex _mu_stack_end;
	std::condition_variable _cv_stack;
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



void deleteDeadPlayer(std::unordered_map<Uint32, std::unique_ptr<OtherPlayers>>& players);