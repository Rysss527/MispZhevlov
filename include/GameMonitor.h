#pragma once
#include <SFML/Window/Cursor.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <cmath>
#include <sstream>
#include <iomanip>
#include "TowerFactory.h"
#include "EnemyFactory.h"
#include "Resource.h"
#include "StateManager.h"
#include "GameCursor.h"
#include "Store.h"
#include "DefenseUnitFactory.h"
#include "GamePlayer.h"
#include "SoundManager.h"
#include "Constants.h"

namespace DefenseSynth {

class GameInspector
{
private:
    sf::RectangleShape panelBackground;
    sf::Text titleText;
    sf::Text contentText;
    sf::Font* font;
    bool isVisible;
    float fadeAlpha;
    float targetAlpha;
    sf::Clock displayTimer;
    float autoHideDelay;
    
    enum PanelType {
        TOWER,
        ENEMY,
        WAVE,
        RESOURCE
    } currentType;
    
public:
    GameInspector() : isVisible(false), fadeAlpha(0), targetAlpha(0), 
                      autoHideDelay(5.0f), font(nullptr) {
        
        panelBackground.setSize(sf::Vector2f(250, 150));
        panelBackground.setFillColor(sf::Color(40, 40, 40, 200));
        panelBackground.setOutlineThickness(2);
        panelBackground.setOutlineColor(sf::Color(100, 100, 100, 255));
        
        titleText.setCharacterSize(18);
        titleText.setFillColor(sf::Color::White);
        titleText.setStyle(sf::Text::Bold);
        
        contentText.setCharacterSize(14);
        contentText.setFillColor(sf::Color(220, 220, 220));
    }
    
    void setFont(sf::Font* f) {
        font = f;
        if (font) {
            titleText.setFont(*font);
            contentText.setFont(*font);
        }
    }
    
    template<typename T>
    void showTowerStats(const T* tower) {
        if (!tower) return;
        
        currentType = TOWER;
        
        std::stringstream title;
        title << tower->type;
        if (tower->level > 1) {
            title << " (Lv." << tower->level << ")";
        }
        titleText.setString(title.str());
        
        std::stringstream content;
        content << "Health: " << tower->vitality << "/" << tower->maxVitality << "\n";
        content << "Damage: " << tower->damage << "\n";
        content << "Cost: " << tower->cost << "\n";
        
        if (tower->attackCooldown > 0) {
            content << "Attack Speed: " << std::fixed << std::setprecision(1) 
                   << (1.0f / tower->attackCooldown) << "/s\n";
        }
        
        if (tower->level < 5) {
            content << "Upgrade Cost: " << (tower->cost / 2) << " crystals";
        } else {
            content << "Max Level Reached";
        }
        
        contentText.setString(content.str());
        
        show();
    }
    
    template<typename T>
    void showEnemyInfo(const T* enemy) {
        if (!enemy) return;
        
        currentType = ENEMY;
        
        std::stringstream title;
        title << enemy->type;
        if (enemy->level > 1) {
            title << " (Lv." << enemy->level << ")";
        }
        titleText.setString(title.str());
        
        std::stringstream content;
        content << "Health: " << enemy->hitpoints << "\n";
        content << "Speed: " << enemy->velocity << "\n";
        content << "Damage: " << enemy->attackDamage << "\n";
        content << "Score Value: " << enemy->scoreValue << "\n";
        
        if (enemy->is_frozen) {
            content << "[FROZEN]";
        }
        if (enemy->is_poisoned) {
            content << " [POISONED]";
        }
        if (enemy->is_burning) {
            content << " [BURNING]";
        }
        
        contentText.setString(content.str());
        
        show();
    }
    
    void showWaveProgress(int current, int total, int waveNumber) {
        currentType = WAVE;
        
        titleText.setString("Wave " + std::to_string(waveNumber));
        
        std::stringstream content;
        content << "Enemies Remaining: " << (total - current) << "/" << total << "\n";
        
        float progress = static_cast<float>(current) / static_cast<float>(total);
        content << "Progress: " << std::fixed << std::setprecision(0) 
               << (progress * 100) << "%\n";
        
        int barLength = 20;
        int filled = static_cast<int>(progress * barLength);
        content << "[";
        for (int i = 0; i < barLength; ++i) {
            content << (i < filled ? "=" : "-");
        }
        content << "]";
        
        contentText.setString(content.str());
        
        show();
    }
    
    void showResourceFlow(int gained, int spent, int current) {
        currentType = RESOURCE;
        
        titleText.setString("Resource Flow");
        
        std::stringstream content;
        content << "Current: " << current << " crystals\n";
        
        if (gained > 0) {
            content << "Gained: +" << gained << "\n";
        }
        
        if (spent > 0) {
            content << "Spent: -" << spent << "\n";
        }
        
        int netFlow = gained - spent;
        content << "Net: ";
        if (netFlow > 0) {
            content << "+";
        }
        content << netFlow;
        
        contentText.setString(content.str());
        
        show();
    }
    
    void show() {
        isVisible = true;
        targetAlpha = 255;
        displayTimer.restart();
    }
    
    void hide() {
        targetAlpha = 0;
    }
    
    void setPosition(float x, float y) {
        panelBackground.setPosition(x, y);
        titleText.setPosition(x + 10, y + 5);
        contentText.setPosition(x + 10, y + 30);
    }
    
    void setAutoHide(float seconds) {
        autoHideDelay = seconds;
    }
    
    void update(float deltaTime) {
        if (autoHideDelay > 0 && displayTimer.getElapsedTime().asSeconds() > autoHideDelay) {
            hide();
        }
        
        float fadeSpeed = 500.0f;
        if (fadeAlpha < targetAlpha) {
            fadeAlpha = std::min(fadeAlpha + fadeSpeed * deltaTime, targetAlpha);
        } else if (fadeAlpha > targetAlpha) {
            fadeAlpha = std::max(fadeAlpha - fadeSpeed * deltaTime, targetAlpha);
        }
        
        if (fadeAlpha <= 0) {
            isVisible = false;
        }
        
        sf::Uint8 alpha = static_cast<sf::Uint8>(fadeAlpha);
        
        sf::Color bgColor = panelBackground.getFillColor();
        bgColor.a = static_cast<sf::Uint8>(alpha * 0.78f);
        panelBackground.setFillColor(bgColor);
        
        sf::Color outlineColor = panelBackground.getOutlineColor();
        outlineColor.a = alpha;
        panelBackground.setOutlineColor(outlineColor);
        
        sf::Color titleColor = titleText.getFillColor();
        titleColor.a = alpha;
        titleText.setFillColor(titleColor);
        
        sf::Color contentColor = contentText.getFillColor();
        contentColor.a = static_cast<sf::Uint8>(alpha * 0.86f);
        contentText.setFillColor(contentColor);
    }
    
    void draw(sf::RenderWindow& window) {
        if (!isVisible || fadeAlpha <= 0) return;
        
        window.draw(panelBackground);
        
        if (font) {
            window.draw(titleText);
            window.draw(contentText);
        }
    }
    
    bool getIsVisible() const {
        return isVisible;
    }
    
    void toggleVisibility() {
        if (isVisible) {
            hide();
        } else {
            show();
        }
    }
	void gatherResources(coordinates& clickPosition, SunFactory* sunFactory, TowerFactory* towerFactory, Player& player) {
        if (sunFactory->isSunThere(clickPosition.x, clickPosition.y))
        {
            sunFactory->moveSunToOrigin(clickPosition.x, clickPosition.y);
            player.currency +=25;
            SoundManager::getInstance().playSound("points.wav");
        }
        for (int i = 0; i < towerFactory->towers_created; i++)
        {
            if (towerFactory->towers[i]->type == "ResourceGenerator")
            {
                ResourceGenerator* resourceGen = static_cast<ResourceGenerator*>(towerFactory->towers[i].get());

                if (resourceGen->sunFactory.isSunThere(clickPosition.x, clickPosition.y))
                {
                    resourceGen->sunFactory.restartClock();
                    resourceGen->sunFactory.moveSunToOrigin(clickPosition.x, clickPosition.y);
                    player.currency += 25;
                    SoundManager::getInstance().playSound("points.wav");
                }
            }
        }
	}
    void placeTowerAtPosition(coordinates& clickPosition, TowerFactory* towerFactory, GameCursor& cursor, Player& player, const std::map<std::string, int>* upgradeLevels = nullptr) {
        if (!towerFactory->isTowerThere(clickPosition.x, clickPosition.y))
        {
            if (clickPosition.y >= 200 && clickPosition.y <= 700 && clickPosition.x >= 200 && clickPosition.x <= 1100 && cursor.getCurrentCursor() != "default")
            {
                auto getUpgradeLevel = [&](const std::string& unitType) -> int {
                    if (upgradeLevels) {
                        auto it = upgradeLevels->find(unitType);
                        return (it != upgradeLevels->end()) ? it->second : 1;
                    }
                    int stateLevel = StateManager::getInstance()->getUpgradeLevel(unitType);
                    return stateLevel;
                };
                
                if (cursor.getCurrentCursor() == "basicshooter" && player.currency >= 50)
                {
                    int level = getUpgradeLevel("basicshooter");
                    towerFactory->createTower(clickPosition.x, clickPosition.y, cursor.getCurrentCursor(), level);
                    player.currency -= 50;
                    SoundManager::getInstance().playSound("plant1.wav");
                }
                else if (cursor.getCurrentCursor() == "rapidshooter" && player.currency >= 200)
                {
                    int level = getUpgradeLevel("rapidshooter");
                    towerFactory->createTower(clickPosition.x, clickPosition.y, cursor.getCurrentCursor(), level);
                    player.currency -= 200;
                    SoundManager::getInstance().playSound("plant1.wav");
                }
                else if (cursor.getCurrentCursor() == "freezetower" && player.currency >= 100)
                {
                    int level = getUpgradeLevel("freezetower");
                    towerFactory->createTower(clickPosition.x, clickPosition.y, cursor.getCurrentCursor(), level);
                    player.currency -= 100;
                    SoundManager::getInstance().playSound("plant1.wav");
                }
                else if (cursor.getCurrentCursor() == "areaattacktower" && player.currency >= 75)
                {
                    int level = getUpgradeLevel("areaattacktower");
                    towerFactory->createTower(clickPosition.x, clickPosition.y, cursor.getCurrentCursor(), level);
                    player.currency -= 75;
                    SoundManager::getInstance().playSound("plant1.wav");
                }
                else if (cursor.getCurrentCursor() == "barriertower" && player.currency >= 50)
                {
                    int level = getUpgradeLevel("barriertower");
                    towerFactory->createTower(clickPosition.x, clickPosition.y, cursor.getCurrentCursor(), level);
                    player.currency -= 50;
                    SoundManager::getInstance().playSound("plant1.wav");
                }
                else if (cursor.getCurrentCursor() == "bombtower" && player.currency >= 150)
                {
                    int level = getUpgradeLevel("bombtower");
                    towerFactory->createTower(clickPosition.x, clickPosition.y, cursor.getCurrentCursor(), level);
                    player.currency -= 150;
                    SoundManager::getInstance().playSound("plant1.wav");
                }
                else if (cursor.getCurrentCursor() == "resourcegenerator" && player.currency >= 100)
                {
                    int level = getUpgradeLevel("resourcegenerator");
                    towerFactory->createTower(clickPosition.x, clickPosition.y, cursor.getCurrentCursor(), level);
                    player.currency -= 100;
                    SoundManager::getInstance().playSound("plant1.wav");
                }
            }
        }
        if (cursor.getCurrentCursor() == "shovel" && towerFactory->isTowerThere(clickPosition.x, clickPosition.y))
        {
            towerFactory->removeTower(clickPosition.x, clickPosition.y);
        }
    }

    

    void processCombatInteractions(TowerFactory* towerFactory, EnemyFactory* enemyFactory, Player& player){
        for (int i = 0; i < towerFactory->towers_created; i++)
        { 
            if (towerFactory->towers[i]->operational) {
                if (towerFactory->towers[i]->category == "AttackTower")
                {
                    
                }
                towerFactory->towers[i]->updateBullet();
                if (towerFactory->towers[i]->type == "BarrierTower" && towerFactory->towers[i]->isMoving)
                {
                    towerFactory->towers[i]->move();
                }
            
            if (towerFactory->towers[i]->type == "FreezeTower")
            {
                FreezeTower* freezeTower = static_cast<FreezeTower*>(towerFactory->towers[i].get());

                if (freezeTower->massFreeze && !freezeTower->freezeApplied)
                {
                    enemyFactory->freezeEnemiesInLine(static_cast<int>(freezeTower->targetX), static_cast<int>(freezeTower->targetY), 200);
                    freezeTower->freezeApplied = true;
                }
            }
            if (towerFactory->towers[i]->type == "BombTower")
            {
                BombTower* bombTower = static_cast<BombTower*>(towerFactory->towers[i].get());
                
                bool shouldExplode = false;
                int nearbyEnemyCount = 0;
                int highValueTargets = 0;
                
                for (int k = 0; k < enemyFactory->enemies_created; k++) {
                    if (enemyFactory->enemies[k]->enabled) {
                        float distance = static_cast<float>(std::sqrt(std::pow(enemyFactory->enemies[k]->coords.x - bombTower->location.x, 2.0f) + 
                                            std::pow(enemyFactory->enemies[k]->coords.y - bombTower->location.y, 2.0f)));
                        if (LaneSystem::sameLane(enemyFactory->enemies[k]->coords.y, bombTower->location.y) && distance <= bombTower->radius) {
                            nearbyEnemyCount++;
                            if (enemyFactory->enemies[k]->type.find("boss_") == 0 || enemyFactory->enemies[k]->level >= 3) {
                                highValueTargets++;
                            }
                        }
                        if (LaneSystem::sameLane(enemyFactory->enemies[k]->coords.y, bombTower->location.y) && distance <= 80 && enemyFactory->enemies[k]->hitpoints > bombTower->damage * 0.7f) {
                            shouldExplode = true;
                        }
                    }
                }
                
                if (nearbyEnemyCount >= 3 || highValueTargets >= 1) {
                    shouldExplode = true;
                }
                
                if (bombTower->timer.getElapsedTime().asSeconds() >= (4.5f - bombTower->level * 0.3f)) {
                    shouldExplode = true;
                }
                
                if (shouldExplode || bombTower->vitality <= 0) {
                    SoundManager::getInstance().playSound("cherrybomb.wav");
                    for (int k = 0; k < enemyFactory->enemies_created; k++) {
                        if (enemyFactory->enemies[k]->enabled) {
                            float distance = static_cast<float>(std::sqrt(std::pow(enemyFactory->enemies[k]->coords.x - bombTower->location.x, 2.0f) + 
                                                std::pow(enemyFactory->enemies[k]->coords.y - bombTower->location.y, 2.0f)));
                            if (LaneSystem::sameLane(enemyFactory->enemies[k]->coords.y, bombTower->location.y) && distance <= bombTower->radius) {
                                bool isBoss = (enemyFactory->enemies[k]->type.find("boss_") == 0);
                                int finalDamage = bombTower->damage;
                                if (distance <= bombTower->radius * 0.5f) {
                                    finalDamage = static_cast<int>(finalDamage * 1.5f);
                                }
                                
                                enemyFactory->enemies[k]->hitpoints -= finalDamage;
                                if (enemyFactory->enemies[k]->hitpoints <= 0 || (!isBoss && bombTower->canKillBosses)) {
                                    if (isBoss && !bombTower->canKillBosses && enemyFactory->enemies[k]->hitpoints > 0) {
                                        enemyFactory->enemies[k]->hitpoints = 1;
                                    } else {
                                        player.score += enemyFactory->enemies[k]->points;
                                        enemyFactory->enemies[k]->triggerDeath();
                                    }
                                }
                            }
                        }
                    }
                    bombTower->vitality = 0;
                    bombTower->operational = false;
                }
            }

            for (int j = 0; j < enemyFactory->enemies_created; j++)
            {
                if (towerFactory->towers[i]->operational && enemyFactory->enemies[j]->enabled) {
                if (towerFactory->towers[i]->category=="AttackTower") {
                    AttackTower* attacker = static_cast<AttackTower*>(towerFactory->towers[i].get());
                    for (int k = 0; k < attacker->bulletFactory.bulletCount; k++)
                    {

                        if (attacker->bulletFactory.bullets[k]->exist && enemyFactory->enemies[j]->enabled)
                        {
                            sf::FloatRect bulletBounds = attacker->bulletFactory.bullets[k]->sprite.getGlobalBounds();
                            sf::FloatRect enemyBounds = enemyFactory->enemies[j]->display.getGlobalBounds();

                            bool isFlying = (enemyFactory->enemies[j]->type == GameConstants::FLYING_ZOMBIE || enemyFactory->enemies[j]->type == "boss_flying");
                            bool canTargetFlying = (attacker->type == "AreaAttackTower" || attacker->type == "RapidShooter");
                            bool canHit = false;
                            
                            if (isFlying) {
                                canHit = canTargetFlying && bulletBounds.intersects(enemyBounds);
                            } else {
                                canHit = LaneSystem::sameLane(enemyFactory->enemies[j]->coords.y, attacker->location.y) && bulletBounds.intersects(enemyBounds);
                            }
                            
                            if (canHit)
                            {
                                int bulletDamage = attacker->bulletFactory.bullets[k]->damage;
                                if (attacker->level > 1) {
                                    int damageBonus = static_cast<int>((attacker->level - 1) * 4 * std::pow(GameConstants::LEVEL_DAMAGE_SCALING, static_cast<float>(attacker->level - 2)));
                                    bulletDamage += damageBonus;
                                }
                                
                                if (attacker->type == "FreezeTower")
                                {
                                    bool isBoss = (enemyFactory->enemies[j]->type.find("boss") != std::string::npos);
                                    int freezeChance = isBoss ? 30 : 50;
                                    if (rand() % 100 < freezeChance) {
                                        float freezeTime = 3.5f + (attacker->level * 1.2f);
                                        if (isBoss) {
                                            freezeTime = freezeTime * 0.4f;
                                        }
                                        enemyFactory->enemies[j]->freeze_duration = freezeTime;
                                        enemyFactory->enemies[j]->is_frozen = true;
                                        enemyFactory->enemies[j]->freeze_timer.restart();
                                    }
                                    enemyFactory->enemies[j]->hitpoints -= bulletDamage;
                                }
                                else if (attacker->type == "AreaAttackTower")
                                {
                                    float poisonTime = 8.0f + (attacker->level * 1.5f);
                                    bool isBoss = (enemyFactory->enemies[j]->type.find("boss") != std::string::npos);
                                    if (isBoss) {
                                        poisonTime = poisonTime * 0.6f;
                                    }
                                    enemyFactory->enemies[j]->poison_duration = poisonTime;
                                    enemyFactory->enemies[j]->is_poisoned = true;
                                    enemyFactory->enemies[j]->poison_timer.restart();
                                    enemyFactory->enemies[j]->hitpoints -= bulletDamage;
                                }
                                else if (attacker->type == "RapidShooter")
                                {
                                    float fireTime = 5.0f + (attacker->level * 0.8f);
                                    bool isBoss = (enemyFactory->enemies[j]->type.find("boss") != std::string::npos);
                                    if (isBoss) {
                                        fireTime = fireTime * 0.5f;
                                    }
                                    enemyFactory->enemies[j]->fire_duration = fireTime;
                                    enemyFactory->enemies[j]->is_burning = true;
                                    enemyFactory->enemies[j]->fire_timer.restart();
                                    enemyFactory->enemies[j]->hitpoints -= bulletDamage;
                                }
                                else
                                {
                                    enemyFactory->enemies[j]->hitpoints -= bulletDamage;
                                }
                                
                                if (enemyFactory->enemies[j]->hitpoints <= 0)
                                {
                                    enemyFactory->enemies[j]->triggerDeath();
                                    player.score += enemyFactory->enemies[j]->points;
                                    
                                }
                                attacker->bulletFactory.bullets[k]->exist = false;
                            }
                        }
                    }
                }
                sf::FloatRect towerBounds = towerFactory->towers[i]->visual.getGlobalBounds();
                sf::FloatRect enemyBounds = enemyFactory->enemies[j]->display.getGlobalBounds();
                if (towerBounds.intersects(enemyBounds) && LaneSystem::sameLane(enemyFactory->enemies[j]->coords.y, towerFactory->towers[i]->location.y))
                {
                    if (enemyFactory->enemies[j]->type.find("flying") == std::string::npos)
                    {
                        if (enemyFactory->enemies[j]->canAdvance) {
                            enemyFactory->enemies[j]->clock2.restart();
                        }
                        enemyFactory->enemies[j]->canAdvance = false;
                        
                        if (enemyFactory->enemies[j]->clock2.getElapsedTime().asSeconds() >= 1.5)
                        {
                            int baseDamage = GameConstants::BASE_CITIZEN_DAMAGE;
                            if (enemyFactory->enemies[j]->type == "armored" || enemyFactory->enemies[j]->type == "boss_armored") {
                                baseDamage = static_cast<int>(GameConstants::BASE_CITIZEN_DAMAGE * 1.8f);
                            } else if (enemyFactory->enemies[j]->type == "flying" || enemyFactory->enemies[j]->type == "boss_flying") {
                                baseDamage = static_cast<int>(GameConstants::BASE_CITIZEN_DAMAGE * 1.3f);
                            } else if (enemyFactory->enemies[j]->type == "dancing" || enemyFactory->enemies[j]->type == "boss_dancing") {
                                baseDamage = static_cast<int>(GameConstants::BASE_CITIZEN_DAMAGE * 1.5f);
                            } else if (enemyFactory->enemies[j]->type == "boss_basic") {
                                baseDamage = static_cast<int>(GameConstants::BASE_CITIZEN_DAMAGE * 1.4f);
                            }
                            
                            if (enemyFactory->enemies[j]->type.find("boss_") == 0) {
                                baseDamage = static_cast<int>(baseDamage * GameConstants::BOSS_DAMAGE_MULTIPLIER);
                            }
                            
                            int levelBonus = static_cast<int>((enemyFactory->enemies[j]->level - 1) * 8 * std::pow(GameConstants::LEVEL_DAMAGE_SCALING, static_cast<float>(enemyFactory->enemies[j]->level - 2)));
                            int finalDamage = baseDamage + levelBonus;
                            towerFactory->towers[i]->vitality -= finalDamage;
                            
                            if (towerFactory->towers[i]->type != "BombTower") {
                                towerFactory->towers[i]->regenerationClock.restart();
                            }
                            
                            enemyFactory->enemies[j]->clock2.restart();
                            
                            if (towerFactory->towers[i]->vitality <= 0) {
                                if (towerFactory->towers[i]->type == "BombTower") {
                                    BombTower* bombTower = static_cast<BombTower*>(towerFactory->towers[i].get());
                                    SoundManager::getInstance().playSound("cherrybomb.wav");
                                    for (int k = 0; k < enemyFactory->enemies_created; k++) {
                                        if (enemyFactory->enemies[k]->enabled) {
                                            float distance = static_cast<float>(std::sqrt(std::pow(enemyFactory->enemies[k]->coords.x - bombTower->location.x, 2.0f) + 
                                                                std::pow(enemyFactory->enemies[k]->coords.y - bombTower->location.y, 2.0f)));
                                            if (LaneSystem::sameLane(enemyFactory->enemies[k]->coords.y, bombTower->location.y) && distance <= bombTower->radius) {
                                                bool isBoss = (enemyFactory->enemies[k]->type.find("boss_") == 0);
                                                int explosionDamage = bombTower->damage;
                                                if (distance <= bombTower->radius * 0.5f) {
                                                    explosionDamage = static_cast<int>(explosionDamage * 1.5f);
                                                }
                                                enemyFactory->enemies[k]->hitpoints -= explosionDamage;
                                                if (enemyFactory->enemies[k]->hitpoints <= 0 || (!isBoss && bombTower->canKillBosses)) {
                                                    if (isBoss && !bombTower->canKillBosses && enemyFactory->enemies[k]->hitpoints > 0) {
                                                        enemyFactory->enemies[k]->hitpoints = 1;
                                                    } else {
                                                        player.score += enemyFactory->enemies[k]->points;
                                                        enemyFactory->enemies[k]->triggerDeath();
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                towerFactory->towers[i]->operational = false;
                            }
                        }
                    }
                }
                }

            }
            }
        }
    }

    void checkBoundaryBreach(EnemyFactory* enemyFactory, Player& player, int& cityPopulation, LawnMowerFactory* lawnMowerFactory = nullptr) {
        for (auto& enemy : enemyFactory->enemies)
        {
            if (enemy && enemy->enabled && enemy->reachedEnd)
            {
                bool canAttackCitizens = true;
                if (lawnMowerFactory) {
                    int enemyLane = LaneSystem::getLane(enemy->coords.y);
                    for (const auto& mower : lawnMowerFactory->lawnmowers) {
                        if (mower && mower->exist) {
                            int mowerLane = LaneSystem::getLane(mower->position.y);
                            if (mowerLane == enemyLane) {
                                canAttackCitizens = false;
                                break;
                            }
                        }
                    }
                }
                
                if (canAttackCitizens) {
                    auto* context = DefenseSynth::GameContext::getInstance();
                    float difficulty = context->getDifficulty();
                    
                    int baseCitizenDamage = GameConstants::BASE_CITIZEN_DAMAGE;
                    float difficultyMultiplier = 1.0f + (difficulty - 1.0f) * 0.8f;
                    baseCitizenDamage = static_cast<int>(baseCitizenDamage * difficultyMultiplier);
                    
                    int citizenDamage = baseCitizenDamage;
                    if (enemy->type.find("boss_") == 0) {
                        citizenDamage = static_cast<int>(citizenDamage * (3.0f + difficulty * 0.5f));
                        player.lives -= 2;
                    } else if (enemy->type == "armored") {
                        citizenDamage = static_cast<int>(citizenDamage * (1.8f + difficulty * 0.2f));
                        player.lives -= 1;
                    } else if (enemy->type == "dancing") {
                        citizenDamage = static_cast<int>(citizenDamage * (1.5f + difficulty * 0.15f));
                        player.lives -= 1;
                    } else if (enemy->type == "flying" || enemy->type == "boss_flying") {
                        citizenDamage = static_cast<int>(citizenDamage * (1.3f + difficulty * 0.1f));
                        if (enemy->type == "boss_flying") {
                            player.lives -= 2;
                        } else {
                            player.lives -= 1;
                        }
                    } else {
                        player.lives -= 1;
                    }
                    
                    int levelBonus = static_cast<int>(enemy->level * 4 * (1.0f + difficulty * 0.3f));
                    citizenDamage += levelBonus;
                    
                    cityPopulation -= citizenDamage;
                    SoundManager::getInstance().playSound("scream.wav");
                }
                enemy->triggerDeath();
            }
        }
    }
};

}
