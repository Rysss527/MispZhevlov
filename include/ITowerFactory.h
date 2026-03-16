#pragma once

#include <memory>
#include <string>
#include "Tower.h"

namespace DefenseSynth {

class ITowerFactory {
public:
    virtual ~ITowerFactory() = default;
    virtual std::unique_ptr<Tower> createTower(const std::string& type, float x, float y) = 0;
    virtual std::unique_ptr<Tower> createTower(const std::string& type, float x, float y, int upgradeLevel) = 0;
    virtual bool canCreateTower(const std::string& type) const = 0;
    virtual int getTowerCost(const std::string& type) const = 0;
};

class BasicTowerFactory : public ITowerFactory {
public:
    std::unique_ptr<Tower> createTower(const std::string& type, float x, float y) override;
    std::unique_ptr<Tower> createTower(const std::string& type, float x, float y, int upgradeLevel) override;
    bool canCreateTower(const std::string& type) const override;
    int getTowerCost(const std::string& type) const override;
};

class AttackTowerFactory : public ITowerFactory {
public:
    std::unique_ptr<Tower> createTower(const std::string& type, float x, float y) override;
    std::unique_ptr<Tower> createTower(const std::string& type, float x, float y, int upgradeLevel) override;
    bool canCreateTower(const std::string& type) const override;
    int getTowerCost(const std::string& type) const override;
};

class SupportTowerFactory : public ITowerFactory {
public:
    std::unique_ptr<Tower> createTower(const std::string& type, float x, float y) override;
    std::unique_ptr<Tower> createTower(const std::string& type, float x, float y, int upgradeLevel) override;
    bool canCreateTower(const std::string& type) const override;
    int getTowerCost(const std::string& type) const override;
};

class TowerFactoryManager {
private:
    std::unique_ptr<BasicTowerFactory> basicFactory;
    std::unique_ptr<AttackTowerFactory> attackFactory;
    std::unique_ptr<SupportTowerFactory> supportFactory;

public:
    TowerFactoryManager();
    std::unique_ptr<Tower> createTower(const std::string& type, float x, float y);
    std::unique_ptr<Tower> createTower(const std::string& type, float x, float y, int upgradeLevel);
    bool canCreateTower(const std::string& type) const;
    int getTowerCost(const std::string& type) const;

private:
    ITowerFactory* getFactoryForType(const std::string& type) const;
};

}
