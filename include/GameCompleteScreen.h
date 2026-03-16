#pragma once

#include "ScreenBase.h"
#include "GameContext.h"
#include "SoundManager.h"
#include "Constants.h"
#include <fstream>

namespace DefenseSynth {

class GameCompleteScreen : public Screen
{
public:
    sf::Text youWonText;
    sf::Text namePromptText;
    sf::Text inputText;
    sf::Text instructionText;
    sf::Font font;
    std::string playerName;
    bool namingMode;
    
    GameCompleteScreen(sf::RenderWindow &window);
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen) override;
};

}
