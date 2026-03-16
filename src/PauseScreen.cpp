#include "PauseScreen.h"
#include "ResourceManager.h"
#include "SoundManager.h"
#include "Constants.h"

namespace DefenseSynth {

PauseScreen::PauseScreen(sf::RenderWindow &window) : Screen(window)
{
    auto& rm = ResourceManager::getInstance();
    rm.loadTextureWithRetry("assets/images/pausesccreen.png", "pausescreen", 1);
    background.setTexture(rm.getTexture("pausescreen"));
}

void PauseScreen::renderScreen(sf::RenderWindow &window, std::string &currentScreen)
{
    SoundManager::getInstance().loopSound("dancer.wav", false);
    static bool pauseInitialized = false;
    static std::string prevBattleTrack;
    static float prevBattleOffsetSeconds;
    
    if (!pauseInitialized) {
        prevBattleTrack = SoundManager::getInstance().getCurrentTrack();
        prevBattleOffsetSeconds = SoundManager::getInstance().getCurrentOffsetSeconds();
        SoundManager::getInstance().setPlaylist(UIConstants::Audio::PAUSE_TRACKS, true);
        pauseInitialized = true;
    }
    
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            pauseInitialized = false;
            currentScreen = "exit";
            return;
        }
        if (event.type == sf::Event::KeyPressed &&
            (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::P || event.key.code == sf::Keyboard::Space))
        {
            SoundManager::getInstance().setPlaylistStartingAt(UIConstants::Audio::BATTLE_TRACKS, prevBattleTrack, prevBattleOffsetSeconds);
            pauseInitialized = false;
            currentScreen = "battlefield";
            return;
        }
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            float mouseX = static_cast<float>(event.mouseButton.x);
            float mouseY = static_cast<float>(event.mouseButton.y);
            sf::Vector2u windowSize = window.getSize();
            
            float scaleX = windowSize.x / UIConstants::BASE_WINDOW_WIDTH;
            float scaleY = windowSize.y / UIConstants::BASE_WINDOW_HEIGHT;
            
            if ((mouseX >= UIConstants::PauseScreen::RESUME_BUTTON_LEFT * scaleX && 
                 mouseX <= UIConstants::PauseScreen::RESUME_BUTTON_RIGHT * scaleX) && 
                (mouseY >= UIConstants::PauseScreen::RESUME_BUTTON_TOP * scaleY && 
                 mouseY <= UIConstants::PauseScreen::RESUME_BUTTON_BOTTOM * scaleY))
            {
                SoundManager::getInstance().playSound("buttonclick.wav");
                SoundManager::getInstance().setPlaylistStartingAt(UIConstants::Audio::BATTLE_TRACKS, prevBattleTrack, prevBattleOffsetSeconds);
                pauseInitialized = false;
                currentScreen = "battlefield";
                return;
            }
            else if ((mouseX >= UIConstants::PauseScreen::MAIN_MENU_BUTTON_LEFT * scaleX && 
                      mouseX <= UIConstants::PauseScreen::MAIN_MENU_BUTTON_RIGHT * scaleX) && 
                     (mouseY >= UIConstants::PauseScreen::MAIN_MENU_BUTTON_TOP * scaleY && 
                      mouseY <= UIConstants::PauseScreen::MAIN_MENU_BUTTON_BOTTOM * scaleY))
            {
                SoundManager::getInstance().playSound("buttonclick.wav");
                SoundManager::getInstance().setPlaylist(UIConstants::Audio::MENU_TRACKS);
                StateManager::getInstance()->endBattleSession();
                pauseInitialized = false;
                currentScreen = "mainMenu";
                return;
            }
        }
    }
    SoundManager::getInstance().updateMusic();
    window.clear();
    window.draw(background);
    window.display();
}

}
