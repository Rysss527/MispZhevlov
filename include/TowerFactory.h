#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <algorithm>
#include <vector>
#include <memory>
#include "Tower.h"
#include "SoundManager.h"
#include "Constants.h"

class TowerFactory
{
public:
	std::vector<std::unique_ptr<Tower>> towers;
	int towers_created;
	
	TowerFactory(int numTowers = 0)
	{
		towers_created = 0;
		towers.reserve(numTowers > 0 ? numTowers : 150);
	}
	
	TowerFactory(TowerFactory&& other) noexcept
		: towers(std::move(other.towers)), towers_created(other.towers_created)
	{
		other.towers_created = 0;
	}
	
	TowerFactory& operator=(TowerFactory&& other) noexcept
	{
		if (this != &other)
		{
			towers = std::move(other.towers);
			towers_created = other.towers_created;
			other.towers_created = 0;
		}
		return *this;
	}
	
	TowerFactory(const TowerFactory&) = delete;
	TowerFactory& operator=(const TowerFactory&) = delete;
	
	~TowerFactory() = default;
	void createTower(float x, float y, std::string cursor)
	{
		createTower(x, y, cursor, 1);
	}
	
	void createTower(float x, float y, std::string cursor, int upgradeLevel)
	{
		float newX = static_cast<float>(static_cast<int>(x / 100) * 100);
		float newY = static_cast<float>(static_cast<int>(y / 100) * 100);

		std::unique_ptr<Tower> newTower;
		if (cursor == "resourcegenerator")
		{
			newTower = std::make_unique<ResourceGenerator>(newX, newY);
		}
		else if (cursor == "basicshooter")
		{
			newTower = std::make_unique<BasicShooter>(newX, newY);
		}
		else if (cursor == "rapidshooter")
		{
			newTower = std::make_unique<RapidShooter>(newX, newY);
		}
		else if (cursor == "freezetower")
		{
			newTower = std::make_unique<FreezeTower>(newX, newY);
		}
		else if (cursor == "areaattacktower")
		{
			newTower = std::make_unique<AreaAttackTower>(newX, newY);
		}
		else if (cursor == "barriertower")
		{
			newTower = std::make_unique<BarrierTower>(newX, newY);
		}
		else
		{
			newTower = std::make_unique<BombTower>(newX, newY);
		}
		
		newTower->setUpgradeLevel(upgradeLevel);
		towers.push_back(std::move(newTower));
		towers_created++;
	}
	bool isTowerThere(float x, float y)
	{
		for (const auto& tower : towers)
		{
			if (tower && tower->operational)
			{
				sf::FloatRect towerBounds = tower->visual.getGlobalBounds();
				if (towerBounds.contains(x, y))
				{
					return true;
				}
			}
		}
		return false;
	}

	void removeInactiveTowers()
	{
		auto it = std::remove_if(towers.begin(), towers.end(),
			[](const std::unique_ptr<Tower>& tower) {
				return !tower || !tower->operational;
			});
		towers.erase(it, towers.end());
		towers_created = static_cast<int>(towers.size());
	}

	void removeTower(float x, float y)
	{
		auto it = std::find_if(towers.begin(), towers.end(),
			[x, y](const std::unique_ptr<Tower>& tower) {
				if (!tower) return false;
				sf::FloatRect towerBounds = tower->visual.getGlobalBounds();
				return towerBounds.contains(x, y);
			});
		
		if (it != towers.end())
		{
			towers.erase(it);
			towers_created = static_cast<int>(towers.size());
		}
	}
	void draw(sf::RenderWindow &window)
	{
		drawSprites(window);
	}
	
	void drawSprites(sf::RenderWindow &window)
	{
		for (const auto& tower : towers)
		{
			if (tower && tower->operational)
			{
				tower->drawSprite(window);
				if (tower->category == "AttackTower")
				{
					auto* attackTower = static_cast<AttackTower*>(tower.get());
					attackTower->updateBullet();
					attackTower->drawBullet(window);
				}
			}
		}
	}
	
	void drawLabels(sf::RenderWindow &window)
	{
		for (const auto& tower : towers)
		{
			if (tower && tower->operational)
			{
				tower->drawLabel(window);
			}
		}
	}
	void move()
	{
		for (const auto& tower : towers)
		{
			if (!tower) continue;
			
			if (tower->vitality <= 0)
			{
				tower->operational = false;
				if (tower->type == "BombTower")
				{
					SoundManager::getInstance().playSound("cherrybomb.wav");
				}
			}
			
			if (tower->operational)
			{
				tower->move();
				if (tower->type == "BasicShooter")
				{
				}
				else if (tower->type == "ResourceGenerator")
				{
					static_cast<ResourceGenerator*>(tower.get())->generateResource();
				}
				else if (tower->type == "RapidShooter")
				{
				}
				else if (tower->type == "FreezeTower")
				{
				}
				else if (tower->type == "AreaAttackTower")
				{
				}
			}
		}
	}
	void Serialize(std::ostream &stream) const
	{
		stream << towers_created << std::endl;
		for (const auto& tower : towers)
		{
			if (tower)
			{
				tower->Serialize(stream);
			}
		}
	}
	void Deserialize(std::istream &stream)
	{
		int numTowers;
		stream >> numTowers;
		towers.clear();
		towers_created = numTowers;
		towers.reserve(numTowers);
		
		for (int i = 0; i < numTowers; i++)
		{
			std::string towerType;
			std::string category;
			stream >> towerType;
			stream >> category;

			std::unique_ptr<Tower> tower;
			if (towerType == "BasicShooter")
			{
				tower = std::make_unique<BasicShooter>();
			}
			else if (towerType == "ResourceGenerator")
			{
				tower = std::make_unique<ResourceGenerator>();
			}
			else if (towerType == "RapidShooter")
			{
				tower = std::make_unique<RapidShooter>();
			}
			else if (towerType == "FreezeTower")
			{
				tower = std::make_unique<FreezeTower>();
			}
			else if (towerType == "BarrierTower")
			{
				tower = std::make_unique<BarrierTower>();
			}
			else if (towerType == "BombTower")
			{
				tower = std::make_unique<BombTower>();
			}
			else if (towerType == "AreaAttackTower")
			{
				tower = std::make_unique<AreaAttackTower>();
			}
			else
			{
				tower = std::make_unique<Tower>();
			}
			tower->Deserialize(stream);
			towers.push_back(std::move(tower));
		}
	}
};
