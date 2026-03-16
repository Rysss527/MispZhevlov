#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <memory>
#include <cmath>
#include "Constants.h"
#include "GameCursor.h"
#include "EffectManager.h"

namespace DefenseSynth {

class StuckDetector {
public:
    struct TrackingData {
        sf::Vector2f lastPosition;
        sf::Vector2f previousPosition;
        sf::Clock checkTimer;
        sf::Clock correctionCooldown;
        float totalMovement;
        int stuckCount;
        bool wasAttacking;
        float lastCheckInterval;
    };

private:
    std::map<void*, std::unique_ptr<TrackingData>> trackedEnemies;
    static StuckDetector instance;
    
    StuckDetector() = default;

public:
    static StuckDetector& getInstance() {
        return instance;
    }
    
    void registerEnemy(void* enemy) {
        if (trackedEnemies.find(enemy) == trackedEnemies.end()) {
            auto data = std::make_unique<TrackingData>();
            data->lastPosition = sf::Vector2f(0, 0);
            data->previousPosition = sf::Vector2f(0, 0);
            data->totalMovement = 0;
            data->stuckCount = 0;
            data->wasAttacking = false;
            data->lastCheckInterval = 2.0f;
            trackedEnemies[enemy] = std::move(data);
        }
    }
    
    void registerEnemy(void* enemy, const sf::Vector2f& pos) {
        if (trackedEnemies.find(enemy) == trackedEnemies.end()) {
            auto data = std::make_unique<TrackingData>();
            data->lastPosition = pos;
            data->previousPosition = pos;
            data->totalMovement = 0;
            data->stuckCount = 0;
            data->wasAttacking = false;
            data->lastCheckInterval = 2.0f;
            trackedEnemies[enemy] = std::move(data);
        }
    }
    
    void unregisterEnemy(void* enemy) {
        trackedEnemies.erase(enemy);
    }
    
    bool checkStuck(void* enemyPtr, const sf::Vector2f& currentPos, bool isAttacking, 
                    bool isFrozen, const std::string& type) {
        
        auto it = trackedEnemies.find(enemyPtr);
        if (it == trackedEnemies.end()) {
            registerEnemy(enemyPtr, currentPos);
            it = trackedEnemies.find(enemyPtr);
        }
        
        auto& data = *it->second;
        
        if (isFrozen || isAttacking) {
            data.wasAttacking = isAttacking;
            data.lastPosition = currentPos;
            data.checkTimer.restart();
            return false;
        }
        
        if (data.wasAttacking && !isAttacking) {
            data.lastPosition = currentPos;
            data.previousPosition = currentPos;
            data.checkTimer.restart();
            data.wasAttacking = false;
            return false;
        }
        
        float checkInterval = getCheckInterval(type);
        float elapsed = data.checkTimer.getElapsedTime().asSeconds();
        
        if (elapsed >= checkInterval) {
            float distance = std::sqrt(
                std::pow(currentPos.x - data.lastPosition.x, 2.0f) + 
                std::pow(currentPos.y - data.lastPosition.y, 2.0f)
            );
            
            float threshold = getMovementThreshold(type);
            
            if (distance < threshold) {
                data.stuckCount++;
                
                if (data.correctionCooldown.getElapsedTime().asSeconds() > 3.0f) {
                    data.correctionCooldown.restart();
                    data.lastPosition = currentPos;
                    data.checkTimer.restart();
                    return true;
                }
            } else {
                data.stuckCount = 0;
            }
            
            data.previousPosition = data.lastPosition;
            data.lastPosition = currentPos;
            data.checkTimer.restart();
        }
        
        return false;
    }
    
    sf::Vector2f getCorrectionVector(const std::string& type, const sf::Vector2f& currentPos, int lane) {
        sf::Vector2f correction(0, 0);
        
        bool isFlying = (type == GameConstants::FLYING_ZOMBIE || type == "boss_flying");
        bool isBoss = (type.find("boss") != std::string::npos);
        
        if (isFlying) {
            correction.y = (rand() % 2 ? 25.0f : -25.0f);
            correction.x = 10.0f;
        } else if (isBoss) {
            correction.y = (rand() % 2 ? 20.0f : -20.0f);
            correction.x = 15.0f;
        } else {
            correction.y = (rand() % 2 ? 15.0f : -15.0f);
            correction.x = 5.0f;
        }
        
        float minY = isFlying ? 100.0f : 150.0f;
        float maxY = isFlying ? 600.0f : 550.0f;
        
        if (currentPos.y + correction.y < minY) {
            correction.y = minY - currentPos.y;
        }
        if (currentPos.y + correction.y > maxY) {
            correction.y = maxY - currentPos.y;
        }
        
        return correction;
    }
    
    float getSpeedBoost(const std::string& type) {
        if (type.find("boss") != std::string::npos) {
            return 2.0f;
        } else if (type == GameConstants::FLYING_ZOMBIE) {
            return 1.8f;
        } else if (type == GameConstants::ARMORED_ZOMBIE) {
            return 1.3f;
        } else {
            return 1.5f;
        }
    }
    
    void detectAndCorrectStuck(void* enemyPtr, coordinates& coords, float& velocity, 
                               const std::string& type, bool isAttacking, bool isFrozen, 
                               bool& canAdvance, float deltaTime) {
        if (checkStuck(enemyPtr, sf::Vector2f(coords.x, coords.y), isAttacking, isFrozen, type)) {
            int lane = LaneSystem::getLane(coords.y);
            sf::Vector2f correction = getCorrectionVector(type, sf::Vector2f(coords.x, coords.y), lane);
            
            coords.x += correction.x;
            coords.y += correction.y;
            
            float speedBoost = getSpeedBoost(type);
            velocity *= speedBoost;
            
            DefenseSynth::EffectManager::getInstance().addSparks(
                sf::Vector2f(coords.x, coords.y), 10);
            
            canAdvance = true;
        }
        
        if (!canAdvance && isFrozen == false && isAttacking == false) {
            canAdvance = true;
        }
    }

private:
    float getCheckInterval(const std::string& type) const {
        if (type.find("boss") != std::string::npos) {
            return 3.0f;
        } else if (type == GameConstants::FLYING_ZOMBIE || type == "boss_flying") {
            return 1.8f;
        } else if (type == GameConstants::ARMORED_ZOMBIE) {
            return 2.5f;
        } else {
            return 2.0f;
        }
    }
    
    float getMovementThreshold(const std::string& type) const {
        if (type.find("boss") != std::string::npos) {
            return 4.0f;
        } else if (type == GameConstants::FLYING_ZOMBIE || type == "boss_flying") {
            return 12.0f;
        } else if (type == GameConstants::ARMORED_ZOMBIE) {
            return 6.0f;
        } else {
            return 8.0f;
        }
    }
};

}
