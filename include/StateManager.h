#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include "IGameState.h"


class StateManager {
private:
    static StateManager* instance;
    std::unique_ptr<IGameState> currentState;
    std::unordered_map<std::string, std::function<std::unique_ptr<IGameState>()>> stateFactories;
    
    
    int level;
    int crystals;
    std::vector<std::string> selectedUnits;
    std::map<std::string, int> upgradeLevels;
    int maxSelection;
    int sunUpgradeLevel;
    int crystalUpgradeLevel;
    int difficultyIndex;
    
    
    std::vector<std::string> unlockedCards;
    std::map<std::string, int> cardLevels;
    
    
    bool isBattleActive;
    sf::Clock battleSessionTimer;
    
    std::string saveFileName;
    
    StateManager();
    
public:
    static StateManager* getInstance();
    
    
    int getLevel() const { return level; }
    int getCrystals() const { return crystals; }
    const std::vector<std::string>& getSelectedUnits() const { return selectedUnits; }
    int getUpgradeLevel(const std::string& unitType) const;
    int getMaxSelection() const { return maxSelection; }
    int getSunUpgradeLevel() const { return sunUpgradeLevel; }
    int getCrystalUpgradeLevel() const { return crystalUpgradeLevel; }
    int getDifficultyIndex() const { return difficultyIndex; }
    
    void setLevel(int newLevel) { level = (newLevel < 1) ? 1 : newLevel; }
    void setCrystals(int newCrystals) { crystals = newCrystals; }
    void setSelectedUnits(const std::vector<std::string>& units) { selectedUnits = units; }
    void setUpgradeLevel(const std::string& unitType, int level) { upgradeLevels[unitType] = level; }
    void setMaxSelection(int maxSel) { maxSelection = (maxSel < 1) ? 1 : ((maxSel > 7) ? 7 : maxSel); }
    void setSunUpgradeLevel(int level) { sunUpgradeLevel = level; }
    void setCrystalUpgradeLevel(int level) { crystalUpgradeLevel = level; }
    void setDifficultyIndex(int index) { difficultyIndex = index; }
    
    static int calculateMaxSelection(int playerLevel) {
        return std::min(3 + (playerLevel - 1), 7);
    }
    
    
    const std::vector<std::string>& getUnlockedCards() const { return unlockedCards; }
    int getCardLevel(const std::string& cardType) const;
    void unlockCard(const std::string& cardtype);
    void setCardLevel(const std::string& cardType, int level);
    bool isCardUnlocked(const std::string& cardType) const;
    void clearUnlockedCards() { unlockedCards.clear(); cardLevels.clear(); }
    
    
    bool isBattleSessionActive() const { return isBattleActive; }
    void startBattleSession();
    void endBattleSession();
    float getBattleSessionTime() const;
    
    
    void addSelectedUnit(const std::string& unit);
    void removeSelectedUnit(const std::string& unit);
    bool isUnitSelected(const std::string& unit) const;
    
    
    bool save();
    bool load();
    
    
    void reset();
    
    
    void setState(std::unique_ptr<IGameState> newState);
    IGameState* getCurrentState() const;
    void registerStateFactory(const std::string& stateName, std::function<std::unique_ptr<IGameState>()> factory);
    std::unique_ptr<IGameState> createState(const std::string& stateName);
    void update(sf::RenderWindow& window, float deltaTime);
    void render(sf::RenderWindow& window);
    void handleInput(const sf::Event& event);
};
