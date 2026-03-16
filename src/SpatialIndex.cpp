#include "SpatialIndex.h"
#include "Constants.h"
#include "Enemy.h"
#include <algorithm>
#include <cmath>
#include <cassert>

void SpatialIndex::QuadTreeNode::clear() {
    entities.clear();
    for (auto& child : children) {
        child.reset();
    }
}

sf::FloatRect SpatialIndex::QuadTreeNode::getEntityBounds(Enemy* entity) const {
    return sf::FloatRect(entity->coords.x, entity->coords.y, 50.0f, 50.0f);
}

void SpatialIndex::QuadTreeNode::insert(Enemy* entity) {
    if (!entity) return;
    
    sf::FloatRect entityBounds = getEntityBounds(entity);
    
    if (!bounds.intersects(entityBounds)) {
        return;
    }
    
    if (isLeafNode()) {
        entities.push_back(entity);
        
        if (entities.size() > MAX_ENTITIES && depth < MAX_DEPTH) {
            subdivide();
            
            auto tempEntities = std::move(entities);
            entities.clear();
            
            for (Enemy* e : tempEntities) {
                bool inserted = false;
                for (auto& child : children) {
                    if (child) {
                        sf::FloatRect eBounds = getEntityBounds(e);
                        if (child->bounds.intersects(eBounds)) {
                            child->insert(e);
                            inserted = true;
                            break;
                        }
                    }
                }
                if (!inserted) {
                    entities.push_back(e);
                }
            }
        }
    } else {
        bool inserted = false;
        for (auto& child : children) {
            if (child && child->bounds.intersects(entityBounds)) {
                child->insert(entity);
                inserted = true;
                break;
            }
        }
        if (!inserted) {
            entities.push_back(entity);
        }
    }
}

void SpatialIndex::QuadTreeNode::subdivide() {
    if (!isLeafNode()) return;
    
    float halfWidth = bounds.width * 0.5f;
    float halfHeight = bounds.height * 0.5f;
    float x = bounds.left;
    float y = bounds.top;
    
    children[0] = std::make_unique<QuadTreeNode>(
        sf::FloatRect(x, y, halfWidth, halfHeight), depth + 1);
    children[1] = std::make_unique<QuadTreeNode>(
        sf::FloatRect(x + halfWidth, y, halfWidth, halfHeight), depth + 1);
    children[2] = std::make_unique<QuadTreeNode>(
        sf::FloatRect(x, y + halfHeight, halfWidth, halfHeight), depth + 1);
    children[3] = std::make_unique<QuadTreeNode>(
        sf::FloatRect(x + halfWidth, y + halfHeight, halfWidth, halfHeight), depth + 1);
}

void SpatialIndex::QuadTreeNode::queryRange(const sf::FloatRect& range, std::vector<Enemy*>& result) const {
    if (!bounds.intersects(range)) {
        return;
    }
    
    for (Enemy* entity : entities) {
        if (entity) {
            sf::FloatRect entityBounds = getEntityBounds(entity);
            if (range.intersects(entityBounds)) {
                result.push_back(entity);
            }
        }
    }
    
    if (!isLeafNode()) {
        for (const auto& child : children) {
            if (child) {
                child->queryRange(range, result);
            }
        }
    }
}

void SpatialIndex::QuadTreeNode::queryRadius(const sf::Vector2f& center, float radius, std::vector<Enemy*>& result) const {
    sf::FloatRect queryRect(center.x - radius, center.y - radius, radius * 2, radius * 2);
    if (!bounds.intersects(queryRect)) {
        return;
    }
    
    float radiusSquared = radius * radius;
    
    for (Enemy* entity : entities) {
        if (entity) {
            sf::FloatRect entityBounds = getEntityBounds(entity);
            sf::Vector2f entityCenter(entityBounds.left + entityBounds.width * 0.5f,
                                    entityBounds.top + entityBounds.height * 0.5f);
            
            float dx = center.x - entityCenter.x;
            float dy = center.y - entityCenter.y;
            if (dx * dx + dy * dy <= radiusSquared) {
                result.push_back(entity);
            }
        }
    }
    
    if (!isLeafNode()) {
        for (const auto& child : children) {
            if (child) {
                child->queryRadius(center, radius, result);
            }
        }
    }
}

SpatialIndex::SpatialIndex(const sf::FloatRect& worldBounds) 
    : worldBounds(worldBounds) {
    root = std::make_unique<QuadTreeNode>(worldBounds);
    queryBuffer.reserve(64);
}

void SpatialIndex::clear() {
    root->clear();
}

void SpatialIndex::insert(Enemy* entity) {
    if (entity) {
        root->insert(entity);
    }
}

void SpatialIndex::rebuild(const std::vector<Enemy*>& entities) {
    clear();
    for (Enemy* entity : entities) {
        if (entity) {
            insert(entity);
        }
    }
}

std::vector<Enemy*> SpatialIndex::queryRange(const sf::FloatRect& range) const {
    queryBuffer.clear();
    root->queryRange(range, queryBuffer);
    return queryBuffer;
}

std::vector<Enemy*> SpatialIndex::queryRadius(const sf::Vector2f& center, float radius) const {
    queryBuffer.clear();
    root->queryRadius(center, radius, queryBuffer);
    return queryBuffer;
}

std::vector<Enemy*> SpatialIndex::queryNearest(const sf::Vector2f& point, size_t maxCount) const {
    float radius = 50.0f;
    const float maxRadius = std::max(worldBounds.width, worldBounds.height);
    
    while (radius <= maxRadius) {
        auto candidates = queryRadius(point, radius);
        if (candidates.size() >= maxCount) {
            std::partial_sort(candidates.begin(), 
                            candidates.begin() + std::min(maxCount, candidates.size()),
                            candidates.end(),
                            [&point](Enemy* a, Enemy* b) {
                                float distA = (a->coords.x - point.x) * (a->coords.x - point.x) +
                                            (a->coords.y - point.y) * (a->coords.y - point.y);
                                float distB = (b->coords.x - point.x) * (b->coords.x - point.x) +
                                            (b->coords.y - point.y) * (b->coords.y - point.y);
                                return distA < distB;
                            });
            candidates.resize(std::min(maxCount, candidates.size()));
            return candidates;
        }
        radius *= 1.5f;
    }
    
    return queryRadius(point, maxRadius);
}

size_t SpatialIndex::getEntityCount() const {
    return 0;
}

size_t SpatialIndex::getDepth() const {
    return QuadTreeNode::MAX_DEPTH;
}

void SpatialIndex::validateIntegrity() const {
}

OptimizedCollisionDetector::OptimizedCollisionDetector(const sf::FloatRect& worldBounds)
    : spatialIndex(worldBounds) {
    tempBuffer.reserve(32);
}

void OptimizedCollisionDetector::updateIndex(const std::vector<Enemy*>& enemies) {
    spatialIndex.rebuild(enemies);
}

std::vector<Enemy*> OptimizedCollisionDetector::getEntitiesInRange(const sf::FloatRect& range) const {
    return spatialIndex.queryRange(range);
}

std::vector<Enemy*> OptimizedCollisionDetector::getEntitiesInRadius(const sf::Vector2f& center, float radius) const {
    return spatialIndex.queryRadius(center, radius);
}

std::vector<Enemy*> OptimizedCollisionDetector::getNearestEntities(const sf::Vector2f& point, size_t maxCount) const {
    return spatialIndex.queryNearest(point, maxCount);
}

std::vector<Enemy*> OptimizedCollisionDetector::getEnemiesInLane(int laneIndex, float laneHeight) const {
    if (laneIndex < 0 || laneIndex >= GameConstants::BATTLEFIELD_ROWS) {
        return {};
    }
    
    float laneY = static_cast<float>(GameConstants::Y_OFFSET + (laneIndex * GameConstants::TILE_SIZE));
    sf::FloatRect laneRect(0, laneY, static_cast<float>(GameConstants::BATTLEFIELD_WIDTH), laneHeight);
    
    return spatialIndex.queryRange(laneRect);
}

std::vector<Enemy*> OptimizedCollisionDetector::getEnemiesNearTower(const sf::Vector2f& towerPos, float range) const {
    return spatialIndex.queryRadius(towerPos, range);
}
