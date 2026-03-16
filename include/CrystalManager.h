#pragma once
#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>


class StateManager;

class CrystalManager {
private:
    static inline CrystalManager* instance = nullptr;
    int crystalCount;
    int sunUpgradeLevel;
    int crystalUpgradeLevel;
    int gameLevel;
    
    CrystalManager() : crystalCount(50), sunUpgradeLevel(0), crystalUpgradeLevel(0), gameLevel(1) {} 
    
public:
    static CrystalManager& getInstance() {
        if (!instance) {
            instance = new CrystalManager();
        }
        return *instance;
    }
    
    int getCrystalCount() const {
        return crystalCount;
    }
    
    bool canAfford(int amount) const {
        return crystalCount >= amount;
    }
    
    void setCrystalCount(int amount) {
        crystalCount = amount;
    }

    void syncWithStateManager() {
    }
    
    void reset() {
        crystalCount = 50;
        sunUpgradeLevel = 0;
        crystalUpgradeLevel = 0;
        gameLevel = 1;
    }
    
    void addCrystals(int amount) {
        if (amount > 0) {
            crystalCount += amount;
        }
    }
    
    bool spendCrystals(int amount) {
        try {
            if (amount <= 0) return false;
            if (crystalCount < amount) return false;
            crystalCount -= amount;
            return true;
        } catch (...) {
            return false;
        }
    }
    
    int getUpgradeCost(int currentLevel) const {
        if (currentLevel == 0) return 15;
        if (currentLevel == 1) return 30;
        if (currentLevel == 2) return 50;
        if (currentLevel == 3) return 75;
        if (currentLevel == 4) return 100;
        if (currentLevel >= 5) return -1;
        return 15;
    }
    
    bool isMaxLevel(int currentLevel) const {
        return currentLevel >= 5;
    }
    
    int getLevelCompletionReward(int level) {
        int baseReward = 50 + (level * 25);
        float crystalMultiplier = getCrystalRewardMultiplier();
        return static_cast<int>(baseReward * crystalMultiplier);
    }
    
    int getMaxUpgradeLevel() const {
        return 5;
    }
    
    int getTotalUpgradeCost(int targetLevel) const {
        int total = 0;
        for (int i = 1; i < targetLevel; i++) {
            total += getUpgradeCost(i);
        }
        return total;
    }
    
    int getSunUpgradeLevel() const {
        return sunUpgradeLevel;
    }
    
    int getCrystalUpgradeLevel() const {
        return crystalUpgradeLevel;
    }
    
    bool canUpgradeSun() const {
        return sunUpgradeLevel < 5;
    }
    
    bool canUpgradeCrystal() const {
        return crystalUpgradeLevel < 5;
    }
    
    int getSunUpgradeCost() const {
        return getUpgradeCost(sunUpgradeLevel);
    }
    
    int getCrystalUpgradeCost() const {
        return getUpgradeCost(crystalUpgradeLevel);
    }
    
    bool upgradeSun() {
        try {
            if (!canUpgradeSun()) return false;
            int cost = getSunUpgradeCost();
            if (!canAfford(cost)) return false;
            if (!spendCrystals(cost)) return false;
            sunUpgradeLevel++;
            return true;
        } catch (...) {
            return false;
        }
    }
    
    bool upgradeCrystal() {
        try {
            if (!canUpgradeCrystal()) return false;
            int cost = getCrystalUpgradeCost();
            if (!canAfford(cost)) return false;
            if (!spendCrystals(cost)) return false;
            crystalUpgradeLevel++;
            
            if (crystalUpgradeLevel == 5) {
                int compensation = calculateLateUpgradeCompensation();
                addCrystals(compensation);
            }
            return true;
        } catch (...) {
            return false;
        }
    }
    
    float getSunFrequencyMultiplier() const {
        return 1.0f + sunUpgradeLevel * 0.3f;
    }
    
    float getSunDurationMultiplier() const {
        return 1.0f + sunUpgradeLevel * 0.4f;
    }
    
    float getCrystalRewardMultiplier() const {
        return 1.0f + crystalUpgradeLevel * 0.25f;
    }
    
    void setSunUpgradeLevel(int level) {
        sunUpgradeLevel = std::max(0, std::min(5, level));
    }
    
    void setCrystalUpgradeLevel(int level) {
        crystalUpgradeLevel = std::max(0, std::min(5, level));
    }
    
    int calculateTotalUpgradeCostsForAllUnits(int unitCount) const {
        int totalCostPerUnit = getTotalUpgradeCost(5);
        int resourceUpgradeCosts = getTotalUpgradeCost(5) * 2;
        return (totalCostPerUnit * unitCount) + resourceUpgradeCosts;
    }
    
    int calculateRecommendedCrystalsForLevel(int level) const {
        int unitCount = 7;
        int baseNeeded = calculateTotalUpgradeCostsForAllUnits(unitCount);
        return baseNeeded + (level * 50);
    }
    
    int calculateLateUpgradeCompensation() const {
        return calculateLateUpgradeCompensationForLevel(getCurrentGameLevel());
    }
    
    int calculateLateUpgradeCompensationForLevel(int currentLevel) const {
        if (currentLevel <= 3) return 0;
        
        int missedBonusLevel = currentLevel - 3;
        int baseReward = 50 + (missedBonusLevel * 25);
        float bonusMultiplier = 0.25f * 4;
        int missedReward = static_cast<int>(baseReward * bonusMultiplier * missedBonusLevel);
        
        return std::max(0, std::min(500, missedReward));
    }
    
    int getCurrentGameLevel() const {
        return gameLevel;
    }
    
    void setCurrentGameLevel(int level) {
        gameLevel = level;
    }
};
