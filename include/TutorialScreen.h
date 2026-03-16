#pragma once

#include "ScreenBase.h"
#include "SoundManager.h"
#include "Constants.h"

namespace DefenseSynth {

class TutorialScreen : public Screen
{
public:
    TutorialScreen(sf::RenderWindow &window);
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen) override;
};

}
