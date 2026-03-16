#include "CollisionManager.h"
#include "Tower.h"
#include "GamePlayer.h"
#include "DefenseUnitFactory.h"
#include "SafeOperations.h"
#include <algorithm>
#include <vector>
#include "TowerFactory.h"
#include <cmath>
#include "Constants.h"
#include "GameContext.h"

void QuadTreeNode::insert(Enemy* enemy) {
    sf::FloatRect enemyBounds(enemy->coords.x, enemy->coords.y, 50, 50);
    
    if (!bounds.intersects(enemyBounds)) {
        return;
    }
    
    if (isLeafNode()) {
        enemies.push_back(enemy);
        
        if (enemies.size() > MAX_ENEMIES && depth < MAX_DEPTH) {
            subdivide();
            auto tempEnemies = enemies;
            enemies.clear();
            
            for (Enemy* e : tempEnemies) {
                for (auto& child : children) {
                    child->insert(e);
                }
            }
        }
    } else {
        for (auto& child : children) {
            child->insert(enemy);
        }
    }
}

void QuadTreeNode::subdivide() {
    float halfWidth = bounds.width / 2.0f;
    float halfHeight = bounds.height / 2.0f;
    float x = bounds.left;
    float y = bounds.top;
    children.resize(4);
    
    children[0] = std::make_unique<QuadTreeNode>(
        sf::FloatRect(x, y, halfWidth, halfHeight), depth + 1);
    children[1] = std::make_unique<QuadTreeNode>(
        sf::FloatRect(x + halfWidth, y, halfWidth, halfHeight), depth + 1);
    children[2] = std::make_unique<QuadTreeNode>(
        sf::FloatRect(x, y + halfHeight, halfWidth, halfHeight), depth + 1);
    children[3] = std::make_unique<QuadTreeNode>(
        sf::FloatRect(x + halfWidth, y + halfHeight, halfWidth, halfHeight), depth + 1);
}

std::vector<Enemy*> QuadTreeNode::query(const sf::FloatRect& range) {
    std::vector<Enemy*> result;
    
    if (!bounds.intersects(range)) {
        return result;
    }
    
    for (Enemy* enemy : enemies) {
        sf::FloatRect enemyBounds(enemy->coords.x, enemy->coords.y, 50, 50);
        if (range.intersects(enemyBounds)) {
            result.push_back(enemy);
        }
    }
    
    for (auto& child : children) {
        auto childResult = child->query(range);
        result.insert(result.end(), childResult.begin(), childResult.end());
    }
    
    return result;
}

void CollisionManager::checkCollisions(TowerFactory* towerFactory, EnemyFactory* enemyFactory, Player& player)
{
    if (!towerFactory || !enemyFactory) {
        throw std::invalid_argument("Null factory pointer passed to checkCollisions");
    }
    
    if (enemyFactory->enemies_created < 0 || enemyFactory->enemies_created > 10000) {
        throw std::invalid_argument("Invalid enemy count: " + std::to_string(enemyFactory->enemies_created));
    }
    
    std::vector<std::vector<Enemy*>> zombieGrid(GameConstants::BATTLEFIELD_ROWS);
    std::vector<bool> zombieColliding(SafeOperations::clamp<size_t>(enemyFactory->enemies_created, 0, 10000), false);
    
    for (int i = 0; i < enemyFactory->enemies_created; ++i)
    {
        if (SafeOperations::isValidArrayIndex(i, 10000) && enemyFactory->enemies[i] && enemyFactory->enemies[i]->enabled)
        {
            Enemy* e = enemyFactory->enemies[i].get();
            int lane = LaneSystem::getLane(e->coords.y);
            if (lane >= 0 && lane < GameConstants::BATTLEFIELD_ROWS) {
                zombieGrid[lane].push_back(e);
            }
        }
    }

    for (int i = 0; i < towerFactory->towers_created; ++i)
    {
        if (!towerFactory->towers[i]->operational) continue;

        Tower* tower = towerFactory->towers[i].get();
        int towerRow = LaneSystem::getLane(tower->location.y);

        if (towerRow < 0 || towerRow >= GameConstants::BATTLEFIELD_ROWS) continue;

        for (Enemy* enemy : zombieGrid[towerRow])
        {
            if (!enemy->enabled) continue;
            
            if (!LaneSystem::sameLane(enemy->coords.y, tower->location.y)) continue;
            
            sf::FloatRect towerBounds = tower->visual.getGlobalBounds();
            sf::FloatRect enemyBounds = enemy->display.getGlobalBounds();
            
            float meleeVMargin = 24.0f;
            float meleeHMargin = 12.0f;
            enemyBounds.top -= meleeVMargin;
            enemyBounds.height += meleeVMargin * 2.0f;
            enemyBounds.left -= meleeHMargin;
            enemyBounds.width += meleeHMargin * 2.0f;
            if (towerBounds.intersects(enemyBounds) && enemy->type.find("flying") == std::string::npos)
            {
                if (enemy->canAdvance) {
                    enemy->clock2.restart();
                }
                enemy->canAdvance = false;

                for (int z = 0; z < enemyFactory->enemies_created; ++z) {
                    if (enemyFactory->enemies[z].get() == enemy) {
                        zombieColliding[z] = true;
                        break;
                    }
                }

                float contactX = towerBounds.left + towerBounds.width - 1.0f;
                if (enemy->coords.x < contactX) {
                    enemy->coords.x = contactX;
                    enemy->display.setPosition(enemy->coords.x, enemy->coords.y);
                }

                if (enemy->clock2.getElapsedTime().asSeconds() >= 0.4f)
                {
                    int baseDamage = 15;
                    if (enemy->type == "armored" || enemy->type == "boss_armored") {
                        baseDamage = 30;
                    } else if (enemy->type == "flying" || enemy->type == "boss_flying") {
                        baseDamage = 20;
                    } else if (enemy->type == "dancing" || enemy->type == "boss_dancing") {
                        baseDamage = 25;
                    } else if (enemy->type == "boss_basic") {
                        baseDamage = 25;
                    }
                    
                    baseDamage = SafeOperations::validateDamage(baseDamage);
                    
                    if (enemy->type.find("boss_") == 0) {
                        baseDamage = SafeOperations::validateDamage(static_cast<int>(baseDamage * 1.5f));
                    }
                    
                    int validLevel = SafeOperations::validateLevel(enemy->level);
                    int levelBonus = SafeOperations::safeMultiply(validLevel - 1, 5);
                    int finalDamage = SafeOperations::safeAdd(baseDamage, levelBonus);
                    finalDamage = SafeOperations::validateDamage(finalDamage);
                    
                    tower->vitality = SafeOperations::validateHealth(tower->vitality - finalDamage);
                    
                    if (tower->type != "BombTower") {
                        tower->regenerationClock.restart();
                    }
                    
                    enemy->clock2.restart();
                    const char* options_melee[] = {
                        "plastichit.wav",
                        "groan1.wav","groan2.wav","groan3.wav","groan4.wav","groan5.wav","groan6.wav",
                        "chomp.wav","chompsoft.wav","gravebusterchomp.wav",
                        "newspaper_rarrgh2.wav"
                    };
                    int idx_m = rand() % 11;
                    SoundManager::getInstance().playSound(options_melee[idx_m]);
                    
                    if (effectManager) {
                        sf::Vector2f bitePos(tower->location.x, tower->location.y);
                        effectManager->addHitImpact(bitePos);
                        effectManager->addSparks(bitePos, 10);
                        bool bossBiter = (enemy->type.find("boss_") == 0);
                    }
                    
                    if (tower->vitality <= 0)
                    {
                        if (tower->type == "BombTower") {
                            BombTower* bombTower = static_cast<BombTower*>(tower);
                            SoundManager::getInstance().playSound("cherrybomb.wav");
                            
                            if (effectManager) {
                                float visualRadius = static_cast<float>(bombTower->radius) * 1.5f;
                                effectManager->addExplosion(sf::Vector2f(bombTower->location.x, bombTower->location.y), visualRadius);
                            }
                            for (int ei = 0; ei < enemyFactory->enemies_created; ++ei) {
                                if (!enemyFactory->enemies[ei] || !enemyFactory->enemies[ei]->enabled) continue;
                                Enemy* e = enemyFactory->enemies[ei].get();
                                float distance = static_cast<float>(sqrt(pow(e->coords.x - bombTower->location.x, 2) +
                                                     pow(e->coords.y - bombTower->location.y, 2)));
                                if (distance <= bombTower->radius) {
                                    bool isBoss = (e->type.find("boss_") == 0);
                                    float diff = DefenseSynth::GameContext::getInstance()->getDifficulty();
                                    float scale = diff > 0.f ? 1.0f / diff : 1.0f;
                                    int explosionDamage = static_cast<int>(std::round(bombTower->damage * scale));
                                    if (distance <= bombTower->radius * 0.5f) {
                                        explosionDamage = static_cast<int>(std::round(explosionDamage * 1.5f));
                                    }
                                    if (explosionDamage < 1) explosionDamage = 1;
                                    e->hitpoints -= explosionDamage;
                                    if (e->hitpoints <= 0 || (!isBoss && bombTower->canKillBosses)) {
                                        if (isBoss && !bombTower->canKillBosses && e->hitpoints > 0) {
                                            e->hitpoints = 1;
                                        } else {
                                            player.score += e->scoreValue;
                                            e->triggerDeath();
                                        }
                                    }
                                }
                            }
                        } else {
                            SoundManager::getInstance().playSound("plant2.wav");
                        }
                        tower->operational = false;
                        if (effectManager) {
                            effectManager->addPlantDeath(sf::Vector2f(tower->location.x, tower->location.y));
                        }
                        
                        for (Enemy* zombieNearby : zombieGrid[towerRow]) {
                            if (zombieNearby->enabled) {
                                sf::FloatRect zombieBounds = zombieNearby->display.getGlobalBounds();
                                zombieBounds.top -= meleeVMargin;
                                zombieBounds.height += meleeVMargin * 2.0f;
                                zombieBounds.left -= meleeHMargin;
                                zombieBounds.width += meleeHMargin * 2.0f;
                                if (towerBounds.intersects(zombieBounds)) {
                                    zombieNearby->canAdvance = true;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (tower->category == "AttackTower")
        {
            AttackTower* attacker = static_cast<AttackTower*>(tower);
            bool zombieInLane = false;
            for (Enemy* enemy : zombieGrid[towerRow])
            {
                if (!enemy || !enemy->enabled) continue;
                if (enemy->coords.x > tower->location.x && enemy->coords.x <= UIConstants::Battlefield::PAUSE_BUTTON_X)
                {
                    zombieInLane = true;
                    break;
                }
            }

            if (!zombieInLane)
            {
                float cellLeft = tower->location.x;
                float cellRight = cellLeft + GameConstants::TILE_SIZE;
                float margin = 20.0f;
                for (Enemy* enemy : zombieGrid[towerRow])
                {
                    if (!enemy || !enemy->enabled) continue;
                    float ex = enemy->coords.x;
                    if (ex >= (cellLeft - margin) && ex <= (cellRight + margin))
                    {
                        zombieInLane = true;
                        break;
                    }
                }
            }

            if (zombieInLane)
            {
                attacker->attackTarget();
            }

            attacker->updateBullet();

            for (int k = 0; k < attacker->bulletFactory.bulletCount; ++k)
            {
                if (!attacker->bulletFactory.bullets[k]->exist) continue;

                Bullet* bullet = attacker->bulletFactory.bullets[k].get();
                
                for (int ei = 0; ei < enemyFactory->enemies_created; ++ei)
                {
                    if (!enemyFactory->enemies[ei] || !enemyFactory->enemies[ei]->enabled) continue;
                    Enemy* enemy = enemyFactory->enemies[ei].get();
                    
                    bool isFlying = (enemy->type == GameConstants::FLYING_ZOMBIE || enemy->type == "boss_flying");
                    bool canTargetFlying = (attacker->type == "AreaAttackTower" || attacker->type == "RapidShooter");
                    bool canHit = false;
                    
                    if (isFlying) {
                        canHit = canTargetFlying;
                    } else {
                        canHit = LaneSystem::sameLane(enemy->coords.y, tower->location.y);
                    }
                    
                    if (!canHit) continue;
                    
                    sf::FloatRect enemyBounds = enemy->display.getGlobalBounds();
                    bool isBoss = (enemy->type.find("boss_") == 0);
                    if (isBoss) {
                        enemyBounds.left -= 10;
                        enemyBounds.width += 20;
                    }
                    float vMargin = GameConstants::TILE_SIZE * 0.5f;
                    float hMargin = 12.0f;
                    enemyBounds.top -= vMargin;
                    enemyBounds.height += vMargin * 2.0f;
                    enemyBounds.left -= hMargin;
                    enemyBounds.width += hMargin * 2.0f;
                    
                    if (bullet->sprite.getGlobalBounds().intersects(enemyBounds))
                    {
                        bullet->exist = false;
                        float diff = DefenseSynth::GameContext::getInstance()->getDifficulty();
                        float scale = diff > 0.f ? 1.0f / diff : 1.0f;
                        int scaledDamage = static_cast<int>(std::round(bullet->damage * scale));
                        if (scaledDamage < 1) scaledDamage = 1;
                        enemy->hitpoints -= scaledDamage;
                        SoundManager::getInstance().playSound("splat1.wav");
                        if (effectManager) {
                            effectManager->addHitImpact(sf::Vector2f(enemy->coords.x, enemy->coords.y));
                            effectManager->addSparks(sf::Vector2f(enemy->coords.x, enemy->coords.y), 6);
                        }

                        if (attacker->type == "FreezeTower")
                        {
                            float freezeTime = 2.0f + (attacker->level * 1.0f);
                            bool isBoss = (enemy->type.find("boss_") == 0);
                            if (isBoss) {
                                freezeTime = freezeTime * 0.5f;
                            }
                            
                            if (effectManager) {
                                effectManager->addFreezeEffect(sf::Vector2f(enemy->coords.x, enemy->coords.y));
                                effectManager->addIceCrystalsEffect(sf::Vector2f(enemy->coords.x, enemy->coords.y));
                            }
                            enemy->freeze_duration = freezeTime;
                            enemy->is_frozen = true;
                            enemy->freeze_timer.restart();
                            SoundManager::getInstance().playSound("frozen.wav");
                        }
                        else if (attacker->type == "AreaAttackTower")
                        {
                            float poisonTime = 5.0f + (attacker->level * 1.0f);
                            bool isBoss = (enemy->type.find("boss_") == 0);
                            if (isBoss) {
                                poisonTime = poisonTime * 0.7f;
                            }
                            enemy->poison_duration = poisonTime;
                            enemy->is_poisoned = true;
                            enemy->poison_timer.restart();
                            if (effectManager) {
                                effectManager->addPoisonVaporEffect(sf::Vector2f(enemy->coords.x, enemy->coords.y));
                            }
                            SoundManager::getInstance().playSound("puff.wav");
                        }
                        else if (attacker->type == "RapidShooter")
                        {
                            float fireTime = 3.0f + (attacker->level * 0.5f);
                            bool isBoss = (enemy->type.find("boss_") == 0);
                            if (isBoss) {
                                fireTime = fireTime * 0.6f;
                            }
                            enemy->fire_duration = fireTime;
                            enemy->is_burning = true;
                            enemy->fire_timer.restart();
                            if (effectManager) {
                                effectManager->addBurningZombieEffect(sf::Vector2f(enemy->coords.x, enemy->coords.y));
                            }
                        }

                        if (enemy->hitpoints <= 0)
                        {
                            enemy->triggerDeath();
                            player.score += enemy->scoreValue;
                            if (enemy->type.find("boss_") == 0) {
                                SoundManager::getInstance().playSound("groan6.wav");
                            } else {
                                SoundManager::getInstance().playSound("splat2.wav");
                            }
                        }
                        break;
                    }
                }
            }
        }
        else if (tower->type == "BombTower")
        {
            BombTower* bomb = static_cast<BombTower*>(tower);
            bool shouldExplode = false;
            
            for (int ei = 0; ei < enemyFactory->enemies_created; ++ei) {
                if (!enemyFactory->enemies[ei]) continue;
                Enemy* enemy = enemyFactory->enemies[ei].get();
                bool isBoss = (enemy->type.find("boss_") == 0);
                if (!enemy->enabled && !isBoss) continue;
                float dx = bomb->location.x - enemy->coords.x;
                float dy = bomb->location.y - enemy->coords.y;
                float distance = std::sqrt(dx*dx + dy*dy);
                float triggerRadius = GameConstants::TILE_SIZE * 1.6f;
                if (distance < triggerRadius) {
                    shouldExplode = true;
                    break;
                }
            }
            
            if (shouldExplode)
            {
                SoundManager::getInstance().playSound("cherrybomb.wav");
                
                float explodeRadius = static_cast<float>(bomb->radius);
                
                if (effectManager) {
                    effectManager->addExplosion(sf::Vector2f(bomb->location.x, bomb->location.y), explodeRadius * 1.5f);
                }
                
                for (int ei = 0; ei < enemyFactory->enemies_created; ++ei) {
                    if (!enemyFactory->enemies[ei]) continue;
                    Enemy* enemy = enemyFactory->enemies[ei].get();
                    bool isBoss = (enemy->type.find("boss_") == 0);
                    if (!enemy->enabled && !isBoss) continue;
                    if (!LaneSystem::sameLane(enemy->coords.y, tower->location.y)) continue;
                    float dx = bomb->location.x - enemy->coords.x;
                    float dy = bomb->location.y - enemy->coords.y;
                    float distance = std::sqrt(dx*dx + dy*dy);
                    if (distance <= explodeRadius)
                    {
                        float diff = DefenseSynth::GameContext::getInstance()->getDifficulty();
                        float scale = diff > 0.f ? 1.0f / diff : 1.0f;
                        int explosionDamage = static_cast<int>(std::round(bomb->damage * 3 * scale));
                        if (explosionDamage < 1) explosionDamage = 1;
                        enemy->hitpoints -= explosionDamage;
                        if (enemy->hitpoints <= 0)
                        {
                            enemy->triggerDeath();
                            player.score += enemy->scoreValue;
                            if (enemy->type.find("boss_") == 0) {
                                SoundManager::getInstance().playSound("groan6.wav");
                            } else {
                                SoundManager::getInstance().playSound("splat1.wav");
                            }
                            if (effectManager) {
                                effectManager->addExplosion(sf::Vector2f(enemy->coords.x, enemy->coords.y), explodeRadius);
                                effectManager->addDisintegrationEffect(sf::Vector2f(enemy->coords.x, enemy->coords.y));
                            }
                        }
                    }
                }
                towerFactory->removeTower(bomb->location.x, bomb->location.y);
                continue;
            }
        }
    }
    for (int i = 0; i < enemyFactory->enemies_created; ++i)
    {
        if (!SafeOperations::isValidArrayIndex(i, 10000)) continue;
        if (!enemyFactory->enemies[i]) continue;
        if (!enemyFactory->enemies[i]->enabled) continue;
        enemyFactory->enemies[i]->canAdvance = !zombieColliding[i];
    }

    for (int i = 0; i < enemyFactory->enemies_created; ++i)
    {
        if (!enemyFactory->enemies[i]->enabled) continue;
        Enemy* enemy = enemyFactory->enemies[i].get();
        
        if (enemy->hasShotActive && enemy->type != "basic_zombie" && enemy->type != "basic_zombie1")
        {
            for (int j = 0; j < towerFactory->towers_created; ++j)
            {
                if (!towerFactory->towers[j]->operational) continue;
                Tower* tower = towerFactory->towers[j].get();
                int enemyRow = LaneSystem::getLane(enemy->coords.y);
                int towerRow2 = LaneSystem::getLane(tower->location.y);
                if (enemyRow != towerRow2) continue;
                
                if (enemy->shotSprite.getGlobalBounds().intersects(tower->visual.getGlobalBounds()))
                {
                    bool bossShot = (enemy->type.find("boss_") == 0);
                    if (effectManager) {
                        sf::Vector2f impactPos = enemy->shotSprite.getPosition();
                        effectManager->addHitImpact(impactPos);
                        effectManager->addSparks(sf::Vector2f(tower->location.x, tower->location.y), 12);
                    }
                    int baseDamage = 100;
                    if (enemy->type == "armored" || enemy->type == "boss_armored") {
                        baseDamage = 150;
                    } else if (enemy->type == "flying" || enemy->type == "boss_flying") {
                        baseDamage = 120;
                    } else if (enemy->type == "dancing" || enemy->type == "boss_dancing") {
                        baseDamage = 130;
                    } else if (enemy->type == "boss_basic") {
                        baseDamage = 140;
                    }
                    
                    if (enemy->type.find("boss_") == 0) {
                        baseDamage = static_cast<int>(baseDamage * GameConstants::BOSS_DAMAGE_MULTIPLIER);
                    }
                    
                    int levelBonus = (enemy->level - 1) * 30;
                    int finalDamage = baseDamage + levelBonus;

                    bool isBossShooter = (enemy->type.find("boss_") == 0);
                    if (isBossShooter) {
                        int capByMax = static_cast<int>(std::round(tower->maxVitality * 0.45f));
                        if (capByMax < 1) capByMax = 1;
                        if (finalDamage > capByMax) finalDamage = capByMax;
                    }

                    if (true) {
                        tower->vitality -= finalDamage;
                        if (tower->type != "BombTower") {
                            tower->regenerationClock.restart();
                        }
                        const char* options[] = {"groan1.wav","groan2.wav","groan3.wav","groan4.wav","groan5.wav","groan6.wav","squash_hmm.wav","splat1.wav","splat2.wav","splat3.wav","wakeup.wav"};
                        int idx = rand() % 11;
                        SoundManager::getInstance().playSound(options[idx]);
                    }

                    enemy->hasShotActive = false;
                    
                    if (tower->vitality <= 0)
                    {
                        if (tower->type == "BombTower") {
                            BombTower* bombTower = static_cast<BombTower*>(tower);
                            for (int ei = 0; ei < enemyFactory->enemies_created; ++ei) {
                                if (!enemyFactory->enemies[ei] || !enemyFactory->enemies[ei]->enabled) continue;
                                Enemy* e = enemyFactory->enemies[ei].get();
                                float distance = static_cast<float>(sqrt(pow(e->coords.x - bombTower->location.x, 2) +
                                                     pow(e->coords.y - bombTower->location.y, 2)));
                                if (distance <= bombTower->radius) {
                                    bool isBoss = (e->type.find("boss_") == 0);
                                    float diff = DefenseSynth::GameContext::getInstance()->getDifficulty();
                                    float scale = diff > 0.f ? 1.0f / diff : 1.0f;
                                    int explosionDamage = static_cast<int>(std::round(bombTower->damage * scale));
                                    if (distance <= bombTower->radius * 0.5f) {
                                        explosionDamage = static_cast<int>(std::round(explosionDamage * 1.5f));
                                    }
                                    if (explosionDamage < 1) explosionDamage = 1;
                                    e->hitpoints -= explosionDamage;
                                    if (e->hitpoints <= 0 || (!isBoss && bombTower->canKillBosses)) {
                                        if (isBoss && !bombTower->canKillBosses && e->hitpoints > 0) {
                                            e->hitpoints = 1;
                                        } else {
                                            player.score += e->scoreValue;
                                            e->triggerDeath();
                                        }
                                    }
                                }
                            }
                        }
                        tower->operational = false;
                        if (effectManager) {
                            effectManager->addPlantDeath(sf::Vector2f(tower->location.x, tower->location.y));
                        }
                    }
                    break;
                }
            }
        }
    }
    
}

void CollisionManager::checkLawnmowerCollisions(LawnMowerFactory* lawnMowerFactory, EnemyFactory* enemyFactory, Player& player)
{
    for (int i = 0; i < static_cast<int>(lawnMowerFactory->lawnmowers.size()); ++i)
    {
        if (lawnMowerFactory->lawnmowers[i]->exist)
        {
            LawnMower* mower = lawnMowerFactory->lawnmowers[i].get();
            if (mower->shouldMove) continue; 
            
            int mowerRow = LaneSystem::getLane(mower->position.y);
            if (mowerRow < 0 || mowerRow >= GameConstants::BATTLEFIELD_ROWS) continue;

            for (int j = 0; j < enemyFactory->enemies_created; ++j)
            {
                if (enemyFactory->enemies[j]->enabled)
                {
                    Enemy* enemy = enemyFactory->enemies[j].get();
                    int enemyRow = LaneSystem::getLane(enemy->coords.y);

                    sf::FloatRect enemyBounds = enemy->display.getGlobalBounds();
                    float vMargin = 24.0f;
                    enemyBounds.top -= vMargin;
                    enemyBounds.height += vMargin * 2.0f;
                    if (mowerRow == enemyRow && mower->sprite.getGlobalBounds().intersects(enemyBounds))
                    {
                        mower->shouldMove = true;
                        SoundManager::getInstance().playSound("lawnmower.wav");
                        if (effectManager) {
                            effectManager->addHitImpact(sf::Vector2f(enemy->coords.x, enemy->coords.y));
                            effectManager->addSparks(sf::Vector2f(enemy->coords.x, enemy->coords.y), 25);
                        }
                        break; 
                    }
                }
            }
        }
    }

    for (int i = 0; i < static_cast<int>(lawnMowerFactory->lawnmowers.size()); ++i)
    {
        LawnMower* mower = lawnMowerFactory->lawnmowers[i].get();
        if (mower->exist && mower->shouldMove)
        {
            int mowerRow = LaneSystem::getLane(mower->position.y);
            if (mowerRow < 0 || mowerRow >= GameConstants::BATTLEFIELD_ROWS) continue;

            for (int j = 0; j < enemyFactory->enemies_created; ++j)
            {
                if (enemyFactory->enemies[j]->enabled)
                {
                    Enemy* enemy = enemyFactory->enemies[j].get();
                    int enemyRow = LaneSystem::getLane(enemy->coords.y);

                    if (mowerRow == enemyRow)
                    {
                        enemy->triggerDeath();
                        player.score += enemy->scoreValue;
                        if (enemy->type.find("boss_") == 0) {
                            SoundManager::getInstance().playSound("groan6.wav");
                        } else {
                            SoundManager::getInstance().playSound("splat2.wav");
                        }
                        if (effectManager) {
                            effectManager->addDisintegrationEffect(sf::Vector2f(enemy->coords.x, enemy->coords.y));
                            effectManager->addSparks(sf::Vector2f(enemy->coords.x, enemy->coords.y), 18);
                        }
                    }
                }
            }
        }
    }
}

void CollisionManager::updateQuadTree(EnemyFactory* enemyFactory) {
    quadTree.clear();
    
    for (int i = 0; i < enemyFactory->enemies_created; ++i) {
        if (enemyFactory->enemies[i]->enabled) {
            quadTree.insert(enemyFactory->enemies[i].get());
        }
    }
}

void CollisionManager::resolveEnemyCollisions(EnemyFactory* enemyFactory) {
    if (!enemyFactory) return;
    
    updateSpatialIndex(enemyFactory);
    
    std::vector<Enemy*> allEnemies;
    allEnemies.reserve(enemyFactory->enemies_created);
    
    for (int i = 0; i < enemyFactory->enemies_created; ++i) {
        if (SafeOperations::isValidArrayIndex(i, 10000) && 
            enemyFactory->enemies[i] && enemyFactory->enemies[i]->enabled) {
            allEnemies.push_back(enemyFactory->enemies[i].get());
        }
    }
    Enemy::setAllEnemiesReference(allEnemies);
    
    const float SEPARATION_RADIUS = 60.0f;
    
    for (Enemy* enemy : allEnemies) {
        if (!enemy || !enemy->enabled) continue;
        
        sf::Vector2f validatedPos = SafeOperations::validatePosition(sf::Vector2f(enemy->coords.x, enemy->coords.y));
        std::vector<Enemy*> nearbyEnemies = spatialDetector->getEntitiesInRadius(validatedPos, SEPARATION_RADIUS);
        
        sf::Vector2f separationForce(0, 0);
        int neighborCount = 0;
        
        for (Enemy* other : nearbyEnemies) {
            if (other == enemy || !other->enabled) continue;
            
            try {
                float distance = SafeOperations::safeDistance(validatedPos, sf::Vector2f(other->coords.x, other->coords.y));
                
                if (distance > 0 && distance < SEPARATION_RADIUS) {
                    float dx = validatedPos.x - other->coords.x;
                    float dy = validatedPos.y - other->coords.y;
                    sf::Vector2f diff(dx / distance, dy / distance);
                    diff *= (SEPARATION_RADIUS - distance) / SEPARATION_RADIUS;
                    
                    separationForce += diff;
                    neighborCount++;
                }
            } catch (const std::exception&) {
                continue;
            }
        }
        
        if (neighborCount > 0) {
            separationForce /= static_cast<float>(neighborCount);
            separationForce *= 20.0f;
            
            sf::Vector2f newPos = validatedPos;
            newPos.x += separationForce.x * 0.5f;
            
            newPos = SafeOperations::validatePosition(newPos);
            
            enemy->coords.x = newPos.x;
            enemy->display.setPosition(newPos.x, enemy->coords.y);
        }
    }
}

void CollisionManager::updateSpatialIndex(EnemyFactory* enemyFactory) {
    if (!enemyFactory || !spatialDetector) return;
    
    std::vector<Enemy*> activeEnemies;
    activeEnemies.reserve(enemyFactory->enemies_created);
    
    for (int i = 0; i < enemyFactory->enemies_created; ++i) {
        if (SafeOperations::isValidArrayIndex(i, 10000) && 
            enemyFactory->enemies[i] && enemyFactory->enemies[i]->enabled) {
            activeEnemies.push_back(enemyFactory->enemies[i].get());
        }
    }
    
    spatialDetector->updateIndex(activeEnemies);
}

std::vector<Enemy*> CollisionManager::getEnemiesInLane(int laneIndex) const {
    if (!spatialDetector || !SafeOperations::isValidGridPosition(laneIndex, 0)) {
        return {};
    }
    
    return spatialDetector->getEnemiesInLane(laneIndex, GameConstants::TILE_SIZE);
}

std::vector<Enemy*> CollisionManager::getEnemiesNearTower(const sf::Vector2f& towerPos, float radius) const {
    if (!spatialDetector) {
        return {};
    }
    
    sf::Vector2f validPos = SafeOperations::validatePosition(towerPos);
    return spatialDetector->getEnemiesNearTower(validPos, radius);
}

std::vector<Enemy*> CollisionManager::getEnemiesInRadius(const sf::Vector2f& center, float radius) const {
    if (!spatialDetector) {
        return {};
    }
    
    sf::Vector2f validCenter = SafeOperations::validatePosition(center);
    return spatialDetector->getEntitiesInRadius(validCenter, radius);
}

CollisionManager::CollisionStats CollisionManager::getPerformanceStats() const {
    if (!spatialDetector) {
        return {0, 0, 0, 0.0};
    }
    
    auto currentTime = std::chrono::steady_clock::now();
    auto timeDiff = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastFrameTime);
    
    return {
        spatialDetector->getIndexedEntityCount(),
        spatialDetector->getMaxDepth(),
        collisionChecksPerFrame,
        timeDiff.count() / 1000.0
    };
}

void CollisionManager::resetPerformanceCounters() {
    collisionChecksPerFrame = 0;
    lastFrameTime = std::chrono::steady_clock::now();
}
