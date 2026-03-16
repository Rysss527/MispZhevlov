#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "Constants.h"

namespace DefenseSynth {

class Screen
{
public:
    sf::Texture bgImg;
    sf::Sprite background;
    
    Screen(sf::RenderWindow &window);
    virtual void renderScreen(sf::RenderWindow &window, std::string &currentScreen) = 0;
    virtual ~Screen() {}
};

}
