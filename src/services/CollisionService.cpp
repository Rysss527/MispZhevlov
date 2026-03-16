#include "services/CollisionService.h"
#include <cmath>
#include <algorithm>
#include "GameContext.h"
#include "SoundManager.h"

namespace DefenseSynth {

CollisionService::CollisionService() {
    spatialIndex.resize(GRID_ROWS);
    for (auto& row : spatialIndex) {
        row.resize(GRID_COLS);
    }
}

CollisionService::CollisionResult CollisionService::checkTowerEnemyCollision(Tower* tower, Enemy* enemy) {
    CollisionResult result;
    result.hasCollision = false;
    
    if (!tower || !enemy || !tower->operational || !enemy->enabled) {
        return result;
    }
    
    sf::FloatRect towerBounds = tower->visual.getGlobalBounds();
    sf::FloatRect enemyBounds = enemy->display.getGlobalBounds();
    
    if (towerBounds.intersects(enemyBounds)) {
        result.hasCollision = true;
        result.collisionPoint.x = (towerBounds.left + enemyBounds.left) / 2.0f;
        result.collisionPoint.y = (towerBounds.top + enemyBounds.top) / 2.0f;
        
        float overlapX = std::min(towerBounds.left + towerBounds.width, enemyBounds.left + enemyBounds.width) 
                       - std::max(towerBounds.left, enemyBounds.left);
        float overlapY = std::min(towerBounds.top + towerBounds.height, enemyBounds.top + enemyBounds.height) 
                       - std::max(towerBounds.top, enemyBounds.top);
        result.penetrationDepth = std::min(overlapX, overlapY);
        
        result.object1 = tower;
        result.object2 = enemy;
        result.type1 = "tower";
        result.type2 = "enemy";
    }
    
    return result;
}

std::vector<CollisionService::CollisionResult> CollisionService::processAllCollisions(
    TowerFactory* towers, 
    EnemyFactory* enemies,
    SunFactory* suns,
    LawnMowerFactory* lawnMowers) {
    
    std::vector<CollisionResult> results;
    
    checkZombiePlantCollisions(enemies, towers);
    checkProjectileEnemyCollisions(towers, enemies);
    
    if (lawnMowers) {
        checkLawnMowerEnemyCollisions(lawnMowers, enemies);
    }
    
    return results;
}

void CollisionService::checkZombiePlantCollisions(EnemyFactory* enemies, TowerFactory* plants) {
    if (!enemies || !plants) return;
    
    for (auto& zombie : enemies->enemies) {
        if (!zombie || !zombie->enabled) continue;
        
        int zombieRow = getRow(zombie->coords.y);
        bool zombieIsAttacking = false;
        
        for (auto& plant : plants->towers) {
            if (!plant || !plant->operational) continue;
            
            int plantRow = getRow(plant->location.y);
            if (zombieRow != plantRow) continue;
            
            sf::FloatRect zombieBounds = zombie->display.getGlobalBounds();
            sf::FloatRect plantBounds = plant->visual.getGlobalBounds();
            
            if (zombieBounds.intersects(plantBounds)) {
                zombie->canAdvance = false;
                zombieIsAttacking = true;
                
                if (zombie->behaviorClock.getElapsedTime().asSeconds() > 1.0f) {
                    plant->vitality -= 1;
                    zombie->behaviorClock.restart();
                    const char* options[] = {
                        "plastichit.wav",
                        "groan1.wav","groan2.wav","groan3.wav","groan4.wav","groan5.wav","groan6.wav",
                        "chomp.wav","chompsoft.wav","gravebusterchomp.wav",
                        "newspaper_rarrgh2.wav"
                    };
                    int idx = rand() % 11;
                    SoundManager::getInstance().playSound(options[idx]);
                    
                    if (plant->vitality <= 0) {
                        plant->operational = false;
                    }
                }
                break;
            }
        }
        
        if (!zombieIsAttacking && !zombie->canAdvance) {
            zombie->canAdvance = true;
        }
    }
}

void CollisionService::checkProjectileEnemyCollisions(TowerFactory* towers, EnemyFactory* enemies) {
    if (!towers || !enemies) return;
    for (auto& towerPtr : towers->towers) {
        if (!towerPtr || !towerPtr->operational) continue;
        if (towerPtr->category != "AttackTower") continue;
        AttackTower* attacker = static_cast<AttackTower*>(towerPtr.get());
        for (int k = 0; k < attacker->bulletFactory.bulletCount; ++k) {
            if (!attacker->bulletFactory.bullets[k] || !attacker->bulletFactory.bullets[k]->exist) continue;
            Bullet* bullet = attacker->bulletFactory.bullets[k].get();
            int bulletRow = getRow(bullet->position.y);
            for (auto& enemyPtr : enemies->enemies) {
                if (!enemyPtr || !enemyPtr->enabled) continue;
                Enemy* enemy = enemyPtr.get();
                int enemyRow = getRow(enemy->coords.y);
                if (enemyRow != bulletRow) continue;
                sf::FloatRect enemyBounds = enemy->display.getGlobalBounds();
                bool isBoss = (enemy->type.find("boss_") == 0);
                if (isBoss) {
                    enemyBounds.left -= 10;
                    enemyBounds.width += 20;
                }
                if (bullet->sprite.getGlobalBounds().intersects(enemyBounds)) {
                    bullet->exist = false;
                    float diff = DefenseSynth::GameContext::getInstance()->getDifficulty();
                    float scale = diff > 0.f ? 1.0f / diff : 1.0f;
                    int scaledDamage = static_cast<int>(std::round(bullet->damage * scale));
                    if (scaledDamage < 1) scaledDamage = 1;
                    enemy->hitpoints -= scaledDamage;
                    if (attacker->type == "FreezeTower") {
                        float t = 2.0f + (attacker->level * 1.0f);
                        if (isBoss) t *= 0.5f;
                        enemy->freeze_duration = t;
                        enemy->is_frozen = true;
                        enemy->freeze_timer.restart();
                    } else if (attacker->type == "AreaAttackTower") {
                        float t = 5.0f + (attacker->level * 1.0f);
                        if (isBoss) t *= 0.7f;
                        enemy->poison_duration = t;
                        enemy->is_poisoned = true;
                        enemy->poison_timer.restart();
                    } else if (attacker->type == "RapidShooter") {
                        float t = 3.0f + (attacker->level * 0.5f);
                        if (isBoss) t *= 0.6f;
                        enemy->fire_duration = t;
                        enemy->is_burning = true;
                        enemy->fire_timer.restart();
                    }
                    if (enemy->hitpoints <= 0) {
                        enemy->triggerDeath();
                    }
                    break;
                }
            }
        }
    }
}

void CollisionService::checkLawnMowerEnemyCollisions(LawnMowerFactory* lawnMowers, EnemyFactory* enemies) {
    if (!lawnMowers || !enemies) return;
    
    for (size_t i = 0; i < lawnMowers->lawnmowers.size(); ++i) {
        LawnMower* mower = lawnMowers->lawnmowers[i].get();
        if (!mower || !mower->exist) continue;
        
        for (auto& enemy : enemies->enemies) {
            if (!enemy || !enemy->enabled) continue;
            
            int mowerRow = getRow(mower->position.y);
            int enemyRow = getRow(enemy->coords.y);
            
            if (mowerRow != enemyRow) continue;
            
            sf::FloatRect mowerBounds = mower->sprite.getGlobalBounds();
            sf::FloatRect enemyBounds = enemy->display.getGlobalBounds();
            
            if (mowerBounds.intersects(enemyBounds)) {
                if (!mower->shouldMove) {
                    mower->shouldMove = true;
                }
                enemy->triggerDeath();
            }
        }
    }
}

bool CollisionService::checkSunClick(SunFactory* suns, float x, float y) {
    if (!suns) return false;
    
    for (const auto& sun : suns->suns) {
        if (!sun || !sun->exist) continue;
        
        sf::FloatRect sunBounds = sun->sprite.getGlobalBounds();
        if (sunBounds.contains(x, y)) {
            return true;
        }
    }
    return false;
}

int CollisionService::getRow(float y) const {
    float adjustedY = y - GRID_OFFSET_Y;
    int row = static_cast<int>(adjustedY / TILE_HEIGHT);
    return std::clamp(row, 0, GRID_ROWS - 1);
}

int CollisionService::getColumn(float x) const {
    float adjustedX = x - GRID_OFFSET_X;
    int col = static_cast<int>(adjustedX / TILE_WIDTH);
    return std::clamp(col, 0, GRID_COLS - 1);
}

bool CollisionService::isOnSameRow(float y1, float y2) const {
    return getRow(y1) == getRow(y2);
}

bool CollisionService::isValidGridPosition(float x, float y) const {
    if (x < GRID_OFFSET_X || x > GRID_OFFSET_X + GRID_COLS * TILE_WIDTH) {
        return false;
    }
    if (y < GRID_OFFSET_Y || y > GRID_OFFSET_Y + GRID_ROWS * TILE_HEIGHT) {
        return false;
    }
    return true;
}

void CollisionService::buildSpatialIndex(TowerFactory* towers, EnemyFactory* enemies) {
    clearSpatialIndex();
    
    if (towers) {
        for (auto& tower : towers->towers) {
            if (tower && tower->operational) {
                int row = getRow(tower->location.y);
                int col = getColumn(tower->location.x);
                if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS) {
                    spatialIndex[row][col].towers.push_back(tower.get());
                }
            }
        }
    }
    
    if (enemies) {
        for (auto& enemy : enemies->enemies) {
            if (enemy && enemy->enabled) {
                int row = getRow(enemy->coords.y);
                int col = getColumn(enemy->coords.x);
                if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS) {
                    spatialIndex[row][col].enemies.push_back(enemy.get());
                }
            }
        }
    }
}

void CollisionService::clearSpatialIndex() {
    for (auto& row : spatialIndex) {
        for (auto& cell : row) {
            cell.enemies.clear();
            cell.towers.clear();
            cell.suns.clear();
        }
    }
}

}