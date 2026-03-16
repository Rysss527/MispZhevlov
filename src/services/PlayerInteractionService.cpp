#include "services/PlayerInteractionService.h"
#include <algorithm>
#include <cmath>
#include "SoundManager.h"
#include "CrystalManager.h"

namespace DefenseSynth {

PlayerInteractionService::PlayerInteractionService() : selectedCard("") {
    sunGenerationClock.restart();
}

void PlayerInteractionService::handleSunCollection(const sf::Vector2f& clickPos, Player& player, SunFactory* sunFactory) {
    if (!sunFactory) return;
    
    if (sunFactory->isSunThere(clickPos.x, clickPos.y)) {
        int lvl = CrystalManager::getInstance().getSunUpgradeLevel();
        float mult = 1.0f + lvl * 0.2f;
        int gained = static_cast<int>(std::round(SUN_VALUE * mult));
        if (gained < 1) gained = 1;
        player.currency += gained;
        SoundManager::getInstance().playSound("points.wav");
        sunFactory->moveSunToOrigin(clickPos.x, clickPos.y);
    }
}

bool PlayerInteractionService::handleTowerPlacement(const sf::Vector2f& clickPos, 
                                                   const std::string& towerType,
                                                   Player& player,
                                                   TowerFactory* towerFactory,
                                                   int upgradeLevel) {
    if (!towerFactory) return false;
    
    if (!isValidPlacementPosition(clickPos, towerFactory)) {
        return false;
    }
    
    int cost = getTowerCost(towerType, upgradeLevel);
    if (!canAffordTower(player, towerType, upgradeLevel)) {
        return false;
    }
    
    sf::Vector2f gridPos = snapToGrid(clickPos);
    
    if (towerFactory->isTowerThere(gridPos.x, gridPos.y)) {
        return false;
    }
    
    towerFactory->createTower(gridPos.x, gridPos.y, towerType, upgradeLevel);
    player.currency -= cost;
    return true;
}

bool PlayerInteractionService::handleTowerRemoval(const sf::Vector2f& clickPos, 
                                                 Player& player,
                                                 TowerFactory* towerFactory) {
    if (!towerFactory) return false;
    
    sf::Vector2f gridPos = snapToGrid(clickPos);
    
    Tower* tower = nullptr;
    std::string towerType;
    int upgradeLevel = 1;
    
    for (auto& t : towerFactory->towers) {
        if (t && t->operational && 
            std::abs(t->location.x - gridPos.x) < 10 && 
            std::abs(t->location.y - gridPos.y) < 10) {
            tower = t.get();
            towerType = t->type;
            break;
        }
    }
    
    if (!tower) return false;
    
    int refund = getTowerRefund(towerType, upgradeLevel);
    player.currency += refund;
    
    towerFactory->removeTower(gridPos.x, gridPos.y);
    return true;
}

bool PlayerInteractionService::canAffordTower(const Player& player, const std::string& towerType, int upgradeLevel) const {
    int cost = getTowerCost(towerType, upgradeLevel);
    return player.currency >= cost;
}

int PlayerInteractionService::getTowerCost(const std::string& towerType, int upgradeLevel) const {
    int baseCost = 50;
    
    if (towerType == "peashooter") baseCost = 100;
    else if (towerType == "sunflower") baseCost = 50;
    else if (towerType == "cherrybomb") baseCost = 150;
    else if (towerType == "wallnut") baseCost = 50;
    else if (towerType == "potatomine") baseCost = 25;
    else if (towerType == "snowpea") baseCost = 175;
    else if (towerType == "chomper") baseCost = 150;
    else if (towerType == "repeater") baseCost = 200;
    
    return baseCost + (upgradeLevel - 1) * 25;
}

int PlayerInteractionService::getTowerRefund(const std::string& towerType, int upgradeLevel) const {
    int cost = getTowerCost(towerType, upgradeLevel);
    return cost / 2;
}

bool PlayerInteractionService::isValidPlacementPosition(const sf::Vector2f& pos, TowerFactory* towerFactory) const {
    if (!isWithinGrid(pos.x, pos.y)) {
        return false;
    }
    
    sf::Vector2f gridPos = snapToGrid(pos);
    
    if (towerFactory && towerFactory->isTowerThere(gridPos.x, gridPos.y)) {
        return false;
    }
    
    return true;
}

sf::Vector2f PlayerInteractionService::snapToGrid(const sf::Vector2f& pos) const {
    int col = getGridColumn(pos.x);
    int row = getGridRow(pos.y);
    
    float snappedX = GRID_OFFSET_X + col * TILE_WIDTH + TILE_WIDTH / 2;
    float snappedY = GRID_OFFSET_Y + row * TILE_HEIGHT + TILE_HEIGHT / 2;
    
    return sf::Vector2f(snappedX, snappedY);
}

void PlayerInteractionService::updateSelectedCard(const std::string& cardType) {
    selectedCard = cardType;
}

void PlayerInteractionService::processSunGeneration(Player& player, SunFactory* sunFactory, float deltaTime) {
    float freq = CrystalManager::getInstance().getSunFrequencyMultiplier();
    float interval = SUN_GENERATION_INTERVAL;
    if (freq > 0.0f) interval = SUN_GENERATION_INTERVAL / freq;
    if (sunGenerationClock.getElapsedTime().asSeconds() >= interval) {
        if (sunFactory) {
            int col = rand() % GRID_COLS;
            float x = GRID_OFFSET_X + col * TILE_WIDTH + TILE_WIDTH / 2.0f;
            float y = -TILE_HEIGHT;
            sunFactory->addFallingSun(x, y);
        }
        sunGenerationClock.restart();
    }
}

int PlayerInteractionService::getGridRow(float y) const {
    float adjustedY = y - GRID_OFFSET_Y;
    int row = static_cast<int>(adjustedY / TILE_HEIGHT);
    return std::clamp(row, 0, GRID_ROWS - 1);
}

int PlayerInteractionService::getGridColumn(float x) const {
    float adjustedX = x - GRID_OFFSET_X;
    int col = static_cast<int>(adjustedX / TILE_WIDTH);
    return std::clamp(col, 0, GRID_COLS - 1);
}

bool PlayerInteractionService::isWithinGrid(float x, float y) const {
    return x >= GRID_OFFSET_X && x <= GRID_OFFSET_X + GRID_COLS * TILE_WIDTH &&
           y >= GRID_OFFSET_Y && y <= GRID_OFFSET_Y + GRID_ROWS * TILE_HEIGHT;
}

}