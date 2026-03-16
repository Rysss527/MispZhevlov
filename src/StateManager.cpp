#include "StateManager.h"
#include <fstream>
#include <filesystem>

StateManager* StateManager::instance = nullptr;

StateManager::StateManager() : 
    level(1), 
    crystals(30), 
    maxSelection(3),
    sunUpgradeLevel(0),
    crystalUpgradeLevel(0),
    difficultyIndex(2),
    saveFileName("save/game_save.dat"),
    isBattleActive(false) {
    
    unlockedCards = {"basicshooter", "resourcegenerator", "barriertower",
                     "rapidshooter", "freezetower", "bombtower", "areaattacktower"};
    for (const auto& card : unlockedCards) {
        cardLevels[card] = 0;
    }
    
    std::filesystem::create_directories("save");
}

StateManager* StateManager::getInstance() {
    if (!instance) {
        instance = new StateManager();
    }
    return instance;
}

int StateManager::getUpgradeLevel(const std::string& unitType) const {
    auto it = upgradeLevels.find(unitType);
    if (it != upgradeLevels.end()) {
        return it->second;
    }
    return 1;
}

void StateManager::addSelectedUnit(const std::string& unit) {
    if (std::find(selectedUnits.begin(), selectedUnits.end(), unit) == selectedUnits.end()) {
        if (static_cast<int>(selectedUnits.size()) < maxSelection) {
            selectedUnits.push_back(unit);
        }
    }
}

void StateManager::removeSelectedUnit(const std::string& unit) {
    selectedUnits.erase(
        std::remove(selectedUnits.begin(), selectedUnits.end(), unit), 
        selectedUnits.end()
    );
}

bool StateManager::isUnitSelected(const std::string& unit) const {
    return std::find(selectedUnits.begin(), selectedUnits.end(), unit) != selectedUnits.end();
}

bool StateManager::save() {
    try {
        std::ofstream file(saveFileName);
        if (file.is_open()) {
            maxSelection = calculateMaxSelection(level);
            file << level << std::endl;
            file << crystals << std::endl;
            file << maxSelection << std::endl;
            file << sunUpgradeLevel << std::endl;
            file << crystalUpgradeLevel << std::endl;
            file << difficultyIndex << std::endl;
            file << (isBattleActive ? 1 : 0) << std::endl;
            
            #ifdef _DEBUG
            std::cout << "StateManager::save() - Level: " << level << ", Crystals: " << crystals << ", MaxSelection: " << maxSelection << std::endl;
            #endif
            
            file << selectedUnits.size() << std::endl;
            for (const auto& unit : selectedUnits) {
                file << unit << std::endl;
            }
            
            file << upgradeLevels.size() << std::endl;
            for (const auto& pair : upgradeLevels) {
                file << pair.first << std::endl;
                file << pair.second << std::endl;
            }
            
            file << unlockedCards.size() << std::endl;
            for (const auto& card : unlockedCards) {
                file << card << std::endl;
            }
            
            file << cardLevels.size() << std::endl;
            for (const auto& pair : cardLevels) {
                file << pair.first << std::endl;
                file << pair.second << std::endl;
            }
            
            file.close();
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error saving game state: " << e.what() << std::endl;
    }
    return false;
}

bool StateManager::load() {
    try {
        std::ifstream file(saveFileName);
        if (file.is_open()) {
            file >> level;
            file >> crystals;
            file >> maxSelection;
            file >> sunUpgradeLevel;
            file >> crystalUpgradeLevel;
            file >> difficultyIndex;
            maxSelection = calculateMaxSelection(level);
            
            #ifdef _DEBUG
            std::cout << "StateManager::load() - Level: " << level << ", Crystals: " << crystals << ", MaxSelection: " << maxSelection << std::endl;
            #endif
            
            int battleStatus = 0;
            if (file >> battleStatus) {
                isBattleActive = (battleStatus == 1);
            } else {
                isBattleActive = false; 
            }
            
            selectedUnits.clear();
            size_t unitCount;
            file >> unitCount;
            file.ignore(); 
            for (size_t i = 0; i < unitCount; ++i) {
                std::string unit;
                std::getline(file, unit);
                selectedUnits.push_back(unit);
            }
            
            upgradeLevels.clear();
            size_t upgradeCount;
            file >> upgradeCount;
            file.ignore(); 
            for (size_t i = 0; i < upgradeCount; ++i) {
                std::string unitType;
                int level;
                std::getline(file, unitType);
                file >> level;
                file.ignore(); 
                upgradeLevels[unitType] = level;
            }
            
            unlockedCards.clear();
            size_t cardCount;
            if (file >> cardCount) {
                file.ignore(); 
                for (size_t i = 0; i < cardCount; ++i) {
                    std::string card;
                    std::getline(file, card);
                    unlockedCards.push_back(card);
                }
                
                cardLevels.clear();
                size_t cardLevelCount;
                if (file >> cardLevelCount) {
                    file.ignore(); 
                    for (size_t i = 0; i < cardLevelCount; ++i) {
                        std::string cardType;
                        int level;
                        std::getline(file, cardType);
                        file >> level;
                        file.ignore(); 
                        cardLevels[cardType] = level;
                    }
                }
            } else {
                reset();
            }
            
            file.close();
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading game state: " << e.what() << std::endl;
        reset(); 
    }
    return false;
}

void StateManager::reset() {
    level = 1;
    crystals = 30;
    maxSelection = 3;
    sunUpgradeLevel = 0;
    crystalUpgradeLevel = 0;
    difficultyIndex = 2;
    selectedUnits.clear();
    upgradeLevels.clear();
    
    isBattleActive = false;
    battleSessionTimer.restart();
    
    unlockedCards.clear();
    cardLevels.clear();
    unlockedCards = {"basicshooter", "resourcegenerator", "barriertower",
                     "rapidshooter", "freezetower", "bombtower", "areaattacktower"};
    for (const auto& card : unlockedCards) {
        cardLevels[card] = 0;
    }
}

void StateManager::setState(std::unique_ptr<IGameState> newState) {
    currentState = std::move(newState);
}

IGameState* StateManager::getCurrentState() const {
    return currentState.get();
}

void StateManager::registerStateFactory(const std::string& stateName, std::function<std::unique_ptr<IGameState>()> factory) {
    stateFactories[stateName] = factory;
}

std::unique_ptr<IGameState> StateManager::createState(const std::string& stateName) {
    auto it = stateFactories.find(stateName);
    if (it != stateFactories.end()) {
        return it->second();
    }
    return nullptr;
}

int StateManager::getCardLevel(const std::string& cardType) const {
    auto it = cardLevels.find(cardType);
    return (it != cardLevels.end()) ? it->second : 0;
}

void StateManager::unlockCard(const std::string& cardType) {
    if (!isCardUnlocked(cardType)) {
        unlockedCards.push_back(cardType);
        cardLevels[cardType] = 0;
    }
}

void StateManager::setCardLevel(const std::string& cardType, int level) {
    if (isCardUnlocked(cardType)) {
        cardLevels[cardType] = level;
    }
}

bool StateManager::isCardUnlocked(const std::string& cardType) const {
    return std::find(unlockedCards.begin(), unlockedCards.end(), cardType) != unlockedCards.end();
}

void StateManager::update(sf::RenderWindow& window, float deltaTime) {
    if (currentState) {
        currentState->update(window, deltaTime);
        
        if (currentState->isFinished()) {
            std::string nextState = currentState->getNextState();
            if (!nextState.empty()) {
                auto newState = createState(nextState);
                if (newState) {
                    setState(std::move(newState));
                }
            }
        }
    }
}

void StateManager::render(sf::RenderWindow& window) {
    if (currentState) {
        currentState->render(window);
    }
}

void StateManager::handleInput(const sf::Event& event) {
    if (currentState) {
        currentState->handleInput(event);
    }
}

void StateManager::startBattleSession() {
    isBattleActive = true;
    battleSessionTimer.restart();
}

void StateManager::endBattleSession() {
    isBattleActive = false;
}

float StateManager::getBattleSessionTime() const {
    return battleSessionTimer.getElapsedTime().asSeconds();
}
