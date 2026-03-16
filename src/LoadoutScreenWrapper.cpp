#include "LoadoutScreenWrapper.h"
#include <iostream>

namespace DefenseSynth {

LoadoutScreenWrapper::LoadoutScreenWrapper(sf::RenderWindow &window) : Screen(window)
{
    StateManager* sm = StateManager::getInstance();
    int currentLevel = sm->getLevel();
    
    sm->unlockCard("basicshooter");
    sm->unlockCard("resourcegenerator");
    sm->unlockCard("barriertower");
    sm->unlockCard("rapidshooter");
    sm->unlockCard("freezetower");
    sm->unlockCard("bombtower");
    sm->unlockCard("areaattacktower");
    
    CrystalManager::getInstance().setCrystalCount(sm->getCrystals());
    
    cardCollection.loadProgressFromFile();
    
    std::vector<std::string> allCardTypes = {"basicshooter", "resourcegenerator", "barriertower", 
                                            "rapidshooter", "freezetower", "bombtower", "areaattacktower"};
    for (const auto& cardType : allCardTypes) {
        UnitCard* card = cardCollection.getCard(cardType);
        if (card) {
            card->unlocked = sm->isCardUnlocked(cardType);
            if (card->unlocked) {
                card->level = sm->getCardLevel(cardType);
                if (card->level < 0) card->level = 0;
            }
        }
    }
    
    loadoutScreen = new LoadoutScreen(window, cardCollection);
}

LoadoutScreenWrapper::~LoadoutScreenWrapper() {
    delete loadoutScreen;
}

void LoadoutScreenWrapper::refreshFromStateManager(sf::RenderWindow &window) {
    StateManager* sm = StateManager::getInstance();
    
    sm->unlockCard("basicshooter");
    sm->unlockCard("resourcegenerator");
    sm->unlockCard("barriertower");
    sm->unlockCard("rapidshooter");
    sm->unlockCard("freezetower");
    sm->unlockCard("bombtower");
    sm->unlockCard("areaattacktower");
    
    CrystalManager::getInstance().setCrystalCount(sm->getCrystals());
    
    cardCollection.loadProgressFromFile();
    
    std::vector<std::string> allCardTypes = {"basicshooter", "resourcegenerator", "barriertower", 
                                            "rapidshooter", "freezetower", "bombtower", "areaattacktower"};
    for (const auto& cardType : allCardTypes) {
        UnitCard* card = cardCollection.getCard(cardType);
        if (card) {
            card->unlocked = sm->isCardUnlocked(cardType);
            if (card->unlocked) {
                card->level = sm->getCardLevel(cardType);
                if (card->level < 0) card->level = 0;
            }
        }
    }
    
    delete loadoutScreen;
    loadoutScreen = new LoadoutScreen(window, cardCollection);
}

void LoadoutScreenWrapper::renderScreen(sf::RenderWindow &window, std::string &currentScreen)
{
    refreshFromStateManager(window);
    
    std::string currentView = "loadout";
    loadoutScreen->renderScreen(window, currentView);
    
    if (currentView == "battlefield") {
        cachedSelection = loadoutScreen->getSelectedUnits();
        
        #ifdef _DEBUG
        std::cout << "LoadoutScreenWrapper::renderScreen - Caching " << cachedSelection.size() << " selected units:" << std::endl;
        for (const auto& unit : cachedSelection) {
            std::cout << "  - " << unit << std::endl;
        }
        #endif
        
        cachedUpgradeLevels.clear();
        for (const std::string& unitType : cachedSelection) {
            UnitCard* card = cardCollection.getCard(unitType);
            int level = card ? card->level : 0;
            cachedUpgradeLevels[unitType] = level;
        }
        
        StateManager::getInstance()->setSelectedUnits(cachedSelection);
        StateManager::getInstance()->save();
        
        StateManager* sm = StateManager::getInstance();
        for (const auto& card : cardCollection.getAllCards()) {
            if (card.unlocked) {
                sm->setCardLevel(card.unitType, card.level);
                sm->unlockCard(card.unitType);
            }
        }
        sm->save();
        currentScreen = "battlefield";
    } else if (currentView == "mainMenu") {
        currentScreen = "mainMenu";
    }
}

const std::vector<std::string> LoadoutScreenWrapper::getSelectedUnits() const { 
    std::vector<std::string> actualSelection = StateManager::getInstance()->getSelectedUnits();
    
    #ifdef _DEBUG
    std::cout << "LoadoutScreenWrapper::getSelectedUnits() - Returning " << actualSelection.size() << " units:" << std::endl;
    for (const auto& unit : actualSelection) {
        std::cout << "  - " << unit << std::endl;
    }
    #endif
    
    return actualSelection;
}

std::map<std::string, int> LoadoutScreenWrapper::getUpgradeLevels() const { 
    std::map<std::string, int> actualUpgradeLevels;
    std::vector<std::string> selectedUnits = getSelectedUnits();
    
    for (const std::string& unitType : selectedUnits) {
        UnitCard* card = const_cast<CardCollection&>(cardCollection).getCard(unitType);
        int level = card ? card->level : 0;
        actualUpgradeLevels[unitType] = level;
        StateManager::getInstance()->setUpgradeLevel(unitType, level);
    }
    
    #ifdef _DEBUG
    std::cout << "LoadoutScreenWrapper::getUpgradeLevels() - Returning " << actualUpgradeLevels.size() << " upgrades:" << std::endl;
    for (const auto& pair : actualUpgradeLevels) {
        std::cout << "  - " << pair.first << ": level " << pair.second << std::endl;
    }
    #endif
    
    return actualUpgradeLevels;
}

int LoadoutScreenWrapper::getUpgradeLevel(const std::string& unitType) const {
    auto upgradeLevels = getUpgradeLevels();
    auto it = upgradeLevels.find(unitType);
    return (it != upgradeLevels.end()) ? it->second : 0;
}

float LoadoutScreenWrapper::getDifficultyValue() const {
    return loadoutScreen ? loadoutScreen->getDifficultyValue() : 1.0f;
}

}