#include "ProgressScreen.h"
#include "ResourceManager.h"

namespace DefenseSynth {

ProgressScreen::ProgressScreen(sf::RenderWindow &window) : Screen(window)
{
    auto& rm = ResourceManager::getInstance();
    rm.loadTextureWithRetry("assets/images/levelup.png", "levelup", 1);
    background.setTexture(rm.getTexture("levelup"));
}

void ProgressScreen::renderScreen(sf::RenderWindow &window, std::string &currentScreen)
{
    if (clock.getElapsedTime().asSeconds() == 0) {
        clock.restart();
    }

    if (clock.getElapsedTime().asSeconds() > UIConstants::LevelUp::DISPLAY_TIME_SECONDS)
    {
        currentScreen = "battlefield";
        clock.restart();
        return;
    }
    
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            window.close();
        }
    }
    window.clear();
    window.draw(background);
    window.display();
}

}
