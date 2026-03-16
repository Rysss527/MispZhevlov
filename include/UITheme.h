#pragma once
#ifndef UI_THEME_H
#define UI_THEME_H

#include <SFML/Graphics.hpp>


namespace UITheme {
    extern const sf::Color PRIMARY_TEXT;
    extern const sf::Color SECONDARY_TEXT;
    extern const sf::Color ACCENT_TEXT;
    extern const sf::Color SUCCESS_COLOR;
    extern const sf::Color WARNING_COLOR;
    extern const sf::Color ERROR_COLOR;
    extern const sf::Color INFO_COLOR;
    
    extern const sf::Color BUTTON_PRIMARY;
    extern const sf::Color BUTTON_PRIMARY_HOVER;
    extern const sf::Color BUTTON_SECONDARY;
    extern const sf::Color BUTTON_SECONDARY_HOVER;
    extern const sf::Color BUTTON_DISABLED;
    
    extern const sf::Color UI_BACKGROUND;
    extern const sf::Color UI_PANEL;
    extern const sf::Color UI_BORDER;
    extern const sf::Color UI_BORDER_HIGHLIGHT;
    
    extern const sf::Color TEXT_OUTLINE;
    extern const sf::Color SHADOW_COLOR;
    
    extern const sf::Color GOLD_COLOR;
    extern const sf::Color SUN_COLOR;
    extern const sf::Color HEALTH_FULL;
    extern const sf::Color HEALTH_MEDIUM;
    extern const sf::Color HEALTH_LOW;
    
    extern const int TITLE_SIZE;
    extern const int SUBTITLE_SIZE;
    extern const int BODY_SIZE;
    extern const int BUTTON_SIZE;
    extern const int SMALL_SIZE;
    extern const int CAPTION_SIZE;
    extern const int LARGE_SIZE;
    
    extern const float OUTLINE_THICKNESS;
    extern const sf::Vector2f SHADOW_OFFSET;
}

class UIHelper {
public:
    static void drawTextWithOutline(sf::RenderWindow& window, sf::Text& text, const sf::Color& textColor, const sf::Color& outlineColor = UITheme::TEXT_OUTLINE, float outlineThickness = UITheme::OUTLINE_THICKNESS) {
        sf::Vector2f originalPos = text.getPosition();
        sf::Color originalColor = text.getFillColor();
        
        text.setFillColor(outlineColor);
        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                if (x != 0 || y != 0) {
                    text.setPosition(originalPos.x + x * outlineThickness, originalPos.y + y * outlineThickness);
                    window.draw(text);
                }
            }
        }
        
        text.setPosition(originalPos);
        text.setFillColor(textColor);
        window.draw(text);
    }
    
    static void drawTextWithShadow(sf::RenderWindow& window, sf::Text& text, const sf::Color& textColor, const sf::Color& shadowColor = UITheme::SHADOW_COLOR, const sf::Vector2f& shadowOffset = UITheme::SHADOW_OFFSET) {
        sf::Vector2f originalPos = text.getPosition();
        sf::Color originalColor = text.getFillColor();
        
        text.setPosition(originalPos + shadowOffset);
        text.setFillColor(shadowColor);
        window.draw(text);
        
        text.setPosition(originalPos);
        text.setFillColor(textColor);
        window.draw(text);
    }
    
    static sf::RectangleShape createPanel(float x, float y, float width, float height, const sf::Color& backgroundColor = UITheme::UI_PANEL, const sf::Color& borderColor = UITheme::UI_BORDER, float borderThickness = 2.0f) {
        sf::RectangleShape panel;
        panel.setPosition(x, y);
        panel.setSize(sf::Vector2f(width, height));
        panel.setFillColor(backgroundColor);
        panel.setOutlineColor(borderColor);
        panel.setOutlineThickness(borderThickness);
        return panel;
    }
    
    static sf::RectangleShape createButton(float x, float y, float width, float height, bool isHovered = false, bool isDisabled = false) {
        sf::RectangleShape button;
        button.setPosition(x, y);
        button.setSize(sf::Vector2f(width, height));
        
        if (isDisabled) {
            button.setFillColor(UITheme::BUTTON_DISABLED);
            button.setOutlineColor(UITheme::UI_BORDER);
        } else if (isHovered) {
            button.setFillColor(UITheme::BUTTON_PRIMARY_HOVER);
            button.setOutlineColor(UITheme::UI_BORDER_HIGHLIGHT);
        } else {
            button.setFillColor(UITheme::BUTTON_PRIMARY);
            button.setOutlineColor(UITheme::UI_BORDER);
        }
        
        button.setOutlineThickness(2.0f);
        return button;
    }
    
    static sf::Color getHealthColor(float healthPercent) {
        if (healthPercent > 0.6f) {
            return UITheme::HEALTH_FULL;
        } else if (healthPercent > 0.3f) {
            return UITheme::HEALTH_MEDIUM;
        } else {
            return UITheme::HEALTH_LOW;
        }
    }
    
    static std::string createStars(int level) {
        std::string stars = "";
        if (level > 0) {
            for (int i = 0; i < std::min(level, 5); i++) {
                stars += "*";
            }
        }
        return stars;
    }
};

#endif