#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "../GamePlayer.h"
#include "../Resource.h"
#include "../TowerFactory.h"

namespace DefenseSynth {

class PlayerInteractionService {
public:
    PlayerInteractionService();
    ~PlayerInteractionService() = default;
    
    void handleSunCollection(const sf::Vector2f& clickPos, Player& player, SunFactory* sunFactory);
    bool handleTowerPlacement(const sf::Vector2f& clickPos, 
                             const std::string& towerType,
                             Player& player,
                             TowerFactory* towerFactory,
                             int upgradeLevel = 1);
    bool handleTowerRemoval(const sf::Vector2f& clickPos, 
                           Player& player,
                           TowerFactory* towerFactory);
    
    bool canAffordTower(const Player& player, const std::string& towerType, int upgradeLevel = 1) const;
    int getTowerCost(const std::string& towerType, int upgradeLevel = 1) const;
    int getTowerRefund(const std::string& towerType, int upgradeLevel = 1) const;
    
    bool isValidPlacementPosition(const sf::Vector2f& pos, TowerFactory* towerFactory) const;
    sf::Vector2f snapToGrid(const sf::Vector2f& pos) const;
    
    void updateSelectedCard(const std::string& cardType);
    std::string getSelectedCard() const { return selectedCard; }
    void clearSelection() { selectedCard = ""; }
    
    void processSunGeneration(Player& player, SunFactory* sunFactory, float deltaTime);
    
private:
    std::string selectedCard;
    sf::Clock sunGenerationClock;
    
    static constexpr float GRID_OFFSET_X = 296.0f;
    static constexpr float GRID_OFFSET_Y = 162.0f;
    static constexpr float TILE_WIDTH = 100.0f;
    static constexpr float TILE_HEIGHT = 100.0f;
    static constexpr int GRID_ROWS = 5;
    static constexpr int GRID_COLS = 9;
    static constexpr float SUN_GENERATION_INTERVAL = 3.0f;
    static constexpr int SUN_VALUE = 10;
    
    int getGridRow(float y) const;
    int getGridColumn(float x) const;
    bool isWithinGrid(float x, float y) const;
};

}
