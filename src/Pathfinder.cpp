#include "Pathfinder.h"
#include <cmath>
#include <algorithm>
#include <set>

std::unique_ptr<NavGrid> Pathfinder::navigationGrid = nullptr;

void Pathfinder::initialize(int width, int height, float nodeSize) {
    navigationGrid = std::make_unique<NavGrid>(width, height, nodeSize);
}

void Pathfinder::updateGrid(float worldX, float worldY, bool walkable) {
    if (navigationGrid) {
        navigationGrid->updateFromTowerPosition(worldX, worldY, walkable);
    }
}

std::vector<sf::Vector2f> Pathfinder::findPath(sf::Vector2f start, sf::Vector2f end) {
    std::vector<sf::Vector2f> path;
    
    if (!navigationGrid) {
        return path;
    }
    
    navigationGrid->reset();
    
    sf::Vector2i startGrid = navigationGrid->worldToGrid(start.x, start.y);
    sf::Vector2i endGrid = navigationGrid->worldToGrid(end.x, end.y);
    
    GridNode* startNode = navigationGrid->getNode(startGrid.x, startGrid.y);
    GridNode* endNode = navigationGrid->getNode(endGrid.x, endGrid.y);
    
    if (!startNode || !endNode || !endNode->walkable) {
        return path;
    }
    
    std::vector<GridNode*> openSet;
    std::set<GridNode*> closedSet;
    
    openSet.push_back(startNode);
    
    while (!openSet.empty()) {
        GridNode* currentNode = *std::min_element(openSet.begin(), openSet.end(),
            [](GridNode* a, GridNode* b) {
                if (std::abs(a->fCost - b->fCost) < 0.001f) {
                    return a->hCost < b->hCost;
                }
                return a->fCost < b->fCost;
            });
        
        openSet.erase(std::remove(openSet.begin(), openSet.end(), currentNode), openSet.end());
        closedSet.insert(currentNode);
        
        if (currentNode == endNode) {
            return retracePath(startNode, endNode);
        }
        
        std::vector<GridNode*> neighbors = navigationGrid->getNeighbors(currentNode);
        for (GridNode* neighbor : neighbors) {
            if (!neighbor->walkable || closedSet.find(neighbor) != closedSet.end()) {
                continue;
            }
            
            float newCostToNeighbor = currentNode->gCost + getDistance(currentNode, neighbor);
            
            if (std::find(openSet.begin(), openSet.end(), neighbor) == openSet.end() || 
                newCostToNeighbor < neighbor->gCost) {
                
                neighbor->gCost = newCostToNeighbor;
                neighbor->hCost = getHeuristic(neighbor, endNode);
                neighbor->calculateFCost();
                neighbor->parent = currentNode;
                
                if (std::find(openSet.begin(), openSet.end(), neighbor) == openSet.end()) {
                    openSet.push_back(neighbor);
                }
            }
        }
    }
    
    return path;
}

NavGrid* Pathfinder::getGrid() {
    return navigationGrid.get();
}

float Pathfinder::getDistance(GridNode* nodeA, GridNode* nodeB) {
    int dx = std::abs(nodeA->x - nodeB->x);
    int dy = std::abs(nodeA->y - nodeB->y);
    
    if (dx > dy) {
        return static_cast<float>(14 * dy + 10 * (dx - dy));
    } else {
        return static_cast<float>(14 * dx + 10 * (dy - dx));
    }
}

float Pathfinder::getHeuristic(GridNode* nodeA, GridNode* nodeB) {
    return getDistance(nodeA, nodeB);
}

void Pathfinder::clearGrid() {
    if (navigationGrid) {
        navigationGrid->clear();
    }
}

std::vector<sf::Vector2f> Pathfinder::retracePath(GridNode* startNode, GridNode* endNode) {
    std::vector<sf::Vector2f> path;
    GridNode* currentNode = endNode;
    
    while (currentNode != startNode) {
        sf::Vector2f worldPos = navigationGrid->gridToWorld(currentNode->x, currentNode->y);
        path.push_back(worldPos);
        currentNode = currentNode->parent;
    }
    
    std::reverse(path.begin(), path.end());
    return path;
}
