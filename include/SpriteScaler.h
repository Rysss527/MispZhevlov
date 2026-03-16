#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include "Constants.h"



class SpriteScaler {
public:

    static void scaleAndCenterSprite(sf::Sprite& sprite, const sf::Texture& texture, 
                                    float targetSize, float paddingFactor = 0.9f,
                                    float centerX = 0.0f, float centerY = 0.0f) {
        sf::Vector2u texSize = texture.getSize();
        float texWidth = static_cast<float>(texSize.x);
        float texHeight = static_cast<float>(texSize.y);
        
        if (texWidth > 0 && texHeight > 0) {
            float effectiveSize = targetSize * paddingFactor;
            float scaleX = effectiveSize / texWidth;
            float scaleY = effectiveSize / texHeight;
            float scale = std::min(scaleX, scaleY);
            
            sprite.setScale(scale, scale);
            sprite.setOrigin(texWidth / 2.0f, texHeight / 2.0f);
            sprite.setPosition(centerX, centerY);
        } else {
            sprite.setScale(1.0f, 1.0f);
            sprite.setPosition(centerX, centerY);
        }
    }
    

    static void scaleToGridCell(sf::Sprite& sprite, const sf::Texture& texture,
                               float cellX, float cellY, float paddingFactor = 1.2f) {
        float targetSize = static_cast<float>(std::max(GameConstants::TILE_WIDTH, GameConstants::TILE_HEIGHT));
        scaleAndCenterSprite(sprite, texture, targetSize, paddingFactor, cellX, cellY);
    }
    
    static void scaleToFieldGrid(sf::Sprite& sprite, const sf::Texture& texture,
                                int gridX, int gridY, float paddingFactor = 1.2f) {
        float centerX = GameConstants::FIELD_START_X + gridX * GameConstants::TILE_WIDTH + GameConstants::TILE_WIDTH / 2.0f;
        float centerY = GameConstants::FIELD_START_Y + gridY * GameConstants::TILE_HEIGHT + GameConstants::TILE_HEIGHT / 2.0f;
        float targetSize = static_cast<float>(std::max(GameConstants::TILE_WIDTH, GameConstants::TILE_HEIGHT));
        scaleAndCenterSprite(sprite, texture, targetSize, paddingFactor, centerX, centerY);
    }
    
    static void scaleForUICard(sf::Sprite& sprite, const sf::Texture& texture,
                              float cardWidth, float cardHeight,
                              float centerX, float centerY, float paddingFactor = 0.8f) {
        float targetSize = std::min(cardWidth, cardHeight) * paddingFactor;
        scaleAndCenterSprite(sprite, texture, targetSize, 1.0f, centerX, centerY);
    }
    
    static void scaleSpecialEnemy(sf::Sprite& sprite, const sf::Texture& texture,
                                 float cellX, float cellY, 
                                 float sizeMultiplier = 1.0f, float paddingFactor = 0.9f) {
        float centerX = cellX + GameConstants::TILE_SIZE / 2.0f;
        float centerY = cellY + GameConstants::TILE_SIZE / 2.0f;
        float effectiveSize = GameConstants::TILE_SIZE * sizeMultiplier;
        scaleAndCenterSprite(sprite, texture, effectiveSize, paddingFactor, centerX, centerY);
    }
    
    static void scaleToSize(sf::Sprite& sprite, const sf::Texture& texture, 
                           float targetWidth, float targetHeight) {
        sf::Vector2u texSize = texture.getSize();
        float texWidth = static_cast<float>(texSize.x);
        float texHeight = static_cast<float>(texSize.y);
        
        if (texWidth > 0 && texHeight > 0) {
            float scaleX = targetWidth / texWidth;
            float scaleY = targetHeight / texHeight;
            float scale = std::min(scaleX, scaleY);
            
            sprite.setScale(scale, scale);
            sprite.setOrigin(texWidth / 2.0f, texHeight / 2.0f);
        } else {
            sprite.setScale(1.0f, 1.0f);
        }
    }
    
    static float getScaleFactor(const sf::Texture& texture, float targetSize, float paddingFactor = 0.9f) {
        sf::Vector2u texSize = texture.getSize();
        float texWidth = static_cast<float>(texSize.x);
        float texHeight = static_cast<float>(texSize.y);
        
        if (texWidth > 0 && texHeight > 0) {
            float effectiveSize = targetSize * paddingFactor;
            float scaleX = effectiveSize / texWidth;
            float scaleY = effectiveSize / texHeight;
            return std::min(scaleX, scaleY);
        }
        return 1.0f;
    }
};
