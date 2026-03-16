#pragma once

#include "ScreenBase.h"
#include "SoundManager.h"
#include "Constants.h"
#include <fstream>
#include <iostream>

namespace DefenseSynth {

class RankingScreen : public Screen
{
private:
    sf::Font font;
    sf::Text highScoresText;

public:
    RankingScreen(sf::RenderWindow& window);
    void renderScreen(sf::RenderWindow& window, std::string& currentScreen) override;
};

}
