#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <cstdio>
#include "CardCollection.h"
#include "Constants.h"
#include "SpriteScaler.h"
#include "UITheme.h"
#include "CrystalManager.h"
#include "StateManager.h"



class LoadoutScreen {
private:
    sf::Texture bgImg;
    sf::Sprite background;
    std::vector<std::string> availableUnits;
    std::vector<std::string> selectedUnits;
    int MAX_SELECTED;
    
    sf::RectangleShape sunUpgradeCard;
    sf::RectangleShape crystalUpgradeCard;
    sf::Sprite sunUpgradeSprite;
    sf::Sprite crystalUpgradeSprite;
    sf::Texture sunUpgradeTexture;
    sf::Texture crystalUpgradeTexture;
    sf::Text sunUpgradeText;
    sf::Text crystalUpgradeText; 
    sf::Text sunUpgradeTextTop;
    sf::Text sunUpgradeTextBottom;
    sf::Text crystalUpgradeTextTop;
    sf::Text crystalUpgradeTextBottom;
    sf::Font font;
    sf::Text titleText;
    sf::Text instructionText;
    sf::Text upgradeInstructionText;
    sf::Text plantUpgradeHeaderText;
    sf::Text resourceUpgradeHeaderText;
    sf::Text difficultyHeaderText;
    std::vector<sf::RectangleShape> unitCards;
    std::vector<sf::Text> unitTexts;
    std::vector<std::vector<sf::Text>> unitLineTexts;
    std::vector<sf::Sprite> unitSprites;
    std::vector<sf::Texture> unitTextures;
    sf::RectangleShape startButton;
    sf::Text startButtonText;
    sf::RectangleShape backButton;
    sf::Text backButtonText;
    sf::RenderWindow& window;
    CardCollection& cardCollection;
    
    sf::Text crystalCountText;
    std::vector<sf::Text> upgradeCostTexts;
    sf::Text crystalRewardText;
    sf::Text sunCostText;
    sf::Text crystalCostText;
    
    std::vector<sf::RectangleShape> difficultyButtons;
    std::vector<sf::Text> difficultyButtonTexts;
    sf::Text difficultyLabel;
    sf::Text difficultyValueText;
    int selectedDifficultyIndex;
    std::vector<std::string> difficultyNames;
    std::vector<float> difficultyValues;
    
    inline static int sCurrentLevel = 1;
    
    std::string getImagePath(const std::string& unitType) {
        if (unitType == "basicshooter") return "assets/images/peeshooter.png";
        if (unitType == "resourcegenerator") return "assets/images/sunflower.png";
        if (unitType == "barriertower") return "assets/images/wallnut.png";
        if (unitType == "rapidshooter") return "assets/images/repeater.png";
        if (unitType == "freezetower") return "assets/images/snowpea.png";
        if (unitType == "bombtower") return "assets/images/cherrybomb.png";
        if (unitType == "areaattacktower") return "assets/images/fumeshroom.png";
        if (unitType == "sun_upgrade") return "assets/images/sun.png";
        if (unitType == "crystal_upgrade") return "assets/images/crystal.png";
        
        return "assets/images/plant.png";
    }
    
public:
    static void setCurrentLevel(int lvl) { sCurrentLevel = lvl; }
    static int getCurrentLevel() { return sCurrentLevel; }
    
    LoadoutScreen(sf::RenderWindow& window, CardCollection& cards) : window(window), cardCollection(cards) {
        StateManager* sm = StateManager::getInstance();
        int currentLevel = sm->getLevel();
        int calculatedMax = StateManager::calculateMaxSelection(currentLevel);
        sm->setMaxSelection(calculatedMax);
        MAX_SELECTED = calculatedMax;
        
        sCurrentLevel = currentLevel;
        
        selectedUnits = StateManager::getInstance()->getSelectedUnits();
        
        #ifdef _DEBUG
        std::cout << "LoadoutScreen constructor - Loaded " << selectedUnits.size() << " selected units from StateManager:" << std::endl;
        for (const auto& unit : selectedUnits) {
            std::cout << "  - " << unit << std::endl;
        }
        #endif
        
        if (!bgImg.loadFromFile("assets/images/background.png")) {
            bgImg.create(GameConstants::BATTLEFIELD_WIDTH, GameConstants::BATTLEFIELD_HEIGHT);
            sf::Image brownBackground;
            brownBackground.create(GameConstants::BATTLEFIELD_WIDTH, GameConstants::BATTLEFIELD_HEIGHT, sf::Color(139, 69, 19));
            bgImg.loadFromImage(brownBackground);
        }
        bgImg.setSmooth(false);
        background.setTexture(bgImg);
        loadAvailableUnits();
        
        #ifdef _DEBUG
        std::cout << "Loading LoadoutScreen..." << std::endl;
        std::cout << "Level: " << currentLevel << ", MaxSelection: " << MAX_SELECTED << std::endl;
        std::cout << "Available units count: " << availableUnits.size() << std::endl;
        #endif
        
        if (!font.loadFromFile("arial.ttf")) {
            if (!font.loadFromFile("new.ttf")) {
                font.loadFromFile("assets/fonts/arial.ttf");
            }
        }
        
        titleText.setFont(font);
        titleText.setString("Choose units for battle");
        titleText.setCharacterSize(UITheme::TITLE_SIZE);
        titleText.setFillColor(UITheme::PRIMARY_TEXT);
        
        instructionText.setFont(font);
        instructionText.setCharacterSize(UITheme::BODY_SIZE);
        instructionText.setFillColor(UITheme::SECONDARY_TEXT);
        
        upgradeInstructionText.setFont(font);
        upgradeInstructionText.setString("Left-click to select, Right-click to upgrade");
        upgradeInstructionText.setCharacterSize(UITheme::BODY_SIZE);
        upgradeInstructionText.setFillColor(UITheme::ACCENT_TEXT);
        
        plantUpgradeHeaderText.setFont(font);
        plantUpgradeHeaderText.setString("Plant Upgrades");
        plantUpgradeHeaderText.setCharacterSize(UITheme::BODY_SIZE);
        plantUpgradeHeaderText.setFillColor(UITheme::PRIMARY_TEXT);
        plantUpgradeHeaderText.setStyle(sf::Text::Bold);
        plantUpgradeHeaderText.setPosition(50, 130);
        
        resourceUpgradeHeaderText.setFont(font);
        resourceUpgradeHeaderText.setString("Resource Upgrades");
        resourceUpgradeHeaderText.setCharacterSize(UITheme::BODY_SIZE);
        resourceUpgradeHeaderText.setFillColor(UITheme::PRIMARY_TEXT);
        resourceUpgradeHeaderText.setStyle(sf::Text::Bold);
        resourceUpgradeHeaderText.setPosition(950, 130);
        
        difficultyHeaderText.setFont(font);
        difficultyHeaderText.setString("Difficulty Settings");
        difficultyHeaderText.setCharacterSize(UITheme::BODY_SIZE);
        difficultyHeaderText.setFillColor(UITheme::PRIMARY_TEXT);
        difficultyHeaderText.setStyle(sf::Text::Bold);
        difficultyHeaderText.setPosition(50, 650);
        
        startButton.setSize(sf::Vector2f(200, 60));
        startButton.setFillColor(UITheme::BUTTON_PRIMARY);
        
        startButtonText.setFont(font);
        startButtonText.setString("Start Battle");
        startButtonText.setCharacterSize(UITheme::BUTTON_SIZE);
        startButtonText.setFillColor(UITheme::PRIMARY_TEXT);
        
        backButton.setSize(sf::Vector2f(150, 50));
        backButton.setFillColor(UITheme::BUTTON_DISABLED);
        
        backButtonText.setFont(font);
        backButtonText.setString("Back to Menu");
        backButtonText.setCharacterSize(UITheme::SMALL_SIZE);
        backButtonText.setFillColor(UITheme::PRIMARY_TEXT);
        
        crystalCountText.setFont(font);
        crystalCountText.setFillColor(sf::Color::Yellow);
        
        crystalRewardText.setFont(font);
        crystalRewardText.setFillColor(sf::Color::Yellow);
        
        selectedDifficultyIndex = sm->getDifficultyIndex();
        
        difficultyNames = {"Very Easy", "Easy", "Normal", "Hard", "Extreme"};
        difficultyValues = {0.3f, 0.6f, 1.0f, 2.0f, 4.0f};
        
        setupDifficultyButtons();
        
        difficultyLabel.setFont(font);
        difficultyLabel.setString("Difficulty:");
        difficultyLabel.setCharacterSize(UITheme::BODY_SIZE);
        difficultyLabel.setFillColor(UITheme::PRIMARY_TEXT);
        difficultyLabel.setPosition(50, 680);
        
        difficultyValueText.setFont(font);
        difficultyValueText.setCharacterSize(UITheme::BODY_SIZE);
        difficultyValueText.setFillColor(sf::Color::Yellow);
        updateDifficultyText();
        
        setupResourceUpgrades();
        setupUnitCards();
        updateUIText();
    }
    
    void setupResourceUpgrades() {
        CrystalManager& cm = CrystalManager::getInstance();
        
        sunUpgradeCard.setSize(sf::Vector2f(150, 200));
        sunUpgradeCard.setPosition(950, 200);
        
        int sunLevel = cm.getSunUpgradeLevel();
        if (sunLevel == 5) {
            sunUpgradeCard.setFillColor(sf::Color(60, 120, 60, 220));
            sunUpgradeCard.setOutlineColor(sf::Color::White);
            sunUpgradeCard.setOutlineThickness(4);
        } else {
            sunUpgradeCard.setFillColor(sf::Color(60, 120, 60, 220));
            sunUpgradeCard.setOutlineColor(cm.canUpgradeSun() ? sf::Color::Yellow : sf::Color::White);
            sunUpgradeCard.setOutlineThickness(4);
        }
        
        crystalUpgradeCard.setSize(sf::Vector2f(150, 200));
        crystalUpgradeCard.setPosition(1120, 200);
        
        int crystalLevel = cm.getCrystalUpgradeLevel();
        if (crystalLevel == 5) {
            crystalUpgradeCard.setFillColor(sf::Color(120, 60, 120, 220));
            crystalUpgradeCard.setOutlineColor(sf::Color::White);
            crystalUpgradeCard.setOutlineThickness(4);
        } else {
            crystalUpgradeCard.setFillColor(sf::Color(120, 60, 120, 220));
            crystalUpgradeCard.setOutlineColor(cm.canUpgradeCrystal() ? sf::Color::Yellow : sf::Color::White);
            crystalUpgradeCard.setOutlineThickness(4);
        }
        
        if (!sunUpgradeTexture.loadFromFile("assets/images/sun.png")) {
            sf::Image sunImage;
            sunImage.create(64, 64, sf::Color::Yellow);
            sunUpgradeTexture.loadFromImage(sunImage);
        }
        sunUpgradeTexture.setSmooth(false);
        sunUpgradeSprite.setTexture(sunUpgradeTexture);
        
        sf::FloatRect sunBounds = sunUpgradeSprite.getLocalBounds();
        sunUpgradeSprite.setScale(100.0f / sunBounds.width, 100.0f / sunBounds.height);
        sunUpgradeSprite.setPosition(sunUpgradeCard.getPosition().x + 25, sunUpgradeCard.getPosition().y + 25);
        
        if (!crystalUpgradeTexture.loadFromFile("assets/images/crystal.png")) {
            sf::Image crystalImage;
            crystalImage.create(64, 64, sf::Color::Cyan);
            crystalUpgradeTexture.loadFromImage(crystalImage);
        }
        crystalUpgradeTexture.setSmooth(false);
        crystalUpgradeSprite.setTexture(crystalUpgradeTexture);
        
        sf::FloatRect crystalBounds = crystalUpgradeSprite.getLocalBounds();
        crystalUpgradeSprite.setScale(70.0f / crystalBounds.width, 70.0f / crystalBounds.height);
        crystalUpgradeSprite.setPosition(crystalUpgradeCard.getPosition().x + 40, crystalUpgradeCard.getPosition().y + 35);
        
        std::string sunLevelStars = "";
        if (sunLevel > 0) {
            sunLevelStars = " " + UIHelper::createStars(sunLevel);
        }
        sunUpgradeTextTop.setFont(font);
        sunUpgradeTextTop.setCharacterSize(UITheme::SMALL_SIZE);
        sunUpgradeTextTop.setStyle(sf::Text::Regular);
        sunUpgradeTextTop.setString("Sun");
        sunUpgradeTextBottom.setFont(font);
        sunUpgradeTextBottom.setCharacterSize(UITheme::SMALL_SIZE);
        sunUpgradeTextBottom.setStyle(sf::Text::Regular);
        sunUpgradeTextBottom.setString(std::string("Production") + sunLevelStars);
        sf::Color sunColor = cm.canUpgradeSun() ? sf::Color::White : sf::Color(255, 215, 0);
        sunUpgradeTextTop.setFillColor(sunColor);
        sunUpgradeTextBottom.setFillColor(sunColor);
        sf::FloatRect sTopB = sunUpgradeTextTop.getLocalBounds();
        sf::FloatRect sBotB = sunUpgradeTextBottom.getLocalBounds();
        float sBaseY = sunUpgradeCard.getPosition().y + 125.0f;
        float sTopX = sunUpgradeCard.getPosition().x + (150 - sTopB.width) / 2.0f - sTopB.left;
        float sTopY = sBaseY - sTopB.top - 6.0f;
        float sBotX = sunUpgradeCard.getPosition().x + (150 - sBotB.width) / 2.0f - sBotB.left;
        float sBotY = sBaseY - sBotB.top + 14.0f;
        sunUpgradeTextTop.setPosition(sTopX, sTopY);
        sunUpgradeTextBottom.setPosition(sBotX, sBotY);
        
        std::string crystalLevelStars = "";
        if (crystalLevel > 0) {
            crystalLevelStars = " " + UIHelper::createStars(crystalLevel);
        }
        crystalUpgradeTextTop.setFont(font);
        crystalUpgradeTextTop.setCharacterSize(UITheme::SMALL_SIZE);
        crystalUpgradeTextTop.setStyle(sf::Text::Regular);
        crystalUpgradeTextTop.setString("Crystal");
        crystalUpgradeTextBottom.setFont(font);
        crystalUpgradeTextBottom.setCharacterSize(UITheme::SMALL_SIZE);
        crystalUpgradeTextBottom.setStyle(sf::Text::Regular);
        crystalUpgradeTextBottom.setString(std::string("Production") + crystalLevelStars);
        sf::Color crystalColor = cm.canUpgradeCrystal() ? sf::Color::White : sf::Color(255, 215, 0);
        crystalUpgradeTextTop.setFillColor(crystalColor);
        crystalUpgradeTextBottom.setFillColor(crystalColor);
        sf::FloatRect cTopB = crystalUpgradeTextTop.getLocalBounds();
        sf::FloatRect cBotB = crystalUpgradeTextBottom.getLocalBounds();
        float cBaseY = crystalUpgradeCard.getPosition().y + 125.0f;
        float cTopX = crystalUpgradeCard.getPosition().x + (150 - cTopB.width) / 2.0f - cTopB.left;
        float cTopY = cBaseY - cTopB.top - 6.0f;
        float cBotX = crystalUpgradeCard.getPosition().x + (150 - cBotB.width) / 2.0f - cBotB.left;
        float cBotY = cBaseY - cBotB.top + 14.0f;
        crystalUpgradeTextTop.setPosition(cTopX, cTopY);
        crystalUpgradeTextBottom.setPosition(cBotX, cBotY);
        
        sunCostText.setFont(font);
        sunCostText.setCharacterSize(UITheme::SMALL_SIZE - 4);
        if (cm.canUpgradeSun()) {
            int cost = cm.getSunUpgradeCost();
            sunCostText.setString("Cost: " + std::to_string(cost));
            sunCostText.setFillColor(cm.canAfford(cost) ? sf::Color::Cyan : sf::Color::Red);
        } else {
            sunCostText.setString("MAX LEVEL");
            sunCostText.setFillColor(sf::Color(255, 215, 0));
        }
        sf::FloatRect sunCostBounds = sunCostText.getLocalBounds();
        float sunCostX = sunUpgradeCard.getPosition().x + (150 - sunCostBounds.width) / 2.0f - sunCostBounds.left;
        sunCostText.setPosition(sunCostX, sunUpgradeCard.getPosition().y + 175);
        
        crystalCostText.setFont(font);
        crystalCostText.setCharacterSize(UITheme::SMALL_SIZE - 4);
        if (cm.canUpgradeCrystal()) {
            int cost = cm.getCrystalUpgradeCost();
            crystalCostText.setString("Cost: " + std::to_string(cost));
            crystalCostText.setFillColor(cm.canAfford(cost) ? sf::Color::Cyan : sf::Color::Red);
        } else {
            crystalCostText.setString("MAX LEVEL");
            crystalCostText.setFillColor(sf::Color(255, 215, 0));
        }
        sf::FloatRect crystalCostBounds = crystalCostText.getLocalBounds();
        float crystalCostX = crystalUpgradeCard.getPosition().x + (150 - crystalCostBounds.width) / 2.0f - crystalCostBounds.left;
        crystalCostText.setPosition(crystalCostX, crystalUpgradeCard.getPosition().y + 175);
    }
    
    void updateUIText() {
        int availableCount = static_cast<int>(availableUnits.size());
        int minRequired = GameConstants::MIN_SELECTED_UNITS;
        StateManager* sm = StateManager::getInstance();
        int currentLevel = sm->getLevel();
        int calculatedMax = StateManager::calculateMaxSelection(currentLevel);
        sm->setMaxSelection(calculatedMax);
        MAX_SELECTED = calculatedMax;
        int maxSelectable = std::min(availableCount, MAX_SELECTED);
        
        #ifdef _DEBUG
        std::cout << "LoadoutScreen updateUIText: Level=" << currentLevel << ", MaxSelection=" << MAX_SELECTED << ", Available=" << availableCount << std::endl;
        #endif
        
        
        CrystalManager& crystalManager = CrystalManager::getInstance();
        crystalCountText.setString("Crystals: " + std::to_string(crystalManager.getCrystalCount()));
        
        int lvl = sCurrentLevel;
        int reward = crystalManager.getLevelCompletionReward(lvl);
        crystalRewardText.setString("Level " + std::to_string(lvl) + " reward: +" + std::to_string(reward) + " crystals");
    }
    
    void loadAvailableUnits() {
        availableUnits.clear();
        auto unlockedCards = cardCollection.getUnlockedCards();
        
        #ifdef _DEBUG
        std::cout << "DEBUG: Found " << unlockedCards.size() << " unlocked cards" << std::endl;
        #endif
        
        for (const auto& card : unlockedCards) {
            availableUnits.push_back(card.unitType);
            #ifdef _DEBUG
            std::cout << "DEBUG: Added unit: " << card.unitType << " (level: " << card.level << ")" << std::endl;
            #endif
        }
        
        #ifdef _DEBUG
        std::cout << "DEBUG: Total available units: " << availableUnits.size() << std::endl;
        for (const auto& unit : availableUnits) {
            std::cout << "DEBUG: - " << unit << std::endl;
        }
        #endif
        
        updateUIText();
    }
    
    void setupUnitCards() {
        unitCards.clear();
        unitTexts.clear();
        unitLineTexts.clear();
        unitSprites.clear();
        unitTextures.clear();
        
        StateManager* sm = StateManager::getInstance();
        for (auto& card : cardCollection.getAllCards()) {
            if (card.unlocked) {
                int storedLevel = sm->getCardLevel(card.unitType);
                if (storedLevel > 0) {
                    card.level = storedLevel;
                }
            }
        }
        upgradeCostTexts.clear();
        
        float scaleX = 1.0f;
        float scaleY = 1.0f;
        
        float cardWidth = 150 * scaleX;
        float cardHeight = 200 * scaleY;
        float spacing = 15 * scaleX;

        int totalCards = static_cast<int>(availableUnits.size());
        const int MAX_COLS_ON_ROW = 5;

        int cols = std::min(MAX_COLS_ON_ROW, totalCards);
        int rows = static_cast<int>(std::ceil(static_cast<float>(totalCards) / cols));
        
        float gridWidth = cols * cardWidth + (cols - 1) * spacing;
        float gridHeight = rows * cardHeight + (rows - 1) * spacing;
        
        float leftMargin = 40 * scaleX;
        float startX = leftMargin;
        
        if (startX + gridWidth > GameConstants::BATTLEFIELD_WIDTH - leftMargin) {
            startX = std::max(leftMargin, GameConstants::BATTLEFIELD_WIDTH - gridWidth - leftMargin);
            if (startX < leftMargin) {
                float availableWidth = GameConstants::BATTLEFIELD_WIDTH - 2 * leftMargin;
                cardWidth = std::max(100.0f * scaleX, (availableWidth - (cols - 1) * spacing) / cols);
                gridWidth = cols * cardWidth + (cols - 1) * spacing;
                startX = leftMargin;
            }
        }
        float startY = 200 * scaleY;

        int maxDisplayCards = std::min(totalCards, cols * rows);

        unitTextures.reserve(maxDisplayCards);
        
        for (int i = 0; i < maxDisplayCards; i++) {
            int row = i / cols;
            int col = i % cols;
            
            UnitCard* card_ptr = cardCollection.getCard(availableUnits[i]);
            int level = card_ptr ? card_ptr->level : 1;
            
            sf::RectangleShape card(sf::Vector2f(cardWidth, cardHeight));
            float cardX = startX + col * (cardWidth + spacing);
            float cardY = startY + row * (cardHeight + spacing);
            
            cardX = std::min(cardX, GameConstants::BATTLEFIELD_WIDTH - cardWidth - 10.f * scaleX);
            cardY = std::min(cardY, GameConstants::BATTLEFIELD_HEIGHT - cardHeight - 50.f * scaleY); 
            
            card.setPosition(cardX, cardY);
            
            bool isMaxLevel = card_ptr && card_ptr->isMaxLevel();
            
            if (isMaxLevel) {
                card.setFillColor(sf::Color(160, 100, 40, 220));
            } else if (level == 1) {
                card.setFillColor(sf::Color(70, 70, 90, 220));
            } else if (level == 2) {
                card.setFillColor(sf::Color(60, 90, 60, 220));
            } else if (level == 3) {
                card.setFillColor(sf::Color(90, 60, 120, 220));
            } else {
                card.setFillColor(sf::Color(160, 100, 40, 220));
            }
            
            if (isMaxLevel) {
                card.setOutlineThickness(6);
            } else {
                card.setOutlineThickness(4);
            }
            
            bool isSelected = std::find(selectedUnits.begin(), selectedUnits.end(), availableUnits[i]) != selectedUnits.end();
            
            if (isSelected) {
                card.setOutlineColor(sf::Color::Green);
            } else if (isMaxLevel) {
                card.setOutlineColor(sf::Color::White);
            } else if (card_ptr && card_ptr->canUpgrade()) {
                card.setOutlineColor(sf::Color::Yellow);
            } else {
                card.setOutlineColor(sf::Color::White);
            }
            unitCards.push_back(card);
            
            sf::Text unitName;
            unitName.setFont(font);
            std::string displayName = availableUnits[i];
            
            if (displayName == "basicshooter") displayName = "Basic\nShooter";
            else if (displayName == "resourcegenerator") displayName = "Sun\nGenerator";
            else if (displayName == "barriertower") displayName = "Wall-nut";
            else if (displayName == "rapidshooter") displayName = "Repeater";
            else if (displayName == "freezetower") displayName = "Snow\nPea";
            else if (displayName == "bombtower") displayName = "Cherry\nBomb";
            else if (displayName == "areaattacktower") displayName = "Fume-\nshroom";
            else {
                displayName = "Unknown Unit";
                #ifdef _DEBUG
                std::cout << "Warning: Unknown unit type: " << availableUnits[i] << std::endl;
                #endif
            }
            
            if (!isMaxLevel && displayName.find('\n') == std::string::npos && displayName.length() > 14) {
                displayName = displayName.substr(0, 11) + "...";
            } else if (isMaxLevel && displayName.find('\n') == std::string::npos && displayName.length() > 10) {
                displayName = displayName.substr(0, 8) + "..";
            }
            
            std::string levelIndicator = "";
            if (isMaxLevel) {
                levelIndicator = " " + UIHelper::createStars(5);
            } else if (level > 0) {
                levelIndicator = " " + UIHelper::createStars(level);
            }
            
            unitName.setString(displayName + levelIndicator);
            unitName.setCharacterSize(UITheme::SMALL_SIZE);
            unitName.setFillColor(UITheme::PRIMARY_TEXT);
            unitName.setStyle(sf::Text::Regular);

            std::vector<sf::Text> lines;
            if (displayName.find('\n') != std::string::npos) {
                std::string top = displayName.substr(0, displayName.find('\n'));
                std::string bottom = displayName.substr(displayName.find('\n') + 1);
                bottom += levelIndicator;

                sf::Text topText;
                topText.setFont(font);
                topText.setCharacterSize(UITheme::SMALL_SIZE);
                topText.setFillColor(UITheme::PRIMARY_TEXT);
                topText.setStyle(sf::Text::Regular);
                topText.setString(top);
                sf::FloatRect tb = topText.getLocalBounds();
                float tx = cardX + (cardWidth - tb.width) / 2.0f - tb.left;
                float baseY = cardY + cardHeight - 86.0f;
                float ty = baseY - tb.top;
                topText.setPosition(tx, ty);

                sf::Text botText;
                botText.setFont(font);
                botText.setCharacterSize(UITheme::SMALL_SIZE);
                botText.setFillColor(UITheme::PRIMARY_TEXT);
                botText.setStyle(sf::Text::Regular);
                botText.setString(bottom);
                sf::FloatRect bb = botText.getLocalBounds();
                float bx = cardX + (cardWidth - bb.width) / 2.0f - bb.left;
                float by = baseY + 24.0f - bb.top;
                botText.setPosition(bx, by);

                lines.push_back(topText);
                lines.push_back(botText);
            }

            sf::FloatRect textBounds = unitName.getLocalBounds();
            float textX = cardX + (cardWidth - textBounds.width) / 2.0f - textBounds.left;
            float textY = cardY + cardHeight - 71.0f - textBounds.top;
            unitName.setPosition(textX, textY);
            unitTexts.push_back(unitName);
            unitLineTexts.push_back(lines);
            
            if (card_ptr) {
                sf::Text costText;
                costText.setFont(font);
                costText.setCharacterSize(UITheme::SMALL_SIZE - 4);
                
                if (card_ptr->isMaxLevel()) {
                    costText.setString("Max level");
                    costText.setFillColor(sf::Color::White);
                    costText.setStyle(sf::Text::Regular);
                } else if (card_ptr->canUpgrade()) {
                    int upgradeCost = cardCollection.getUpgradeCost(availableUnits[i]);
                    bool canAfford = cardCollection.canAffordUpgrade(availableUnits[i]);
                    costText.setString("Cost: " + std::to_string(upgradeCost));
                    costText.setFillColor(canAfford ? sf::Color::Cyan : sf::Color::Red);
                } else {
                    costText.setString("LOCKED");
                    costText.setFillColor(sf::Color(100, 100, 100));
                }
                
                sf::FloatRect costBounds = costText.getLocalBounds();
                float costTextX = cardX + (cardWidth - costBounds.width) / 2.0f;
                float costTextY = cardY + cardHeight - 20; 
                costText.setPosition(costTextX, costTextY);
                upgradeCostTexts.push_back(costText);
            } else {
                sf::Text emptyText;
                upgradeCostTexts.push_back(emptyText);
            }
            
            sf::Texture texture;
            std::string imagePath = getImagePath(availableUnits[i]);
            
            bool imageLoaded = texture.loadFromFile(imagePath);
            if (!imageLoaded) {
                imageLoaded = texture.loadFromFile("./" + imagePath);
            }
            if (!imageLoaded) {
                imageLoaded = texture.loadFromFile("assets/images/plant.png");
            }
            if (!imageLoaded) {
                imageLoaded = texture.loadFromFile("assets/images/plant.png");
            }
            if (!imageLoaded) {
                sf::Image coloredSquare;
                sf::Color unitColor = sf::Color::Green;
                
                if (availableUnits[i] == "basicshooter") unitColor = sf::Color::Green;
                else if (availableUnits[i] == "resourcegenerator") unitColor = sf::Color::Yellow;
                else if (availableUnits[i] == "barriertower") unitColor = sf::Color(139, 69, 19); 
                else if (availableUnits[i] == "rapidshooter") unitColor = sf::Color(0, 128, 0); 
                else if (availableUnits[i] == "freezetower") unitColor = sf::Color::Cyan;
                else if (availableUnits[i] == "bombtower") unitColor = sf::Color::Red;
                else if (availableUnits[i] == "areaattacktower") unitColor = sf::Color::Magenta;
                
                coloredSquare.create(128, 128, unitColor);
                texture.loadFromImage(coloredSquare);
                imageLoaded = true;
                
                #ifdef _DEBUG
                std::cout << "Created colored square for: " << availableUnits[i] << std::endl;
                #endif
            }
            
            #ifdef _DEBUG
            if (imageLoaded) {
                std::cout << "Successfully loaded: " << imagePath << std::endl;
            } else {
                std::cout << "Failed to load: " << imagePath << std::endl;
            }
            #endif
            
            texture.setSmooth(false);
            
            unitTextures.push_back(texture);
            
            sf::Sprite sprite;
            sprite.setTexture(unitTextures.back());
            
            float centerX = cardX + cardWidth / 2;
            float centerY = cardY + (cardHeight - 60) / 2;
            SpriteScaler::scaleForUICard(sprite, unitTextures.back(), cardWidth - 40, cardHeight - 80, 
                                       centerX, centerY, 0.9f);
            
            unitSprites.push_back(sprite);
        }
    }
    
    void handleClick(const sf::Vector2f& pos) {
        for (size_t i = 0; i < unitCards.size(); i++) {
            if (unitCards[i].getGlobalBounds().contains(pos)) {
                std::string unit = availableUnits[i];
                
                auto it = std::find(selectedUnits.begin(), selectedUnits.end(), unit);
                if (it != selectedUnits.end()) {
                    selectedUnits.erase(it);
                    UnitCard* card_ptr = cardCollection.getCard(unit);
                    if (card_ptr && card_ptr->canUpgrade()) {
                        unitCards[i].setOutlineColor(sf::Color::Yellow);
                    } else {
                        unitCards[i].setOutlineColor(sf::Color::White);
                    }
                } else if (selectedUnits.size() < static_cast<size_t>(MAX_SELECTED)) {
                    selectedUnits.push_back(unit);
                    unitCards[i].setOutlineColor(sf::Color::Green);
                }
                
                StateManager* sm = StateManager::getInstance();
                sm->setSelectedUnits(selectedUnits);
                sm->save();
                
                break;
            }
        }
    }
    
    void handleUpgradeClick(const sf::Vector2f& pos) {
        try {
            CrystalManager& cm = CrystalManager::getInstance();
            
            if (sunUpgradeCard.getGlobalBounds().contains(pos)) {
                if (cm.canUpgradeSun() && cm.canAfford(cm.getSunUpgradeCost())) {
                    if (cm.upgradeSun()) {
                    #ifdef _DEBUG
                    std::cout << "Upgraded sun to level " << cm.getSunUpgradeLevel() << "!" << std::endl;
                    #endif
                    
                    StateManager* sm = StateManager::getInstance();
                    sm->setSunUpgradeLevel(cm.getSunUpgradeLevel());
                    sm->setCrystals(cm.getCrystalCount());
                    sm->save();
                    
                    setupResourceUpgrades();
                    updateUIText();
                }
            }
            return;
        }
        
        if (crystalUpgradeCard.getGlobalBounds().contains(pos)) {
            if (cm.canUpgradeCrystal() && cm.canAfford(cm.getCrystalUpgradeCost())) {
                if (cm.upgradeCrystal()) {
                    #ifdef _DEBUG
                    std::cout << "Upgraded crystal to level " << cm.getCrystalUpgradeLevel() << "!" << std::endl;
                    #endif
                    
                    StateManager* sm = StateManager::getInstance();
                    sm->setCrystalUpgradeLevel(cm.getCrystalUpgradeLevel());
                    sm->setCrystals(cm.getCrystalCount());
                    sm->save();
                    
                    setupResourceUpgrades();
                    updateUIText();
                }
            }
            return;
        }
        
        for (size_t i = 0; i < unitCards.size(); i++) {
            if (unitCards[i].getGlobalBounds().contains(pos)) {
                std::string unit = availableUnits[i];
                UnitCard* card = cardCollection.getCard(unit);
                
                if (card && card->canUpgrade() && cardCollection.canAffordUpgrade(unit)) {
                    if (cardCollection.upgradeCard(unit)) {
                        #ifdef _DEBUG
                        std::cout << "Upgraded " << unit << " to level " << card->level << "!" << std::endl;
                        #endif
                        
                        StateManager* sm = StateManager::getInstance();
                        sm->setCardLevel(unit, card->level);
                        sm->setCrystals(CrystalManager::getInstance().getCrystalCount());
                        sm->save();
                        
                        setupUnitCards();
                        updateUIText();
                    } else {
                        #ifdef _DEBUG
                        std::cout << "Failed to upgrade " << unit << " - insufficient crystals!" << std::endl;
                        #endif
                    }
                } else {
                    #ifdef _DEBUG
                    if (card && card->canUpgrade()) {
                        std::cout << "Cannot afford to upgrade " << unit << "!" << std::endl;
                    }
                    #endif
                }
                break;
            }
        }
        } catch (...) {
            
        }
    }
    
    void renderScreen(sf::RenderWindow& window, std::string& currentView) {
        CrystalManager& cm = CrystalManager::getInstance();
        crystalCountText.setString("Crystals: " + std::to_string(cm.getCrystalCount()));
        
        titleText.setCharacterSize(UITheme::TITLE_SIZE);
        sf::FloatRect titleBounds = titleText.getLocalBounds();
        titleText.setPosition((GameConstants::BATTLEFIELD_WIDTH - titleBounds.width) / 2.0f, 50);

        upgradeInstructionText.setCharacterSize(UITheme::BODY_SIZE);
        sf::FloatRect upgradeBounds = upgradeInstructionText.getLocalBounds();
        upgradeInstructionText.setPosition((GameConstants::BATTLEFIELD_WIDTH - upgradeBounds.width) / 2.0f, 110);


        background.setScale(1.0f, 1.0f);

        startButton.setSize(sf::Vector2f(200.f, 60.f));
        startButton.setPosition((GameConstants::BATTLEFIELD_WIDTH - 200.f) / 2.0f, GameConstants::BATTLEFIELD_HEIGHT - 180.f);

        startButtonText.setCharacterSize(UITheme::BUTTON_SIZE);
        sf::FloatRect startBounds = startButtonText.getLocalBounds();
        startButtonText.setPosition(startButton.getPosition().x + (startButton.getSize().x - startBounds.width) / 2.0f,
                                   startButton.getPosition().y + (startButton.getSize().y - startBounds.height) / 2.0f);

        backButton.setSize(sf::Vector2f(150.f, 50.f));
        backButton.setPosition(40.f, 40.f);
        
        backButtonText.setCharacterSize(UITheme::SMALL_SIZE);
        sf::FloatRect backBounds = backButtonText.getLocalBounds();
        backButtonText.setPosition(backButton.getPosition().x + (backButton.getSize().x - backBounds.width) / 2.0f,
                                  backButton.getPosition().y + (backButton.getSize().y - backBounds.height) / 2.0f);

        crystalCountText.setCharacterSize(UITheme::BODY_SIZE);
        sf::FloatRect cryLocalBounds = crystalCountText.getLocalBounds();
        
        float crystalX = GameConstants::BATTLEFIELD_WIDTH - 250.f;
        float crystalY = 55.f;
        crystalCountText.setPosition(crystalX, crystalY);

        crystalRewardText.setCharacterSize(UITheme::SMALL_SIZE);
        sf::FloatRect rewLocalBounds = crystalRewardText.getLocalBounds();
        
        float crystalCenterX = crystalCountText.getPosition().x + cryLocalBounds.width / 2.f;
        float rewardX = crystalCenterX - rewLocalBounds.width / 2.f;
        float rewardY = crystalY + cryLocalBounds.height + 8.f;
        
        crystalRewardText.setPosition(rewardX, rewardY);
        
        setupUnitCards();
        
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
                    
                    if (startButton.getGlobalBounds().contains(mousePos) && selectedUnits.size() >= GameConstants::MIN_SELECTED_UNITS) {
                        currentView = "battlefield";
                        return;
                    }
                    if (backButton.getGlobalBounds().contains(mousePos)) {
                        currentView = "mainMenu";
                        return;
                    }
                    
                    bool clickedDifficulty = false;
                    for (size_t i = 0; i < difficultyButtons.size(); i++) {
                        if (difficultyButtons[i].getGlobalBounds().contains(mousePos)) {
                            selectedDifficultyIndex = static_cast<int>(i);
                            StateManager* sm = StateManager::getInstance();
                            sm->setDifficultyIndex(selectedDifficultyIndex);
                            sm->save();
                            updateDifficultyButtons();
                            updateDifficultyText();
                            clickedDifficulty = true;
                            break;
                        }
                    }
                    
                    if (!clickedDifficulty) {
                        handleClick(mousePos);
                    }
                } else if (event.mouseButton.button == sf::Mouse::Right) {
                    sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
                    handleUpgradeClick(mousePos);
                }
            }
        }

        window.clear();
        window.draw(background);
        window.draw(titleText);
        window.draw(upgradeInstructionText);
        
        for (size_t i = 0; i < unitCards.size(); i++) {
            
            bool isSelected = std::find(selectedUnits.begin(), selectedUnits.end(), availableUnits[i]) != selectedUnits.end();
            bool maxReached = selectedUnits.size() >= static_cast<size_t>(MAX_SELECTED);
            UnitCard* card_ptr = cardCollection.getCard(availableUnits[i]);
            bool isMaxLevel = card_ptr && card_ptr->isMaxLevel();
            
            if (maxReached && !isSelected) {
                sf::RectangleShape dimmedCard = unitCards[i];
                dimmedCard.setFillColor(sf::Color(50, 50, 50, 150));
                window.draw(dimmedCard);
            } else {
                if (isMaxLevel && !maxReached) {
                    sf::RectangleShape glowCard = unitCards[i];
                    glowCard.setFillColor(sf::Color::Transparent);
                    glowCard.setOutlineColor(sf::Color(255, 215, 0, 100));
                    glowCard.setOutlineThickness(8);
                    window.draw(glowCard);
                }
                window.draw(unitCards[i]);
            }
            
            
            if (i < unitSprites.size()) {
                if (maxReached && !isSelected) {
                    sf::Sprite dimmedSprite = unitSprites[i];
                    dimmedSprite.setColor(sf::Color(100, 100, 100, 180));
                    window.draw(dimmedSprite);
                } else {
                    window.draw(unitSprites[i]);
                }
            }
            
            
            if (i < unitTexts.size()) {
                bool hasLines = (i < unitLineTexts.size() && !unitLineTexts[i].empty());
                if (hasLines) {
                    for (auto t : unitLineTexts[i]) {
                        if (maxReached && !isSelected) {
                            t.setFillColor(sf::Color(150, 150, 150, 200));
                        }
                        window.draw(t);
                    }
                } else {
                    if (maxReached && !isSelected) {
                        sf::Text dimmedText = unitTexts[i];
                        dimmedText.setFillColor(sf::Color(150, 150, 150, 200));
                        window.draw(dimmedText);
                    } else {
                        window.draw(unitTexts[i]);
                    }
                }
            }
            
            if (i < upgradeCostTexts.size() && upgradeCostTexts[i].getString() != "") {
                if (maxReached && !isSelected) {
                    sf::Text dimmedCost = upgradeCostTexts[i];
                    dimmedCost.setFillColor(sf::Color(150, 150, 150, 200));
                    window.draw(dimmedCost);
                } else {
                    window.draw(upgradeCostTexts[i]);
                }
            }
        }
        
        
        bool canStart = selectedUnits.size() >= GameConstants::MIN_SELECTED_UNITS;
        if (canStart) {
            startButton.setFillColor(UITheme::BUTTON_PRIMARY);
            startButtonText.setFillColor(UITheme::PRIMARY_TEXT);
        } else {
            startButton.setFillColor(UITheme::BUTTON_DISABLED);
            startButtonText.setFillColor(UITheme::SECONDARY_TEXT);
        }
        
        window.draw(backButton);
        window.draw(backButtonText);
        
        window.draw(startButton);
        window.draw(startButtonText);
        
        window.draw(difficultyLabel);
        window.draw(difficultyValueText);
        
        for (size_t i = 0; i < difficultyButtons.size(); i++) {
            window.draw(difficultyButtons[i]);
            window.draw(difficultyButtonTexts[i]);
        }
        
        sf::Text selectedText;
        selectedText.setFont(font);
        
        int availableCount = static_cast<int>(availableUnits.size());
        int actualMaximum = std::min(availableCount, MAX_SELECTED);
        
        selectedText.setString("Selected: " + std::to_string(selectedUnits.size()) + "/" + 
                             std::to_string(actualMaximum) + " (min: " + std::to_string(GameConstants::MIN_SELECTED_UNITS) + ")");
        selectedText.setCharacterSize(UITheme::BODY_SIZE);
        selectedText.setFillColor(canStart ? UITheme::SUCCESS_COLOR : UITheme::WARNING_COLOR);
        selectedText.setOutlineColor(sf::Color::Black);
        selectedText.setOutlineThickness(2);
        
        sf::FloatRect selectedBounds = selectedText.getLocalBounds();
        sf::FloatRect btnBounds = startButton.getGlobalBounds();
        float selectedX = btnBounds.left + (btnBounds.width - selectedBounds.width) / 2.0f;
        float selectedY = startButton.getPosition().y - 40;
        selectedText.setPosition(selectedX, selectedY);
        window.draw(selectedText);
        
        window.draw(crystalCountText);
        window.draw(crystalRewardText);
        
        window.draw(plantUpgradeHeaderText);
        window.draw(resourceUpgradeHeaderText);
        window.draw(difficultyHeaderText);
        
        window.draw(sunUpgradeCard);
        window.draw(sunUpgradeSprite);
        window.draw(sunUpgradeTextTop);
        window.draw(sunUpgradeTextBottom);
        window.draw(sunCostText);
        
        window.draw(crystalUpgradeCard);
        window.draw(crystalUpgradeSprite);
        window.draw(crystalUpgradeTextTop);
        window.draw(crystalUpgradeTextBottom);
        window.draw(crystalCostText);
        
        window.display();
    }
    
    std::vector<std::string> getSelectedUnits() const {
        #ifdef _DEBUG
        std::cout << "LoadoutScreen::getSelectedUnits() - Returning " << selectedUnits.size() << " units:" << std::endl;
        for (const auto& unit : selectedUnits) {
            std::cout << "  - " << unit << std::endl;
        }
        #endif
        return selectedUnits;
    }
    
    void setupDifficultyButtons() {
        difficultyButtons.clear();
        difficultyButtonTexts.clear();
        
        float buttonWidth = 80;
        float buttonHeight = 35;
        float spacing = 10;
        float startX = 50;
        float startY = 720;
        
        for (size_t i = 0; i < difficultyNames.size(); i++) {
            sf::RectangleShape button;
            button.setSize(sf::Vector2f(buttonWidth, buttonHeight));
            button.setPosition(startX + i * (buttonWidth + spacing), startY);
            
            sf::Text buttonText;
            buttonText.setFont(font);
            buttonText.setString(difficultyNames[i]);
            buttonText.setCharacterSize(UITheme::SMALL_SIZE - 2);
            
            sf::FloatRect textBounds = buttonText.getLocalBounds();
            float textX = button.getPosition().x + (buttonWidth - textBounds.width) / 2.0f;
            float textY = button.getPosition().y + (buttonHeight - textBounds.height) / 2.0f;
            buttonText.setPosition(textX, textY);
            
            difficultyButtons.push_back(button);
            difficultyButtonTexts.push_back(buttonText);
        }
        
        updateDifficultyButtons();
    }
    
    void updateDifficultyButtons() {
        for (size_t i = 0; i < difficultyButtons.size(); i++) {
            if (i == selectedDifficultyIndex) {
                difficultyButtons[i].setFillColor(UITheme::BUTTON_PRIMARY);
                difficultyButtons[i].setOutlineColor(sf::Color::Yellow);
                difficultyButtons[i].setOutlineThickness(3);
                difficultyButtonTexts[i].setFillColor(UITheme::PRIMARY_TEXT);
            } else {
                difficultyButtons[i].setFillColor(UITheme::BUTTON_DISABLED);
                difficultyButtons[i].setOutlineColor(sf::Color::White);
                difficultyButtons[i].setOutlineThickness(1);
                difficultyButtonTexts[i].setFillColor(UITheme::SECONDARY_TEXT);
            }
        }
    }
    
    void updateDifficultyText() {
        float difficultyMultiplier = difficultyValues[selectedDifficultyIndex];
        char multiplierStr[16];
        snprintf(multiplierStr, sizeof(multiplierStr), "%.1f", difficultyMultiplier);
        
        difficultyValueText.setString(difficultyNames[selectedDifficultyIndex] + " (" + std::string(multiplierStr) + "x)");
        difficultyValueText.setPosition(270, 680);
    }
    
    float getDifficultyValue() const {
        return difficultyValues[selectedDifficultyIndex];
    }
};
