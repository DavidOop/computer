#pragma once
#include "Game.h"
#include <iostream>
#include <vector>
#include <Windows.h>
#include <queue>
//====================================================================================
//================================ CONSTRUCTOR =======================================
//====================================================================================
Game::Game(const Images &images, Uint32 image_id, sf::View& view)
	:m_me(std::make_unique<MyPlayer>()),
	m_background(images[int(BACKGROUND)]),
	m_view(view)
{
	if (m_socket.connect(sf::IpAddress::LocalHost, 5555) != sf::TcpSocket::Done)
		//	if (m_socket.connect("10.2.16.95", 5555) != sf::TcpSocket::Done)
		std::cout << "no connecting\n";
	setSquare();
	sf::Packet packet;
	packet << image_id; //����� ���� �� ������ ���
	if (m_socket.send(packet) != sf::TcpSocket::Done)
		std::cout << "no sending image\n";

	receive(images);//����� ���� �����
}
//--------------------------------------------------------------------------
void Game::receive(const Images &images)
{
	sf::Packet packet;
	std::pair <Uint32, sf::Vector2f> temp;
	Uint32 image;
	float radius;

	m_socket.setBlocking(true);//**********************************
	Sleep(100);//*********************************

	auto status = m_socket.receive(packet);

	if (status == sf::TcpSocket::Done)
	{
		while (!packet.endOfPacket())//����� �� �� ������ ��� ����
		{
			//std::cout << "while\n";
			packet >> temp;

			if (temp.first >= FOOD_LOWER && temp.first <= BOMBS_UPPER)
				m_objectsOnBoard.insert(temp);

			else if (temp.first >= PLAYER_LOWER && temp.first <= PLAYER_UPPER)//???????????????????????
			{
				packet >> radius >> image;
				m_players.emplace(temp.first, std::make_unique<OtherPlayers>(temp.first, images[image], radius, temp.second));
			}
		}
	}

	m_players.erase(temp.first); //����� ������ ��� �������� ������

	m_me->setId(temp.first);//����� ������ ���
	m_me->setTexture(images[image]);
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
unsigned Game::play(sf::RenderWindow &w, const Images &images)
{
	m_socket.setBlocking(false);
	draw(w);

	sf::Packet packet;
	while (true)
	{
		auto speed = TimeClass::instance().RestartClock();

		//����� �� �����
		if (m_receive) // �� ��� ��� �� ������ ������ ���
			if (!updateMove(speed))
				return m_me->getScore();
		m_receive = false;


		//���� ���� �����
		if (!receiveChanges(images))
			return m_me->getScore();

		//draw(w);
	}


	return 0;
}

//====================================================================================
//===========================      UPDATE MOVE       =================================
//====================================================================================
//����� ��� �� ����
bool Game::updateMove(float speed)
{
	sf::Packet packet;
	packet.clear();
	bool temp = true;
	static unsigned num = 10;
	static char selected;
	move(speed);
	//std::unique_ptr<MyPlayer> tempMe = std::make_unique<MyPlayer>(*m_me.get());

	std::vector<Uint32> deleted;

	temp = m_me->collision(deleted, m_objectsOnBoard, m_players, m_me.get());

	if (m_me->getRadius() < NEW_PLAYER)
		temp = false;

	if (!temp)
		deleted.push_back(m_me->getId()); // �� ����

	packet << m_me->getId() << m_me->getRadius() << m_me->getPosition() << deleted;

	if (m_socket.send(packet) != sf::TcpSocket::Done)
		std::cout << "no sending data\n";
	if (!temp)
		Sleep(200);

	return temp;
}
//-----------------------------------------------
void Game::move(float speed) {
	sf::Vector2u s{ unsigned(m_me->getCenter().x / SQUARE) ,unsigned(m_me->getCenter().y / SQUARE) };
	//std::cout << 's'<<s.x << " " << s.y << '\n';
	auto d = bfs(m_squares[s.x][s.y]);
	//std::cout << 'd'<<direction->_ver.x << " " << direction->_ver.y << '\n';
	if (m_squares[d->_ver.x / SQUARE][d->_ver.y / SQUARE] == m_squares[s.x][s.y]->_up)m_me->move(0, speed*MOVE);
	if (m_squares[d->_ver.x / SQUARE][d->_ver.y / SQUARE] == m_squares[s.x][s.y]->_down)m_me->move(0, -speed*MOVE);
	if (m_squares[d->_ver.x / SQUARE][d->_ver.y / SQUARE] == m_squares[s.x][s.y]->_left)m_me->move(-speed*MOVE, 0);
	if (m_squares[d->_ver.x / SQUARE][d->_ver.y / SQUARE] == m_squares[s.x][s.y]->_right)m_me->move(speed*MOVE, 0);
	//std::cout << m_me->getCenter().x<<" "<< m_me->getCenter().y<<'\n';
	clear();

}
//===================== BFS ==================================
sq Game::bfs(sq square) {
	std::queue<sq> curr;//the current nodes that are taken care of
	curr.push(square);
	square->_visited = true;
	square->_parent;
	std::set<sf::Uint32> intersection;
	//for (auto i = m_objectsOnBoard.begin(); i != m_objectsOnBoard.end(); i++)
	//	intersection.emplace(i->first);
	while (!curr.empty()) {
		auto& tempC = curr.front();
		//	std::cout << tempC->_ver.x << " " << tempC->_ver.y << '\n';
		auto intersection = m_objectsOnBoard.colliding(pair(tempC->limitsLower(FOOD_RADIUS), tempC->limitsUpper(FOOD_RADIUS)));
		//if (!intersection.empty())
	//		std::cout << "ss\n";
		if (safeSquare(intersection, isFood, *tempC)) {
			auto t = tempC->findParent(square);
		//	while (!curr.empty()) {
		//		curr.front()->clear();
		//		curr.front()->_visited = false;
		//		curr.pop();
		//	}
		//	square->_visited = false;
			return t;
		}
		if (tempC->_down->update(tempC, (*this)))curr.push(std::ref(tempC->_down));
		if (tempC->_right->update(tempC, (*this)))curr.push(std::ref(tempC->_right));
		if (tempC->_up->update(tempC, (*this)))curr.push(std::ref(tempC->_up));
		if (tempC->_left->update(tempC, (*this)))curr.push(std::ref(tempC->_left));
		curr.pop();
	}

	return square;
}
//=================================================================
void Game::clear() {
	//if (!this && !_parent && !_visited)
	//	return;
	////*this = *_parent;
	//*this = *_parent;
	//clear();
	//_visited = false;
	//_parent = nullptr;
	for(size_t i=0;i<m_squares.size();i++)
		for (size_t j = 0; j < m_squares.size(); j++)
		{
			m_squares[i][j]->_parent = nullptr;
			m_squares[i][j]->_visited = false;
		}

}
//==============================================================
sq Square::findParent(const sq& root) {
	while (_parent && _parent->_ver != root->_ver) {
		_visited = false;
		*this = *_parent;
	}
	return std::make_shared<Square>(this->_ver);
	//_parent = nullptr;
}
//======================================================
bool Square::update(sq& parent, const Game& game) {
	if (!this || _visited)
		return false;

	auto intersection = game.getObjectsOnBoard().colliding(pair(limitsLower(BOMB_RADIUS), limitsUpper(BOMB_RADIUS)));
	if (game.safeSquare(intersection, isBomb, (*this)))
		return false;

	_parent = parent;
	_visited = true;
	return true;
}
//=====================================================================================

//=================================================================
bool Square::collide(Circle* c)const {
	//int a;
	//std::cout << c->getCenter().x << " " << c->getCenter().y << '\n';
//	if(c->getCenter(), _ver)
	//std::cout << _ver.x << " " << _ver.y << '\n';
	//return c->getGlobalBounds().contains(_ver);
	return (distance(c->getCenter(), _ver) < c->getRadius()
		&& distance(c->getCenter(), _ver + sf::Vector2f{ 0,float(SQUARE) }) < c->getRadius()
		&& distance(c->getCenter(), _ver + sf::Vector2f{ float(SQUARE),0 }) < c->getRadius()
		&& distance(c->getCenter(), _ver + sf::Vector2f{ float(SQUARE),float(SQUARE) }) < c->getRadius());
	//	return true;
		//return true;
}
//====================================================================================
//===========================      RECEIVE DATA      =================================
//====================================================================================
//����� ��� �� ����
bool Game::receiveChanges(const Images &images)
{
	sf::Packet packet;

	static int receiv = 0;
	//std::cout << "receive\n";

	m_socket.receive(packet);

	while (!packet.endOfPacket())
	{
		std::pair<Uint32, sf::Vector2f> temp;
		if (!(packet >> temp))
			continue;
		std::vector<Uint32> del;

		//	std::cout << temp.first << std::endl;

		if (temp.first >= FOOD_LOWER && temp.first <= BOMBS_UPPER) // ���� �� ����� �����
		{
			m_objectsOnBoard.insert(temp);
			//	std::cout << "receive " << receiv <<" "<<temp.first<< '\n';
			//std::cout << temp.first << std::endl;
		}
		else if (temp.first >= PLAYER_LOWER && temp.first <= PLAYER_UPPER)// ����
		{
			if (temp.first == m_me->getId())// ����� ���
			{
				m_receive = true;
			}

			else if (m_players.find(temp.first) != m_players.end())// ����� �� ���� (���� ����..)�
			{
				//	m_players[temp.first]->setPosition(temp.second);
					//m_players[temp.first]->setCenter(m_players[temp.first]->getPosition() + Vector2f{ m_players[temp.first]->getRadius(),m_players[temp.first]->getRadius() });
				m_players[temp.first]->setCenter(temp.second);
				if (!m_players[temp.first]->collision(del, m_objectsOnBoard, m_players, m_me.get()))
					return false; //�� ����� ��� ����
			}
			else // ���� ���
			{
				addPlayer(temp, packet, images);
				/*Uint32 image;
				packet >> image;
				m_players.emplace(temp.first, std::make_unique<OtherPlayers>(temp.first, images[image], NEW_PLAYER, temp.second));*/
			}
		}
	}
	return true;
}
//------------------------------------------------------------------------------------
void Game::addPlayer(const std::pair<Uint32, sf::Vector2f> &temp, sf::Packet &packet, const Images &images)
{
	Uint32 image;
	packet >> image;
	m_players.emplace(temp.first, std::make_unique<OtherPlayers>(temp.first, images[image], NEW_PLAYER, temp.second));
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
// ����� ��� �� ����
bool Player::collision(std::vector<Uint32> &deleted, Maps &objectsOnBoard, std::unordered_map<Uint32, std::unique_ptr<OtherPlayers>>& players, Player *me)
{
	checkFoodAndBomb(deleted, objectsOnBoard);
	return checkPlayers(deleted, players, me);
}
//--------------------------------------------------------------------------
bool Player::checkPlayers(std::vector<Uint32> &deleted, std::unordered_map<Uint32, std::unique_ptr<OtherPlayers>>& players, Player *me)
{
	bool temp = true;
	std::vector<Uint32> del;
	for (auto &player : players)
	{
		if (player.second->getId() == getId())
			continue;
		if (circlesCollide(player.second.get()))
			if (getRadius() > player.second->getRadius()) //�� ���� ������� ��� ����� ����� ����� ��
			{
				setScore(Uint32(player.second->getRadius()));
				newRadius(player.second.get());
				del.push_back(player.first);
				deleted.push_back(player.first);
			}
			else
				temp = (dynamic_cast<MyPlayer*>(this)) ? false : true; //�� ������ �� (�� ����� ���, ����� �����)�
	}

	//if (getId() != me->getId()) //����� �� ���� ����� ��� ����� ���
	if (dynamic_cast<OtherPlayers*>(this))
	{
		if (circlesCollide(me))
			if (getRadius() > me->getRadius())
				temp = false;
			else
			{
				del.push_back(getId());
				me->setScore(Uint32(getRadius()));
				me->newRadius(this);
			}
	}

	for (auto pl : del)
		players.erase(pl);

	return temp;
}
//--------------------------------------------------------------------------
void Player::checkFoodAndBomb(std::vector<Uint32> &deleted, Maps &objectsOnBoard)
{
	std::set<Uint32> check = objectsOnBoard.colliding(getCenter(), getRadius());

	for (auto it : check) //����� �� ���� ������ ������ ������
		if (circlesCollide(objectsOnBoard[it].get())) {
			newRadius(objectsOnBoard[it].get());
			objectsOnBoard.eraseFromData(it);
			deleted.push_back(it);
		}
}