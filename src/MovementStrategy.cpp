#include "MovementStrategy.h"
#include "Enemy.h"
#include "Pathfinder.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

void LinearMovementStrategy::update(Enemy& enemy, float dt, const std::vector<Enemy*>& allEnemies) {
    if (!enemy.enabled || enemy.is_frozen || !enemy.canAdvance) return;
    
    sf::Vector2f movement = calculateMovement(enemy, dt, allEnemies);
    bool isFlying = (enemy.type == GameConstants::FLYING_ZOMBIE) || (enemy.type.find("boss_flying") != std::string::npos);
    if (!isFlying) {
        movement.y = 0.0f;
    }
    enemy.coords.x += movement.x;
    enemy.coords.y += movement.y;
    if (!isFlying) {
        int lane = LaneSystem::getLane(enemy.coords.y);
        if (lane >= 0) {
            enemy.coords.y = LaneSystem::getLaneCenterY(lane) + 8.0f;
        }
    }
    enemy.display.setPosition(enemy.coords.x, enemy.coords.y);
    
    if (enemy.coords.x < 0) {
        enemy.enabled = false;
    }
}

sf::Vector2f LinearMovementStrategy::calculateMovement(Enemy& enemy, float dt, const std::vector<Enemy*>& allEnemies) {
    sf::Vector2f movement = direction * baseSpeed * dt;
    
    sf::Vector2f separation = SeparationForce::calculateSeparation(enemy, allEnemies);
    movement += separation * dt;
    
    return movement;
}

void WanderingMovementStrategy::update(Enemy& enemy, float dt, const std::vector<Enemy*>& allEnemies) {
    if (!enemy.enabled || enemy.is_frozen || !enemy.canAdvance) return;
    
    sf::Vector2f movement = calculateMovement(enemy, dt, allEnemies);
    bool isFlying = (enemy.type == GameConstants::FLYING_ZOMBIE) || (enemy.type.find("boss_flying") != std::string::npos);
    if (!isFlying) {
        movement.y = 0.0f;
    }
    enemy.coords.x += movement.x;
    enemy.coords.y += movement.y;
    if (!isFlying) {
        int lane = LaneSystem::getLane(enemy.coords.y);
        if (lane >= 0) {
            enemy.coords.y = LaneSystem::getLaneCenterY(lane);
        }
    }
    enemy.display.setPosition(enemy.coords.x, enemy.coords.y);
    
    if (enemy.coords.x < 0) {
        enemy.enabled = false;
    }
}

sf::Vector2f WanderingMovementStrategy::calculateMovement(Enemy& enemy, float dt, const std::vector<Enemy*>& allEnemies) {
    if (wanderTimer.getElapsedTime().asSeconds() > wanderRate) {
        if (rand() % 100 < 2) {
            direction.y += (rand() % 2 ? wanderStrength : -wanderStrength);
            direction.y = std::max(-2.0f, std::min(2.0f, direction.y));
        }
        wanderTimer.restart();
    }
    
    sf::Vector2f movement = direction * baseSpeed * dt;
    
    sf::Vector2f separation = SeparationForce::calculateSeparation(enemy, allEnemies);
    movement += separation * dt;
    
    return movement;
}

void PathfindingMovementStrategy::update(Enemy& enemy, float dt, const std::vector<Enemy*>& allEnemies) {
    if (!enemy.enabled || enemy.is_frozen || !enemy.canAdvance) return;
    
    if (needsNewPath || pathUpdateTimer.getElapsedTime().asSeconds() > 2.0f) {
        updatePath(enemy);
        pathUpdateTimer.restart();
        needsNewPath = false;
    }
    
    sf::Vector2f movement = calculateMovement(enemy, dt, allEnemies);
    bool isFlying = (enemy.type == GameConstants::FLYING_ZOMBIE) || (enemy.type.find("boss_flying") != std::string::npos);
    if (!isFlying) {
        movement.y = 0.0f;
    }
    enemy.coords.x += movement.x;
    enemy.coords.y += movement.y;
    if (!isFlying) {
        int lane = LaneSystem::getLane(enemy.coords.y);
        if (lane >= 0) {
            enemy.coords.y = LaneSystem::getLaneCenterY(lane);
        }
    }
    enemy.display.setPosition(enemy.coords.x, enemy.coords.y);
    
    if (enemy.coords.x < 0) {
        enemy.enabled = false;
    }
}

sf::Vector2f PathfindingMovementStrategy::calculateMovement(Enemy& enemy, float dt, const std::vector<Enemy*>& allEnemies) {
    sf::Vector2f movement(0.0f, 0.0f);
    
    if (!currentPath.empty() && currentWaypoint < currentPath.size()) {
        sf::Vector2f currentPos(enemy.coords.x, enemy.coords.y);
        sf::Vector2f targetPos = currentPath[currentWaypoint];
        
        float distanceToWaypoint = static_cast<float>(std::sqrt(std::pow(targetPos.x - currentPos.x, 2) + 
                                           std::pow(targetPos.y - currentPos.y, 2)));
        
        if (distanceToWaypoint < 10.0f) {
            currentWaypoint++;
            if (currentWaypoint >= currentPath.size()) {
                needsNewPath = true;
            }
        } else {
            movement = getDirectionToWaypoint(currentPos, targetPos) * baseSpeed * dt;
        }
    } else {
        movement = sf::Vector2f(-baseSpeed * dt, 0.0f);
    }
    
    sf::Vector2f separation = SeparationForce::calculateSeparation(enemy, allEnemies);
    movement += separation * 0.5f * dt;
    
    return movement;
}

void PathfindingMovementStrategy::updatePath(Enemy& enemy) {
    sf::Vector2f start(enemy.coords.x, enemy.coords.y);
    sf::Vector2f goal(0.0f, enemy.coords.y);
    
    currentPath = Pathfinder::findPath(start, goal);
    currentWaypoint = 0;
    
    if (currentPath.empty()) {
        currentPath.clear();
        int numWaypoints = 5;
        for (int i = 1; i <= numWaypoints; i++) {
            float t = static_cast<float>(i) / numWaypoints;
            sf::Vector2f waypoint = start + t * (goal - start);
            currentPath.push_back(waypoint);
        }
    }
}

sf::Vector2f PathfindingMovementStrategy::getDirectionToWaypoint(const sf::Vector2f& currentPos, const sf::Vector2f& targetPos) {
    sf::Vector2f direction = targetPos - currentPos;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    
    if (length > 0) {
        direction /= length;
    }
    
    return direction;
}

sf::Vector2f SeparationForce::calculateSeparation(Enemy& enemy, const std::vector<Enemy*>& allEnemies, 
                                                  float separationRadius, float separationStrength) {
    sf::Vector2f separationVector(0.0f, 0.0f);
    int neighborCount = 0;
    
    sf::Vector2f enemyPos(enemy.coords.x, enemy.coords.y);
    
    for (Enemy* other : allEnemies) {
        if (other == &enemy || !other->enabled) continue;
        
        sf::Vector2f otherPos(other->coords.x, other->coords.y);
        float distance = static_cast<float>(std::sqrt(std::pow(enemyPos.x - otherPos.x, 2) + 
                                 std::pow(enemyPos.y - otherPos.y, 2)));
        
        if (distance > 0 && distance < separationRadius) {
            sf::Vector2f diff = enemyPos - otherPos;
            diff /= distance;
            diff /= distance;
            
            separationVector += diff;
            neighborCount++;
        }
    }
    
    if (neighborCount > 0) {
        separationVector /= static_cast<float>(neighborCount);
        separationVector *= separationStrength;
    }
    
    return separationVector;
}
