#pragma once

#include "ScreenBase.h"
#include "SoundManager.h"
#include "StateManager.h"
#include "Constants.h"

namespace DefenseSynth {

class WinScreen : public Screen
{
public:
    sf::Text winText;
    sf::Text namePromptText;
    sf::Text enterConfirmText;
    sf::Text scoreText;
    sf::Font font;
    std::string playerName;
    bool nameEntered;
    
    WinScreen(sf::RenderWindow &window);
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen) override;
};

}
