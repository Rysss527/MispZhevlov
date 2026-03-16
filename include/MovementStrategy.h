#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "GameCursor.h"

class Enemy;
class Pathfinder;

class IMovementStrategy {
public:
    virtual ~IMovementStrategy() = default;
    virtual void update(Enemy& enemy, float dt, const std::vector<Enemy*>& allEnemies) = 0;
    virtual sf::Vector2f calculateMovement(Enemy& enemy, float dt, const std::vector<Enemy*>& allEnemies) = 0;
};

class LinearMovementStrategy : public IMovementStrategy {
private:
    float baseSpeed;
    sf::Vector2f direction;
    
public:
    LinearMovementStrategy(float speed = 2.0f, sf::Vector2f dir = sf::Vector2f(-1.0f, 0.0f)) 
        : baseSpeed(speed), direction(dir) {}
    
    virtual void update(Enemy& enemy, float dt, const std::vector<Enemy*>& allEnemies) override;
    virtual sf::Vector2f calculateMovement(Enemy& enemy, float dt, const std::vector<Enemy*>& allEnemies) override;
};

class WanderingMovementStrategy : public IMovementStrategy {
private:
    float baseSpeed;
    sf::Vector2f direction;
    sf::Clock wanderTimer;
    float wanderRate;
    float wanderStrength;
    
public:
    WanderingMovementStrategy(float speed = 3.0f, float wanderR = 0.02f, float wanderS = 1.0f) 
        : baseSpeed(speed), direction(-1.0f, 0.0f), wanderRate(wanderR), wanderStrength(wanderS) {}
    
    virtual void update(Enemy& enemy, float dt, const std::vector<Enemy*>& allEnemies) override;
    virtual sf::Vector2f calculateMovement(Enemy& enemy, float dt, const std::vector<Enemy*>& allEnemies) override;
};

class PathfindingMovementStrategy : public IMovementStrategy {
private:
    float baseSpeed;
    std::vector<sf::Vector2f> currentPath;
    size_t currentWaypoint;
    bool needsNewPath;
    sf::Clock pathUpdateTimer;
    
public:
    PathfindingMovementStrategy(float speed = 2.0f) 
        : baseSpeed(speed), currentWaypoint(0), needsNewPath(true) {}
    
    virtual void update(Enemy& enemy, float dt, const std::vector<Enemy*>& allEnemies) override;
    virtual sf::Vector2f calculateMovement(Enemy& enemy, float dt, const std::vector<Enemy*>& allEnemies) override;
    
private:
    void updatePath(Enemy& enemy);
    sf::Vector2f getDirectionToWaypoint(const sf::Vector2f& currentPos, const sf::Vector2f& targetPos);
};

class SeparationForce {
public:
    static sf::Vector2f calculateSeparation(Enemy& enemy, const std::vector<Enemy*>& allEnemies, 
                                          float separationRadius = 50.0f, float separationStrength = 30.0f);
};
