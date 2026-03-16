#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <memory>
#include "Projectile.h"
#include "GameCursor.h"
#include <fstream>
#include "Resource.h"
#include "Constants.h"
#include "SpriteScaler.h"

class Tower {
public:
    bool isMoving;
    int vitality;
    int cost;
    coordinates location;
    sf::Sprite visual;
    sf::Texture texture;
    sf::Clock timer;
    int attackCooldown;
    int targetX;
    std::string type;
    std::string category;
    bool operational;
    int numframes;
    int frame;
    int targetY;
    bool massFreeze;
    bool freezeApplied;
    bool hasExploded;
    int level;
    int damage;
    bool isDying;
    bool isBarrier;
    sf::Clock deathClock;
    sf::Texture deathTexture;
    sf::Clock regenerationClock;
    int maxVitality;
    
    sf::Font upgradeFont;
    sf::Text upgradeLevelText;
    bool hasUpgradeIndicator;
    
    virtual ~Tower() {}
    
    virtual void drawSprite(sf::RenderWindow& window) { 
        draw(window);
    }
    virtual void drawLabel(sf::RenderWindow& window) {
        if (operational && level > 0) {
            upgradeLevelText.setPosition(location.x + 35, location.y + 80);
            window.draw(upgradeLevelText);
        }
    }
    
    virtual void update(float deltaTime) {
        if (vitality <= 0) {
            operational = false;
        }
        if (type != "BombTower" && vitality < maxVitality && regenerationClock.getElapsedTime().asSeconds() >= 3.0f) {
            int regenAmount = std::max(8, maxVitality / 15);
            vitality = std::min(maxVitality, vitality + regenAmount);
            regenerationClock.restart();
        }
    }
    
    virtual void drawTower(sf::RenderWindow& window) {
        if (operational) {
            window.draw(visual);
            if (level > 0) {
                upgradeLevelText.setPosition(location.x + 35, location.y + 80);
                window.draw(upgradeLevelText);
            }
        }
    }
    
    void setupUpgradeIndicator() {
        if (!upgradeFont.loadFromFile("arial.ttf")) {
            if (!upgradeFont.loadFromFile("new.ttf")) {
                upgradeFont.loadFromFile("assets/fonts/arial.ttf");
            }
        }
        upgradeLevelText.setFont(upgradeFont);
        upgradeLevelText.setCharacterSize(20);
        upgradeLevelText.setFillColor(sf::Color::Yellow);
        upgradeLevelText.setStyle(sf::Text::Bold);
        upgradeLevelText.setOutlineThickness(2);
        upgradeLevelText.setOutlineColor(sf::Color::Black);
        hasUpgradeIndicator = true;
        updateUpgradeIndicator();
    }
    
    void updateUpgradeIndicator() {
        hasUpgradeIndicator = true;
        std::string indicator = "Lvl: " + std::to_string(level);
        upgradeLevelText.setString(indicator);
        upgradeLevelText.setPosition(location.x + 35, location.y + 80);
    }
    
    void setUpgradeLevel(int upgradeLevel) {
        level = std::max(0, std::min(5, upgradeLevel));
        if (level > 0) {
            if (type != "BombTower") {
                int baseHealthBonus = level * GameConstants::LEVEL_HEALTH_BASE_BONUS * level * 3;
                if (type == "BarrierTower") {
                    vitality = 400 + baseHealthBonus * 4;
                    maxVitality = 400 + baseHealthBonus * 4;
                } else {
                    vitality = 200 + baseHealthBonus;
                    maxVitality = 200 + baseHealthBonus;
                }
            }
            
            if (category == "AttackTower") {
                int damageBonus = level * GameConstants::LEVEL_DAMAGE_BASE_BONUS * level;
                damage = GameConstants::BASE_TOWER_DAMAGE + damageBonus;
            } else if (type == "ResourceGenerator") {
                float abilityBoost = level * 0.2f;
                attackCooldown = static_cast<int>(std::max(4.0f, 10.0f * (1.0f - abilityBoost)));
            }
            
            cost += level * 15;
        }
        if (category == "AttackTower") {
            attackCooldown = (level == 0) ? 3 : std::max(2, 3 - level / 2);
        }
        setupUpgradeIndicator();
        updateUpgradeIndicator();
    }
    
    Tower(float x = 0, float y = 0) {
        vitality = 200;
        maxVitality = 200;
        cost = 50;
        location.x = x;
        location.y = y;
        if (!texture.loadFromFile("assets/images/plant.png")) {
            texture.create(50, 50);
        }
        setupSprite();
        attackCooldown = 1;
        type = "Tower";
        category = "Tower";
        operational = true;
        isMoving = true;
        targetX = 0;
        massFreeze = false;
        freezeApplied = false;
        targetY = 0;
        hasExploded = false;
        level = 0;
        damage = GameConstants::BASE_TOWER_DAMAGE;
        isDying = false;
        isBarrier = false;
        hasUpgradeIndicator = true;
        setupUpgradeIndicator();
        regenerationClock.restart();
    }
    
    void setupSprite(float cellSize = 100.0f) {
        texture.setSmooth(true);
        visual.setTexture(texture);
        visual.setTextureRect(sf::IntRect(0, 0, 100, 100));
        visual.setPosition(location.x, location.y);
    }
    
    void setupSpriteForGrid(int gridX, int gridY) {  
        texture.setSmooth(true);
        visual.setTexture(texture);
        visual.setTextureRect(sf::IntRect(0, 0, 100, 100));
        visual.setPosition(location.x, location.y);
    }
    
    Tower(const Tower &tower) {
        vitality = tower.vitality;
        cost = tower.cost;
        location.x = tower.location.x;
        location.y = tower.location.y;
        texture = tower.texture;
        visual = tower.visual;
        timer = tower.timer;
        attackCooldown = tower.attackCooldown;
        type = "Tower";
        category = "Tower";
        targetX = tower.targetX;
        isMoving = tower.isMoving;
        massFreeze = tower.massFreeze;
        freezeApplied = tower.freezeApplied;
        targetY = tower.targetY;
        hasExploded = tower.hasExploded;
        level = tower.level;
        damage = tower.damage;
        isDying = tower.isDying;
        isBarrier = tower.isBarrier;
        hasUpgradeIndicator = true;
        upgradeFont = tower.upgradeFont;
        upgradeLevelText = tower.upgradeLevelText;
        setupUpgradeIndicator();
    }
    
    virtual void draw(sf::RenderWindow &window) {
        if (operational) {
            window.draw(visual);
            if (level > 0) {
                std::string indicator = "Lvl: " + std::to_string(level);
                upgradeLevelText.setString(indicator);
                upgradeLevelText.setPosition(location.x + 35, location.y + 80);
                window.draw(upgradeLevelText);
            }
        }
    }
    
    
    void changePosition(float x, float y, float cellSize = 100.0f) {
        location.x = x;
        location.y = y;
        SpriteScaler::scaleToGridCell(visual, texture, location.x, location.y, 0.9f);
    }
    
    virtual void attackTarget() {}
    
    virtual void updateBullet() {};
    virtual void drawBullet(sf::RenderWindow &window) {};
    
    virtual void move() {}
    
    virtual void Serialize(std::ostream &stream) const {
        stream << type << std::endl;
        stream << category << std::endl;
        stream << location.x << std::endl;
        stream << location.y << std::endl;
        stream << attackCooldown << std::endl;
        stream << targetX << std::endl;
        stream << targetY << std::endl;
        stream << massFreeze << std::endl;
        stream << freezeApplied << std::endl;
        stream << isMoving << std::endl;
        stream << vitality << std::endl;
        stream << cost << std::endl;
        stream << damage << std::endl;
        stream << operational << std::endl;
        stream << frame << std::endl;
        stream << numframes << std::endl;
    }
    
    virtual void Deserialize(std::istream &stream) {
        stream >> location.x;
        stream >> location.y;
        visual.setPosition(location.x, location.y);
        stream >> attackCooldown;
        stream >> targetX;
        stream >> targetY;
        stream >> massFreeze;
        stream >> freezeApplied;
        stream >> isMoving;
        stream >> vitality;
        stream >> cost;
        stream >> damage;
        stream >> operational;
        stream >> frame;
        stream >> numframes;
    }
};

class AttackTower : public Tower {
public:
    BulletFactory bulletFactory;
    
    AttackTower(float x = 0, float y = 0) : Tower(x, y) {
        type = "AttackTower";
        category = "AttackTower";
        attackCooldown = (level == 0) ? 3 : std::max(2, 3 - level / 2);
    }
    
    AttackTower(const AttackTower &tower) : Tower(tower) {
        type = "AttackTower";
        category = "AttackTower";
    }
    
    virtual void Serialize(std::ostream &stream) const {
        Tower::Serialize(stream);
        bulletFactory.Serialize(stream);
    }
    
    virtual void Deserialize(std::istream &stream) {
        Tower::Deserialize(stream);
        bulletFactory.Deserialize(stream);
    }
    
    void attackTarget() {
        bulletFactory.removeNonExistantBullets();
        if (timer.getElapsedTime().asSeconds() > attackCooldown && location.x < 1150) {
            if (bulletFactory.bulletCount < 4) {
                bulletFactory.addBullet(location.x + 100, location.y + 30);
                timer.restart();
            }
        }
    }
    
    virtual void updateBullet() {
        for (int i = 0; i < bulletFactory.bulletCount; i++) {
            if (bulletFactory.bullets[i]->exist) {
                bulletFactory.bullets[i]->move();
                if (bulletFactory.bullets[i]->reachedRightEdge(static_cast<int>(UIConstants::Battlefield::PAUSE_BUTTON_X))) {
                    bulletFactory.bullets[i]->exist = false;
                }
            }
        }
    }
    
    void drawBullet(sf::RenderWindow &window) {
        bulletFactory.drawBullets(window);
    }
};

class SupportTower : public Tower {
public:
    SupportTower(float x = 0, float y = 0) : Tower(x, y) {
        type = "SupportTower";
        category = "SupportTower";
    }
    
    SupportTower(const SupportTower &tower) : Tower(tower) {
        type = "SupportTower";
        category = "SupportTower";
    }
};

class BasicShooter : public AttackTower {
public:
    sf::Clock timer1;
    
    BasicShooter() : AttackTower() {
        type = "BasicShooter";
        category = "AttackTower";
        numframes = 20;
        frame = 0;
        if (!texture.loadFromFile("assets/images/peeshoot.png")) {
            texture.create(50, 50);
        }
        setupSprite();
        attackCooldown = (level == 0) ? 3 : std::max(2, 3 - level / 2);
    }
    
    BasicShooter(float x, float y) : AttackTower(x, y) {
        damage = GameConstants::BASE_TOWER_DAMAGE + 2;
        if (!texture.loadFromFile("assets/images/peeshoot.png")) {
            texture.create(50, 50);
        }
        setupSprite();
        type = "BasicShooter";
        category = "AttackTower";
        numframes = 20;
        frame = 0;
        setupUpgradeIndicator();
        attackCooldown = (level == 0) ? 3 : std::max(2, 3 - level / 2);
    }
    
    BasicShooter(const BasicShooter &tower) : AttackTower(tower) {
        type = "BasicShooter";
        category = "AttackTower";
        timer1 = tower.timer1;
    }
    
    virtual void move() {
        if (timer1.getElapsedTime().asMilliseconds() > 20) {
            if (operational) {
                frame++;
                visual.setTextureRect(sf::IntRect(100 * (frame), 0, 100, 100));
                frame = frame % numframes;
                timer1.restart();
            }
        }
    }
};

class ResourceGenerator : public SupportTower {
public:
    SunFactory sunFactory;
    SunFactory* globalSunFactory;

    ResourceGenerator() : SupportTower() {
        type = "ResourceGenerator";
        category = "SupportTower";
        attackCooldown = 10;
        cost = 100;
        globalSunFactory = nullptr;
        texture.loadFromFile("assets/images/sunflower.png");
        setupSprite();
    }

    ResourceGenerator(float x, float y) : SupportTower(x, y) {
        cost = 100;
        attackCooldown = 10;
        globalSunFactory = nullptr;
        texture.loadFromFile("assets/images/sunflower.png");
        setupSprite();
        type = "ResourceGenerator";
        category = "SupportTower";
        setupUpgradeIndicator();
    }
    
    
    ResourceGenerator(const ResourceGenerator &tower) = delete;
    ResourceGenerator& operator=(const ResourceGenerator& tower) = delete;
    
    virtual void Serialize(std::ostream &stream) const {
        Tower::Serialize(stream);
        sunFactory.Serialize(stream);
    }
    
    virtual void Deserialize(std::istream &stream) {
        Tower::Deserialize(stream);
        sunFactory.Deserialize(stream);
    }
    
    void setGlobalSunFactory(SunFactory* factory) {
        globalSunFactory = factory;
        if (level > 1 && factory) {
            factory->setGeneratorPosition(&location);
        }
    }
    
    void generateResource() {
        float elapsed1 = timer.getElapsedTime().asSeconds();
        if (elapsed1 >= attackCooldown) {
            int count = 0;
            for (const auto& sun : sunFactory.suns) {
                if (sun->exist == true) {
                    count++;
                }
            }
            if (count < 1) {
                sunFactory.addSunFromPlant(location.x, location.y);
                timer.restart();
            }
        }
    }
    
    void updateBullet() {
        sunFactory.move();
    }
    
    void drawBullet(sf::RenderWindow &window) {
        sunFactory.draw(window);
    }
};

class RapidShooter : public AttackTower {
public:
    RapidShooter() : AttackTower() {
        type = "RapidShooter";
        category = "AttackTower";
        cost = 200;
        texture.loadFromFile("assets/images/repeater.png");
        setupSprite();
        attackCooldown = (level == 0) ? 3 : std::max(2, 3 - level / 2);
    }
    
    RapidShooter(float x, float y) : AttackTower(x, y) {
        cost = 200;
        damage = GameConstants::BASE_TOWER_DAMAGE + 4;
        texture.loadFromFile("assets/images/repeater.png");
        setupSprite();
        type = "RapidShooter";
        setupUpgradeIndicator();
        category = "AttackTower";
        attackCooldown = (level == 0) ? 3 : std::max(2, 3 - level / 2);
    }
    
    RapidShooter(const RapidShooter &tower) : AttackTower(tower) {
        type = "RapidShooter";
        category = "AttackTower";
    }
    
    void attackTarget() {
        float elapsed1 = timer.getElapsedTime().asSeconds();
        if (elapsed1 >= attackCooldown && location.x < 1150) {
            bulletFactory.removeNonExistantBullets();
            if (bulletFactory.bulletCount <= 2) {
                bulletFactory.addBullet(location.x + 90, location.y + 30, "fire");
                bulletFactory.addBullet(location.x + 130, location.y + 30, "fire");
                timer.restart();
            }
        }
    }
};

class BarrierTower : public SupportTower {
public:
    float targetX;
    
    BarrierTower() : SupportTower() {
        type = "BarrierTower";
        category = "SupportTower";
        texture.loadFromFile("assets/images/wallnut.png");
        setupSprite();
        isMoving = true;
        targetX = 0.0f;
        vitality = 400;
        maxVitality = 400;
    }
    
    BarrierTower(float x, float y) : SupportTower(x, y) {
        texture.loadFromFile("assets/images/wallnut.png");
        setupSprite();
        type = "BarrierTower";
        isMoving = true;
        targetX = 0.0f;
        vitality = 400;
        maxVitality = 400;
        setupUpgradeIndicator();
        category = "SupportTower";
    }
    
    BarrierTower(const BarrierTower &tower) : SupportTower(tower) {
        type = "BarrierTower";
        isMoving = tower.isMoving;
        targetX = tower.targetX;
        category = "SupportTower";
    }
    
    virtual void draw(sf::RenderWindow &window) {
        if (operational) {
            window.draw(visual);
            if (level > 0) {
                std::string indicator = "Lvl: " + std::to_string(level);
                upgradeLevelText.setString(indicator);
                upgradeLevelText.setPosition(location.x + 35, location.y + 80);
                window.draw(upgradeLevelText);
            }
        }
    }
    
    virtual void move() {
        float speed = 5.0f;
        visual.setOrigin(50, 50);
        if (targetX < location.x) {
            targetX += speed;
            visual.setPosition(targetX, location.y);
            visual.rotate(10);
        }
        else {
            isMoving = false;
            visual.setOrigin(0, 0);
            visual.rotate(360 - visual.getRotation());
        }
    }
};

class FreezeTower : public AttackTower {
public:
    std::unique_ptr<Bomb> bomb;
    
    FreezeTower() : AttackTower() {
        type = "FreezeTower";
        category = "AttackTower";
        cost = 100;
        texture.loadFromFile("assets/images/snowpea.png");
        setupSprite();
        targetX = static_cast<int>(location.x + 200);
        targetY = static_cast<int>(location.y);
        bomb = std::make_unique<Bomb>();
        massFreeze = false;
        freezeApplied = false;
        attackCooldown = (level == 0) ? 3 : std::max(2, 3 - level / 2);
    }
    
    FreezeTower(float x, float y) : AttackTower(x, y) {
        cost = 100;
        damage = GameConstants::BASE_TOWER_DAMAGE - 2;
        texture.loadFromFile("assets/images/snowpea.png");
        setupSprite();
        type = "FreezeTower";
        setupUpgradeIndicator();
        targetX = static_cast<int>(location.x + 200);
        targetY = static_cast<int>(location.y);
        bomb = std::make_unique<Bomb>(static_cast<float>(targetX), -50.0f);
        massFreeze = false;
        freezeApplied = false;
        category = "AttackTower";
        attackCooldown = (level == 0) ? 3 : std::max(2, 3 - level / 2);
    }
    
    FreezeTower(const FreezeTower &tower) : AttackTower(tower) {
        type = "FreezeTower";
        targetX = tower.targetX;
        targetY = tower.targetY;
        bomb = std::make_unique<Bomb>(*tower.bomb);
        massFreeze = tower.massFreeze;
        freezeApplied = tower.freezeApplied;
        category = "AttackTower";
    }
    
    virtual void Serialize(std::ostream &stream) const {
        AttackTower::Serialize(stream);
        bomb->Serialize(stream);
    }
    
    virtual void Deserialize(std::istream &stream) {
        AttackTower::Deserialize(stream);
        bomb->Deserialize(stream);
    }
    
    void attackTarget() {
        if (timer.getElapsedTime().asSeconds() > attackCooldown && location.x < 1150) {
            bulletFactory.removeNonExistantBullets();
            if (bulletFactory.bulletCount < 4) {
                bulletFactory.addBullet(location.x + 100, location.y + 30, "freeze");
                timer.restart();
            }
        }
    }
    
    void updateBullet() {
        for (int i = 0; i < bulletFactory.bulletCount; i++) {
            if (bulletFactory.bullets[i]->exist) {
                bulletFactory.bullets[i]->move();
                if (bulletFactory.bullets[i]->reachedRightEdge(static_cast<int>(UIConstants::Battlefield::PAUSE_BUTTON_X))) {
                    bulletFactory.bullets[i]->exist = false;
                }
            }
        }
    }
    
    void drawBullet(sf::RenderWindow &window) {
        bulletFactory.drawBullets(window);
    }
};

class BombTower : public SupportTower {
public:
    int radius;
    bool canKillBosses;
    
    BombTower() : SupportTower() {
        type = "BombTower";
        category = "SupportTower";
        vitality = 20;
        maxVitality = 20;
        cost = 150;
        damage = GameConstants::CHERRY_BOMB_BASE_DAMAGE;
        texture.loadFromFile("assets/images/cherrybomb.png");
        setupSprite();
        radius = GameConstants::CHERRY_BOMB_RADIUS;
        canKillBosses = false;
    }
    
    BombTower(float x, float y) {
        vitality = 20;
        maxVitality = 20;
        cost = 150;
        damage = GameConstants::CHERRY_BOMB_BASE_DAMAGE;
        location.x = x;
        location.y = y;
        setupUpgradeIndicator();
        texture.loadFromFile("assets/images/cherrybomb.png");
        texture.setSmooth(true);
        setupSprite();
        visual.setPosition(location.x, location.y); 
        type = "BombTower";
        radius = GameConstants::CHERRY_BOMB_RADIUS;
        canKillBosses = false;
        category = "SupportTower";
    }
    
    BombTower(const BombTower &tower) {
        vitality = tower.vitality;
        maxVitality = tower.maxVitality;
        cost = tower.cost;
        damage = tower.damage;
        location.x = tower.location.x;
        location.y = tower.location.y;
        texture = tower.texture;
        visual = tower.visual;
        timer = tower.timer;
        radius = tower.radius;
        canKillBosses = tower.canKillBosses;
        type = "BombTower";
        category = "SupportTower";
    }
    
    bool isExploded() {
        return timer.getElapsedTime().asSeconds() >= (5.0f - level * 0.5f);
    }
    
    void setUpgradeLevel(int upgradeLevel) {
        level = std::max(0, std::min(5, upgradeLevel));
        setupUpgradeIndicator();
        updateUpgradeIndicator();
        if (level > 0) {
            radius = GameConstants::CHERRY_BOMB_RADIUS + level * 20;
            int damageBonus = level * GameConstants::LEVEL_DAMAGE_BASE_BONUS * 2;
            damage += damageBonus;
            if (level >= 3) {
                canKillBosses = true;
            }
        }
    }
};

class AreaAttackTower : public AttackTower {
public:
    AreaAttackTower() : AttackTower() {
        type = "AreaAttackTower";
        category = "AttackTower";
        cost = 75;
        texture.loadFromFile("assets/images/fumeshroom.png");
        setupSprite();
        attackCooldown = (level == 0) ? 3 : std::max(2, 3 - level / 2);
    }
    
    AreaAttackTower(float x, float y) {
        vitality = 180;
        cost = 75;
        damage = GameConstants::BASE_TOWER_DAMAGE - 1;
        location.x = x;
        location.y = y;
        setupUpgradeIndicator();
        texture.loadFromFile("assets/images/fumeshroom.png");
        texture.setSmooth(true);
        setupSprite();
        visual.setPosition(location.x, location.y);
        type = "AreaAttackTower";
        category = "AttackTower";
        attackCooldown = (level == 0) ? 3 : std::max(2, 3 - level / 2);
    }
    
    AreaAttackTower(const AreaAttackTower &tower) {
        vitality = tower.vitality;
        cost = tower.cost;
        attackCooldown = tower.attackCooldown;
        location.x = tower.location.x;
        location.y = tower.location.y;
        texture = tower.texture;
        visual = tower.visual;
        timer = tower.timer;
        type = "AreaAttackTower";
        category = "AttackTower";
    }
    
    void attackTarget() {
        if (timer.getElapsedTime().asSeconds() > attackCooldown && location.x < 1150) {
            bulletFactory.removeNonExistantBullets();
            if (bulletFactory.bulletCount < 4) {
                bulletFactory.addBullet(location.x + 100, location.y + 30, "poison");
                timer.restart();
            }
        }
    }
};
