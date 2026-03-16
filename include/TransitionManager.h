#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>

namespace DefenseSynth {

enum class TransitionType {
    NONE,
    FADE,
    SLIDE_LEFT,
    SLIDE_RIGHT,
    SLIDE_UP,
    SLIDE_DOWN,
    SCALE,
    ROTATE,
    IRIS,
    DISSOLVE,
    CROSSFADE
};

class TransitionManager {
private:
    sf::RenderTexture fromTexture;
    sf::RenderTexture toTexture;
    sf::Sprite fromSprite;
    sf::Sprite toSprite;
    
    TransitionType currentType;
    float duration;
    float elapsedTime;
    bool isTransitioning;
    
    std::function<void()> onCompleteCallback;
    
    sf::RectangleShape fadeOverlay;
    sf::Shader dissolveShader;
    sf::Shader irisShader;
    
    sf::Vector2u windowSize;
    
public:
    TransitionManager(const sf::Vector2u& winSize) 
        : windowSize(winSize), currentType(TransitionType::NONE),
          duration(0.5f), elapsedTime(0), isTransitioning(false) {
        
        fromTexture.create(windowSize.x, windowSize.y);
        toTexture.create(windowSize.x, windowSize.y);
        
        fadeOverlay.setSize(sf::Vector2f(static_cast<float>(windowSize.x), 
                                        static_cast<float>(windowSize.y)));
        fadeOverlay.setFillColor(sf::Color::Black);
        
        initShaders();
    }
    
    void startTransition(TransitionType type, float transitionDuration = 0.5f) {
        currentType = type;
        duration = transitionDuration;
        elapsedTime = 0;
        isTransitioning = true;
        
        fromSprite.setTexture(fromTexture.getTexture());
        toSprite.setTexture(toTexture.getTexture());
    }
    
    void captureFromScreen(sf::RenderWindow& window) {
        fromTexture.clear();
        sf::View currentView = window.getView();
        fromTexture.setView(currentView);
        
        window.display();
        sf::Texture screenshot;
        screenshot.create(window.getSize().x, window.getSize().y);
        screenshot.update(window);
        sf::Sprite screenshotSprite(screenshot);
        fromTexture.draw(screenshotSprite);
        fromTexture.display();
    }
    
    void captureToScreen(sf::RenderWindow& window) {
        toTexture.clear();
        sf::View currentView = window.getView();
        toTexture.setView(currentView);
        
        window.display();
        sf::Texture screenshot;
        screenshot.create(window.getSize().x, window.getSize().y);
        screenshot.update(window);
        sf::Sprite screenshotSprite(screenshot);
        toTexture.draw(screenshotSprite);
        toTexture.display();
    }
    
    bool update(float deltaTime) {
        if (!isTransitioning) return false;
        
        elapsedTime += deltaTime;
        float progress = std::min(1.0f, elapsedTime / duration);
        
        if (progress >= 1.0f) {
            isTransitioning = false;
            if (onCompleteCallback) {
                onCompleteCallback();
            }
            return false;
        }
        
        return true;
    }
    
    void render(sf::RenderWindow& window) {
        if (!isTransitioning) return;
        
        float progress = elapsedTime / duration;
        
        switch(currentType) {
            case TransitionType::FADE:
                renderFade(window, progress);
                break;
                
            case TransitionType::SLIDE_LEFT:
                renderSlide(window, progress, -1, 0);
                break;
                
            case TransitionType::SLIDE_RIGHT:
                renderSlide(window, progress, 1, 0);
                break;
                
            case TransitionType::SLIDE_UP:
                renderSlide(window, progress, 0, -1);
                break;
                
            case TransitionType::SLIDE_DOWN:
                renderSlide(window, progress, 0, 1);
                break;
                
            case TransitionType::SCALE:
                renderScale(window, progress);
                break;
                
            case TransitionType::ROTATE:
                renderRotate(window, progress);
                break;
                
            case TransitionType::IRIS:
                renderIris(window, progress);
                break;
                
            case TransitionType::DISSOLVE:
                renderDissolve(window, progress);
                break;
                
            case TransitionType::CROSSFADE:
                renderCrossfade(window, progress);
                break;
                
            default:
                window.draw(toSprite);
                break;
        }
    }
    
    void setOnComplete(std::function<void()> callback) {
        onCompleteCallback = callback;
    }
    
    bool inTransition() const {
        return isTransitioning;
    }
    
    float getProgress() const {
        if (!isTransitioning) return 1.0f;
        return std::min(1.0f, elapsedTime / duration);
    }
    
    void fadeToBlack(float fadeDuration = 0.5f) {
        startTransition(TransitionType::FADE, fadeDuration);
    }
    
    void slideToScreen(TransitionType direction, float slideDuration = 0.3f) {
        if (direction >= TransitionType::SLIDE_LEFT && 
            direction <= TransitionType::SLIDE_DOWN) {
            startTransition(direction, slideDuration);
        }
    }
    
    void scaleTransition(float scaleDuration = 0.4f) {
        startTransition(TransitionType::SCALE, scaleDuration);
    }
    
private:
    void initShaders() {
        
    }
    
    void renderFade(sf::RenderWindow& window, float progress) {
        window.draw(fromSprite);
        
        fadeOverlay.setFillColor(sf::Color(0, 0, 0, 
            static_cast<sf::Uint8>(progress * 255)));
        window.draw(fadeOverlay);
        
        if (progress > 0.5f) {
            window.draw(toSprite);
            
            float fadeBack = (1.0f - progress) * 2.0f;
            fadeOverlay.setFillColor(sf::Color(0, 0, 0, 
                static_cast<sf::Uint8>(fadeBack * 255)));
            window.draw(fadeOverlay);
        }
    }
    
    void renderSlide(sf::RenderWindow& window, float progress, float dirX, float dirY) {
        float easeProgress = easeInOutCubic(progress);
        
        float offsetX = dirX * windowSize.x * easeProgress;
        float offsetY = dirY * windowSize.y * easeProgress;
        
        fromSprite.setPosition(-offsetX, -offsetY);
        window.draw(fromSprite);
        
        toSprite.setPosition(windowSize.x * dirX - offsetX, 
                            windowSize.y * dirY - offsetY);
        window.draw(toSprite);
    }
    
    void renderScale(sf::RenderWindow& window, float progress) {
        float easeProgress = easeInOutCubic(progress);
        
        if (progress < 0.5f) {
            float scale = 1.0f - easeProgress;
            fromSprite.setScale(scale, scale);
            fromSprite.setOrigin(windowSize.x / 2.0f, windowSize.y / 2.0f);
            fromSprite.setPosition(windowSize.x / 2.0f, windowSize.y / 2.0f);
            
            sf::Uint8 alpha = static_cast<sf::Uint8>((1.0f - easeProgress * 2.0f) * 255);
            fromSprite.setColor(sf::Color(255, 255, 255, alpha));
            
            window.draw(fromSprite);
        } else {
            float scale = (easeProgress - 0.5f) * 2.0f;
            toSprite.setScale(scale, scale);
            toSprite.setOrigin(windowSize.x / 2.0f, windowSize.y / 2.0f);
            toSprite.setPosition(windowSize.x / 2.0f, windowSize.y / 2.0f);
            
            sf::Uint8 alpha = static_cast<sf::Uint8>((scale) * 255);
            toSprite.setColor(sf::Color(255, 255, 255, alpha));
            
            window.draw(toSprite);
        }
    }
    
    void renderRotate(sf::RenderWindow& window, float progress) {
        float easeProgress = easeInOutCubic(progress);
        float rotation = easeProgress * 360.0f;
        
        if (progress < 0.5f) {
            fromSprite.setOrigin(windowSize.x / 2.0f, windowSize.y / 2.0f);
            fromSprite.setPosition(windowSize.x / 2.0f, windowSize.y / 2.0f);
            fromSprite.setRotation(rotation);
            
            float scale = 1.0f - easeProgress;
            fromSprite.setScale(scale, scale);
            
            window.draw(fromSprite);
        } else {
            toSprite.setOrigin(windowSize.x / 2.0f, windowSize.y / 2.0f);
            toSprite.setPosition(windowSize.x / 2.0f, windowSize.y / 2.0f);
            toSprite.setRotation(rotation - 180.0f);
            
            float scale = (easeProgress - 0.5f) * 2.0f;
            toSprite.setScale(scale, scale);
            
            window.draw(toSprite);
        }
    }
    
    void renderIris(sf::RenderWindow& window, float progress) {
        window.draw(fromSprite);
        
        if (progress > 0.5f) {
            window.draw(toSprite);
        }
        
        float radius = (1.0f - std::abs(progress - 0.5f) * 2.0f) * 
                      std::max(windowSize.x, windowSize.y);
        
        sf::CircleShape iris(radius);
        iris.setFillColor(sf::Color::Black);
        iris.setOrigin(radius, radius);
        iris.setPosition(windowSize.x / 2.0f, windowSize.y / 2.0f);
        
        sf::RenderTexture maskTexture;
        maskTexture.create(windowSize.x, windowSize.y);
        maskTexture.clear(sf::Color::Black);
        maskTexture.draw(iris);
        maskTexture.display();
    }
    
    void renderDissolve(sf::RenderWindow& window, float progress) {
        window.draw(fromSprite);
        
        toSprite.setColor(sf::Color(255, 255, 255, 
            static_cast<sf::Uint8>(progress * 255)));
        window.draw(toSprite);
    }
    
    void renderCrossfade(sf::RenderWindow& window, float progress) {
        fromSprite.setColor(sf::Color(255, 255, 255, 
            static_cast<sf::Uint8>((1.0f - progress) * 255)));
        window.draw(fromSprite);
        
        toSprite.setColor(sf::Color(255, 255, 255, 
            static_cast<sf::Uint8>(progress * 255)));
        window.draw(toSprite);
    }
    
    float easeInOutCubic(float t) {
        if (t < 0.5f) {
            return 4.0f * t * t * t;
        } else {
            float p = 2.0f * t - 2.0f;
            return 1.0f + p * p * p / 2.0f;
        }
    }
    
    float easeInOutQuad(float t) {
        if (t < 0.5f) {
            return 2.0f * t * t;
        } else {
            return -1.0f + (4.0f - 2.0f * t) * t;
        }
    }
    
    float easeInOutSine(float t) {
        return -(std::cos(3.14159f * t) - 1.0f) / 2.0f;
    }
};

}
