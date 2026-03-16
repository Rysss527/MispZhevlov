#include "RankingScreen.h"
#include "ResourceManager.h"

namespace DefenseSynth {

RankingScreen::RankingScreen(sf::RenderWindow& window) : Screen(window)
{
    auto& rm = ResourceManager::getInstance();
    background.setTexture(rm.getTexture("rankings"));
    
    const sf::Font& fontRef = rm.getFont("arial");
    font = fontRef;
    
    highScoresText.setFont(font);
    highScoresText.setCharacterSize(UIConstants::Rankings::HIGHSCORES_FONT_SIZE);
    highScoresText.setFillColor(sf::Color::White);
    highScoresText.setPosition(UIConstants::Rankings::HIGHSCORES_X, UIConstants::Rankings::HIGHSCORES_Y);
}

void RankingScreen::renderScreen(sf::RenderWindow& window, std::string& currentScreen)
{
    std::ifstream highScoresFile("highscores.txt");
    std::string highScoresStr;
    if (highScoresFile.is_open())
    {
        std::string line;
        int count = 0;
        while (std::getline(highScoresFile, line) && count < UIConstants::Rankings::MAX_DISPLAYED_SCORES)
        {
            size_t spacePos = line.find_last_of(' ');
            if (spacePos != std::string::npos) {
                std::string name = line.substr(0, spacePos);
                std::string score = line.substr(spacePos + 1);
                highScoresStr += name + " Score: " + score + "\n";
            } else {
                highScoresStr += line + "\n";
            }
            count++;
        }
        highScoresFile.close();
    }
    else
    {
        highScoresStr = "No highscores yet!";
    }
    highScoresText.setString(highScoresStr);

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
    window.draw(highScoresText);
    window.display();
}

} 
