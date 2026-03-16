#include "WinScreen.h"
#include "ResourceManager.h"
#include "GamePlayer.h"
#include <iostream>

extern Player player;

namespace DefenseSynth {

WinScreen::WinScreen(sf::RenderWindow &window) : Screen(window)
{
    auto& rm = ResourceManager::getInstance();
    rm.loadTextureWithRetry("assets/images/winscreen.png", "winscreen", 1);
    background.setTexture(rm.getTexture("winscreen"));
    
    bool fontOk = false;
    if (rm.loadFont("assets/fonts/new.ttf", "font_new")) { font = rm.getFont("font_new"); fontOk = true; }
    else if (rm.loadFont("assets/fonts/arial.ttf", "font_arial")) { font = rm.getFont("font_arial"); fontOk = true; }
    else { std::cout << "ERROR LOADING FONT" << std::endl; }
    
    playerName = "";
    nameEntered = false;
    
    winText.setFont(font);
    winText.setCharacterSize(48);
    winText.setFillColor(sf::Color::Green);
    winText.setString("You win");
    winText.setOrigin(winText.getLocalBounds().width / 2, winText.getLocalBounds().height / 2);
    winText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, GameConstants::BATTLEFIELD_HEIGHT / 2 - 80);
    
    namePromptText.setFont(font);
    namePromptText.setCharacterSize(28);
    namePromptText.setFillColor(sf::Color::White);
    namePromptText.setString("Enter Your name");
    namePromptText.setOrigin(namePromptText.getLocalBounds().width / 2, namePromptText.getLocalBounds().height / 2);
    namePromptText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, GameConstants::BATTLEFIELD_HEIGHT / 2 - 20);
    
    enterConfirmText.setFont(font);
    enterConfirmText.setCharacterSize(22);
    enterConfirmText.setFillColor(sf::Color::Yellow);
    enterConfirmText.setString("Press Enter to confirm");
    enterConfirmText.setOrigin(enterConfirmText.getLocalBounds().width / 2, enterConfirmText.getLocalBounds().height / 2);
    enterConfirmText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, GameConstants::BATTLEFIELD_HEIGHT / 2 + 40);
    
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::Cyan);
    scoreText.setString("Score: " + std::to_string(player.score));
    scoreText.setOrigin(scoreText.getLocalBounds().width / 2, scoreText.getLocalBounds().height / 2);
    scoreText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, GameConstants::BATTLEFIELD_HEIGHT / 2 + 100);
}

void WinScreen::renderScreen(sf::RenderWindow &window, std::string &currentScreen)
{
    SoundManager::getInstance().loopSound("dancer.wav", false);
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            window.close();
        }
        if (event.type == sf::Event::TextEntered && !nameEntered)
        {
            if (event.text.unicode == 8 && !playerName.empty())
            {
                playerName.pop_back();
            }
            else if (event.text.unicode >= 32 && event.text.unicode < 127 && playerName.length() < 20)
            {
                playerName += static_cast<char>(event.text.unicode);
            }
            namePromptText.setString("Enter Your name: " + playerName);
            namePromptText.setOrigin(namePromptText.getLocalBounds().width / 2, namePromptText.getLocalBounds().height / 2);
            namePromptText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, GameConstants::BATTLEFIELD_HEIGHT / 2 - 20);
        }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter && !playerName.empty())
        {
            SoundManager::getInstance().playSound("buttonclick.wav");
            StateManager::getInstance()->endBattleSession();
            currentScreen = "mainMenu";
        }
    }
    window.clear();
    window.draw(background);
    window.draw(winText);
    window.draw(namePromptText);
    if (!playerName.empty())
    {
        window.draw(enterConfirmText);
    }
    window.draw(scoreText);
    window.display();
}

}
