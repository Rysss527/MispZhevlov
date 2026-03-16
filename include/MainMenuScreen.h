#pragma once

#include "ScreenBase.h"
#include "SoundManager.h"
#include "StateManager.h"
#include "CrystalManager.h"
#include "Constants.h"
#include <vector>
#include <string>

namespace DefenseSynth {

class MainMenuScreen : public Screen
{
private:
    static inline std::vector<std::string> menuTracks;
    static inline int currentTrack = 0;
    sf::Font font;
    sf::Text playButtonText;
    sf::Text exitButtonText;

public:
    MainMenuScreen(sf::RenderWindow &window);
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen) override;
};

}