#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <unordered_set>
#include <functional>

class Enemy;

class SpatialIndex {
public:
    struct QuadTreeNode {
        sf::FloatRect bounds;
        std::vector<Enemy*> entities;
        std::unique_ptr<QuadTreeNode> children[4];
        
        static constexpr size_t MAX_ENTITIES = 8;
        static constexpr size_t MAX_DEPTH = 6;
        size_t depth;
        
        QuadTreeNode(const sf::FloatRect& bounds, size_t depth = 0) 
            : bounds(bounds), depth(depth) {
            entities.reserve(MAX_ENTITIES);
        }
        
        void clear();
        void insert(Enemy* entity);
        void subdivide();
        void queryRange(const sf::FloatRect& range, std::vector<Enemy*>& result) const;
        void queryRadius(const sf::Vector2f& center, float radius, std::vector<Enemy*>& result) const;
        
    private:
        bool isLeafNode() const { return !children[0]; }
        sf::FloatRect getEntityBounds(Enemy* entity) const;
    };
    
private:
    std::unique_ptr<QuadTreeNode> root;
    sf::FloatRect worldBounds;
    mutable std::vector<Enemy*> queryBuffer;
    
public:
    explicit SpatialIndex(const sf::FloatRect& worldBounds);
    ~SpatialIndex() = default;
    
    SpatialIndex(const SpatialIndex&) = delete;
    SpatialIndex& operator=(const SpatialIndex&) = delete;
    SpatialIndex(SpatialIndex&&) = default;
    SpatialIndex& operator=(SpatialIndex&&) = default;
    
    void clear();
    void insert(Enemy* entity);
    void rebuild(const std::vector<Enemy*>& entities);
    
    std::vector<Enemy*> queryRange(const sf::FloatRect& range) const;
    std::vector<Enemy*> queryRadius(const sf::Vector2f& center, float radius) const;
    std::vector<Enemy*> queryNearest(const sf::Vector2f& point, size_t maxCount) const;
    
    size_t getEntityCount() const;
    size_t getDepth() const;
    void validateIntegrity() const;
};

class OptimizedCollisionDetector {
private:
    SpatialIndex spatialIndex;
    mutable std::vector<Enemy*> tempBuffer;
    
public:
    explicit OptimizedCollisionDetector(const sf::FloatRect& worldBounds);
    
    void updateIndex(const std::vector<Enemy*>& enemies);
    
    std::vector<Enemy*> getEntitiesInRange(const sf::FloatRect& range) const;
    std::vector<Enemy*> getEntitiesInRadius(const sf::Vector2f& center, float radius) const;
    std::vector<Enemy*> getNearestEntities(const sf::Vector2f& point, size_t maxCount) const;
    
    std::vector<Enemy*> getEnemiesInLane(int laneIndex, float laneHeight) const;
    std::vector<Enemy*> getEnemiesNearTower(const sf::Vector2f& towerPos, float range) const;
    
    size_t getIndexedEntityCount() const { return spatialIndex.getEntityCount(); }
    size_t getMaxDepth() const { return spatialIndex.getDepth(); }
};
