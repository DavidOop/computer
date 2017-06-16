#pragma once
#include <SFML\Graphics.hpp>
#include<SFML\Network.hpp>
#include <memory>
#include <unordered_map>
#include <set>
#include "Circle.h"
const unsigned SQUARE = 5;

struct Square;
class Game;
typedef std::shared_ptr<Square> sq;
//================================================================================
struct Square {
	Square(const sf::Vector2f& ver) :_ver(ver) {}
	sf::Vector2f _ver;
	sq _up{};
	sq _down{};
	sq _left{};
	sq _right{};

	sq _parent{};
	bool _visited{ false };
public:
	bool update(sq&, const Game&);
	//std::set<sf::Uint32> playerOnSquare(const Game&);
	void findParent(const sq&);
	bool Square::collide(Circle* c)const;
	void clear();
	sf::Vector2f limitsLower(const float RADIUS) const { return{ _ver.x - RADIUS - SQUARE , _ver.y - RADIUS - SQUARE }; }
	sf::Vector2f limitsUpper(const float RADIUS) const { return{ _ver.x + RADIUS + SQUARE , _ver.y + RADIUS + SQUARE }; }
};
//==========================================================================
//=================================MAPS=====================================
//==========================================================================
//================================ Coordinates class =======================
/***************************************************************************
this class is for all the food and bombs in the game:
it add's food and bombs when needed and removes them when a ball
crashes into them.
Both multimaps are for finding the colliding food and bombs with the player'

******************************************************************************************/
class Maps :private std::unordered_map<Uint32, std::unique_ptr<FoodAndBomb>> {
public:
	Maps() {}
	~Maps() {}

	using std::unordered_map<Uint32, std::unique_ptr<FoodAndBomb>>::emplace;
	using std::unordered_map<Uint32, std::unique_ptr<FoodAndBomb>>::erase;
	using std::unordered_map<Uint32, std::unique_ptr<FoodAndBomb>>::operator[];
	using std::unordered_map<Uint32, std::unique_ptr<FoodAndBomb>>::begin;
	using std::unordered_map<Uint32, std::unique_ptr<FoodAndBomb>>::end;
	using std::unordered_map<Uint32, std::unique_ptr<FoodAndBomb>>::find;

	void insert(const std::pair<Uint32, sf::Vector2f> &);
	void eraseFromData(sf::Uint32);
	std::set<sf::Uint32> Maps::colliding(const pair& ver)const;
	std::set<Uint32> Maps::colliding(const Vector2f& ver, const float radius);
private:

	std::multimap<float, Uint32> m_x;
	std::multimap<float, Uint32> m_y;
};
//==========================================================================
//================================= Time ===================================
//==========================================================================
/*
		singleton class for managing time to move the player

******************************************************************************/
class TimeClass
{
public:
	inline static TimeClass& instance() { static TimeClass tone; return tone; }
	~TimeClass() {}

	inline float RestartClock() { return m_clock.restart().asSeconds(); }
private:
	TimeClass() = default;
	TimeClass(const TimeClass&) = delete;
	TimeClass& operator=(const TimeClass&) = delete;

	sf::Clock m_clock;
};

//==========================================================================
//================================PACKET====================================
//==========================================================================

sf::Packet& operator >> (sf::Packet& packet, std::pair <Uint32, sf::Vector2f>& pair);
sf::Packet& operator >> (sf::Packet& packet, sf::Vector2f& vertex);

sf::Packet& operator << (sf::Packet& packet, const sf::Vector2f &vertex);
sf::Packet& operator << (sf::Packet& packet, const std::vector<Uint32>& deleted);
//==============================================================

inline bool isFood(sf::Uint32 id) { return id >= FOOD_LOWER &&id <= FOOD_UPPER; }
inline bool isBomb(sf::Uint32 id) { return id >= BOMBS_LOWER &&id <= BOMBS_UPPER; }
inline bool isPlayer(sf::Uint32 id) { return id >= PLAYER_LOWER &&id <= PLAYER_UPPER; }