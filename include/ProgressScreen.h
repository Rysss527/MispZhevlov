#pragma once

#include "ScreenBase.h"
#include "Constants.h"

namespace DefenseSynth {

class ProgressScreen : public Screen
{
public:
    sf::Clock clock;
    
    ProgressScreen(sf::RenderWindow &window);
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen) override;
};

}
