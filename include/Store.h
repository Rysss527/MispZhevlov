#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include "GameCursor.h"
#include "GamePlayer.h"
#include "ResourceManager.h"


struct Shop {
    std::vector<std::string> allowedUnits;
    void setAllowedUnits(const std::vector<std::string>& units) { allowedUnits = units; }
	coordinates cordinates;
	sf::Sprite sprite;
	sf::Texture texture;
	sf::Texture disable;
	sf::Sprite* disable_array;

	Shop() {
		cordinates.x = 0;
		cordinates.y = 0;
		auto& rm = DefenseSynth::ResourceManager::getInstance();
		rm.loadTextureWithRetry("assets/images/shop.png", "shop", 1);
		sprite.setTexture(rm.getTexture("shop"));
		rm.loadTextureWithRetry("assets/images/disabler.png", "disabler", 1);
		disable_array = new sf::Sprite[7];
		for (int i = 0; i < 7; i++)
		{
			disable_array[i].setTexture(rm.getTexture("disabler"));
			disable_array[i].setPosition(static_cast<float>(100 * (i + 1)), 0.0f);
			sf::Color color = disable_array[i].getColor();
			color.a = 180;
			disable_array[i].setColor(color);
		}
	}
	void draw(sf::RenderWindow& window, Player& player,int level) {
		window.draw(sprite);
        
        auto getCost = [](const std::string& type) -> int {
            if (type == "basicshooter") return 50;
            if (type == "rapidshooter") return 200;
            if (type == "freezetower") return 100;
            if (type == "areaattacktower") return 75;
            if (type == "barriertower") return 50;
            if (type == "bombtower") return 150;
            if (type == "resourcegenerator") return 100;
            return 50;
        };
        
        auto requiresLevel = [](const std::string& type) -> int {
            return 1;
        };
        
        auto getSlotForUnit = [](const std::string& unitType) -> int {
            if (unitType == "basicshooter") return 0;
            if (unitType == "rapidshooter") return 1;
            if (unitType == "freezetower") return 2;
            if (unitType == "areaattacktower") return 3;
            if (unitType == "barriertower") return 4;
            if (unitType == "bombtower") return 5;
            if (unitType == "resourcegenerator") return 6;
            return -1;
        };
        
        std::vector<bool> slotEnabled(7, false);
        
        for (const auto& unitType : allowedUnits) {
            int slot = getSlotForUnit(unitType);
            if (slot >= 0 && slot < 7) {
                int cost = getCost(unitType);
                int requiredLevel = requiresLevel(unitType);
                if (player.currency >= cost && level >= requiredLevel) {
                    slotEnabled[slot] = true;
                }
            }
        }
        
        for (int i = 0; i < 7; i++) {
            if (!slotEnabled[i]) {
                window.draw(disable_array[i]);
            }
        }

	}
};
