#include "IEnemyBehavior.h"
#include "Enemy.h"
#include "TowerFactory.h"
#include "Constants.h"
#include "SoundManager.h"
#include <cmath>
#include <random>
#include <algorithm>

namespace DefenseSynth {

LinearMoveBehavior::LinearMoveBehavior(float moveSpeed, sf::Vector2f moveDirection)
    : speed(moveSpeed), direction(moveDirection) {}

void LinearMoveBehavior::execute(Enemy& enemy, float deltaTime, const std::vector<Enemy*>& allEnemies, const TowerFactory* towers) {
    if (enemy.is_frozen || !enemy.canAdvance) return;
    
    enemy.coords.x += direction.x * speed * deltaTime;
    enemy.coords.y += direction.y * speed * deltaTime;
    
    int currentLane = LaneSystem::getLane(enemy.coords.y);
    if (currentLane >= 0 && currentLane < LaneSystem::NUM_LANES) {
        float laneTop = LaneSystem::LANE_TOP_Y + (currentLane * LaneSystem::LANE_HEIGHT);
        float laneBottom = laneTop + LaneSystem::LANE_HEIGHT;
        
        if (enemy.coords.y < laneTop) enemy.coords.y = laneTop;
        if (enemy.coords.y > laneBottom) enemy.coords.y = laneBottom;
    }
    
    enemy.display.setPosition(enemy.coords.x, enemy.coords.y);
    
    if (enemy.coords.x <= GameConstants::LEFT_BOUNDARY) {
        enemy.coords.x = GameConstants::LEFT_BOUNDARY;
    }
}

AggressiveBehavior::AggressiveBehavior(float speed, float aggression, float range)
    : baseSpeed(speed), aggressionMultiplier(aggression), detectionRange(range) {}

void AggressiveBehavior::execute(Enemy& enemy, float deltaTime, const std::vector<Enemy*>& allEnemies, const TowerFactory* towers) {
    if (enemy.is_frozen || !enemy.canAdvance) return;
    
    float currentSpeed = baseSpeed;
    
    if (isNearTowers(enemy, towers)) {
        currentSpeed *= aggressionMultiplier;
        enemy.display.setColor(sf::Color(255, 150, 150));
    } else {
        enemy.display.setColor(sf::Color::White);
    }
    
    enemy.coords.x -= currentSpeed * deltaTime;
    
    int currentLane = LaneSystem::getLane(enemy.coords.y);
    if (currentLane >= 0 && currentLane < LaneSystem::NUM_LANES) {
        float laneCenterY = LaneSystem::getLaneCenterY(currentLane);
        enemy.coords.y = laneCenterY;
    }
    
    enemy.display.setPosition(enemy.coords.x, enemy.coords.y);
    
    enemy.coords.x = std::max(enemy.coords.x, GameConstants::LEFT_BOUNDARY);
}

bool AggressiveBehavior::isNearTowers(const Enemy& enemy, const TowerFactory* towers) const {
    if (!towers) return false;
    
    for (const auto& tower : towers->towers) {
        if (tower && tower->operational) {
            float dx = enemy.coords.x - tower->location.x;
            float dy = enemy.coords.y - tower->location.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance < detectionRange) {
                return true;
            }
        }
    }
    return false;
}

DefensiveBehavior::DefensiveBehavior(float speed, float range, float strength)
    : baseSpeed(speed), avoidanceRange(range), avoidanceStrength(strength) {}

void DefensiveBehavior::execute(Enemy& enemy, float deltaTime, const std::vector<Enemy*>& allEnemies, const TowerFactory* towers) {
    if (enemy.is_frozen || !enemy.canAdvance) return;
    
    sf::Vector2f avoidanceVector = calculateAvoidanceVector(enemy, towers);
    
    enemy.coords.x -= (baseSpeed + avoidanceVector.x) * deltaTime;
    enemy.coords.y += (avoidanceVector.y * 0.3f) * deltaTime;
    
    int currentLane = LaneSystem::getLane(enemy.coords.y);
    if (currentLane >= 0 && currentLane < LaneSystem::NUM_LANES) {
        float laneTop = LaneSystem::LANE_TOP_Y + (currentLane * LaneSystem::LANE_HEIGHT);
        float laneBottom = laneTop + LaneSystem::LANE_HEIGHT;
        float laneCenter = (laneTop + laneBottom) / 2.0f;
        
        if (enemy.coords.y < laneTop + 10) enemy.coords.y = laneTop + 10;
        if (enemy.coords.y > laneBottom - 10) enemy.coords.y = laneBottom - 10;
        
        float drift = (enemy.coords.y - laneCenter) * 0.95f;
        enemy.coords.y = laneCenter + drift;
    }
    
    enemy.display.setPosition(enemy.coords.x, enemy.coords.y);
    
    enemy.coords.x = std::max(enemy.coords.x, GameConstants::LEFT_BOUNDARY);
}

sf::Vector2f DefensiveBehavior::calculateAvoidanceVector(const Enemy& enemy, const TowerFactory* towers) const {
    sf::Vector2f avoidance(0.0f, 0.0f);
    
    if (!towers) return avoidance;
    
    for (const auto& tower : towers->towers) {
        if (tower && tower->operational) {
            float dx = enemy.coords.x - tower->location.x;
            float dy = enemy.coords.y - tower->location.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance < avoidanceRange && distance > 0) {
                float normalizedX = dx / distance;
                float normalizedY = dy / distance;
                float strength = avoidanceStrength * (avoidanceRange - distance) / avoidanceRange;
                
                avoidance.x += normalizedX * strength;
                avoidance.y += normalizedY * strength;
            }
        }
    }
    
    return avoidance;
}

SwarmingBehavior::SwarmingBehavior(float speed, float cohesion, float separation, float cohesionStr, float separationStr)
    : baseSpeed(speed), cohesionRange(cohesion), separationRange(separation), 
      cohesionStrength(cohesionStr), separationStrength(separationStr) {}

void SwarmingBehavior::execute(Enemy& enemy, float deltaTime, const std::vector<Enemy*>& allEnemies, const TowerFactory* towers) {
    if (enemy.is_frozen || !enemy.canAdvance) return;
    
    sf::Vector2f cohesion = calculateCohesion(enemy, allEnemies);
    sf::Vector2f separation = calculateSeparation(enemy, allEnemies);
    
    enemy.coords.x -= baseSpeed * deltaTime;
    enemy.coords.x += cohesion.x * cohesionStrength * deltaTime;
    enemy.coords.y += cohesion.y * cohesionStrength * 0.2f * deltaTime;
    enemy.coords.x += separation.x * separationStrength * deltaTime;
    enemy.coords.y += separation.y * separationStrength * 0.2f * deltaTime;
    
    int currentLane = LaneSystem::getLane(enemy.coords.y);
    if (currentLane >= 0 && currentLane < LaneSystem::NUM_LANES) {
        float laneTop = LaneSystem::LANE_TOP_Y + (currentLane * LaneSystem::LANE_HEIGHT);
        float laneBottom = laneTop + LaneSystem::LANE_HEIGHT;
        
        if (enemy.coords.y < laneTop + 5) enemy.coords.y = laneTop + 5;
        if (enemy.coords.y > laneBottom - 5) enemy.coords.y = laneBottom - 5;
    }
    
    enemy.display.setPosition(enemy.coords.x, enemy.coords.y);
    
    if (enemy.coords.x <= GameConstants::LEFT_BOUNDARY) {
        enemy.coords.x = GameConstants::LEFT_BOUNDARY;
    }
}

sf::Vector2f SwarmingBehavior::calculateCohesion(const Enemy& enemy, const std::vector<Enemy*>& allEnemies) const {
    sf::Vector2f centerOfMass(0.0f, 0.0f);
    int neighborCount = 0;
    
    for (Enemy* other : allEnemies) {
        if (other && other->enabled && other != &enemy) {
            float dx = other->coords.x - enemy.coords.x;
            float dy = other->coords.y - enemy.coords.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance < cohesionRange) {
                centerOfMass.x += other->coords.x;
                centerOfMass.y += other->coords.y;
                neighborCount++;
            }
        }
    }
    
    if (neighborCount > 0) {
        centerOfMass.x /= neighborCount;
        centerOfMass.y /= neighborCount;
        
        return sf::Vector2f(centerOfMass.x - enemy.coords.x, centerOfMass.y - enemy.coords.y);
    }
    
    return sf::Vector2f(0.0f, 0.0f);
}

sf::Vector2f SwarmingBehavior::calculateSeparation(const Enemy& enemy, const std::vector<Enemy*>& allEnemies) const {
    sf::Vector2f separation(0.0f, 0.0f);
    
    for (Enemy* other : allEnemies) {
        if (other && other->enabled && other != &enemy) {
            float dx = enemy.coords.x - other->coords.x;
            float dy = enemy.coords.y - other->coords.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance < separationRange && distance > 0) {
                float normalizedX = dx / distance;
                float normalizedY = dy / distance;
                float strength = (separationRange - distance) / separationRange;
                
                separation.x += normalizedX * strength;
                separation.y += normalizedY * strength;
            }
        }
    }
    
    return separation;
}

FlyingBehavior::FlyingBehavior(float speed, float alt, float freq, float amp)
    : baseSpeed(speed), altitude(alt), bobFrequency(freq), bobAmplitude(amp) {}

void FlyingBehavior::execute(Enemy& enemy, float deltaTime, const std::vector<Enemy*>& allEnemies, const TowerFactory* towers) {
    if (enemy.is_frozen || !enemy.canAdvance) return;
    
    enemy.coords.x -= baseSpeed * deltaTime;
    
    int currentLane = LaneSystem::getLane(enemy.coords.y);
    if (currentLane >= 0 && currentLane < LaneSystem::NUM_LANES) {
        float laneCenterY = LaneSystem::getLaneCenterY(currentLane);
        float bobOffset = std::sin(bobTimer.getElapsedTime().asSeconds() * bobFrequency) * bobAmplitude;
        enemy.coords.y = laneCenterY + bobOffset;
        
        float laneTop = LaneSystem::LANE_TOP_Y + (currentLane * LaneSystem::LANE_HEIGHT);
        float laneBottom = laneTop + LaneSystem::LANE_HEIGHT;
        
        if (enemy.coords.y < laneTop) enemy.coords.y = laneTop;
        if (enemy.coords.y > laneBottom) enemy.coords.y = laneBottom;
    }
    
    enemy.display.setPosition(enemy.coords.x, enemy.coords.y);
    
    if (enemy.coords.x <= GameConstants::LEFT_BOUNDARY) {
        enemy.coords.x = GameConstants::LEFT_BOUNDARY;
    }
}

void FlyingBehavior::onStateEnter(Enemy& enemy) {
    bobTimer.restart();
    enemy.display.setColor(sf::Color(200, 200, 255));
}

DancingBehavior::DancingBehavior(float speed, float interval, float randomness)
    : baseSpeed(speed), currentDirection(-1.0f, 0.0f), 
      directionChangeInterval(interval), randomnessStrength(randomness) {}

void DancingBehavior::execute(Enemy& enemy, float deltaTime, const std::vector<Enemy*>& allEnemies, const TowerFactory* towers) {
    if (enemy.is_frozen || !enemy.canAdvance) return;
    
    if (directionChangeTimer.getElapsedTime().asSeconds() > directionChangeInterval) {
        updateDirection();
        directionChangeTimer.restart();
    }
    
    enemy.coords.x += currentDirection.x * baseSpeed * deltaTime;
    enemy.coords.y += currentDirection.y * baseSpeed * randomnessStrength * 0.3f * deltaTime;
    if (currentDirection.x < 0.0f) {
    }
    
    int currentLane = LaneSystem::getLane(enemy.coords.y);
    if (currentLane >= 0 && currentLane < LaneSystem::NUM_LANES) {
        float laneTop = LaneSystem::LANE_TOP_Y + (currentLane * LaneSystem::LANE_HEIGHT);
        float laneBottom = laneTop + LaneSystem::LANE_HEIGHT;
        float laneCenter = (laneTop + laneBottom) / 2.0f;
        
        if (enemy.coords.y < laneTop + 15) {
            enemy.coords.y = laneTop + 15;
            currentDirection.y = std::abs(currentDirection.y);
        }
        if (enemy.coords.y > laneBottom - 15) {
            enemy.coords.y = laneBottom - 15;
            currentDirection.y = -std::abs(currentDirection.y);
        }
        
        float maxDeviation = 25.0f;
        if (std::abs(enemy.coords.y - laneCenter) > maxDeviation) {
            enemy.coords.y = laneCenter + (enemy.coords.y > laneCenter ? maxDeviation : -maxDeviation);
        }
    }
    
    enemy.display.setPosition(enemy.coords.x, enemy.coords.y);
    
    if (enemy.coords.x <= GameConstants::LEFT_BOUNDARY) {
        enemy.coords.x = GameConstants::LEFT_BOUNDARY;
    }
}

void DancingBehavior::onStateEnter(Enemy& enemy) {
    directionChangeTimer.restart();
    updateDirection();
    soundTimer.restart();
}

void DancingBehavior::updateDirection() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
    
    currentDirection.x = -1.0f;
    currentDirection.y = dis(gen);
}

BossBehavior::BossBehavior() : behaviorSwitchInterval(3.0f), healthThreshold(0.5f), isEnraged(false) {
    availableBehaviors.push_back(std::make_unique<AggressiveBehavior>(3.5f, 2.0f, 200.0f));
    availableBehaviors.push_back(std::make_unique<SwarmingBehavior>(2.5f, 120.0f, 60.0f, 0.8f, 1.2f));
    availableBehaviors.push_back(std::make_unique<DefensiveBehavior>(2.0f, 120.0f, 2.5f));
    
    currentBehavior = std::make_unique<AggressiveBehavior>(3.0f, 1.8f, 180.0f);
}

void BossBehavior::execute(Enemy& enemy, float deltaTime, const std::vector<Enemy*>& allEnemies, const TowerFactory* towers) {
    updateBehaviorBasedOnHealth(enemy);
    
    if (behaviorSwitchTimer.getElapsedTime().asSeconds() > behaviorSwitchInterval) {
        switchToRandomBehavior();
        behaviorSwitchTimer.restart();
    }
    
    float prevX = enemy.coords.x;
    if (currentBehavior) {
        currentBehavior->execute(enemy, deltaTime, allEnemies, towers);
    }
    if (enemy.coords.x > prevX) {
        enemy.coords.x = prevX;
        enemy.display.setPosition(enemy.coords.x, enemy.coords.y);
    }
    
    if (isEnraged) {
        enemy.display.setColor(sf::Color(255, 100, 100));
    } else {
        enemy.display.setColor(sf::Color(200, 150, 255));
    }
}

void BossBehavior::onStateEnter(Enemy& enemy) {
    behaviorSwitchTimer.restart();
    if (currentBehavior) {
        currentBehavior->onStateEnter(enemy);
    }
}

void BossBehavior::updateBehaviorBasedOnHealth(Enemy& enemy) {
    float healthPercentage = static_cast<float>(enemy.hitpoints) / static_cast<float>(enemy.hitpoints + 100);
    
    if (healthPercentage < healthThreshold && !isEnraged) {
        isEnraged = true;
        currentBehavior = std::make_unique<AggressiveBehavior>(4.0f, 2.5f, 250.0f);
        behaviorSwitchInterval = 1.5f;
    }
}

void BossBehavior::switchToRandomBehavior() {
    if (availableBehaviors.empty()) return;
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dis(0, availableBehaviors.size() - 1);
    
    size_t index = dis(gen);
    
    std::string behaviorName = availableBehaviors[index]->getBehaviorName();
    if (behaviorName == "Aggressive") {
        currentBehavior = std::make_unique<AggressiveBehavior>(isEnraged ? 4.0f : 3.0f, 
                                                              isEnraged ? 2.5f : 1.8f, 
                                                              isEnraged ? 250.0f : 180.0f);
    } else if (behaviorName == "Swarming") {
        currentBehavior = std::make_unique<SwarmingBehavior>(2.5f, 120.0f, 60.0f, 0.8f, 1.2f);
    } else if (behaviorName == "Defensive") {
        currentBehavior = std::make_unique<DefensiveBehavior>(2.0f, 120.0f, 2.5f);
    }
}

EnemyBehaviorContext::EnemyBehaviorContext(Enemy* enemyRef) : enemy(enemyRef) {}

void EnemyBehaviorContext::setBehavior(std::unique_ptr<IEnemyBehavior> behavior) {
    if (currentBehavior && enemy) {
        currentBehavior->onStateExit(*enemy);
    }
    
    currentBehavior = std::move(behavior);
    
    if (currentBehavior && enemy) {
        currentBehavior->onStateEnter(*enemy);
    }
}

void EnemyBehaviorContext::executeBehavior(float deltaTime, const std::vector<Enemy*>& allEnemies, const TowerFactory* towers) {
    if (currentBehavior && enemy) {
        currentBehavior->execute(*enemy, deltaTime, allEnemies, towers);
    }
}

std::string EnemyBehaviorContext::getCurrentBehaviorName() const {
    return currentBehavior ? currentBehavior->getBehaviorName() : "None";
}

std::unique_ptr<IEnemyBehavior> BehaviorFactory::createBehavior(const std::string& behaviorType) {
    if (behaviorType == "LinearMove") {
        return std::make_unique<LinearMoveBehavior>();
    } else if (behaviorType == "Aggressive") {
        return std::make_unique<AggressiveBehavior>();
    } else if (behaviorType == "Defensive") {
        return std::make_unique<DefensiveBehavior>();
    } else if (behaviorType == "Swarming") {
        return std::make_unique<SwarmingBehavior>();
    } else if (behaviorType == "Flying") {
        return std::make_unique<FlyingBehavior>();
    } else if (behaviorType == "Dancing") {
        return std::make_unique<DancingBehavior>();
    } else if (behaviorType == "Boss") {
        return std::make_unique<BossBehavior>();
    }
    
    return std::make_unique<LinearMoveBehavior>();
}

std::unique_ptr<IEnemyBehavior> BehaviorFactory::createBehaviorForEnemyType(const std::string& enemyType) {
    if (enemyType == "basic_zombie" || enemyType == "basic_zombie1") {
        return std::make_unique<LinearMoveBehavior>(2.0f);
    } else if (enemyType == "armored") {
        return std::make_unique<AggressiveBehavior>(2.5f, 1.3f, 120.0f);
    } else if (enemyType == "flying") {
        return std::make_unique<FlyingBehavior>(2.5f, 0.0f, 2.0f, 15.0f);
    } else if (enemyType == "dancing") {
        return std::make_unique<DancingBehavior>(2.0f, 1.2f, 1.8f);
    } else if (enemyType.find("boss_") == 0) {
        return std::make_unique<BossBehavior>();
    }
    
    return std::make_unique<LinearMoveBehavior>();
}

}
