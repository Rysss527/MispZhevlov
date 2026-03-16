#include "NavGrid.h"
#include <algorithm>
#include "Constants.h"

NavGrid::NavGrid(int width, int height, float nodeSize) 
    : width(width), height(height), nodeSize(nodeSize) {
    grid.resize(height, std::vector<GridNode>(width));
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            grid[y][x] = GridNode(x, y, true);
        }
    }
}

void NavGrid::updateNode(int x, int y, bool walkable) {
    if (isValidPosition(x, y)) {
        grid[y][x].walkable = walkable;
    }
}

void NavGrid::updateFromTowerPosition(float worldX, float worldY, bool walkable) {
    sf::Vector2i gridPos = worldToGrid(worldX, worldY);
    updateNode(gridPos.x, gridPos.y, walkable);
}

GridNode* NavGrid::getNode(int x, int y) {
    if (isValidPosition(x, y)) {
        return &grid[y][x];
    }
    return nullptr;
}

bool NavGrid::isValidPosition(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

bool NavGrid::isWalkable(int x, int y) const {
    if (!isValidPosition(x, y)) return false;
    return grid[y][x].walkable;
}

sf::Vector2i NavGrid::worldToGrid(float worldX, float worldY) const {
    int gridX = static_cast<int>(worldX / nodeSize);
    int gridY = static_cast<int>((worldY - GameConstants::Y_OFFSET) / nodeSize);
    
    gridX = std::max(0, std::min(gridX, width - 1));
    gridY = std::max(0, std::min(gridY, height - 1));
    
    return sf::Vector2i(gridX, gridY);
}

sf::Vector2f NavGrid::gridToWorld(int gridX, int gridY) const {
    float worldX = gridX * nodeSize + nodeSize * 0.5f;
    float worldY = gridY * nodeSize + GameConstants::Y_OFFSET + nodeSize * 0.5f;
    return sf::Vector2f(worldX, worldY);
}

std::vector<GridNode*> NavGrid::getNeighbors(GridNode* node) {
    std::vector<GridNode*> neighbors;
    
    const int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    for (int i = 0; i < 4; i++) {
        int x = node->x + directions[i][0];
        int y = node->y + directions[i][1];
        
        if (isValidPosition(x, y) && isWalkable(x, y)) {
            neighbors.push_back(&grid[y][x]);
        }
    }
    
    return neighbors;
}

void NavGrid::clear() {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            grid[y][x].walkable = true;
        }
    }
}

void NavGrid::reset() {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            grid[y][x].gCost = 0;
            grid[y][x].hCost = 0;
            grid[y][x].fCost = 0;
            grid[y][x].parent = nullptr;
        }
    }
}
