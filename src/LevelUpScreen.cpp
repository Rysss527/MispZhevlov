#include "LevelUpScreen.h"
#include "ResourceManager.h"
#include <iostream>

namespace DefenseSynth {

LevelUpScreen::LevelUpScreen(sf::RenderWindow &window) : Screen(window)
{
    auto& rm = ResourceManager::getInstance();
    rm.loadTextureWithRetry("assets/images/levelup.png", "levelup", 1);
    background.setTexture(rm.getTexture("levelup"));
    
    bool fontOk = false;
    if (rm.loadFont("assets/fonts/new.ttf", "font_new")) { font = rm.getFont("font_new"); fontOk = true; }
    else if (rm.loadFont("assets/fonts/arial.ttf", "font_arial")) { font = rm.getFont("font_arial"); fontOk = true; }
    else { std::cout << "ERROR LOADING FONT" << std::endl; }
    
    levelCompleteText.setFont(font);
    levelCompleteText.setString("YOU WIN!");
    levelCompleteText.setCharacterSize(UIConstants::LevelUp::LEVEL_COMPLETE_FONT_SIZE);
    levelCompleteText.setFillColor(sf::Color::Green);
    levelCompleteText.setOrigin(levelCompleteText.getLocalBounds().width / 2, levelCompleteText.getLocalBounds().height / 2);
    levelCompleteText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, GameConstants::BATTLEFIELD_HEIGHT / 2);
}

void LevelUpScreen::renderScreen(sf::RenderWindow &window, std::string &currentScreen)
{
    clock.restart();
    while (clock.getElapsedTime().asSeconds() < UIConstants::LevelUp::DISPLAY_TIME_SECONDS) 
    {
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
        window.draw(levelCompleteText);
        window.display();
    }
    
    StateManager* sm = StateManager::getInstance();
    int currentLevel = sm->getLevel();
    
    if (currentLevel == 1 && !sm->isCardUnlocked("rapidshooter")) {
        sm->unlockCard("rapidshooter");
    }
    if (currentLevel == 2 && !sm->isCardUnlocked("freezetower")) {
        sm->unlockCard("freezetower");
    }
    if (currentLevel == 3 && !sm->isCardUnlocked("bombtower")) {
        sm->unlockCard("bombtower");
    }
    if (currentLevel == 4 && !sm->isCardUnlocked("areaattacktower")) {
        sm->unlockCard("areaattacktower");
    }
    
    LoadoutScreen::setCurrentLevel(currentLevel);
    currentScreen = "loadout"; 
}

}
