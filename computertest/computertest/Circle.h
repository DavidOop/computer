#pragma once
#include <SFML\Graphics.hpp>
#include<SFML\Network.hpp>
#include <memory>
#include <set>
#include <iostream>
#include <unordered_map>
#include "Fonts.h"
#include "Images.h"


#ifdef _DEBUG
#pragma comment(lib, "sfml-main-d.lib")
#pragma comment(lib, "sfml-system-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#pragma comment(lib, "sfml-graphics-d.lib")
#pragma comment(lib, "sfml-network-d.lib")
#elif defined(NDEBUG)
#pragma comment(lib, "sfml-main.lib")
#pragma comment(lib, "sfml-system.lib")
#pragma comment(lib, "sfml-window.lib")
#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-network.lib")
#else
#error "Unrecognized configuration!"
#endif

class Maps;
class OtherPlayers;

using sf::Uint32;
using sf::CircleShape;
using sf::Vector2f;

typedef std::pair<sf::Vector2f, sf::Vector2f> pair;

const float FOOD_RADIUS = 22;
const float BOMB_RADIUS = 50;
const float NEW_PLAYER = 60;
const unsigned MAX_IMAGE = 100;

const Uint32 PLAYER_LOWER = 200;
const Uint32 PLAYER_UPPER = 300;

const Uint32 FOOD_LOWER = 1000;
const Uint32 FOOD_UPPER = 5000;

const Uint32 BOMBS_LOWER = 6000;
const Uint32 BOMBS_UPPER = 10000;

class Circle :public CircleShape
{
public:
	Circle() = default;
	Circle(Uint32 id, Vector2f center = Vector2f{}) :m_id(id), m_center(center) {}
	Circle(const Circle& c) :Circle(c.getId(), c.getCenter()) {}

	Uint32 getId() const { return m_id; }
	const Vector2f& getCenter() const { return m_center; }
	void setCenter(Vector2f center) { m_center = center; setPosition({ center.x - getRadius(),center.y - getRadius() }); }
	void setCenter() { m_center = getPosition() + Vector2f{ getRadius(), getRadius() }; }

	void virtual f() = 0;

protected:
	Uint32 m_id;
	Vector2f m_center;
};
//-------------------------------------
class Player :public Circle
{
public:
	Player() = default;
	Player(Uint32 id, Vector2f c = Vector2f{}, unsigned s = 0) :Circle(id, c) {}
	Player(const Player& p) :Player(p.getId(), p.getCenter(), p.getScore()) {}

	void collision(std::vector<Uint32> &deleted, Maps &objectsOnBoard, std::unordered_map<Uint32, std::unique_ptr<OtherPlayers>>& players, Player *me);
	void checkPlayers(std::vector<Uint32> &deleted, std::unordered_map<Uint32, std::unique_ptr<OtherPlayers>>& players, Player *me);
	void checkFoodAndBomb(std::vector<Uint32> &deleted, Maps &objectsOnBoard);
	bool circlesCollide(const Circle* p) const;

	void newRadius(Circle *c);
	void move(float x, float y);

	unsigned getScore() const { return m_score; }
	void setScore(Uint32 radius) { m_score += unsigned(radius); }

	bool getLive() const { return m_live; }
	void setLive(bool l) { m_live = l; }

protected:
	unsigned m_score = 0;
	bool m_live = true;
};
//-------------------------------------
class MyPlayer :public Player
{
public:
	MyPlayer();
	MyPlayer(Uint32 id, const sf::Texture &image, const sf::Font &font, sf::Vector2f position /*= { 0.f,0.f }*/, const sf::String name = "no name");
	MyPlayer(const MyPlayer& p) :Player(p) {}
	
	void setId(Uint32 id) { m_id = id; }
	void setTexture(const sf::Texture &image) { CircleShape::setTexture(&image); }
	void f() override {} 
};
//-------------------------------------
class OtherPlayers :public Player
{
public:
	OtherPlayers(const OtherPlayers& p) :Player(p) {}
	OtherPlayers(Uint32 id, const sf::Texture &image, const sf::Font &font, float radius, sf::Vector2f position, const sf::String &name = "no name");

	void f() override {}
};
//-------------------------------------
class FoodAndBomb :public Circle
{
public:
	FoodAndBomb(Uint32 id, sf::Vector2f place) :Circle(id) { setPosition(place); }
	FoodAndBomb(std::pair<Uint32, sf::Vector2f> temp) :FoodAndBomb(temp.first, temp.second) {}
};
//-------------------------------------
class Food :public FoodAndBomb
{
public:
	Food(Uint32 id, sf::Vector2f place, const sf::Texture&);
	Food(std::pair<Uint32, sf::Vector2f> temp, const sf::Texture& t) :Food(temp.first, temp.second, t) {}

	void f() override {}
};
//-------------------------------------
class Bomb :public FoodAndBomb
{
public:
	Bomb(Uint32 id, sf::Vector2f place, const sf::Texture&);
	Bomb(std::pair<Uint32, sf::Vector2f> temp, const sf::Texture& t) :Bomb(temp.first, temp.second, t) {}

	void f() override {}
};
//=============================================================================================
float distance(const sf::Vector2f& p1, const sf::Vector2f& p2);