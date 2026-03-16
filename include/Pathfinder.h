#pragma once
#include "NavGrid.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

class Pathfinder {
private:
    static std::unique_ptr<NavGrid> navigationGrid;
    
public:
    static void initialize(int width = 16, int height = 12, float nodeSize = 50.0f);
    
    static void updateGrid(float worldX, float worldY, bool walkable);
    
    static std::vector<sf::Vector2f> findPath(sf::Vector2f start, sf::Vector2f end);
    
    static NavGrid* getGrid();
    
    static float getDistance(GridNode* nodeA, GridNode* nodeB);
    static float getHeuristic(GridNode* nodeA, GridNode* nodeB);
    
    static void clearGrid();
    
private:
    static std::vector<sf::Vector2f> retracePath(GridNode* startNode, GridNode* endNode);
    static std::vector<GridNode*> getHeap(const std::vector<GridNode*>& openSet);
    static void heapifyUp(std::vector<GridNode*>& heap, int index);
    static void heapifyDown(std::vector<GridNode*>& heap, int index);
};
