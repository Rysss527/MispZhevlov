#include "GameOverScreen.h"
#include "ResourceManager.h"
#include <iostream>

namespace DefenseSynth {

GameOverScreen::GameOverScreen(sf::RenderWindow &window) : Screen(window)
{
    auto& rm = ResourceManager::getInstance();
    rm.loadTextureWithRetry("assets/images/endscreen.png", "endscreen", 1);
    background.setTexture(rm.getTexture("endscreen"));
    
    bool fontOk = false;
    if (rm.loadFont("assets/fonts/new.ttf", "font_new")) { font = rm.getFont("font_new"); fontOk = true; }
    else if (rm.loadFont("assets/fonts/arial.ttf", "font_arial")) { font = rm.getFont("font_arial"); fontOk = true; }
    else { std::cout << "ERROR LOADING FONT" << std::endl; }
    
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(UIConstants::GameOver::GAME_OVER_FONT_SIZE);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setString("THE ZOMBIES ATE YOUR BRAINS!");
    gameOverText.setOrigin(gameOverText.getLocalBounds().width / 2, gameOverText.getLocalBounds().height / 2);
    gameOverText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, 
                           GameConstants::BATTLEFIELD_HEIGHT / 2 + UIConstants::GameOver::GAME_OVER_Y_OFFSET);

    scoreText.setFont(font);
    scoreText.setCharacterSize(UIConstants::GameOver::SCORE_FONT_SIZE);
    scoreText.setFillColor(sf::Color::Yellow);
    scoreText.setPosition(UIConstants::GameOver::SCORE_X, 
                         GameConstants::BATTLEFIELD_HEIGHT / 2 + UIConstants::GameOver::SCORE_Y_OFFSET);

    instructionText.setFont(font);
    instructionText.setCharacterSize(UIConstants::GameOver::INSTRUCTION_FONT_SIZE);
    instructionText.setFillColor(sf::Color::White);
    instructionText.setString("Click anywhere to return to main menu");
    instructionText.setOrigin(instructionText.getLocalBounds().width / 2, instructionText.getLocalBounds().height / 2);
    instructionText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, 
                              GameConstants::BATTLEFIELD_HEIGHT / 2 + UIConstants::GameOver::INSTRUCTION_Y_OFFSET);
    
    
}

void GameOverScreen::renderScreen(sf::RenderWindow &window, std::string &currentScreen)
{
    SoundManager::getInstance().loopSound("dancer.wav", false);
    GameContext* context = GameContext::getInstance();
    scoreText.setString(std::to_string(context->getPlayer().score));
    scoreText.setPosition(UIConstants::GameOver::SCORE_X, 
                         GameConstants::BATTLEFIELD_HEIGHT / 2 + UIConstants::GameOver::SCORE_Y_OFFSET);
    
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            window.close();
        }
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            SoundManager::getInstance().playSound("buttonclick.wav");
            StateManager::getInstance()->endBattleSession();
            SoundManager::getInstance().setPlaylist(UIConstants::Audio::MENU_TRACKS);
            currentScreen = "mainMenu";
        }
    }
    window.clear();
    window.draw(background);
    window.draw(gameOverText);
    window.draw(scoreText);
    window.draw(instructionText);
    window.display();
}

}
