#pragma once

#include "ScreenBase.h"
#include "GameContext.h"
#include "SoundManager.h"
#include "StateManager.h"
#include "Constants.h"

namespace DefenseSynth {

class GameOverScreen : public Screen
{
public:
    sf::Text gameOverText;
    sf::Text scoreText;
    sf::Text instructionText;
    sf::Font font;

    GameOverScreen(sf::RenderWindow &window);
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen) override;
};

}
