#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>
#include "GamePlayer.h"
#include "Constants.h"

struct coordinates {
    float x;
    float y;
};

enum class CursorState {
    NORMAL,
    TARGETING,
    INVALID,
    DRAGGING,
    HOVER,
    LOADING
};

class GameCursor {
private:
    sf::Cursor cursor;
    sf::Image cursorImg;
    sf::Sprite cursorSprite;
    std::string type;
    std::vector<std::string> allowedUnits;
    
    CursorState currentState;
    sf::CircleShape rangeIndicator;
    bool showRange;
    sf::Sprite previewSprite;
    bool showPreview;
    float previewAlpha;
    sf::RectangleShape invalidOverlay;
    sf::CircleShape loadingIndicator;
    float loadingProgress;
    sf::Vector2f position;
    sf::RenderWindow* window;
    sf::Clock animationClock;
    float pulseAnimation;

public:
    GameCursor() : currentState(CursorState::NORMAL), showRange(false), 
                   showPreview(false), previewAlpha(128), loadingProgress(0),
                   window(nullptr), pulseAnimation(0) {
        cursor.loadFromSystem(sf::Cursor::Arrow);
        type = "default";
        
        rangeIndicator.setFillColor(sf::Color(100, 200, 100, 50));
        rangeIndicator.setOutlineColor(sf::Color(50, 255, 50, 150));
        rangeIndicator.setOutlineThickness(2.0f);
        
        invalidOverlay.setFillColor(sf::Color(255, 0, 0, 30));
        invalidOverlay.setOutlineColor(sf::Color(255, 0, 0, 100));
        invalidOverlay.setOutlineThickness(2.0f);
        
        loadingIndicator.setRadius(20);
        loadingIndicator.setPointCount(30);
        loadingIndicator.setFillColor(sf::Color::Transparent);
        loadingIndicator.setOutlineColor(sf::Color(255, 255, 255, 200));
        loadingIndicator.setOutlineThickness(3.0f);
    }
    
    void init(sf::RenderWindow* win) {
        window = win;
    }
    
    void setAllowedUnits(const std::vector<std::string>& units) {
        allowedUnits = units;
    }

    void setCursor(const std::string& imagePath) {
        if (cursorImg.loadFromFile(imagePath)) {
            cursor.loadFromPixels(cursorImg.getPixelsPtr(), cursorImg.getSize(), sf::Vector2u(50, 50));
        }
    }

    void setDefaultCursor() {
        cursor.loadFromSystem(sf::Cursor::Arrow);
        type = "default";
    }

    void renderCursor(coordinates clickPosition, Player& player) {
        auto getSlotForUnit = [](const std::string& unitType) -> int {
            if (unitType == "basicshooter") return 0;
            if (unitType == "rapidshooter") return 1;
            if (unitType == "freezetower") return 2;
            if (unitType == "areaattacktower") return 3;
            if (unitType == "barriertower") return 4;
            if (unitType == "bombtower") return 5;
            if (unitType == "resourcegenerator") return 6;
            return -1;
        };
        
        auto getCost = [](const std::string& unitType) -> int {
            if (unitType == "basicshooter") return 50;
            if (unitType == "rapidshooter") return 200;
            if (unitType == "freezetower") return 100;
            if (unitType == "areaattacktower") return 75;
            if (unitType == "barriertower") return 50;
            if (unitType == "bombtower") return 150;
            if (unitType == "resourcegenerator") return 100;
            return 50;
        };
        
        auto getImage = [](const std::string& unitType) -> std::string {
            if (unitType == "basicshooter") return "assets/images/peeshooter.png";
            if (unitType == "rapidshooter") return "assets/images/repeater.png";
            if (unitType == "freezetower") return "assets/images/snowpea.png";
            if (unitType == "areaattacktower") return "assets/images/fumeshroom.png";
            if (unitType == "barriertower") return "assets/images/wallnut.png";
            if (unitType == "bombtower") return "assets/images/cherrybomb.png";
            if (unitType == "resourcegenerator") return "assets/images/sunflower.png";
            return "assets/images/plant.png";
        };
        
        auto requiresLevel = [](const std::string& unitType) -> int {
            return 1;
        };
        
        if (clickPosition.y >= 0 && clickPosition.y <= 100) {
            for (int i = 0; i < 7; i++) {
                int slotX = 100 + (i * 100);
                if (clickPosition.x >= slotX && clickPosition.x <= slotX + 100) {
                    for (const auto& unitType : allowedUnits) {
                        if (getSlotForUnit(unitType) == i) {
                            int cost = getCost(unitType);
                            int requiredLevel = requiresLevel(unitType);
                            
                            if (player.currency >= cost && player.level >= requiredLevel) {
                                setCursor(getImage(unitType));
                                type = unitType;
                                return;
                            }
                            break;
                        }
                    }
                    break;
                }
            }
        }
        
        if ((clickPosition.x >= 800 && clickPosition.x <= 900) && (clickPosition.y >= 0 && clickPosition.y <= 100)) {
            setCursor("assets/images/shovel.png");
            type = "shovel";
        }
        else if ((clickPosition.x >= 0 && clickPosition.x <= 1300) && (clickPosition.y >= 100 && clickPosition.y <= 900)) {
            setDefaultCursor();
        }
    }
    std::string getCurrentCursor() {
		return type;
	}

    void applyCursor(sf::RenderWindow& window) {
        window.setMouseCursor(cursor);
    }
    
    void setState(CursorState state) {
        currentState = state;
    }
    
    void setPosition(const sf::Vector2f& pos) {
        position = pos;
        cursorSprite.setPosition(pos);
        
        if (showRange) {
            rangeIndicator.setPosition(pos);
        }
        
        if (showPreview) {
            sf::Vector2f gridPos = snapToGrid(pos);
            previewSprite.setPosition(gridPos);
            invalidOverlay.setPosition(gridPos);
        }
    }
    
    void update(float deltaTime) {
        if (window) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
            setPosition(sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)));
        }
        
        pulseAnimation = (std::sin(animationClock.getElapsedTime().asSeconds() * 3.0f) + 1.0f) / 2.0f;
        
        if (showRange) {
            float alpha = 50 + pulseAnimation * 30;
            sf::Color rangeColor = rangeIndicator.getFillColor();
            rangeColor.a = static_cast<sf::Uint8>(alpha);
            rangeIndicator.setFillColor(rangeColor);
        }
    }
    
    void showTowerPreview(const sf::Texture& towerTexture, bool isValidPosition) {
        showPreview = true;
        previewSprite.setTexture(towerTexture);
        previewSprite.setOrigin(towerTexture.getSize().x / 2.0f, towerTexture.getSize().y / 2.0f);
        
        sf::Color color = isValidPosition ? 
            sf::Color(255, 255, 255, static_cast<sf::Uint8>(previewAlpha)) :
            sf::Color(255, 100, 100, static_cast<sf::Uint8>(previewAlpha));
        previewSprite.setColor(color);
        
        currentState = isValidPosition ? CursorState::TARGETING : CursorState::INVALID;
    }
    
    void hideTowerPreview() {
        showPreview = false;
        currentState = CursorState::NORMAL;
    }
    
    void showRangeIndicator(float range) {
        showRange = true;
        rangeIndicator.setRadius(range);
        rangeIndicator.setOrigin(range, range);
        rangeIndicator.setPosition(position);
    }
    
    void hideRangeIndicator() {
        showRange = false;
    }
    
    void draw(sf::RenderWindow& window) {
        if (showRange) {
            window.draw(rangeIndicator);
        }
        
        if (showPreview) {
            window.draw(previewSprite);
            
            if (currentState == CursorState::INVALID) {
                window.draw(invalidOverlay);
            }
        }
    }
    
    sf::Vector2f getPosition() const {
        return position;
    }
    
private:
    sf::Vector2f snapToGrid(const sf::Vector2f& pos) {
        int gridX = static_cast<int>((pos.x - GameConstants::FIELD_START_X) / GameConstants::TILE_WIDTH);
        int gridY = static_cast<int>((pos.y - GameConstants::FIELD_START_Y) / GameConstants::TILE_HEIGHT);
        
        gridX = std::max(0, std::min(gridX, GameConstants::BATTLEFIELD_COLS - 1));
        gridY = std::max(0, std::min(gridY, GameConstants::BATTLEFIELD_ROWS - 1));
        
        return sf::Vector2f(
            GameConstants::FIELD_START_X + gridX * GameConstants::TILE_WIDTH + GameConstants::TILE_WIDTH / 2.0f,
            GameConstants::FIELD_START_Y + gridY * GameConstants::TILE_HEIGHT + GameConstants::TILE_HEIGHT / 2.0f
        );
    }
};
