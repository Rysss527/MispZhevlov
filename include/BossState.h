#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

class BossEnemy;

class BossState {
public:
    virtual ~BossState() = default;
    virtual void enter(BossEnemy& boss) = 0;
    virtual void execute(BossEnemy& boss, float dt) = 0;
    virtual void exit(BossEnemy& boss) = 0;
    virtual std::string getStateName() const = 0;
};

class ChasePlayerState : public BossState {
private:
    sf::Clock stateTimer;
    float duration;
    
public:
    ChasePlayerState(float duration = 5.0f) : duration(duration) {}
    
    virtual void enter(BossEnemy& boss) override;
    virtual void execute(BossEnemy& boss, float dt) override;
    virtual void exit(BossEnemy& boss) override;
    virtual std::string getStateName() const override { return "Chase"; }
};

class SpecialAttackState : public BossState {
private:
    sf::Clock stateTimer;
    sf::Clock attackTimer;
    float duration;
    float attackCooldown;
    bool hasAttacked;
    
public:
    SpecialAttackState(float duration = 3.0f, float cooldown = 0.5f) 
        : duration(duration), attackCooldown(cooldown), hasAttacked(false) {}
    
    virtual void enter(BossEnemy& boss) override;
    virtual void execute(BossEnemy& boss, float dt) override;
    virtual void exit(BossEnemy& boss) override;
    virtual std::string getStateName() const override { return "Attack"; }
};

class IdleState : public BossState {
private:
    sf::Clock stateTimer;
    float duration;
    bool hasSummoned;
    
public:
    IdleState(float duration = 2.0f) : duration(duration), hasSummoned(false) {}
    
    virtual void enter(BossEnemy& boss) override;
    virtual void execute(BossEnemy& boss, float dt) override;
    virtual void exit(BossEnemy& boss) override;
    virtual std::string getStateName() const override { return "Idle"; }
};

class RetreatState : public BossState {
private:
    sf::Clock stateTimer;
    float duration;
    float retreatDistance;
    sf::Vector2f retreatTarget;
    
public:
    RetreatState(float duration = 4.0f, float distance = 100.0f) 
        : duration(duration), retreatDistance(distance) {}
    
    virtual void enter(BossEnemy& boss) override;
    virtual void execute(BossEnemy& boss, float dt) override;
    virtual void exit(BossEnemy& boss) override;
    virtual std::string getStateName() const override { return "Retreat"; }
};
