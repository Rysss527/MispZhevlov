#pragma once
#include "TowerFactory.h"
#include "EnemyFactory.h"
#include "SoundManager.h"
#include "SpatialIndex.h"
#include "SafeOperations.h"
#include "Constants.h"
#include "EffectManager.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <chrono>

class Player;
class LawnMowerFactory;
class Enemy;

struct QuadTreeNode {
    sf::FloatRect bounds;
    std::vector<Enemy*> enemies;
    std::vector<std::unique_ptr<QuadTreeNode>> children;
    static const size_t MAX_ENEMIES = 10;
    static const size_t MAX_DEPTH = 5;
    size_t depth;
    
    QuadTreeNode(sf::FloatRect bounds, size_t depth = 0) 
        : bounds(bounds), depth(depth) {}
    
    void clear() {
        enemies.clear();
        children.clear();
    }
    
    void insert(Enemy* enemy);
    void subdivide();
    std::vector<Enemy*> query(const sf::FloatRect& range);
    bool isLeafNode() const { return children.empty(); }
};

class CollisionManager
{
private:
    std::unique_ptr<OptimizedCollisionDetector> spatialDetector;
    QuadTreeNode quadTree;
    DefenseSynth::EffectManager* effectManager;
    
    mutable size_t collisionChecksPerFrame = 0;
    mutable std::chrono::steady_clock::time_point lastFrameTime;
    
public:
    CollisionManager() 
        : spatialDetector(std::make_unique<OptimizedCollisionDetector>(
            sf::FloatRect(0, 0, GameConstants::BATTLEFIELD_WIDTH, GameConstants::BATTLEFIELD_HEIGHT)))
        , quadTree(sf::FloatRect(0, 0, 800, 600))
        , effectManager(nullptr)
        , lastFrameTime(std::chrono::steady_clock::now()) {}
    
    void setEffectManager(DefenseSynth::EffectManager* manager) { effectManager = manager; }
    
    void checkCollisions(TowerFactory* towerFactory, EnemyFactory* enemyFactory, Player& player);
    void checkLawnmowerCollisions(LawnMowerFactory* lawnMowerFactory, EnemyFactory* enemyFactory, Player& player);
    void resolveEnemyCollisions(EnemyFactory* enemyFactory);
    
    void updateSpatialIndex(EnemyFactory* enemyFactory);
    void updateQuadTree(EnemyFactory* enemyFactory);
    
    std::vector<Enemy*> getEnemiesInLane(int laneIndex) const;
    std::vector<Enemy*> getEnemiesNearTower(const sf::Vector2f& towerPos, float radius) const;
    std::vector<Enemy*> getEnemiesInRadius(const sf::Vector2f& center, float radius) const;
    
    struct CollisionStats {
        size_t spatialIndexSize;
        size_t maxSpatialDepth;
        size_t collisionChecksLastFrame;
        double avgCollisionTime;
    };
    
    CollisionStats getPerformanceStats() const;
    void resetPerformanceCounters();
};
