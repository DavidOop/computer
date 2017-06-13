#pragma once
#include "Game.h"
#include <iostream>
#include <vector>
#include <Windows.h>

//====================================================================================
//================================ CONSTRUCTOR =======================================
//====================================================================================
Game::Game(const Images &images, Uint32 image_id)
	:m_me(std::make_unique<MyPlayer>()),
	m_background(images[int(BACKGROUND)])
{
	if (m_socket.connect(sf::IpAddress::LocalHost, 5555) != sf::TcpSocket::Done)
		//if (m_socket.connect("10.2.15.207", 5555) != sf::TcpSocket::Done)
		std::cout << "no connecting\n";

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

			if (temp.first >= 1000 && temp.first <= 10000)
				m_objectsOnBoard.insert(temp);

			else if (temp.first >= 200 && temp.first <= 300)//???????????????????????
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

//====================================================================================
//================================     PLAY     ======================================
//====================================================================================
unsigned Game::play(sf::RenderWindow &w, const Images &images)
{
	m_socket.setBlocking(false);

	sf::Packet packet;
	while (true)
	{
		auto speed = TimeClass::instance().RestartClock();

		//����� �� �����
		if (!updateMove(speed))
			return m_me->getScore();

		//���� ���� �����
		if (!receiveChanges(images))
			return m_me->getScore();

		draw(w);
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
	/*if (num >= 10) {
		selected = move(speed);
		num = 0;
	}
	else {
		switch (selected)
		{
		case 'u':
			m_me->move(0, MOVE*speed);
			break;
		case 'd':
			m_me->move(0, -MOVE*speed);
			break;
		case 'l':
			m_me->move(-MOVE*speed, 0);
			break;
		case 'r':
			m_me->move(MOVE*speed, 0);
			break;
		default:
			break;
		}
	}*/
	//std::cout << m_me->getCenter().x << " " << m_me->getCenter().y << '\n';
	num++;
	std::vector<Uint32> deleted;

	temp = m_me->collision(deleted, m_objectsOnBoard, m_players, m_me.get());
	packet << m_me->getId() << m_me->getRadius() << m_me->getPosition() << deleted;

	if (m_socket.send(packet) != sf::TcpSocket::Done)
		std::cout << "no sending data\n";


	return temp;
}
//--------------------------------------------------------------------------
//==========================================================================
//char Game::move(float speed) {
//
//	//	std::cout << "move f\n";
//	float max = -1;
//	sf::Vector2f temp1 = m_me->getCenter();
//	char d;
//
//	float up = go({ temp1.x , temp1.y + speed*MOVE }, max);
//	float down = go({ temp1.x , temp1.y - speed*MOVE }, max);
//	float left = go({ temp1.x - speed*MOVE, temp1.y }, max);
//	float right = go({ temp1.x + speed*MOVE, temp1.y }, max);
//
//	if (max == up)d = 'u';
//	if (max == down)d = 'd';
//	if (max == right)d = 'r';
//	if (max == left)d = 'l';
//
//	return d;
//
//}
//-----------------------------------------------------------
//float Game::go(const pair& temp, float& max) {
//	float tempR = direction(temp);
//	if (tempR > max) {
//		max = tempR;
//		m_me->setCenter(temp);
//	}
//	return tempR;
//}
//-----------------------------------------------
char Game::move(float speed) {

	speed *= MOVE;
	const float RADIUS_CHECK = 500;
	float max = -1, tempR;
	auto x = m_me->getCenter().x;
	auto y = m_me->getCenter().y;
	char d;
	sf::Vector2f temp, moveTo;
	std::cout << "r: ";
	temp = sf::Vector2f{ x + speed, y };//right
	if (temp.x + m_me->getRadius()  < BOARD_SIZE.x) {
		tempR = direction(pair({ temp.x, temp.y - RADIUS_CHECK }, { temp.x + RADIUS_CHECK, temp.y + RADIUS_CHECK }));// up
		if (tempR > max) {
			moveTo = temp;
			max = tempR;
			d = 'r';
		//	std::cout << "right: " << tempR << '\n';
		}
	}
	std::cout << "l: ";

	temp = sf::Vector2f{ x - speed, y };//left
	if (temp.x - m_me->getRadius()  > 0) {
		tempR = direction(pair({ temp.x - RADIUS_CHECK, temp.y - RADIUS_CHECK }, { temp.x, temp.y+ RADIUS_CHECK }));
		if (tempR > max) {
			moveTo = temp;
			max = tempR;
			d = 'l';
	//		std::cout << "left: " << tempR << '\n';
		}
	}
	std::cout << "d: ";

	temp = sf::Vector2f{ x , y - speed };
	if (temp.y - m_me->getRadius() > 0) {
		tempR = direction(pair({ temp.x - RADIUS_CHECK, temp.y - RADIUS_CHECK }, { temp.x + RADIUS_CHECK, temp.y }));
		if (tempR > max) {// down
			moveTo = temp;//
			max = tempR;
			d = 'd';
	//		std::cout << "down: " << tempR << '\n';

		}
	}
	std::cout << "u: ";
	temp = sf::Vector2f{ x , y + speed };
	if (temp.y + m_me->getRadius()  < BOARD_SIZE.y) {
		tempR = direction(pair({ temp.x - RADIUS_CHECK , temp.y }, { temp.x + RADIUS_CHECK, temp.y + RADIUS_CHECK }));
		if (tempR > max) {// up
			moveTo = temp;//
			d = 'u';
		//	std::cout << "up: " << tempR << '\n';
		}
	}
	m_me->setCenter(moveTo);
	std::cout << '\n';
	//std::cout << "=========================" << max << '\n';
	return d;
}
//==========================================================================================================
float Game::direction(const pair& ver) {
	
	auto intersection = m_objectsOnBoard.colliding(ver);
	float sum = 0;
	for (auto it = intersection.begin(); it != intersection.end(); ++it) {
		if (*it >= 1000 && *it <= 5000)
		//sum += (FOOD_RADIUS*(float(rand() % 3) + 1.f));
		//else if (isIntersect(m_data.find(*it)->second.getId()))//check there are no bombs
		sum += FOOD_RADIUS;
		//	sum = -100000.f;
	}
	std::cout << sum << '\n';
	return sum;
}
//====================================================================================
//===========================      RECEIVE DATA      =================================
//====================================================================================
//����� ��� �� ����
bool Game::receiveChanges(const Images &images)
{
	sf::Packet packet;
	if (m_socket.receive(packet) == sf::TcpSocket::Done) {
		static int c = 0;
		while (!packet.endOfPacket())
		{
			std::pair<Uint32, sf::Vector2f> temp;
			packet >> temp;
			std::vector<Uint32> del;

			if (temp.first >= 1000 && temp.first <= 10000) { // ���� �� ����� �����
				m_objectsOnBoard.insert(temp);
				c++;
			}

			else if (temp.first >= 200 && temp.first <= 300)// ����
			{
				if (temp.first == m_me->getId())// ����� ���
					continue;
				if (m_players.find(temp.first) != m_players.end())// ����� �� ���� (���� ����..)�
				{
					m_players[temp.first]->setPosition(temp.second);
					m_players[temp.first]->setCenter(m_players[temp.first]->getPosition() + Vector2f{ m_players[temp.first]->getRadius(),m_players[temp.first]->getRadius() });
					if (!m_players[temp.first]->collision(del, m_objectsOnBoard, m_players, m_me.get()))
						return false; //�� ����� ��� ����
				}
				else // ���� ���
				{
					Uint32 image;
					packet >> image;
					m_players.emplace(temp.first, std::make_unique<OtherPlayers>(temp.first, images[image], NEW_PLAYER, temp.second));
				}
			}
		}
		std::cout << "food :" << c << '\n';
	}
	return true;
}

//====================================================================================
//===========================          PRINT         =================================
//====================================================================================
void Game::setView(sf::RenderWindow &w) const
{
	sf::View view;
	view.reset(sf::FloatRect{ 0,0,float(SCREEN_WIDTH),float(SCREEN_HEIGHT) });
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

	view.setCenter(pos);
	w.setView(view);
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
				temp = (getId() == me->getId()) ? false : true; //�� ������ �� (�� ����� ���, ����� �����)�
	}

	if (getId() != me->getId()) //����� �� ���� ����� ��� ����� ���
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
		if (distance(getCenter(), objectsOnBoard[it]->getCenter()) <= getRadius() + objectsOnBoard[it]->getRadius())
		{
			newRadius(objectsOnBoard[it].get());
			objectsOnBoard.eraseFromData(it);
		}
}