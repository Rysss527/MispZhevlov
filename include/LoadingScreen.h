#pragma once
#include <SFML/Graphics.hpp>
#include <string>

namespace DefenseSynth {

class LoadingScreen {
private:
    sf::RenderWindow& window;
    sf::RectangleShape progressBar;
    sf::RectangleShape progressBarBackground;
    sf::Text loadingText;
    sf::Text statusText;
    sf::Font font;
    float progress;
    float targetProgress;
    sf::CircleShape spinner;
    float spinnerRotation;
    sf::Clock animationClock;
    
public:
    LoadingScreen(sf::RenderWindow& win);
    
    void setProgress(float value);
    void setStatus(const std::string& status);
    void update(float deltaTime);
    void draw();
    bool isComplete() const;
    void reset();
};

}
