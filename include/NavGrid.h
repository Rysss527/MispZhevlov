#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Constants.h"

struct GridNode {
    int x, y;
    bool walkable;
    float gCost;
    float hCost;
    float fCost;
    GridNode* parent;
    
    GridNode(int x = 0, int y = 0, bool walkable = true) 
        : x(x), y(y), walkable(walkable), gCost(0), hCost(0), fCost(0), parent(nullptr) {}
    
    void calculateFCost() {
        fCost = gCost + hCost;
    }
};

class NavGrid {
private:
    std::vector<std::vector<GridNode>> grid;
    int width, height;
    float nodeSize;
    
public:
    NavGrid(int width = 16, int height = 12, float nodeSize = 50.0f);
    
    void updateNode(int x, int y, bool walkable);
    void updateFromTowerPosition(float worldX, float worldY, bool walkable);
    
    GridNode* getNode(int x, int y);
    bool isValidPosition(int x, int y) const;
    bool isWalkable(int x, int y) const;
    
    sf::Vector2i worldToGrid(float worldX, float worldY) const;
    sf::Vector2f gridToWorld(int gridX, int gridY) const;
    
    std::vector<GridNode*> getNeighbors(GridNode* node);
    
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    float getNodeSize() const { return nodeSize; }
    
    void clear();
    void reset();
};
