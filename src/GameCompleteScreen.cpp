#include "GameCompleteScreen.h"
#include "ResourceManager.h"
#include <iostream>

namespace DefenseSynth {

GameCompleteScreen::GameCompleteScreen(sf::RenderWindow &window) : Screen(window)
{
    auto& rm = ResourceManager::getInstance();
    rm.loadTextureWithRetry("assets/images/endscreen.png", "endscreen", 1);
    background.setTexture(rm.getTexture("endscreen"));
    namingMode = true;
    
    bool fontOk = false;
    if (rm.loadFont("assets/fonts/new.ttf", "font_new")) { font = rm.getFont("font_new"); fontOk = true; }
    else if (rm.loadFont("assets/fonts/arial.ttf", "font_arial")) { font = rm.getFont("font_arial"); fontOk = true; }
    else { std::cout << "ERROR LOADING FONT" << std::endl; }
    
    youWonText.setFont(font);
    youWonText.setCharacterSize(UIConstants::GameComplete::WIN_TEXT_FONT_SIZE);
    youWonText.setFillColor(sf::Color::Yellow);
    youWonText.setString("YOU WON!");
    youWonText.setOrigin(youWonText.getLocalBounds().width / 2, youWonText.getLocalBounds().height / 2);
    youWonText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, UIConstants::GameComplete::WIN_TEXT_Y);
    
    namePromptText.setFont(font);
    namePromptText.setCharacterSize(UIConstants::GameComplete::NAME_PROMPT_FONT_SIZE);
    namePromptText.setFillColor(sf::Color::White);
    namePromptText.setString("Enter your name:");
    namePromptText.setPosition(UIConstants::GameComplete::NAME_PROMPT_X, UIConstants::GameComplete::NAME_PROMPT_Y);
    
    inputText.setFont(font);
    inputText.setCharacterSize(UIConstants::GameComplete::INPUT_FONT_SIZE);
    inputText.setFillColor(sf::Color::Green);
    inputText.setString("");
    inputText.setPosition(UIConstants::GameComplete::INPUT_X, UIConstants::GameComplete::INPUT_Y);
    
    instructionText.setFont(font);
    instructionText.setCharacterSize(UIConstants::GameComplete::INSTRUCTION_FONT_SIZE);
    instructionText.setFillColor(sf::Color::White);
    instructionText.setString("Press ENTER to confirm");
    instructionText.setOrigin(instructionText.getLocalBounds().width / 2, instructionText.getLocalBounds().height / 2);
    instructionText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, UIConstants::GameComplete::INSTRUCTION_Y);
    
    SoundManager::getInstance().playMusic("winmusic.wav", false);
}

void GameCompleteScreen::renderScreen(sf::RenderWindow &window, std::string &currentScreen)
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            currentScreen = "exit";
            return;
        }
        if (event.type == sf::Event::TextEntered && namingMode)
        {
            if (event.text.unicode == 8 && !playerName.empty())
            {
                playerName.pop_back();
            }
            else if (event.text.unicode >= 32 && event.text.unicode <= 126 && 
                     playerName.length() < UIConstants::GameComplete::MAX_NAME_LENGTH)
            {
                playerName += static_cast<char>(event.text.unicode);
            }
            inputText.setString(playerName);
            inputText.setPosition(UIConstants::GameComplete::INPUT_X, UIConstants::GameComplete::INPUT_Y);
        }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter && namingMode)
        {
            if (!playerName.empty())
            {
                std::ofstream scoreFile("highscores.txt", std::ios::app);
                if (scoreFile.is_open())
                {
                    GameContext* context = GameContext::getInstance();
                    scoreFile << playerName << " " << context->getPlayer().score << "\n";
                    scoreFile.close();
                }
                namingMode = false;
                SoundManager::getInstance().playSound("buttonclick.wav");
                SoundManager::getInstance().stopMusic();
                SoundManager::getInstance().setPlaylist(UIConstants::Audio::MENU_TRACKS);
                currentScreen = "mainMenu";
            }
        }
    }
    
    window.clear();
    window.draw(background);
    window.draw(youWonText);
    if (namingMode)
    {
        window.draw(namePromptText);
        window.draw(inputText);
        window.draw(instructionText);
    }
    window.display();
}

} 
