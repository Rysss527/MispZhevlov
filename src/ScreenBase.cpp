#include "ScreenBase.h"
#include "ResourceManager.h"

namespace DefenseSynth {

Screen::Screen(sf::RenderWindow &window)
{
    auto& rm = ResourceManager::getInstance();
    rm.loadTextureWithRetry("assets/images/background.png", "background", 1);
    background.setTexture(rm.getTexture("background"));
    background.setTextureRect(sf::IntRect(0, 0, GameConstants::BATTLEFIELD_WIDTH, GameConstants::BATTLEFIELD_HEIGHT));
    window.setVerticalSyncEnabled(true);
}

}
