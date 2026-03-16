#include "LoadingScreen.h"
#include <algorithm>
#include <cmath>
#include "ResourceManager.h"

namespace DefenseSynth {

LoadingScreen::LoadingScreen(sf::RenderWindow& win) 
    : window(win), progress(0.0f), targetProgress(0.0f), spinnerRotation(0.0f) {
    
    auto& rm = ResourceManager::getInstance();
    if (!rm.loadFont("assets/fonts/arial.ttf", "arial")) {
        rm.loadFont("assets/fonts/new.ttf", "arial");
    }
    font = rm.getFont("arial");
    
    loadingText.setFont(font);
    loadingText.setString("Loading Resources...");
    loadingText.setCharacterSize(48);
    loadingText.setFillColor(sf::Color::White);
    {
        sf::FloatRect lb = loadingText.getLocalBounds();
        loadingText.setOrigin(lb.left + lb.width / 2.0f, lb.top + lb.height / 2.0f);
    }
    loadingText.setPosition(static_cast<float>(window.getSize().x) / 2.0f, static_cast<float>(window.getSize().y) / 2.0f - 100.0f);
    
    statusText.setFont(font);
    statusText.setString("");
    statusText.setCharacterSize(24);
    statusText.setFillColor(sf::Color(200, 200, 200));
    {
        sf::FloatRect sb = statusText.getLocalBounds();
        statusText.setOrigin(sb.left + sb.width / 2.0f, sb.top + sb.height / 2.0f);
    }
    statusText.setPosition(static_cast<float>(window.getSize().x) / 2.0f, static_cast<float>(window.getSize().y) / 2.0f + 50.0f);
    
    float barWidth = 600;
    float barHeight = 30;
    float barX = (static_cast<float>(window.getSize().x) - barWidth) / 2.0f;
    float barY = static_cast<float>(window.getSize().y) / 2.0f - 20.0f;
    
    progressBarBackground.setSize(sf::Vector2f(barWidth, barHeight));
    progressBarBackground.setPosition(barX, barY);
    progressBarBackground.setFillColor(sf::Color(50, 50, 50));
    progressBarBackground.setOutlineThickness(2);
    progressBarBackground.setOutlineColor(sf::Color::White);
    
    progressBar.setSize(sf::Vector2f(0, barHeight));
    progressBar.setPosition(barX, barY);
    progressBar.setFillColor(sf::Color(0, 200, 0));
    
    spinner.setRadius(20);
    spinner.setPointCount(8);
    spinner.setFillColor(sf::Color(100, 200, 100));
    spinner.setOrigin(20, 20);
    spinner.setPosition(static_cast<float>(window.getSize().x) / 2.0f, static_cast<float>(window.getSize().y) / 2.0f + 100.0f);
}

void LoadingScreen::setProgress(float value) {
    targetProgress = std::min(1.0f, std::max(0.0f, value));
}

void LoadingScreen::setStatus(const std::string& status) {
    statusText.setString(status);
    sf::FloatRect sb = statusText.getLocalBounds();
    statusText.setOrigin(sb.left + sb.width / 2.0f, sb.top + sb.height / 2.0f);
    statusText.setPosition(static_cast<float>(window.getSize().x) / 2.0f, progressBarBackground.getPosition().y + progressBarBackground.getSize().y + 40.0f);
}

void LoadingScreen::update(float deltaTime) {
    float speed = 2.0f;
    if (progress < targetProgress) {
        progress += speed * deltaTime;
        if (progress > targetProgress) {
            progress = targetProgress;
        }
    } else if (progress > targetProgress) {
        progress -= speed * deltaTime;
        if (progress < targetProgress) {
            progress = targetProgress;
        }
    }
    
    float barWidth = 600;
    float currentWidth = barWidth * progress;
    progressBar.setSize(sf::Vector2f(currentWidth, 30));
    
    spinnerRotation += 360.0f * deltaTime;
    if (spinnerRotation > 360.0f) spinnerRotation -= 360.0f;
    spinner.setRotation(spinnerRotation);
    
    float pulse = (std::sin(animationClock.getElapsedTime().asSeconds() * 3.0f) + 1.0f) * 0.5f;
    sf::Uint8 green = static_cast<sf::Uint8>(150 + pulse * 105);
    progressBar.setFillColor(sf::Color(0, green, 0));
}

void LoadingScreen::draw() {
    window.clear(sf::Color(20, 20, 40));
    window.draw(loadingText);
    window.draw(progressBarBackground);
    window.draw(progressBar);
    window.draw(statusText);
    if (progress < 0.99f) {
        window.draw(spinner);
    }
    window.display();
}

bool LoadingScreen::isComplete() const {
    return progress >= 0.99f && targetProgress >= 0.99f;
}

void LoadingScreen::reset() {
    progress = 0.0f;
    targetProgress = 0.0f;
    statusText.setString("");
}

}
