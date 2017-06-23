#pragma once
#include "Game.h"
#include <iostream>
#include <vector>
#include <Windows.h>
#include <queue>
//====================================================================================
//================================ CONSTRUCTOR =======================================
//====================================================================================
Game::Game(const Images &images, const Fonts &fonts, Uint32 image_id, sf::View& view, const sf::String &name)
	:m_me(std::make_unique<MyPlayer>()),
	m_background(images[int(BACKGROUND)]),
	m_view(view)
{
	if (m_socket.connect(sf::IpAddress::LocalHost, 5555) != sf::TcpSocket::Done)
		//if (m_socket.connect("10.2.16.95", 5555) != sf::TcpSocket::Done)
		std::cout << "no connecting\n";
	setSquare();
	sf::Packet packet;
	packet << image_id << "computer"; //שליחה לשרת של התמונה שלי
	if (m_socket.send(packet) != sf::TcpSocket::Done)
		std::cout << "no sending image\n";

	receive(images, fonts);//קליטת מידע מהשרת
}
//--------------------------------------------------------------------------
void Game::receive(const Images &images, const Fonts &fonts)
{
	sf::Packet packet;
	std::pair <Uint32, sf::Vector2f> temp;
	Uint32 image;
	float radius;

	m_socket.setBlocking(true);//**********************************
	Sleep(100);//*********************************

	//auto status = ;

	while (m_socket.receive(packet) != sf::TcpSocket::Done);
	//{
		while (!packet.endOfPacket())//קליטה של כל הדברים שעל הלוח
		{
			packet >> temp;

			if (temp.first >= FOOD_LOWER && temp.first <= BOMBS_UPPER)
				m_objectsOnBoard.insert(temp, images);

			else if (temp.first >= PLAYER_LOWER && temp.first <= PLAYER_UPPER)//???????????????????????
			{
				sf::String s;
				packet >> radius >> image >> s;
				m_players.emplace(temp.first, std::make_unique<OtherPlayers>(temp.first, images[7], fonts[SETTINGS], radius, temp.second));
			}
		}
	//}

	m_players.erase(temp.first); //הורדת העיגול שלי מהשחקנים האחרים
	std::cout << "position: "<<temp.second.x << " " << temp.second.y << '\n';
	m_me->setId(temp.first);//עדכון העיגול שלי
	m_me->setTexture(images[5]);
	m_me->setPosition(temp.second);
	m_me->setCenter(temp.second + Vector2f{ NEW_PLAYER,NEW_PLAYER });
}
//===================================================================================
void Game::setSquare() {

	for (unsigned i = 0; i <= BOARD_SIZE.x / SQUARE; ++i) {
		std::vector<sq> temp;
		for (unsigned j = 0; j <= BOARD_SIZE.y / SQUARE; ++j)
			temp.push_back(std::make_shared<Square>(sf::Vector2f{ float(i*SQUARE),float(j*SQUARE) }));
		m_squares.push_back(temp);
		for (unsigned j = 0; j <= BOARD_SIZE.y / SQUARE; ++j) {
			if (j > 0) { m_squares[i][j]->_left = m_squares[i][j - 1]; m_squares[i][j - 1]->_right = m_squares[i][j]; }
			if (i > 0) { m_squares[i][j]->_down = m_squares[i - 1][j]; m_squares[i - 1][j]->_up = m_squares[i][j]; }
		}
	}
}

//====================================================================================
//================================     PLAY     ======================================
//====================================================================================
unsigned Game::play(sf::RenderWindow &w, const Images &images, const Fonts &fonts)
{
	m_socket.setBlocking(false);

	auto speed = TimeClass::instance().RestartClock();
	sf::Packet packet;
	while (m_me->getLive())
	{

		//תזוזה של השחקן
		if (m_receive) // אם הוא קלט את התזוזה הקודמת שלו
			updateMove(speed);

		//קבלת מידע מהשרת
		receiveChanges(images, fonts);

		draw(w);
	}

	return m_me->getScore();
}

//====================================================================================
//===========================      UPDATE MOVE       =================================
//====================================================================================
void Game::updateMove(float speed)
{
	sf::Packet packet;
	packet.clear();
	static unsigned num = 10;
	static char selected;
	move(speed);

	std::vector<Uint32> deleted;
	m_me->collision(deleted, m_objectsOnBoard, m_players, m_me.get());

	if (!m_me->getLive())
		deleted.push_back(m_me->getId()); // אם מתתי

	packet << m_me->getId() << m_me->getRadius() << m_me->getPosition() << deleted;

	if (m_socket.send(packet) != sf::TcpSocket::Done)
		std::cout << "no sending data\n";

	if (!m_me->getLive())
		Sleep(100);

	m_receive = false;
}
//-----------------------------------------------
void Game::move(float speed) {
	sf::Vector2u ss{ unsigned(m_me->getCenter().x) % SQUARE ,unsigned(m_me->getCenter().y) % SQUARE };
	sf::Vector2u s{ unsigned((m_me->getCenter().x + float(ss.x)) / SQUARE) ,unsigned((m_me->getCenter().y + float(ss.y)) / SQUARE) };
	std::cout <<"s:	"<< s.x << " " << s.y << '\n';
	//static std::stack<sq> dir;
	if (dir.empty()) {
	/*for (int i = 0; i < 600; i++)
			for (int j = 0; j < 600; j++)
				m_squares[i][j]->_visited = false;*/

		dir = bfs(m_squares[s.x][s.y]);
		if (dir.empty()) {
			std::cout << "empty\n";
			return;
		}
		speed = TimeClass::instance().RestartClock();
	}
	speed = TimeClass::instance().RestartClock();

	auto d = dir.top();
	std::cout << "d: "<< d->_ver.x / SQUARE << " " << d->_ver.y / SQUARE << '\n';
	if (m_squares[d->_ver.x / SQUARE][d->_ver.y / SQUARE]->_ver.x > m_squares[s.x][s.y]->_ver.x) {
		m_me->move(MOVE*speed, 0);
		sf::Vector2u ss{ unsigned(m_me->getCenter().x) % SQUARE ,unsigned(m_me->getCenter().y) % SQUARE };
		sf::Vector2u w{ unsigned((m_me->getCenter().x + float(ss.x)) / SQUARE) ,unsigned((m_me->getCenter().y + float(ss.y)) / SQUARE) };
		if (m_squares[d->_ver.x / SQUARE][d->_ver.y / SQUARE]->_ver.x <= m_squares[w.x][w.y]->_ver.x)
			dir.pop();
	}
	else if (m_squares[d->_ver.x / SQUARE][d->_ver.y / SQUARE]->_ver.x < m_squares[s.x][s.y]->_ver.x) {
		m_me->move(-MOVE*speed, 0);
		sf::Vector2u ss{ unsigned(m_me->getCenter().x) % SQUARE ,unsigned(m_me->getCenter().y) % SQUARE };
		sf::Vector2u w{ unsigned((m_me->getCenter().x + float(ss.x)) / SQUARE) ,unsigned((m_me->getCenter().y + float(ss.y)) / SQUARE) };
		if (m_squares[d->_ver.x / SQUARE][d->_ver.y / SQUARE]->_ver.x >= m_squares[w.x][w.y]->_ver.x)
			dir.pop();
	}
	else  if (m_squares[d->_ver.x / SQUARE][d->_ver.y / SQUARE]->_ver.y < m_squares[s.x][s.y]->_ver.y) {
		m_me->move(0, -MOVE*speed);
		sf::Vector2u ss{ unsigned(m_me->getCenter().x) % SQUARE ,unsigned(m_me->getCenter().y) % SQUARE };
		sf::Vector2u w{ unsigned((m_me->getCenter().x + float(ss.x)) / SQUARE) ,unsigned((m_me->getCenter().y + float(ss.y)) / SQUARE) };
		if (m_squares[d->_ver.x / SQUARE][d->_ver.y / SQUARE]->_ver.y >= m_squares[w.x][w.y]->_ver.y)
			dir.pop();
	}
	else if (m_squares[d->_ver.x / SQUARE][d->_ver.y / SQUARE]->_ver.y >= m_squares[s.x][s.y]->_ver.y) {
		m_me->move(0, MOVE*speed);
		sf::Vector2u ss{ unsigned(m_me->getCenter().x) % SQUARE ,unsigned(m_me->getCenter().y) % SQUARE };
		sf::Vector2u w{ unsigned((m_me->getCenter().x + float(ss.x)) / SQUARE) ,unsigned((m_me->getCenter().y + float(ss.y)) / SQUARE) };
		if (m_squares[d->_ver.x / SQUARE][d->_ver.y / SQUARE]->_ver.y <= m_squares[w.x][w.y]->_ver.y)
			dir.pop();
	}
	else { dir.pop(); m_me->move(-float(SQUARE), SQUARE); }
	std::cout << "end looop\n";


}
//===================== BFS ==================================
std::stack<sq> Game::bfs(sq square) {
	std::queue<sq> curr;//the current nodes that are taken care of
	curr.push(square);
	square->_visited = true;
	square->_parent = nullptr;
	std::set<sf::Uint32> intersection;
	std::stack<sq> a;
	a.push(square);
	std::vector<sq> cl;
	cl.push_back(square);
	while (!curr.empty()) {
		auto& tempC = curr.front();
		auto intersection = m_objectsOnBoard.colliding(pair(tempC->limitsLower(FOOD_RADIUS), tempC->limitsUpper(FOOD_RADIUS)));
		if (safeSquare(intersection, isFood, *tempC)) {
			for (auto& it = cl.begin(); it != cl.end(); ++it)
				(*it)->_visited = false;
			return tempC->findParent(square);
		}
		if (tempC->_down->update(tempC, (*this))) { 
			curr.push(std::ref(tempC->_down)); cl.push_back(tempC->_down); }
		if (tempC->_right->update(tempC, (*this))) {
			curr.push(std::ref(tempC->_right)); cl.push_back(tempC->_right);
		}
		if (tempC->_up->update(tempC, (*this))) {
			curr.push(std::ref(tempC->_up)); cl.push_back(tempC->_up);
		}
		if (tempC->_left->update(tempC, (*this))) { curr.push(std::ref(tempC->_left)); cl.push_back(tempC->_left); }
		curr.pop();
	}

	return a;
}
//==============================================================
std::stack<sq> Square::findParent(const sq& root) {
	std::stack<sq> stack;	
	_visited = false;
	while (_parent && _parent != root) {
		stack.push(std::ref(_parent));
		_parent = _parent->_parent;
	}
	if(stack.empty())
		stack.push(root);
	return stack;
}
//======================================================
bool Square::update(sq& parent, const Game& game) {
	if (!this || _visited)
		return false;

	auto intersection = game.getObjectsOnBoard().colliding(pair(limitsLower(BOMB_RADIUS), limitsUpper(BOMB_RADIUS)));
	if (game.safeSquare(intersection, isBomb, (*this))) {
		_visited = true;
		return false;
	}

	_parent = parent;
	_visited = true;
	return true;
}

//=================================================================
bool Square::collide(Circle* c, float r)const {
	return (distance(c->getPosition(), _ver /*- sf::Vector2f{ r,r }*/) < c->getRadius() + r
		|| distance(c->getPosition(), _ver + sf::Vector2f{ 0,float(SQUARE) }) < c->getRadius() + r
		|| distance(c->getPosition(), _ver + sf::Vector2f{ float(SQUARE) - r,0}) < c->getRadius() + r
		|| distance(c->getPosition(), _ver + sf::Vector2f{ float(SQUARE) ,float(SQUARE) }) < c->getRadius() + r);

}
//====================================================================================
//===========================      RECEIVE DATA      =================================
//====================================================================================
void Game::receiveChanges(const Images &images, const Fonts &fonts)
{
	sf::Packet packet;

	m_socket.receive(packet);
	while (!packet.endOfPacket())
	{
		std::pair<Uint32, sf::Vector2f> temp;
		if (!(packet >> temp))
			continue;
		std::vector<Uint32> del;

		if (temp.first >= FOOD_LOWER && temp.first <= BOMBS_UPPER) // אוכל או פצצות חדשות
			m_objectsOnBoard.insert(temp, images);

		else if (temp.first >= PLAYER_LOWER && temp.first <= PLAYER_UPPER)// שחקן
		{
			if (temp.first == m_me->getId())// השחקן שלי
				m_receive = true;

			else if (m_players.find(temp.first) != m_players.end())// תזוזה של שחקן (שחקן קיים..)י
			{
				m_players[temp.first]->setPosition(temp.second);
				m_players[temp.first]->setCenter();
				m_players[temp.first]->collision(del, m_objectsOnBoard, m_players, m_me.get());
			}

			else // שחקן חדש
				addPlayer(temp, packet, images, fonts);
		}
	}
	if(TimeClass::instance().getTime() >= 0.5f)
		m_receive = true;
	deleteDeadPlayer(m_players);
}
//------------------------------------------------------------------------------------
void Game::addPlayer(const std::pair<Uint32, sf::Vector2f> &temp, sf::Packet &packet, const Images &images, const Fonts &fonts)
{
	Uint32 image;
	sf::String s;
	packet >> image >> s;
	m_players.emplace(temp.first, std::make_unique<OtherPlayers>(temp.first, images[7], fonts[SETTINGS], NEW_PLAYER, temp.second));
}
//------------------------------------------------------------------------------------
void deleteDeadPlayer(std::unordered_map<Uint32, std::unique_ptr<OtherPlayers>>& players)
{
	for (auto it = players.begin(); it != players.end();)
		(it->second->getLive()) ? it++ : it = players.erase(it);
}
//====================================================================================
//===========================          PRINT         =================================
//====================================================================================
void Game::setView(sf::RenderWindow &w) const
{
	/*sf::View view;
	view.reset(sf::FloatRect{ 0,0,float(SCREEN_WIDTH),float(SCREEN_HEIGHT) });*/
	sf::Vector2f pos{ float(SCREEN_WIDTH) / 2 , float(SCREEN_HEIGHT) / 2 };

	if (m_me->getCenter().x > SCREEN_WIDTH / 2)
		if (BOARD_SIZE.x - m_me->getCenter().x < SCREEN_WIDTH / 2)
			pos.x = BOARD_SIZE.x - SCREEN_WIDTH / 2;
		else
			pos.x = m_me->getCenter().x;

	if (m_me->getCenter().y > SCREEN_HEIGHT / 2)
		if (BOARD_SIZE.y - m_me->getCenter().y < SCREEN_HEIGHT / 2)
			pos.y = BOARD_SIZE.y - SCREEN_HEIGHT / 2;
		else
			pos.y = m_me->getCenter().y;

	m_view.setCenter(pos);
	w.setView(m_view);
}
//--------------------------------------------------------------------------
void Game::draw(sf::RenderWindow &w) const
{
	setView(w);

	w.clear();
	w.draw(m_background);

	for (auto &x : m_objectsOnBoard)
		w.draw(*x.second.get());

	for (auto &x : m_players)
		w.draw(*x.second.get());

	w.draw(*m_me.get());

	w.display();

}

//*************************************************************************************
//****************************    PLAYER FUNCTION   ***********************************
//*************************************************************************************
void Player::collision(std::vector<Uint32> &deleted, Maps &objectsOnBoard, std::unordered_map<Uint32, std::unique_ptr<OtherPlayers>>& players, Player *me)
{
	checkFoodAndBomb(deleted, objectsOnBoard);
	checkPlayers(deleted, players, me);
}
//--------------------------------------------------------------------------
void Player::checkPlayers(std::vector<Uint32> &deleted, std::unordered_map<Uint32, std::unique_ptr<OtherPlayers>>& players, Player *me)
{
	for (auto &player : players)
	{
		if (player.second->getId() == getId())
			continue;

		if (circlesCollide(player.second.get()))
			if (getRadius() > player.second->getRadius()) //אם היתה התנגשות בין שניים אחרים והאחר מת
			{
				newRadius(player.second.get());
				player.second->setLive(false); //מחיקה, השחקן יודע שהוא מת
				deleted.push_back(player.first);
			}
			else
				m_live = false; // אם השחקן שקרא לפונקציה מת
	}


	if (dynamic_cast<OtherPlayers*>(this)) //בדיקה של שחקן נוכחי מול השחקן שלי
	{
		if (circlesCollide(me))
			if (getRadius() > me->getRadius())
			{
				me->setLive(false);
				newRadius(me);
			}
			else
			{
				m_live = false; //השחקן שמולי מת
				me->newRadius(this);
			}
	}

	deleteDeadPlayer(players);
}
//--------------------------------------------------------------------------
void Player::checkFoodAndBomb(std::vector<Uint32> &deleted, Maps &objectsOnBoard)
{
	std::set<Uint32> check = objectsOnBoard.colliding(getCenter(), getRadius());

	for (auto it : check) //מחיקה של אוכל ופצצות והוספה לוקטור
		if (circlesCollide(objectsOnBoard[it].get()))
		{
			newRadius(objectsOnBoard[it].get());
			objectsOnBoard.eraseFromData(it);
			deleted.push_back(it);
		}

	if (getRadius() < NEW_PLAYER)
		m_live = false;
}