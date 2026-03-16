#pragma once

#include "ScreenBase.h"
#include "SoundManager.h"
#include "StateManager.h"
#include "Constants.h"

namespace DefenseSynth {

class PauseScreen : public Screen
{
public:
    PauseScreen(sf::RenderWindow &window);
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen) override;
private:
    std::string prevBattleTrack;
    float prevBattleOffsetSeconds = 0.0f;
};

}
