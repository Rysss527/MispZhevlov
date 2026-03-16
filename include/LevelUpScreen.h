#pragma once

#include "ScreenBase.h"
#include "StateManager.h"
#include "LoadoutScreen.h"
#include "Constants.h"

namespace DefenseSynth {

class LevelUpScreen : public Screen
{
public:
    sf::Clock clock;
    sf::Font font;
    sf::Text levelCompleteText;
    
    LevelUpScreen(sf::RenderWindow &window);
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen) override;
};

}
