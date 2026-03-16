#pragma once

#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "../Tower.h"
#include "../Enemy.h"
#include "../Resource.h"
#include "../DefenseUnitFactory.h"
#include "../TowerFactory.h"
#include "../EnemyFactory.h"

namespace DefenseSynth {

class CollisionService {
public:
    struct CollisionResult {
        bool hasCollision;
        sf::Vector2f collisionPoint;
        float penetrationDepth;
        void* object1;
        void* object2;
        std::string type1;
        std::string type2;
    };
    
    struct SpatialCell {
        std::vector<Enemy*> enemies;
        std::vector<Tower*> towers;
        std::vector<Sun*> suns;
    };
    
    CollisionService();
    ~CollisionService() = default;
    
    CollisionResult checkTowerEnemyCollision(Tower* tower, Enemy* enemy);
    std::vector<CollisionResult> processAllCollisions(
        TowerFactory* towers, 
        EnemyFactory* enemies,
        SunFactory* suns = nullptr,
        LawnMowerFactory* lawnMowers = nullptr
    );
    
    void checkZombiePlantCollisions(EnemyFactory* enemies, TowerFactory* plants);
    void checkProjectileEnemyCollisions(TowerFactory* towers, EnemyFactory* enemies);
    void checkLawnMowerEnemyCollisions(LawnMowerFactory* lawnMowers, EnemyFactory* enemies);
    bool checkSunClick(SunFactory* suns, float x, float y);
    
    int getRow(float y) const;
    int getColumn(float x) const;
    bool isOnSameRow(float y1, float y2) const;
    bool isValidGridPosition(float x, float y) const;
    
private:
    void buildSpatialIndex(TowerFactory* towers, EnemyFactory* enemies);
    void clearSpatialIndex();
    
    static constexpr float GRID_OFFSET_X = 296.0f;
    static constexpr float GRID_OFFSET_Y = 162.0f;
    static constexpr float TILE_WIDTH = 100.0f;
    static constexpr float TILE_HEIGHT = 100.0f;
    static constexpr int GRID_ROWS = 5;
    static constexpr int GRID_COLS = 9;
    
    std::vector<std::vector<SpatialCell>> spatialIndex;
};

}
