#include "MainMenuScreen.h"
#include "ResourceManager.h"

namespace DefenseSynth {

MainMenuScreen::MainMenuScreen(sf::RenderWindow &window) : Screen(window)
{
    auto& rm = ResourceManager::getInstance();
    background.setTexture(rm.getTexture("mainmenu"));
    
    const sf::Font& fontRef = rm.getFont("arial");
    font = fontRef;
    
    playButtonText.setFont(font);
    playButtonText.setCharacterSize(UIConstants::MainMenu::PLAY_BUTTON_FONT_SIZE);
    playButtonText.setFillColor(sf::Color::White);
    playButtonText.setStyle(sf::Text::Bold);
    
    exitButtonText.setFont(font);
    exitButtonText.setString("Exit");
    exitButtonText.setCharacterSize(UIConstants::MainMenu::EXIT_BUTTON_FONT_SIZE);
    exitButtonText.setFillColor(sf::Color::Red);
    exitButtonText.setStyle(sf::Text::Bold);

    if (menuTracks.empty()) {
        menuTracks = UIConstants::Audio::MENU_TRACKS;
    }
    
    SoundManager::getInstance().setPlaylist(menuTracks);
}

void MainMenuScreen::renderScreen(sf::RenderWindow &window, std::string &currentScreen)
{
    SoundManager::getInstance().loopSound("dancer.wav", false);
    SoundManager& audio = SoundManager::getInstance();
    std::string currentTrack = audio.getCurrentTrack();
    bool inMenu = false;
    for (const auto& t : menuTracks) { if (t == currentTrack) { inMenu = true; break; } }
    if (!inMenu) { audio.setPlaylist(menuTracks); }
    audio.updateMusic();
    
    StateManager* sm = StateManager::getInstance();
    sm->load();
    
    bool hasActiveSave = sm->isBattleSessionActive();
    playButtonText.setString(hasActiveSave ? "Resume" : "Play");
    
    sf::Vector2u windowSize = window.getSize();
    float scaleX = windowSize.x / UIConstants::BASE_WINDOW_WIDTH;
    float scaleY = windowSize.y / UIConstants::BASE_WINDOW_HEIGHT;
    float buttonCenterX = (UIConstants::MainMenu::PLAY_BUTTON_LEFT * scaleX + UIConstants::MainMenu::PLAY_BUTTON_RIGHT * scaleX) / 2.0f;
    float buttonCenterY = (UIConstants::MainMenu::PLAY_BUTTON_TOP * scaleY + UIConstants::MainMenu::PLAY_BUTTON_BOTTOM * scaleY) / 2.0f;
    
    sf::FloatRect textBounds = playButtonText.getLocalBounds();
    playButtonText.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
    playButtonText.setPosition(buttonCenterX, buttonCenterY);
    
    sf::FloatRect exitBounds = exitButtonText.getLocalBounds();
    exitButtonText.setOrigin(exitBounds.width, exitBounds.height);
    exitButtonText.setPosition(windowSize.x - UIConstants::MainMenu::EXIT_BUTTON_MARGIN * scaleX, 
                              windowSize.y - UIConstants::MainMenu::EXIT_BUTTON_MARGIN * scaleY);

    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            currentScreen = "exit";
            return;
        }
        else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            float mouseX = static_cast<float>(event.mouseButton.x);
            float mouseY = static_cast<float>(event.mouseButton.y);
            
            if (mouseX >= UIConstants::MainMenu::PLAY_BUTTON_LEFT * scaleX && 
                mouseX <= UIConstants::MainMenu::PLAY_BUTTON_RIGHT * scaleX && 
                mouseY >= UIConstants::MainMenu::PLAY_BUTTON_TOP * scaleY && 
                mouseY <= UIConstants::MainMenu::PLAY_BUTTON_BOTTOM * scaleY)
            {
                SoundManager::getInstance().playSound("buttonclick.wav");
                
                if (hasActiveSave) {
                    StateManager::getInstance()->load();
                    currentScreen = "battlefield";
                } else {
                    StateManager* sm = StateManager::getInstance();
                    CrystalManager& cm = CrystalManager::getInstance();
                    
                    sm->reset();
                    cm.reset();
                    
                    sm->save();
                    currentScreen = "loadout";
                }
            }
            else if (mouseX >= UIConstants::MainMenu::PLAY_BUTTON_LEFT * scaleX && 
                     mouseX <= UIConstants::MainMenu::PLAY_BUTTON_RIGHT * scaleX && 
                     mouseY >= UIConstants::MainMenu::TUTORIAL_BUTTON_TOP * scaleY && 
                     mouseY <= UIConstants::MainMenu::TUTORIAL_BUTTON_BOTTOM * scaleY)
            {
                SoundManager::getInstance().playSound("buttonclick.wav");
                currentScreen = "tutorial";
            }
            else if (mouseX >= UIConstants::MainMenu::PLAY_BUTTON_LEFT * scaleX && 
                     mouseX <= UIConstants::MainMenu::PLAY_BUTTON_RIGHT * scaleX && 
                     mouseY >= UIConstants::MainMenu::RANKINGS_BUTTON_TOP * scaleY && 
                     mouseY <= UIConstants::MainMenu::RANKINGS_BUTTON_BOTTOM * scaleY)
            {
                SoundManager::getInstance().playSound("buttonclick.wav");
                currentScreen = "rankings";
            }
            else if (mouseX >= windowSize.x - UIConstants::MainMenu::EXIT_BUTTON_SIZE * scaleX && 
                     mouseX <= windowSize.x - 10 * scaleX && 
                     mouseY >= windowSize.y - 60 * scaleY && 
                     mouseY <= windowSize.y - 10 * scaleY)
            {
                window.close();
            }
        }
    }

    window.clear();
    window.draw(background);
    window.draw(exitButtonText);
    window.display();
}

}