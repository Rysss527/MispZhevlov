#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "GameCursor.h"
#include <fstream>
#include <vector>
#include <memory>
#include <algorithm>
#include "ResourceManager.h"

class LawnMower {
public:
	coordinates position;
	sf::Sprite sprite;
	sf::Texture texture;
	float speed;
	bool shouldMove;
	bool exist;
	sf::Clock clock;
	LawnMower(float x = 0, float y = 0) {
		sprite.setTexture(DefenseSynth::ResourceManager::getInstance().getTexture("lawnmower"));
		sprite.setPosition(x, y);
		position.x = x;
		position.y = y;
		speed = 15;
		exist = true;
		shouldMove = false;
	}
	LawnMower(const LawnMower& lawnmower) {
		texture = lawnmower.texture;
		sprite = lawnmower.sprite;
		position.x = lawnmower.position.x;
		position.y = lawnmower.position.y;
		speed = lawnmower.speed;
		exist = lawnmower.exist;
		clock = lawnmower.clock;
		shouldMove = lawnmower.shouldMove;
	}
	void move() {
		if (exist && shouldMove) {
			if (clock.getElapsedTime().asSeconds() > 0.1) {
				position.x += speed;
				sprite.setPosition(position.x, position.y);
				if (position.x > 1300) {
					exist = false;
				}
				clock.restart();
			}
		}
	}
	void draw(sf::RenderWindow& window) {
		if (exist) {
			window.draw(sprite);
		}
	}

	void Serialize(std::ostream& stream) const {
stream << position.x << std::endl;
		stream << position.y << std::endl;
		stream << exist << std::endl;
		stream << shouldMove << std::endl;
	}

	void Deserialize(std::istream& stream) {
stream >> position.x;
		stream >> position.y;
		stream >> exist;
		stream >> shouldMove;
sprite.setPosition(position.x, position.y);
std::cout <<"Deserialized LawnMower" << std::endl;
std::cout <<"Attributes: " << position.x << " " << position.y << " " << exist << " " << shouldMove << std::endl;
	}


};

class LawnMowerFactory {
public:
	std::vector<std::unique_ptr<LawnMower>> lawnmowers;
	LawnMowerFactory(int numLawnMowers = 0) {
		lawnmowers.reserve(numLawnMowers);
		for (int i = 0; i < numLawnMowers; i++) {
			lawnmowers.push_back(std::make_unique<LawnMower>(100.0f, static_cast<float>((i+2)*100)));
		}
	}
	bool isLawnMowerThere(float x, float y) {
		return std::any_of(lawnmowers.begin(), lawnmowers.end(),
			[x, y](const std::unique_ptr<LawnMower>& lawnmower) {
				sf::FloatRect lawnmowerBounds = lawnmower->sprite.getGlobalBounds();
				return lawnmowerBounds.contains(x, y);
			});
	}
	void draw(sf::RenderWindow& window) {
		for (const auto& lawnmower : lawnmowers) {
			lawnmower->draw(window);
		}
	}
	void move() {
		for (const auto& lawnmower : lawnmowers) {
			lawnmower->move();
		}
	}

	void removeLawnMower(float x, float y) {
		auto it = std::find_if(lawnmowers.begin(), lawnmowers.end(),
			[x, y](const std::unique_ptr<LawnMower>& lawnmower) {
				sf::FloatRect lawnmowerBounds = lawnmower->sprite.getGlobalBounds();
				return lawnmowerBounds.contains(x, y);
			});
		if (it != lawnmowers.end()) {
			lawnmowers.erase(it);
		}
	}
	void Serialize(std::ostream& stream) const {
		stream << lawnmowers.size() << std::endl;
		for (const auto& lawnmower : lawnmowers) {
			lawnmower->Serialize(stream);
		}
	}

	void Deserialize(std::istream& stream) {
		size_t temp_created;
		stream >> temp_created;
		lawnmowers.clear();
		lawnmowers.reserve(temp_created);
		for (size_t i = 0; i < temp_created; i++) {
			auto lawnmower = std::make_unique<LawnMower>();
			lawnmower->Deserialize(stream);
			lawnmowers.push_back(std::move(lawnmower));
		}
	}
	
	void resetForNextLevel(int numLawnMowers)
	{
		lawnmowers.clear();
		lawnmowers.reserve(numLawnMowers);
		for (int i = 0; i < numLawnMowers; i++)
		{
			lawnmowers.push_back(std::make_unique<LawnMower>(100.0f, static_cast<float>((i+2)*100)));
		}
	}

};
