#include "ITowerFactory.h"
#include <unordered_set>

namespace DefenseSynth {

std::unique_ptr<Tower> BasicTowerFactory::createTower(const std::string& type, float x, float y) {
    return createTower(type, x, y, 1);
}

std::unique_ptr<Tower> BasicTowerFactory::createTower(const std::string& type, float x, float y, int upgradeLevel) {
    std::unique_ptr<Tower> tower;
    
    if (type == "resourcegenerator") {
        tower = std::make_unique<ResourceGenerator>(x, y);
    }
    else {
        return nullptr;
    }
    
    if (tower) {
        tower->setUpgradeLevel(upgradeLevel);
    }
    return tower;
}

bool BasicTowerFactory::canCreateTower(const std::string& type) const {
    static const std::unordered_set<std::string> supportedTypes = {
        "resourcegenerator"
    };
    return supportedTypes.find(type) != supportedTypes.end();
}

int BasicTowerFactory::getTowerCost(const std::string& type) const {
    if (type == "resourcegenerator") return 100;
    return -1;
}

std::unique_ptr<Tower> AttackTowerFactory::createTower(const std::string& type, float x, float y) {
    return createTower(type, x, y, 1);
}

std::unique_ptr<Tower> AttackTowerFactory::createTower(const std::string& type, float x, float y, int upgradeLevel) {
    std::unique_ptr<Tower> tower;
    
    if (type == "basicshooter") {
        tower = std::make_unique<BasicShooter>(x, y);
    }
    else if (type == "rapidshooter") {
        tower = std::make_unique<RapidShooter>(x, y);
    }
    else if (type == "freezetower") {
        tower = std::make_unique<FreezeTower>(x, y);
    }
    else if (type == "areaattacktower") {
        tower = std::make_unique<AreaAttackTower>(x, y);
    }
    else {
        return nullptr;
    }
    
    if (tower) {
        tower->setUpgradeLevel(upgradeLevel);
    }
    return tower;
}

bool AttackTowerFactory::canCreateTower(const std::string& type) const {
    static const std::unordered_set<std::string> supportedTypes = {
        "basicshooter", "rapidshooter", "freezetower", "areaattacktower"
    };
    return supportedTypes.find(type) != supportedTypes.end();
}

int AttackTowerFactory::getTowerCost(const std::string& type) const {
    if (type == "basicshooter") return 50;
    if (type == "rapidshooter") return 200;
    if (type == "freezetower") return 100;
    if (type == "areaattacktower") return 75;
    return -1;
}

std::unique_ptr<Tower> SupportTowerFactory::createTower(const std::string& type, float x, float y) {
    return createTower(type, x, y, 1);
}

std::unique_ptr<Tower> SupportTowerFactory::createTower(const std::string& type, float x, float y, int upgradeLevel) {
    std::unique_ptr<Tower> tower;
    
    if (type == "barriertower") {
        tower = std::make_unique<BarrierTower>(x, y);
    }
    else if (type == "bombtower") {
        tower = std::make_unique<BombTower>(x, y);
    }
    else {
        return nullptr;
    }
    
    if (tower) {
        tower->setUpgradeLevel(upgradeLevel);
    }
    return tower;
}

bool SupportTowerFactory::canCreateTower(const std::string& type) const {
    static const std::unordered_set<std::string> supportedTypes = {
        "barriertower", "bombtower"
    };
    return supportedTypes.find(type) != supportedTypes.end();
}

int SupportTowerFactory::getTowerCost(const std::string& type) const {
    if (type == "barriertower") return 50;
    if (type == "bombtower") return 150;
    return -1;
}

TowerFactoryManager::TowerFactoryManager() 
    : basicFactory(std::make_unique<BasicTowerFactory>()),
      attackFactory(std::make_unique<AttackTowerFactory>()),
      supportFactory(std::make_unique<SupportTowerFactory>()) {
}

std::unique_ptr<Tower> TowerFactoryManager::createTower(const std::string& type, float x, float y) {
    return createTower(type, x, y, 1);
}

std::unique_ptr<Tower> TowerFactoryManager::createTower(const std::string& type, float x, float y, int upgradeLevel) {
    ITowerFactory* factory = getFactoryForType(type);
    if (factory) {
        return factory->createTower(type, x, y, upgradeLevel);
    }
    return nullptr;
}

bool TowerFactoryManager::canCreateTower(const std::string& type) const {
    return basicFactory->canCreateTower(type) ||
           attackFactory->canCreateTower(type) ||
           supportFactory->canCreateTower(type);
}

int TowerFactoryManager::getTowerCost(const std::string& type) const {
    ITowerFactory* factory = getFactoryForType(type);
    if (factory) {
        return factory->getTowerCost(type);
    }
    return -1;
}

ITowerFactory* TowerFactoryManager::getFactoryForType(const std::string& type) const {
    if (basicFactory->canCreateTower(type)) {
        return basicFactory.get();
    }
    if (attackFactory->canCreateTower(type)) {
        return attackFactory.get();
    }
    if (supportFactory->canCreateTower(type)) {
        return supportFactory.get();
    }
    return nullptr;
}

}