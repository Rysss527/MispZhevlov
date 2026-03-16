#include "TutorialScreen.h"
#include "ResourceManager.h"

namespace DefenseSynth {

TutorialScreen::TutorialScreen(sf::RenderWindow &window) : Screen(window)
{
    auto& rm = ResourceManager::getInstance();
    background.setTexture(rm.getTexture("tutorial"));
}

void TutorialScreen::renderScreen(sf::RenderWindow &window, std::string &currentScreen)
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            window.close();
        }
        else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            float mouseX = static_cast<float>(event.mouseButton.x);
            float mouseY = static_cast<float>(event.mouseButton.y);
            sf::Vector2u windowSize = window.getSize();
            
            float scaleX = windowSize.x / UIConstants::BASE_WINDOW_WIDTH;
            float scaleY = windowSize.y / UIConstants::BASE_WINDOW_HEIGHT;

            if (mouseX >= UIConstants::Navigation::BACK_BUTTON_LEFT * scaleX && 
                mouseX <= UIConstants::Navigation::BACK_BUTTON_RIGHT * scaleX && 
                mouseY >= UIConstants::Navigation::BACK_BUTTON_TOP * scaleY && 
                mouseY <= UIConstants::Navigation::BACK_BUTTON_BOTTOM * scaleY)
            {
                SoundManager::getInstance().playSound("buttonclick.wav");
                currentScreen = "mainMenu";
            }
        }
    }
    window.clear();
    window.draw(background);
    window.display();
}

}
