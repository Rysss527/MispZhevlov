#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <cmath>
#include <memory>
#include "Enemy.h"
#include "BossState.h"
#include "SoundManager.h"
#include "Constants.h"
#include "EffectManager.h"

class BossEnemy : public Enemy
{
private:
    sf::Font bossFont;
    sf::Text bossLabel;
    sf::Font debugFont;
    sf::Text debugLabel;
    
    std::unique_ptr<BossState> currentState;
    sf::Clock stateTimer;
    float stateChangeInterval;
    int maxHealth;
    sf::Clock moveSoundTimer;
    sf::Clock attackSoundTimer;
    
public:
    BossEnemy(float x = 0, float y = 0)
    {
        numframes = 9;
        frame = 0;
        hitpoints = 200;
        attackDamage = 5;
        attackTimer.restart();
        coords.x = x;
        coords.y = y;
        velocity = 25.0f;
        freeze_duration = 0;
        is_frozen = false;
        poison_duration = 0;
        is_poisoned = false;
        poison_damage = 1;
        fire_duration = 0;
        is_burning = false;
        fire_damage = 2;
        enabled = true;
        auto* context = DefenseSynth::GameContext::getInstance();
        level = calculateZombieLevel(context->getCurrentLevel(), context->getDifficulty());
        hasUpgradeIndicator = false;
        int zombieType = rand() % 2;
        if (zombieType == 0) {
            if (!texture.loadFromFile("assets/images/zombie.png")) {
                texture.create(50, 50);
            }
            type = "boss_basic";
            velocity = 21.0f;
        } else {
            if (!texture.loadFromFile("assets/images/zombie1.png")) {
                texture.create(50, 50);
            }
            type = "boss_basic";
            velocity = 24.0f;
        }
        texture.setSmooth(true);
        display.setTexture(texture);
        display.setTextureRect(sf::IntRect(0, 0, 100, 100));
        display.setPosition(x, y);
        canAdvance = true;
        targetY = 0;
        scoreValue = 65;
        points = 65;
        hasSpawnedAllies = false;
        freeze_duration = 0.0f;
        
        if (!shotTexture.loadFromFile("assets/images/shot.png")) {
            shotTexture.create(30, 30);
        }
        shotTexture.setSmooth(true);
        shotSprite.setTexture(shotTexture);
        shotSprite.setTextureRect(sf::IntRect(0, 0, 30, 30));
        hasShotActive = false;
        shotPosition.x = 0;
        shotPosition.y = 0;
        hasExploded = false;
        
        if (!bossFont.loadFromFile("assets/fonts/arial.ttf")) {
            if (!bossFont.loadFromFile("assets/fonts/new.ttf")) {
                bossFont.loadFromFile("assets/fonts/arial.ttf");
            }
        }
        bossLabel.setFont(bossFont);
        bossLabel.setString("BOSS");
        bossLabel.setCharacterSize(18);
        bossLabel.setFillColor(sf::Color(150, 50, 255));
        bossLabel.setOutlineThickness(2);
        bossLabel.setOutlineColor(sf::Color::Black);
        bossLabel.setStyle(sf::Text::Bold);
        sf::FloatRect bl = bossLabel.getLocalBounds();
        bossLabel.setOrigin(bl.width/2.f, bl.height/2.f);
        bossLabel.setPosition(x + display.getGlobalBounds().width/2.f, y - 15);
        
        if (!debugFont.loadFromFile("assets/fonts/arial.ttf")) {
            if (!debugFont.loadFromFile("assets/fonts/new.ttf")) {
                debugFont.loadFromFile("assets/fonts/arial.ttf");
            }
        }
        debugLabel.setFont(debugFont);
        debugLabel.setString(type);
        debugLabel.setCharacterSize(14);
        debugLabel.setFillColor(sf::Color::Yellow);
        debugLabel.setStyle(sf::Text::Bold);
        debugLabel.setPosition(x, y - 50);
        
        display.setScale(GameConstants::BOSS_SCALE_FACTOR, GameConstants::BOSS_SCALE_FACTOR);
        display.setOrigin(50, 50);
        display.setColor(sf::Color(200, 150, 255));
        
        if (level > 0) {
            auto* context = DefenseSynth::GameContext::getInstance();
            float difficulty = context->getDifficulty();
            
            int hitpointsBonus = static_cast<int>(level * 30 * std::pow(GameConstants::LEVEL_DAMAGE_SCALING, static_cast<float>(level)) * (1.0f + difficulty * 0.5f));
            hitpoints += hitpointsBonus;
            float maxVelocityIncrease = velocity * 0.4f;
            float velocityIncrease = std::min(level * (3.0f + difficulty * 1.0f), maxVelocityIncrease);
            velocity = velocity + velocityIncrease;
            int scoreBonus = static_cast<int>(level * 45 * std::pow(GameConstants::LEVEL_DAMAGE_SCALING, static_cast<float>(level)) * (1.0f + difficulty * 0.3f));
            scoreValue += scoreBonus;
            points += scoreBonus;
        }
        
        maxHealth = hitpoints;
        stateChangeInterval = 4.0f + static_cast<float>(rand() % 3);
        currentState = std::make_unique<ChasePlayerState>();
        currentState->enter(*this);
        stateTimer.restart();
    }
    
    virtual void move(float deltaTime = 0.016f) override
    {
        if (!enabled) return;
        
        handleStatusEffects();
        
        if (hitpoints <= 0) {
            triggerDeath();
            return;
        }
        
        updateState();
        
        static sf::Vector2f previousPosition = sf::Vector2f(coords.x, coords.y);
        static sf::Clock stuckTimer;
        static float stuckCheckInterval = 2.0f;
        
        if (currentState && !is_frozen) {
            float dt = moveClock.getElapsedTime().asSeconds();
            if (dt > 0.016f) {
                sf::Vector2f currentPosition(coords.x, coords.y);
                
                if (stuckTimer.getElapsedTime().asSeconds() >= stuckCheckInterval) {
                    float distance = static_cast<float>(std::sqrt(std::pow(currentPosition.x - previousPosition.x, 2.0f) + 
                                                                     std::pow(currentPosition.y - previousPosition.y, 2.0f)));
                    
                    if (distance < 5.0f && currentState->getStateName() != "Attack") {
                        coords.y += (rand() % 2 ? 15 : -15);
                        if (coords.y < 150) coords.y = 150;
                        if (coords.y > 550) coords.y = 550;
                    }
                    
                    previousPosition = currentPosition;
                    stuckTimer.restart();
                }
                
                currentState->execute(*this, dt);
                moveClock.restart();
                if (animationClock.getElapsedTime().asSeconds() >= 0.1f) {
                    frame = (frame + 1) % numframes;
                    animationClock.restart();
                }
                if (isAttacking) {
                    display.setTextureRect(sf::IntRect(100 * (frame), 100, 100, 100));
                } else {
                    display.setTextureRect(sf::IntRect(100 * (frame), 0, 100, 100));
                }
            }
        }
        
        handleShots();
        
        if (type != "boss_basic") {
            if (isAttacking) {
                hasShotActive = true;
                shotPosition.x = coords.x - 20;
                shotPosition.y = coords.y + 20;
                shotSprite.setPosition(shotPosition.x, shotPosition.y);
            } else {
                hasShotActive = false;
            }
        }
        
        sf::Color currentColor = sf::Color(200, 150, 255);
        if (is_frozen) currentColor = sf::Color(150, 150, 255, 255);
        else if (is_poisoned) {
            float rem = (std::max)(0.0f, poison_duration - poison_timer.getElapsedTime().asSeconds());
            float ratio = poison_duration > 0.0f ? rem / poison_duration : 1.0f;
            sf::Uint8 a = static_cast<sf::Uint8>((std::max)(0.0f, (std::min)(255.0f, 255.0f * ratio)));
            currentColor = sf::Color(100, 200, 100, a);
        }
        else if (is_burning) currentColor = sf::Color(255, 150, 150, 255);
        display.setColor(currentColor);
        
        sf::FloatRect bl = bossLabel.getLocalBounds();
        bossLabel.setOrigin(bl.width/2.f, bl.height/2.f);
        bossLabel.setPosition(coords.x + display.getGlobalBounds().width/2.f, coords.y - 15);
        debugLabel.setPosition(coords.x, coords.y - 50);
        
        if (coords.x < 100) {
        }
    }
    
    void changeState(std::unique_ptr<BossState> newState) {
        if (currentState) {
            currentState->exit(*this);
        }
        currentState = std::move(newState);
        if (currentState) {
            currentState->enter(*this);
        }
        stateTimer.restart();
    }
    
    BossState* getCurrentState() const {
        return currentState.get();
    }
    
    bool readyMoveSound(float seconds) const { return moveSoundTimer.getElapsedTime().asSeconds() > seconds; }
    bool readyAttackSound(float seconds) const { return attackSoundTimer.getElapsedTime().asSeconds() > seconds; }
    void restartMoveSoundTimer() { moveSoundTimer.restart(); }
    void restartAttackSoundTimer() { attackSoundTimer.restart(); }
    
private:
    void handleStatusEffects() {
        if (freeze_duration > 0.0f) {
            if (freeze_timer.getElapsedTime().asSeconds() >= freeze_duration) {
                is_frozen = false;
                freeze_duration = 0.0f;
            }
        }

        if (poison_duration > 0.0f) {
            if (poison_timer.getElapsedTime().asSeconds() >= poison_duration) {
                is_poisoned = false;
                poison_duration = 0.0f;
            } else if (poison_timer.getElapsedTime().asSeconds() >= 1.0f) {
                hitpoints -= poison_damage;
                poison_timer.restart();
            }
        }

        if (fire_duration > 0.0f) {
            if (fire_timer.getElapsedTime().asSeconds() >= fire_duration) {
                is_burning = false;
                fire_duration = 0.0f;
            } else if (fire_timer.getElapsedTime().asSeconds() >= 0.5f) {
                hitpoints -= fire_damage;
                fire_timer.restart();
            }
        }
    }
    
    void updateState() {
        float stateTime = stateTimer.getElapsedTime().asSeconds();
        float hitpointsPercentage = static_cast<float>(hitpoints) / maxHealth;
        
        if (hitpointsPercentage < 0.25f && currentState->getStateName() != "Retreat") {
            changeState(std::make_unique<RetreatState>());
        } else if (hitpointsPercentage < 0.5f && stateTime > stateChangeInterval && 
                  currentState->getStateName() != "Attack") {
            changeState(std::make_unique<SpecialAttackState>());
        } else if (stateTime > stateChangeInterval) {
            int choice = rand() % 3;
            std::string currentStateName = currentState->getStateName();
            
            if (choice == 0 && currentStateName != "Chase") {
                changeState(std::make_unique<ChasePlayerState>());
            } else if (choice == 1 && currentStateName != "Attack") {
                changeState(std::make_unique<SpecialAttackState>());
            } else if (choice == 2 && currentStateName != "Idle") {
                changeState(std::make_unique<IdleState>());
            }
        }
    }
    
    void handleShots() {
    }
    
public:
    
    void draw(sf::RenderWindow &window)
    {
        if (enabled) {
            window.draw(display);
            
            if (hasShotActive && type != "boss_basic")
            {
                window.draw(shotSprite);
            }
            
            window.draw(bossLabel);
        }
    }
};
class BossArmoredEnemy : public ArmoredEnemy
{
private:
    sf::Font bossFont;
    sf::Text bossLabel;
    sf::Font debugFont;
    sf::Text debugLabel;
    sf::Clock moveSoundTimer;
    sf::Clock attackSoundTimer;
    
public:
    BossArmoredEnemy(float x = 0, float y = 0) : ArmoredEnemy(x, y)
    {
        hitpoints = 200;
        attackDamage = 6;
        attackTimer.restart();
        scoreValue = 300;
        points = 300;
        velocity = 15.0f;
        type = "boss_armored";
        
        if (!texture.loadFromFile("assets/images/footballzombie.png")) {
            texture.create(50, 50);
        }
        texture.setSmooth(true);
        display.setTexture(texture);
        display.setTextureRect(sf::IntRect(0, 0, 100, 100));
        display.setPosition(x, y);
        numframes = 28;
        
        
        if (!bossFont.loadFromFile("assets/fonts/arial.ttf")) {
            if (!bossFont.loadFromFile("assets/fonts/new.ttf")) {
                bossFont.loadFromFile("assets/fonts/arial.ttf");
            }
        }
        bossLabel.setFont(bossFont);
        bossLabel.setString("BOSS");
        bossLabel.setCharacterSize(18);
        bossLabel.setFillColor(sf::Color(150, 50, 255));
        bossLabel.setOutlineThickness(2);
        bossLabel.setOutlineColor(sf::Color::Black);
        bossLabel.setStyle(sf::Text::Bold);
        sf::FloatRect bl = bossLabel.getLocalBounds();
        bossLabel.setOrigin(bl.width/2.f, bl.height/2.f);
        bossLabel.setPosition(x + display.getGlobalBounds().width/2.f, y - 15);
        
        if (!debugFont.loadFromFile("assets/fonts/arial.ttf")) {
            if (!debugFont.loadFromFile("assets/fonts/new.ttf")) {
                debugFont.loadFromFile("assets/fonts/arial.ttf");
            }
        }
        debugLabel.setFont(debugFont);
        debugLabel.setString(type);
        debugLabel.setCharacterSize(14);
        debugLabel.setFillColor(sf::Color::Yellow);
        debugLabel.setStyle(sf::Text::Bold);
        debugLabel.setPosition(x, y - 50);
        
        display.setScale(GameConstants::BOSS_SCALE_FACTOR, GameConstants::BOSS_SCALE_FACTOR);
        display.setOrigin(50, 50);
        display.setColor(sf::Color(200, 150, 255));
        
        if (level > 1) {
            auto* context = DefenseSynth::GameContext::getInstance();
            hitpoints += static_cast<int>(level * 25 * (1.0f + context->getDifficulty() * 0.4f));
            float velocityIncrease = (level - 1) * 3.0f * (1.0f + context->getDifficulty() * 0.3f);
            velocity = velocity + velocityIncrease;
            scoreValue += (level - 1) * 25;
            points += (level - 1) * 25;
        }
        
        if (!shotTexture.loadFromFile("assets/images/shot.png")) {
            shotTexture.create(30, 30);
        }
        shotTexture.setSmooth(true);
        shotSprite.setTexture(shotTexture);
        shotSprite.setTextureRect(sf::IntRect(0, 0, 30, 30));
        hasShotActive = false;
        shotPosition.x = 0;
        shotPosition.y = 0;
    }
    
    virtual void move(float deltaTime = 0.016f) override
    {
        if (behaviorClock.getElapsedTime().asSeconds() > 5)
        {
            canAdvance = true;
            behaviorClock.restart();
        }

        if (enabled)
        {
            if (freeze_duration > 0.0f)
            {
                if (freeze_timer.getElapsedTime().asSeconds() >= freeze_duration)
                {
                    is_frozen = false;
                    freeze_duration = 0.0f;
                }
            }

            if (poison_duration > 0.0f)
            {
                if (poison_timer.getElapsedTime().asSeconds() >= poison_duration)
                {
                    is_poisoned = false;
                    poison_duration = 0.0f;
                }
                else if (poison_timer.getElapsedTime().asSeconds() >= 1.0f)
                {
                    hitpoints -= poison_damage;
                    poison_timer.restart();
                }
            }

            if (fire_duration > 0.0f)
            {
                if (fire_timer.getElapsedTime().asSeconds() >= fire_duration)
                {
                    is_burning = false;
                    fire_duration = 0.0f;
                }
                else if (fire_timer.getElapsedTime().asSeconds() >= 0.5f)
                {
                    hitpoints -= fire_damage;
                    fire_timer.restart();
                }
            }
            
            if (hitpoints <= 0)
            {
                triggerDeath();
                return;
            }
            

            sf::Color currentColor = sf::Color(200, 150, 255);
            if (is_frozen) currentColor = sf::Color(150, 150, 255, 255);
            else if (is_poisoned) {
                float rem = (std::max)(0.0f, poison_duration - poison_timer.getElapsedTime().asSeconds());
                float ratio = poison_duration > 0.0f ? rem / poison_duration : 1.0f;
                sf::Uint8 a = static_cast<sf::Uint8>((std::max)(0.0f, (std::min)(255.0f, 255.0f * ratio)));
                currentColor = sf::Color(50, 100, 50, a);
            }
            else if (is_burning) currentColor = sf::Color(255, 100, 100, 255);
            display.setColor(currentColor);
            

            if (!is_frozen)
            {
                if (!canAdvance && clock2.getElapsedTime().asSeconds() >= 5.0f) {
                    canAdvance = true;
                }
                if (animationClock.getElapsedTime().asSeconds() >= 0.1f) {
                    frame = (frame + 1) % numframes;
                    animationClock.restart();
                }
                if (canAdvance)
                {
                    float prevX = coords.x;
                    coords.x -= velocity * deltaTime;
                    
                    if (rand() % 100 < 2)
                    {
                        coords.y += (rand() % 2 ? 1 : -1);
                        float minY = LaneSystem::getLaneY(0);
                        float maxY = LaneSystem::getLaneY(LaneSystem::NUM_LANES - 1) + LaneSystem::LANE_HEIGHT;
                        if (coords.y < minY) coords.y = minY;
                        if (coords.y > maxY) coords.y = maxY;
                    }
                    int lane = LaneSystem::getLane(coords.y);
                    if (lane >= 0) {
                        coords.y = LaneSystem::getLaneCenterY(lane) + 8.0f;
                    }
                    display.setPosition(coords.x, coords.y);
                    if (isAttacking) {
                        display.setTextureRect(sf::IntRect(100 * (frame), 0, 100, 100));
                    } else {
                        display.setTextureRect(sf::IntRect(100 * (frame), 0, 100, 100));
                    }
                    
                }
                else
                {
                    if (isAttacking) {
                        display.setTextureRect(sf::IntRect(100 * (frame), 0, 100, 100));
                    } else {
                        display.setTextureRect(sf::IntRect(100 * (frame), 0, 100, 100));
                    }
                }
                moveClock.restart();
                if (coords.x < 100)
                {
                    
                }
            }
        }
        sf::FloatRect bl = bossLabel.getLocalBounds();
        bossLabel.setOrigin(bl.width/2.f, bl.height/2.f);
        bossLabel.setPosition(coords.x + display.getGlobalBounds().width/2.f, coords.y - 15);
        debugLabel.setPosition(coords.x, coords.y - 50);
    }
    
    void draw(sf::RenderWindow &window)
    {
        if (enabled) {
            window.draw(display);
            
            if (hasShotActive && type != "boss_basic")
            {
                window.draw(shotSprite);
            }
            
            window.draw(bossLabel);
        }
    }
};
class BossFlyingEnemy : public FlyingEnemy
{
private:
    sf::Font bossFont;
    sf::Text bossLabel;
    sf::Font debugFont;
    sf::Text debugLabel;
    
public:
    BossFlyingEnemy(float x = 0, float y = 0) : FlyingEnemy(x, y)
    {
        hitpoints = 80;
        attackDamage = 4;
        attackTimer.restart();
        scoreValue = 350;
        points = 350;
        velocity = 65.0f;
        type = "boss_flying";
        targetY = y;
        
        if (!texture.loadFromFile("assets/images/flyingzombie.png")) {
            texture.create(50, 50);
        }
        texture.setSmooth(true);
        display.setTexture(texture);
        display.setTextureRect(sf::IntRect(0, 0, 100, 100));
        
        if (!bossFont.loadFromFile("assets/fonts/arial.ttf")) {
            if (!bossFont.loadFromFile("assets/fonts/new.ttf")) {
                bossFont.loadFromFile("assets/fonts/arial.ttf");
            }
        }
        bossLabel.setFont(bossFont);
        bossLabel.setString("BOSS");
        bossLabel.setCharacterSize(18);
        bossLabel.setFillColor(sf::Color(150, 50, 255));
        bossLabel.setOutlineThickness(2);
        bossLabel.setOutlineColor(sf::Color::Black);
        bossLabel.setStyle(sf::Text::Bold);
        sf::FloatRect bl = bossLabel.getLocalBounds();
        bossLabel.setOrigin(bl.width/2.f, bl.height/2.f);
        bossLabel.setPosition(x + display.getGlobalBounds().width/2.f, y - 15);
        
        if (!debugFont.loadFromFile("assets/fonts/arial.ttf")) {
            if (!debugFont.loadFromFile("assets/fonts/new.ttf")) {
                debugFont.loadFromFile("assets/fonts/arial.ttf");
            }
        }
        debugLabel.setFont(debugFont);
        debugLabel.setString(type);
        debugLabel.setCharacterSize(14);
        debugLabel.setFillColor(sf::Color::Yellow);
        debugLabel.setStyle(sf::Text::Bold);
        debugLabel.setPosition(x, y - 50);
        
        display.setScale(GameConstants::BOSS_SCALE_FACTOR, GameConstants::BOSS_SCALE_FACTOR);
        display.setOrigin(50, 50);
        display.setColor(sf::Color(200, 150, 255));
        
        if (level > 1) {
            auto* context = DefenseSynth::GameContext::getInstance();
            hitpoints += static_cast<int>(level * 25 * (1.0f + context->getDifficulty() * 0.4f));
            float velocityIncrease = (level - 1) * 3.0f * (1.0f + context->getDifficulty() * 0.3f);
            velocity = velocity + velocityIncrease;
            scoreValue += (level - 1) * 25;
            points += (level - 1) * 25;
        }
        
        if (!shotTexture.loadFromFile("assets/images/shot.png")) {
            shotTexture.create(30, 30);
        }
        shotTexture.setSmooth(true);
        shotSprite.setTexture(shotTexture);
        shotSprite.setTextureRect(sf::IntRect(0, 0, 30, 30));
        hasShotActive = false;
        shotPosition.x = 0;
        shotPosition.y = 0;
    }
    
    virtual void move(float deltaTime = 0.016f) override
    {
        if (behaviorClock.getElapsedTime().asSeconds() > 5)
        {
            canAdvance = true;
            behaviorClock.restart();
        }

        if (enabled)
        {
            if (freeze_duration > 0.0f)
            {
                if (freeze_timer.getElapsedTime().asSeconds() >= freeze_duration)
                {
                    is_frozen = false;
                    freeze_duration = 0.0f;
                }
            }

            if (poison_duration > 0.0f)
            {
                if (poison_timer.getElapsedTime().asSeconds() >= poison_duration)
                {
                    is_poisoned = false;
                    poison_duration = 0.0f;
                }
                else if (poison_timer.getElapsedTime().asSeconds() >= 1.0f)
                {
                    hitpoints -= poison_damage;
                    poison_timer.restart();
                }
            }

            if (fire_duration > 0.0f)
            {
                if (fire_timer.getElapsedTime().asSeconds() >= fire_duration)
                {
                    is_burning = false;
                    fire_duration = 0.0f;
                }
                else if (fire_timer.getElapsedTime().asSeconds() >= 0.5f)
                {
                    hitpoints -= fire_damage;
                    fire_timer.restart();
                }
            }
            
            if (hitpoints <= 0)
            {
                triggerDeath();
                return;
            }
            

            bool isBoss = (type.find("boss") != std::string::npos);
            sf::Color currentColor = sf::Color(255, 255, 255, 255);
            if (isBoss) {
                currentColor = sf::Color(200, 150, 255);
            } else {
                if (is_frozen) currentColor = sf::Color(150, 150, 255, 255);
                else if (is_poisoned) {
                    float rem = (std::max)(0.0f, poison_duration - poison_timer.getElapsedTime().asSeconds());
                    float ratio = poison_duration > 0.0f ? rem / poison_duration : 1.0f;
                    sf::Uint8 a = static_cast<sf::Uint8>((std::max)(0.0f, (std::min)(255.0f, 255.0f * ratio)));
                    currentColor = sf::Color(50, 100, 50, a);
                }
                else if (is_burning) currentColor = sf::Color(255, 100, 100, 255);
            }
            display.setColor(currentColor);
            
            if (type != "boss_basic") {
                if (isAttacking) {
                    hasShotActive = true;
                    shotPosition.x = coords.x - 20;
                    shotPosition.y = coords.y + 20;
                    shotSprite.setPosition(shotPosition.x, shotPosition.y);
                } else {
                    hasShotActive = false;
                }
            }

            if (!is_frozen)
            {
                if (!canAdvance && clock2.getElapsedTime().asSeconds() >= 5.0f) {
                    canAdvance = true;
                }
                if (animationClock.getElapsedTime().asSeconds() >= 0.1f) {
                    frame = (frame + 1) % numframes;
                    animationClock.restart();
                }
                if (isAttacking) {
                    display.setTextureRect(sf::IntRect(100 * (frame), 0, 100, 100));
                } else if (canAdvance) {
                    coords.x -= velocity * deltaTime;
                    
                    display.setTextureRect(sf::IntRect(100 * (frame), 0, 100, 100));
                }
                display.setPosition(coords.x, coords.y);
                
                if (coords.x < GameConstants::LEFT_BOUNDARY) {
                    reachedEnd = true;
                    enabled = false;
                }
                moveClock.restart();
            }
        }
        sf::FloatRect bl = bossLabel.getLocalBounds();
        bossLabel.setOrigin(bl.width/2.f, bl.height/2.f);
        bossLabel.setPosition(coords.x + display.getGlobalBounds().width/2.f, coords.y - 15);
        debugLabel.setPosition(coords.x, coords.y - 50);
    }
    
    void draw(sf::RenderWindow &window)
    {
        if (enabled) {
            window.draw(display);
            
            if (hasShotActive && type != "boss_basic")
            {
                window.draw(shotSprite);
            }
            
            window.draw(bossLabel);
        }
    }
};
class BossDancingEnemy : public DancingEnemy
{
private:
    sf::Font bossFont;
    sf::Text bossLabel;
    sf::Font debugFont;
    sf::Text debugLabel;
    
public:
    BossDancingEnemy(float x = 0, float y = 0)
    {
        scoreValue = 400;
        hitpoints = 120;
        attackDamage = 8;
        attackTimer.restart();
        coords.x = x;
        coords.y = y;
        velocity = 45.0f;
        numframes = 9;
        frame = 0;
        enabled = true;
        if (!texture.loadFromFile("assets/images/dancingzombie.png")) {
            texture.create(50, 50);
        }
        texture.setSmooth(true);
        display.setTexture(texture);
        display.setTextureRect(sf::IntRect(0, 0, 100, 100));
        display.setPosition(x, y);
        canAdvance = true;
        type = "boss_dancing";
        
        
        if (!bossFont.loadFromFile("assets/fonts/arial.ttf")) {
            if (!bossFont.loadFromFile("assets/fonts/new.ttf")) {
                bossFont.loadFromFile("assets/fonts/arial.ttf");
            }
        }
        bossLabel.setFont(bossFont);
        bossLabel.setString("BOSS");
        bossLabel.setCharacterSize(18);
        bossLabel.setFillColor(sf::Color(150, 50, 255));
        bossLabel.setOutlineThickness(2);
        bossLabel.setOutlineColor(sf::Color::Black);
        bossLabel.setStyle(sf::Text::Bold);
        sf::FloatRect bl = bossLabel.getLocalBounds();
        bossLabel.setOrigin(bl.width/2.f, bl.height/2.f);
        bossLabel.setPosition(x + display.getGlobalBounds().width/2.f, y - 15);
        
        if (!debugFont.loadFromFile("assets/fonts/arial.ttf")) {
            if (!debugFont.loadFromFile("assets/fonts/new.ttf")) {
                debugFont.loadFromFile("assets/fonts/arial.ttf");
            }
        }
        debugLabel.setFont(debugFont);
        debugLabel.setString(type);
        debugLabel.setCharacterSize(14);
        debugLabel.setFillColor(sf::Color::Yellow);
        debugLabel.setStyle(sf::Text::Bold);
        debugLabel.setPosition(x, y - 50);
        
        display.setScale(GameConstants::BOSS_SCALE_FACTOR, GameConstants::BOSS_SCALE_FACTOR);
        display.setOrigin(50, 50);
        display.setColor(sf::Color(200, 150, 255));
        
        if (level > 1) {
            auto* context = DefenseSynth::GameContext::getInstance();
            hitpoints += static_cast<int>(level * 25 * (1.0f + context->getDifficulty() * 0.4f));
            float velocityIncrease = (level - 1) * 3.0f * (1.0f + context->getDifficulty() * 0.3f);
            velocity = velocity + velocityIncrease;
            scoreValue += (level - 1) * 25;
            points += (level - 1) * 25;
        }
        
        if (!shotTexture.loadFromFile("assets/images/shot.png")) {
            shotTexture.create(30, 30);
        }
        shotTexture.setSmooth(true);
        shotSprite.setTexture(shotTexture);
        shotSprite.setTextureRect(sf::IntRect(0, 0, 30, 30));
        hasShotActive = false;
        shotPosition.x = 0;
        shotPosition.y = 0;
    }
    
    virtual void move(float deltaTime = 0.016f) override
    {
        if (behaviorClock.getElapsedTime().asSeconds() > 5)
        {
            canAdvance = true;
            behaviorClock.restart();
        }

        if (enabled)
        {
            if (freeze_duration > 0.0f)
            {
                if (freeze_timer.getElapsedTime().asSeconds() >= freeze_duration)
                {
                    is_frozen = false;
                    freeze_duration = 0.0f;
                }
            }

            if (poison_duration > 0.0f)
            {
                if (poison_timer.getElapsedTime().asSeconds() >= poison_duration)
                {
                    is_poisoned = false;
                    poison_duration = 0.0f;
                }
                else if (poison_timer.getElapsedTime().asSeconds() >= 1.0f)
                {
                    hitpoints -= poison_damage;
                    poison_timer.restart();
                }
            }

            if (fire_duration > 0.0f)
            {
                if (fire_timer.getElapsedTime().asSeconds() >= fire_duration)
                {
                    is_burning = false;
                    fire_duration = 0.0f;
                }
                else if (fire_timer.getElapsedTime().asSeconds() >= 0.5f)
                {
                    hitpoints -= fire_damage;
                    fire_timer.restart();
                }
            }
            
            if (hitpoints <= 0)
            {
                triggerDeath();
                return;
            }
            

            sf::Color currentColor = sf::Color(200, 150, 255);
            if (is_frozen) currentColor = sf::Color(150, 150, 255, 255);
            else if (is_poisoned) {
                float rem = (std::max)(0.0f, poison_duration - poison_timer.getElapsedTime().asSeconds());
                float ratio = poison_duration > 0.0f ? rem / poison_duration : 1.0f;
                sf::Uint8 a = static_cast<sf::Uint8>((std::max)(0.0f, (std::min)(255.0f, 255.0f * ratio)));
                currentColor = sf::Color(50, 100, 50, a);
            }
            else if (is_burning) currentColor = sf::Color(255, 100, 100, 255);
            
            if (isAttacking) {
                float time = animationClock.getElapsedTime().asSeconds();
                float fireIntensity = 0.5f + 0.5f * sin(time * 15.0f);
                float flicker = 0.8f + 0.2f * sin(time * 25.0f);
                currentColor = sf::Color(
                    255, 
                    static_cast<sf::Uint8>(120 + 120 * fireIntensity * flicker), 
                    static_cast<sf::Uint8>(40 * fireIntensity), 
                    static_cast<sf::Uint8>(220 + 35 * fireIntensity)
                );
            }
            
            display.setColor(currentColor);
            
            if (type != "boss_basic") {
                if (isAttacking) {
                    hasShotActive = true;
                    shotPosition.x = coords.x - 20;
                    shotPosition.y = coords.y + 20;
                    shotSprite.setPosition(shotPosition.x, shotPosition.y);
                } else {
                    hasShotActive = false;
                }
            }

            if (!is_frozen)
            {
                if (!canAdvance && clock2.getElapsedTime().asSeconds() >= 5.0f) {
                    canAdvance = true;
                }
                if (animationClock.getElapsedTime().asSeconds() >= 0.1f) {
                    frame = (frame + 1) % numframes;
                    animationClock.restart();
                }
                if (canAdvance)
                {
                    coords.x -= velocity * deltaTime;
                    
                    if (rand() % 100 < 2)
                    {
                        coords.y += (rand() % 2 ? 1 : -1);
                        if (coords.y < 200) coords.y = 200;
                        if (coords.y > 550) coords.y = 550;
                    }
                    display.setPosition(coords.x, coords.y);
                    display.setTextureRect(sf::IntRect(100 * (frame), 0, 100, 100));
                }
                else
                {
                    display.setTextureRect(sf::IntRect(100 * (frame), 100, 100, 100));
                }
                moveClock.restart();
                if (coords.x < 100)
                {
                    
                }
            }
        }
        sf::FloatRect bl = bossLabel.getLocalBounds();
        bossLabel.setOrigin(bl.width/2.f, bl.height/2.f);
        bossLabel.setPosition(coords.x + display.getGlobalBounds().width/2.f, coords.y - 15);
        debugLabel.setPosition(coords.x, coords.y - 50);
    }
    
    void draw(sf::RenderWindow &window)
    {
        if (enabled) {
            window.draw(display);
            
            if (hasShotActive && type != "boss_basic")
            {
                window.draw(shotSprite);
            }
            
            window.draw(bossLabel);
        }
    }
};
