#pragma once

#include <memory>
#include <vector>
#include <SFML/Graphics.hpp>
#include "GameCursor.h"

class Enemy;
class TowerFactory;

namespace DefenseSynth {

class IEnemyBehavior {
public:
    virtual ~IEnemyBehavior() = default;
    virtual void execute(::Enemy& enemy, float deltaTime, const std::vector<::Enemy*>& allEnemies, const ::TowerFactory* towers) = 0;
    virtual void onStateEnter(::Enemy& enemy) {}
    virtual void onStateExit(::Enemy& enemy) {}
    virtual std::string getBehaviorName() const = 0;
};

class LinearMoveBehavior : public IEnemyBehavior {
private:
    float speed;
    sf::Vector2f direction;

public:
    LinearMoveBehavior(float moveSpeed = 2.0f, sf::Vector2f moveDirection = sf::Vector2f(-1.0f, 0.0f));
    void execute(::Enemy& enemy, float deltaTime, const std::vector<::Enemy*>& allEnemies, const ::TowerFactory* towers) override;
    std::string getBehaviorName() const override { return "LinearMove"; }
};

class AggressiveBehavior : public IEnemyBehavior {
private:
    float baseSpeed;
    float aggressionMultiplier;
    float detectionRange;

public:
    AggressiveBehavior(float speed = 3.0f, float aggression = 1.5f, float range = 150.0f);
    void execute(::Enemy& enemy, float deltaTime, const std::vector<::Enemy*>& allEnemies, const ::TowerFactory* towers) override;
    std::string getBehaviorName() const override { return "Aggressive"; }

private:
    bool isNearTowers(const ::Enemy& enemy, const ::TowerFactory* towers) const;
};

class DefensiveBehavior : public IEnemyBehavior {
private:
    float baseSpeed;
    float avoidanceRange;
    float avoidanceStrength;

public:
    DefensiveBehavior(float speed = 1.5f, float range = 100.0f, float strength = 2.0f);
    void execute(::Enemy& enemy, float deltaTime, const std::vector<::Enemy*>& allEnemies, const ::TowerFactory* towers) override;
    std::string getBehaviorName() const override { return "Defensive"; }

private:
    sf::Vector2f calculateAvoidanceVector(const ::Enemy& enemy, const ::TowerFactory* towers) const;
};

class SwarmingBehavior : public IEnemyBehavior {
private:
    float baseSpeed;
    float cohesionRange;
    float separationRange;
    float cohesionStrength;
    float separationStrength;

public:
    SwarmingBehavior(float speed = 2.0f, float cohesion = 80.0f, float separation = 40.0f, 
                    float cohesionStr = 0.5f, float separationStr = 1.0f);
    void execute(::Enemy& enemy, float deltaTime, const std::vector<::Enemy*>& allEnemies, const ::TowerFactory* towers) override;
    std::string getBehaviorName() const override { return "Swarming"; }

private:
    sf::Vector2f calculateCohesion(const Enemy& enemy, const std::vector<Enemy*>& allEnemies) const;
    sf::Vector2f calculateSeparation(const Enemy& enemy, const std::vector<Enemy*>& allEnemies) const;
};

class FlyingBehavior : public IEnemyBehavior {
private:
    float baseSpeed;
    float altitude;
    float bobFrequency;
    float bobAmplitude;
    sf::Clock bobTimer;

public:
    FlyingBehavior(float speed = 2.5f, float alt = 0.0f, float freq = 2.0f, float amp = 10.0f);
    void execute(::Enemy& enemy, float deltaTime, const std::vector<::Enemy*>& allEnemies, const ::TowerFactory* towers) override;
    void onStateEnter(::Enemy& enemy) override;
    std::string getBehaviorName() const override { return "Flying"; }
};

class DancingBehavior : public IEnemyBehavior {
private:
    float baseSpeed;
    sf::Vector2f currentDirection;
    sf::Clock directionChangeTimer;
    float directionChangeInterval;
    float randomnessStrength;
    sf::Clock soundTimer;

public:
    DancingBehavior(float speed = 2.0f, float interval = 1.0f, float randomness = 1.5f);
    void execute(::Enemy& enemy, float deltaTime, const std::vector<::Enemy*>& allEnemies, const ::TowerFactory* towers) override;
    void onStateEnter(::Enemy& enemy) override;
    std::string getBehaviorName() const override { return "Dancing"; }

private:
    void updateDirection();
};

class BossBehavior : public IEnemyBehavior {
private:
    std::unique_ptr<IEnemyBehavior> currentBehavior;
    std::vector<std::unique_ptr<IEnemyBehavior>> availableBehaviors;
    sf::Clock behaviorSwitchTimer;
    float behaviorSwitchInterval;
    float healthThreshold;
    bool isEnraged;

public:
    BossBehavior();
    void execute(::Enemy& enemy, float deltaTime, const std::vector<::Enemy*>& allEnemies, const ::TowerFactory* towers) override;
    void onStateEnter(::Enemy& enemy) override;
    std::string getBehaviorName() const override { return "Boss"; }

private:
    void updateBehaviorBasedOnHealth(::Enemy& enemy);
    void switchToRandomBehavior();
};

class EnemyBehaviorContext {
private:
    std::unique_ptr<IEnemyBehavior> currentBehavior;
    ::Enemy* enemy;

public:
    EnemyBehaviorContext(::Enemy* enemyRef);
    
    void setBehavior(std::unique_ptr<IEnemyBehavior> behavior);
    void executeBehavior(float deltaTime, const std::vector<::Enemy*>& allEnemies, const ::TowerFactory* towers);
    
    std::string getCurrentBehaviorName() const;
    IEnemyBehavior* getCurrentBehavior() const { return currentBehavior.get(); }
};

class BehaviorFactory {
public:
    static std::unique_ptr<IEnemyBehavior> createBehavior(const std::string& behaviorType);
    static std::unique_ptr<IEnemyBehavior> createBehaviorForEnemyType(const std::string& enemyType);
};

}
