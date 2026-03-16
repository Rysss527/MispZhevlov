#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "CrystalManager.h"
#include "StateManager.h"


class UnitCard {
public:
    std::string unitType;
    std::string name;
    std::string description;
    int cost;
    bool unlocked;
    int level;
    int maxLevel;
    sf::Texture cardTexture;
    sf::Sprite cardSprite;
    std::vector<std::string> levelDescriptions;
    
    UnitCard(std::string type, std::string n, std::string desc, int c) 
        : unitType(type), name(n), description(desc), cost(c), unlocked(true), level(0), maxLevel(5) {
        if (!cardTexture.loadFromFile("assets/images/cards/" + type + ".png")) {
            cardTexture.loadFromFile("assets/images/plant.png");
        }
        cardTexture.setSmooth(false);
        cardSprite.setTexture(cardTexture);
        initializeLevelDescriptions();
    }
    
    void initializeLevelDescriptions() {
        levelDescriptions.push_back(description);
        
        if (unitType == "basicshooter") {
            levelDescriptions.push_back("Enhanced basic shooter with faster firing");
            levelDescriptions.push_back("Advanced basic shooter with bonus damage");
            levelDescriptions.push_back("Elite basic shooter with rapid fire");
            levelDescriptions.push_back("MASTER basic shooter with maximum power");
        } else if (unitType == "resourcegenerator") {
            levelDescriptions.push_back("Improved resource generator produces more sun");
            levelDescriptions.push_back("Advanced resource generator with bonus production");
            levelDescriptions.push_back("Elite resource generator with double sun");
            levelDescriptions.push_back("MASTER resource generator with gold sun");
        } else if (unitType == "barriertower") {
            levelDescriptions.push_back("Reinforced barrier tower with extra health");
            levelDescriptions.push_back("Advanced barrier tower with armor");
            levelDescriptions.push_back("Elite barrier tower with massive health");
            levelDescriptions.push_back("MASTER barrier tower - indestructible");
        } else if (unitType == "rapidshooter") {
            levelDescriptions.push_back("Enhanced rapid shooter with faster shots");
            levelDescriptions.push_back("Advanced rapid shooter with triple shots");
            levelDescriptions.push_back("Elite rapid shooter with burst fire");
            levelDescriptions.push_back("MASTER rapid shooter with homing shots");
        } else if (unitType == "freezetower") {
            levelDescriptions.push_back("Enhanced freeze tower with stronger freeze");
            levelDescriptions.push_back("Advanced freeze tower with area freeze");
            levelDescriptions.push_back("Elite freeze tower with ice damage");
            levelDescriptions.push_back("MASTER freeze tower with permanent freeze");
        } else if (unitType == "bombtower") {
            levelDescriptions.push_back("Enhanced bomb tower with larger blast");
            levelDescriptions.push_back("Advanced bomb tower with double explosion");
            levelDescriptions.push_back("Elite bomb tower with nuclear blast");
            levelDescriptions.push_back("MASTER bomb tower with chain explosions");
        } else if (unitType == "areaattacktower") {
            levelDescriptions.push_back("Enhanced area attack tower with wider range");
            levelDescriptions.push_back("Advanced area attack tower with toxic damage");
            levelDescriptions.push_back("Elite area attack tower with poison cloud");
            levelDescriptions.push_back("MASTER area attack tower with death aura");
        } else {
            levelDescriptions.push_back("Improved " + name);
            levelDescriptions.push_back("Advanced " + name);
            levelDescriptions.push_back("Elite " + name);
            levelDescriptions.push_back("MASTER " + name);
        }
    }
    
    bool canUpgrade() {
        return level < maxLevel && !CrystalManager::getInstance().isMaxLevel(level);
    }
    
    bool isMaxLevel() {
        return level >= maxLevel;
    }
    
    void upgrade() {
        if (canUpgrade()) {
            level++;
        }
    }
    
    std::string getCurrentDescription() {
        if (level == 0) {
            return description;
        }
        if (level - 1 < static_cast<int>(levelDescriptions.size())) {
            return levelDescriptions[level - 1];
        }
        return description;
    }
    
    int getUpgradedCost() {
        return cost + level * 25;
    }
    
    float getDamageMultiplier() {
        return 1.0f + level * 0.3f;
    }
    
    float getHealthMultiplier() {
        return 1.0f + level * 0.5f;
    }
};

class CardCollection {
private:
    std::vector<UnitCard> allCards;
    
public:
    CardCollection() {
        initializeCards();
        
        loadProgressFromFile();
    }
    
    void initializeCards() {
        allCards.push_back(UnitCard("basicshooter", "Basic\nShooter", 
            "Shoots at enemies", 50));
        allCards.push_back(UnitCard("resourcegenerator", "Sun\nGenerator", 
            "Generates sun resources", 100));
        allCards.push_back(UnitCard("barriertower", "Barrier Tower", 
            "Blocks enemies with high health", 50));
        allCards.push_back(UnitCard("rapidshooter", "Rapid Shooter", 
            "Shoots rapidly at enemies", 200));
        allCards.push_back(UnitCard("freezetower", "Snow\nPea", 
            "Slows down enemies with ice", 100));
        allCards.push_back(UnitCard("bombtower", "Cherry\nBomb", 
            "Explodes damaging nearby enemies", 150));
        allCards.push_back(UnitCard("areaattacktower", "Fume-\nshroom", 
            "Attacks multiple enemies at once", 75));
        
 
    }
    
    void unlockCard(std::string unitType) {
        for (auto& card : allCards) {
            if (card.unitType == unitType) {
                card.unlocked = true;
                break;
            }
        }
    }
    
    std::vector<UnitCard> getUnlockedCards() {
        std::vector<UnitCard> unlocked;
        for (const auto& card : allCards) {
            if (card.unlocked) {
                unlocked.push_back(card);
            }
        }
        return unlocked;
    }
    
    std::vector<UnitCard> getAllCards() {
        return allCards;
    }
    
    bool isCardUnlocked(std::string unitType) {
        for (const auto& card : allCards) {
            if (card.unitType == unitType) {
                return card.unlocked;
            }
        }
        return false;
    }
    
    UnitCard* getCard(std::string unitType) {
        for (auto& card : allCards) {
            if (card.unitType == unitType) {
                return &card;
            }
        }
        return nullptr;
    }
    
    bool upgradeCard(const std::string& unitType) {
        CrystalManager& crystalManager = CrystalManager::getInstance();
        int upgradeCost = getUpgradeCost(unitType);
        
        if (!crystalManager.canAfford(upgradeCost)) {
            return false;
        }
        
        for (auto& card : allCards) {
            if (card.unitType == unitType && card.canUpgrade()) {
                crystalManager.spendCrystals(upgradeCost);
                card.level++;
                
                saveProgressToFile();
                return true;
            }
        }
        return false;
    }
    
    int getUpgradeCost(std::string unitType) {
        UnitCard* card = getCard(unitType);
        if (card && card->canUpgrade()) {
            return CrystalManager::getInstance().getUpgradeCost(card->level);
        }
        return -1; 
    }
    
    bool canAffordUpgrade(std::string unitType) {
        UnitCard* card = getCard(unitType);
        if (card && card->canUpgrade()) {
            int cost = CrystalManager::getInstance().getUpgradeCost(card->level);
            return CrystalManager::getInstance().getCrystalCount() >= cost;
        }
        return false;
    }
    
    int getCardLevel(std::string unitType) {
        UnitCard* card = getCard(unitType);
        return card ? card->level : 0;
    }
    
    void reset() {
        for (auto& card : allCards) {
            card.unlocked = false;
            card.level = 0;
        }
 
    }
    
    void saveToFile(std::ostream& stream) {
        for (const auto& card : allCards) {
            stream << card.unitType << " " << card.unlocked << " " << card.level << std::endl;
        }
    }
    
    void loadFromFile(std::istream& stream) {
        std::string unitType;
        bool unlocked;
        int level;
        while (stream >> unitType >> unlocked >> level) {
            for (auto& card : allCards) {
                if (card.unitType == unitType) {
                    card.unlocked = unlocked;
                    card.level = level;
                    break;
                }
            }
        }
    }
    
    void saveProgressToFile() {
        StateManager* sm = StateManager::getInstance();
        for (const auto& card : allCards) {
            if (card.unlocked) {
                sm->unlockCard(card.unitType);
            }
            sm->setCardLevel(card.unitType, card.level);
        }
        sm->setCrystals(CrystalManager::getInstance().getCrystalCount());
        sm->save();
    }
    
    void loadProgressFromFile() {
        StateManager* sm = StateManager::getInstance();
        for (auto& card : allCards) {
            card.unlocked = true;
            int level = sm->getCardLevel(card.unitType);
            if (level < 0) level = 0;
            card.level = level;
        }
    }
    
    friend std::ostream& operator<<(std::ostream& stream, const CardCollection& collection) {
        for (const auto& card : collection.allCards) {
            stream << card.unitType << " " << card.unlocked << " " << card.level << std::endl;
        }
        return stream;
    }
    
    friend std::istream& operator>>(std::istream& stream, CardCollection& collection) {
        std::string unitType;
        bool unlocked;
        int level;
        while (stream >> unitType >> unlocked >> level) {
            for (auto& card : collection.allCards) {
                if (card.unitType == unitType) {
                    card.unlocked = unlocked;
                    card.level = level;
                    break;
                }
            }
        }
        return stream;
    }
};
