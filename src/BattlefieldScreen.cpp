#include "BattlefieldScreen.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include "ResourceManager.h"

namespace DefenseSynth {

BattlefieldScreen::BattlefieldScreen(sf::RenderWindow &window, const std::vector<std::string>& allowedUnits, 
                                   const std::map<std::string, int>& upgradeLevels, float difficulty) : Screen(window)
{
    showWaveText = false;
    unitUpgradeLevels = upgradeLevels;
    for (const auto& pair : upgradeLevels) {
        StateManager::getInstance()->setUpgradeLevel(pair.first, pair.second);
    }
    gameDifficulty = difficulty;
    
    gameController = std::make_unique<DefenseSynth::GameController>();
    eventSubscriptionManager = std::make_unique<DefenseSynth::EventSubscriptionManager>();
    cursor.init(&window);
    
    auto& rm = ResourceManager::getInstance();
    const sf::Font& f = rm.getFont("arial");
    background.setTexture(rm.getTexture("background"));
    font = f;
    
    waveText.setFont(f);
    waveText.setCharacterSize(UIConstants::Battlefield::WAVE_TEXT_FONT_SIZE);
    waveText.setFillColor(sf::Color::Red);
    waveText.setStyle(sf::Text::Bold);
    waveText.setOrigin(waveText.getLocalBounds().width / 2, waveText.getLocalBounds().height / 2);
    waveText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, GameConstants::BATTLEFIELD_HEIGHT / 2);

    currencyText.setFont(f);
    currencyText.setCharacterSize(UIConstants::Battlefield::CURRENCY_FONT_SIZE);
    currencyText.setFillColor(sf::Color::Black);
    currencyText.setPosition(UIConstants::Battlefield::CURRENCY_X, UIConstants::Battlefield::CURRENCY_Y);

    scoreText.setFont(f);
    scoreText.setCharacterSize(UIConstants::Battlefield::SCORE_FONT_SIZE);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(UIConstants::Battlefield::SCORE_X, UIConstants::Battlefield::SCORE_Y);

    levelText.setFont(f);
    levelText.setCharacterSize(UIConstants::Battlefield::LEVEL_FONT_SIZE);
    levelText.setFillColor(sf::Color::Red);
    levelText.setPosition(UIConstants::Battlefield::LEVEL_X, UIConstants::Battlefield::LEVEL_Y);
    
    cityPopulationText.setFont(f);
    cityPopulationText.setCharacterSize(UIConstants::Battlefield::POPULATION_FONT_SIZE);
    cityPopulationText.setFillColor(sf::Color::Blue);
    cityPopulationText.setStyle(sf::Text::Bold);
    cityPopulationText.setPosition(UIConstants::Battlefield::POPULATION_X, UIConstants::Battlefield::POPULATION_Y);

    pauseButton.setTexture(rm.getTexture("pause"));
    pauseButton.setPosition(UIConstants::Battlefield::PAUSE_BUTTON_X, UIConstants::Battlefield::PAUSE_BUTTON_Y);

    gameplayView = window.getDefaultView();
    DefenseSynth::EffectManager::getInstance().setView(&gameplayView);

    towerFactory = std::make_unique<::TowerFactory>();
    sunFactory = std::make_unique<SunFactory>(10);
    enemyFactory = std::make_unique<EnemyFactory>(8, 1, gameDifficulty);
    lawnMowerFactory = std::make_unique<LawnMowerFactory>(5);
    
    sh.setAllowedUnits(allowedUnits);
    cursor.setAllowedUnits(allowedUnits);
    
    #ifdef _DEBUG
    std::cout << "BattlefieldScreen constructor - Setting " << allowedUnits.size() << " allowed units in shop:" << std::endl;
    for (const auto& unit : allowedUnits) {
        std::cout << "  - " << unit << std::endl;
    }
    #endif
    
    StateManager* sm = StateManager::getInstance();
    GameContext* context = GameContext::getInstance();
    Player& player = context->getPlayer();
    player.level = sm->getLevel();
    player.currency = 100; 
    player.lives = 3;     
 
    enemyFactory = std::make_unique<EnemyFactory>(player.level * 4 + static_cast<int>(gameDifficulty * 6), player.level, gameDifficulty);
    sunFactory = std::make_unique<SunFactory>(player.level * 10);
    
    collisionManager.setEffectManager(&DefenseSynth::EffectManager::getInstance());
    
    gameController->initialize((::TowerFactory*)towerFactory.get(), enemyFactory.get(), sunFactory.get(),
                             lawnMowerFactory.get(), &collisionManager, &player);
    gameController->prepareGame();
    
    SoundManager::getInstance().setPlaylist(UIConstants::Audio::BATTLE_TRACKS);
}

BattlefieldScreen::~BattlefieldScreen()
{
    if (enemyFactory) {
        enemyFactory->enemies.clear();
        enemyFactory->enemies_created = 0;
    }
    if (towerFactory) {
        towerFactory->towers.clear();
        towerFactory->towers_created = 0;
    }
    if (sunFactory) {
        sunFactory->suns.clear();
    }
    if (lawnMowerFactory) {
        lawnMowerFactory->lawnmowers.clear();
    }
    
    gameController.reset();
    eventSubscriptionManager.reset();
}

void BattlefieldScreen::startNextLevel()
{
    GameContext* context = GameContext::getInstance();
    Player& player = context->getPlayer();
    
    enemyFactory->resetForNextLevel(4 + player.level * 2, player.level, gameDifficulty);
    sunFactory->resetForNextLevel(player.level * 20);
    lawnMowerFactory->resetForNextLevel(5);
    
    player.currency += 50;
}

void BattlefieldScreen::Serialize()
{
    std::ofstream saveFile("save/game_save.dat", std::ios::trunc);
    if (saveFile.is_open())
    {
        GameContext* context = GameContext::getInstance();
        context->getPlayer().Serialize(saveFile);
        saveFile.close();
    }
}

void BattlefieldScreen::Deserialize()
{
    std::ifstream stream("save/game_save.dat");
    if (stream.is_open())
    {
        GameContext* context = GameContext::getInstance();
        context->getPlayer().Deserialize(stream);
        stream.close();
    }
}

void BattlefieldScreen::showGameOverScreen(sf::RenderWindow &window)
{
    auto& rm2 = ResourceManager::getInstance();
    std::string playerName;
    sf::Text playerNameText;
    sf::Text scoreText;
    
    playerNameText.setFont(rm2.getFont("arial"));
    playerNameText.setCharacterSize(50);
    playerNameText.setFillColor(sf::Color::White);
    playerNameText.setPosition(695, 760);
    
    scoreText.setFont(rm2.getFont("arial"));
    scoreText.setCharacterSize(50);
    scoreText.setFillColor(sf::Color::White);
    
    GameContext* context = GameContext::getInstance();
    scoreText.setString(std::to_string(context->getPlayer().score));
    scoreText.setOrigin(
        scoreText.getLocalBounds().left + scoreText.getLocalBounds().width / 2,
        scoreText.getLocalBounds().top + scoreText.getLocalBounds().height / 2);
    scoreText.setPosition(741, 480);
    
    sf::Sprite endSprite;
    endSprite.setTexture(rm2.getTexture("endscreen"));
    endSprite.setPosition(0, 0);
    
    SoundManager::getInstance().playMusic("losemusic.wav", false);

    while (true)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::TextEntered)
            {
                if (event.text.unicode < 128)
                {
                    if (event.text.unicode == 13)
                    {
                        if (!playerName.empty())
                        {
                            std::ofstream scoreFile("highscores.txt", std::ios::app);
                            if (scoreFile.is_open())
                            {
                                scoreFile << playerName << " " << context->getPlayer().score << "\n";
                                scoreFile.close();
                            }
                        }
                        return;
                    }
                    else if (event.text.unicode == 8 && !playerName.empty())
                    {
                        playerName.pop_back();
                    }
                    else if (event.text.unicode >= 32 && playerName.length() < 20)
                    {
                        playerName += (char)(event.text.unicode);
                    }
                    playerNameText.setString(playerName);
                    playerNameText.setPosition(695, 760);
                }
            }
        }
        window.clear();
        window.draw(endSprite);
        window.draw(scoreText);
        window.draw(playerNameText);
        window.display();
    }
}

void BattlefieldScreen::renderScreen(sf::RenderWindow &window, std::string &currentScreen)
{
    renderScreen(window, currentScreen, 0.016f);
}

void BattlefieldScreen::renderScreen(sf::RenderWindow &window, std::string &currentScreen, float deltaTime)
{
    SoundManager::getInstance().updateMusic();
    GameContext* context = GameContext::getInstance();
    Player& player = context->getPlayer();
    
    
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            StateManager* sm = StateManager::getInstance();
            sm->setLevel(player.level);
            sm->setCrystals(CrystalManager::getInstance().getCrystalCount());
            sm->save();
            window.close();
        }

        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::P)
            {
                SoundManager::getInstance().playSound("pause.wav");
                SoundManager::getInstance().loopSound("dancer.wav", false);
                currentScreen = "pause";
                return;
            }
            else if (event.key.code == sf::Keyboard::Space)
            {
                if (gameController && !gameController->isGameStarted()) {
                    gameController->startGame();
                    StateManager::getInstance()->startBattleSession();
                }
            }
        }

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2u windowSize = window.getSize();
            float scaleX = windowSize.x / UIConstants::BASE_WINDOW_WIDTH;
            float scaleY = windowSize.y / UIConstants::BASE_WINDOW_HEIGHT;

            sf::Vector2f worldPos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

            coordinates clickPosition;
            clickPosition.x = worldPos.x;
            clickPosition.y = worldPos.y;

            if (pauseButton.getGlobalBounds().contains(worldPos))
            {
                SoundManager::getInstance().playSound("pause.wav");
                SoundManager::getInstance().loopSound("dancer.wav", false);
                currentScreen = "pause";
                return;
            }
            else
            {
                inspector.gatherResources(clickPosition, sunFactory.get(), (::TowerFactory*)towerFactory.get(), player);
                
                if (gameController->isGameStarted()) {
                    if (!cursor.getCurrentCursor().empty() && cursor.getCurrentCursor() != "shovel") {
                        int upgradeLevel = unitUpgradeLevels.count(cursor.getCurrentCursor()) ? unitUpgradeLevels[cursor.getCurrentCursor()] : 1;
                        if (gameController->placeTower(cursor.getCurrentCursor(), clickPosition.x, clickPosition.y, upgradeLevel)) {
                        } else {
                            inspector.placeTowerAtPosition(clickPosition, (::TowerFactory*)towerFactory.get(), cursor, player, &unitUpgradeLevels);
                        }
                    } else {
                        inspector.placeTowerAtPosition(clickPosition, (::TowerFactory*)towerFactory.get(), cursor, player, &unitUpgradeLevels);
                    }
                }
                
                cursor.renderCursor(clickPosition, player);
            }
        }
    }

    int cityPopulation = gameController->getCityPopulation();
    ::TowerFactory* globalTowerFactory = (::TowerFactory*)towerFactory.get();
    
    DefenseSynth::EffectManager::getInstance().setView(&gameplayView);
    DefenseSynth::EffectManager::getInstance().update(deltaTime);
    
    cursor.update(deltaTime);
    if (cursor.getCurrentCursor() == "bombtower") {
        cursor.showRangeIndicator(static_cast<float>(GameConstants::CHERRY_BOMB_RADIUS));
    } else {
        cursor.hideRangeIndicator();
    }
    
    if (gameController->isGameStarted()) {
        bool paused = gameController->isPaused();
        if (showWaveText && waveTextTimer.getElapsedTime().asSeconds() > UIConstants::Battlefield::WAVE_TEXT_DISPLAY_TIME) {
            showWaveText = false;
        }

        if (gameController->getCurrentWave() != 2) {
            bossWaveTextShown = false;
        }

        if (gameController->getCurrentWave() == 2 && gameController->isWaveInProgress() && !bossWaveTextShown) {
            showWaveText = true;
            waveText.setString("Wave 2: Boss Invasion!");
            waveText.setOrigin(waveText.getLocalBounds().width / 2, waveText.getLocalBounds().height / 2);
            waveTextTimer.restart();
            bossWaveTextShown = true;
        }

        gameController->update(deltaTime);
        sunService.processSunGeneration(player, sunFactory.get(), deltaTime);
        sunFactory->move();
        sunFactory->removeNonExistantSun();
    }
    
    for (int i = 0; i < globalTowerFactory->towers_created; i++) {
        if (globalTowerFactory->towers[i]->type == "ResourceGenerator") {
            ResourceGenerator* resourceGen = static_cast<ResourceGenerator*>(globalTowerFactory->towers[i].get());
            resourceGen->updateBullet();
        }
    }

    collisionManager.checkLawnmowerCollisions(lawnMowerFactory.get(), enemyFactory.get(), player);
    inspector.checkBoundaryBreach(enemyFactory.get(), player, cityPopulation, lawnMowerFactory.get());
    gameController->setCityPopulation(cityPopulation);

    if (cityPopulation <= 0)
    {
        StateManager::getInstance()->endBattleSession();
        StateManager::getInstance()->save();
        
        showGameOverScreen(window);
        currentScreen = "mainMenu";
        return;
    }

    if (gameController->isWaveComplete())
    {
        int completedLevel = player.level;
        player.level++;
        
        if (player.level >= 10)
        {
            currentScreen = "gameComplete";
            return;
        }
        
        StateManager* sm = StateManager::getInstance();
        sm->setLevel(player.level);
        int newMaxSelection = std::min(3 + player.level - 1, 7);
        sm->setMaxSelection(newMaxSelection);
        
        CrystalManager& crystalManager = CrystalManager::getInstance();
        int crystalReward = crystalManager.getLevelCompletionReward(completedLevel);
        crystalManager.addCrystals(crystalReward);
        sm->setCrystals(crystalManager.getCrystalCount());
        sm->save();
        
        enemyFactory = std::make_unique<EnemyFactory>(player.level * 4 + static_cast<int>(gameDifficulty * 6), player.level, gameDifficulty);
        towerFactory = std::make_unique<::TowerFactory>();
        sunFactory = std::make_unique<SunFactory>(player.level * 10);
        lawnMowerFactory = std::make_unique<LawnMowerFactory>(5);
        
        gameController->initialize((::TowerFactory*)towerFactory.get(), enemyFactory.get(), sunFactory.get(),
                                 lawnMowerFactory.get(), &collisionManager, &player);
        
        int carryOverAmount = static_cast<int>(player.currency * (0.3f + gameDifficulty * 0.1f));
        player.currency = 100 + carryOverAmount;
        currentScreen = "levelUp";
        return;
    }

    window.clear();
    window.setView(gameplayView);
    window.draw(background);
    
    enemyFactory->drawGroundEnemies(window);
    globalTowerFactory->drawSprites(window);
    
    for (int i = 0; i < globalTowerFactory->towers_created; i++) {
        if (globalTowerFactory->towers[i]->category == "AttackTower") {
            AttackTower* attacker = static_cast<AttackTower*>(globalTowerFactory->towers[i].get());
            attacker->drawBullet(window);
        }
    }
    
    for (int i = 0; i < globalTowerFactory->towers_created; i++) {
        if (globalTowerFactory->towers[i]->type == "ResourceGenerator") {
            ResourceGenerator* resourceGen = static_cast<ResourceGenerator*>(globalTowerFactory->towers[i].get());
            resourceGen->drawBullet(window);
        }
    }
    
    lawnMowerFactory->draw(window);
    enemyFactory->drawFlyingEnemies(window);
    
    if (!gameController->isGameStarted()) {
        sf::View prevView = window.getView();
        window.setView(window.getDefaultView());
        sf::Text resumeText;
        resumeText.setFont(font);
        resumeText.setString("Press SPACE to Resume");
        resumeText.setCharacterSize(UIConstants::Battlefield::START_TEXT_FONT_SIZE);
        resumeText.setFillColor(sf::Color::Yellow);
        resumeText.setStyle(sf::Text::Bold);
        resumeText.setOrigin(resumeText.getLocalBounds().width / 2, resumeText.getLocalBounds().height / 2);
        resumeText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, GameConstants::BATTLEFIELD_HEIGHT / 2 - 30);
        
        sf::Text waveText;
        waveText.setFont(font);
        int displayWave = gameController->getCurrentWave();
        if (displayWave == 0) {
            displayWave = 1;
        } else if (gameController->getCurrentWave() == 1 && !gameController->isWaveInProgress()) {
            displayWave = 2;
        }
        waveText.setString("Wave " + std::to_string(displayWave));
        waveText.setCharacterSize(UIConstants::Battlefield::START_TEXT_FONT_SIZE);
        waveText.setFillColor(sf::Color::Red);
        waveText.setStyle(sf::Text::Bold);
        waveText.setOrigin(waveText.getLocalBounds().width / 2, waveText.getLocalBounds().height / 2);
        waveText.setPosition(GameConstants::BATTLEFIELD_WIDTH / 2, GameConstants::BATTLEFIELD_HEIGHT / 2 + 30);
        
        window.draw(resumeText);
        window.draw(waveText);
        window.setView(prevView);
    }
    
    for (int i = 0; i < globalTowerFactory->towers_created; i++) {
        if (globalTowerFactory->towers[i]->operational) {
            Tower* tower = globalTowerFactory->towers[i].get();
            tower->update(deltaTime);
            
            float healthPercentage = static_cast<float>(tower->vitality) / static_cast<float>(tower->maxVitality);
            healthPercentage = std::max(0.0f, std::min(1.0f, healthPercentage));
            
            sf::RectangleShape healthBarBg(sf::Vector2f(UIConstants::Battlefield::HEALTH_BAR_WIDTH, 
                                                       UIConstants::Battlefield::HEALTH_BAR_HEIGHT));
            healthBarBg.setFillColor(sf::Color::Black);
            healthBarBg.setOutlineThickness(1);
            healthBarBg.setOutlineColor(sf::Color::White);
            healthBarBg.setPosition(tower->location.x + UIConstants::Battlefield::HEALTH_BAR_X_OFFSET, 
                                   tower->location.y + UIConstants::Battlefield::HEALTH_BAR_Y_OFFSET);
            window.draw(healthBarBg);
            
            float barHeight = UIConstants::Battlefield::HEALTH_BAR_INNER_HEIGHT * healthPercentage;
            sf::RectangleShape healthBar(sf::Vector2f(UIConstants::Battlefield::HEALTH_BAR_INNER_WIDTH, barHeight));
            
            sf::Color healthColor;
            if (healthPercentage > UIConstants::Battlefield::HEALTH_GOOD_THRESHOLD) {
                healthColor = sf::Color::Green;
            } else if (healthPercentage > UIConstants::Battlefield::HEALTH_WARNING_THRESHOLD) {
                healthColor = sf::Color::Yellow;
            } else {
                healthColor = sf::Color::Red;
            }
            healthBar.setFillColor(healthColor);
            healthBar.setPosition(tower->location.x + UIConstants::Battlefield::HEALTH_BAR_INNER_X_OFFSET, 
                                 tower->location.y + UIConstants::Battlefield::HEALTH_BAR_INNER_Y_OFFSET + 
                                 (UIConstants::Battlefield::HEALTH_BAR_INNER_HEIGHT - barHeight));
            window.draw(healthBar);
            
            if (healthPercentage < 1.0f) {
                float damageHeight = UIConstants::Battlefield::HEALTH_BAR_INNER_HEIGHT * (1.0f - healthPercentage);
                sf::RectangleShape damageBar(sf::Vector2f(UIConstants::Battlefield::HEALTH_BAR_INNER_WIDTH, damageHeight));
                damageBar.setFillColor(sf::Color(80, 80, 80, 180));
                damageBar.setPosition(tower->location.x + UIConstants::Battlefield::HEALTH_BAR_INNER_X_OFFSET, 
                                     tower->location.y + UIConstants::Battlefield::HEALTH_BAR_INNER_Y_OFFSET);
                window.draw(damageBar);
            }
        }
    }
    
    enemyFactory->drawLabels(window);
    
    std::string currencyStr = std::to_string(player.currency);
    currencyText.setString(currencyStr);
    sf::FloatRect currencyBounds = currencyText.getLocalBounds();
    currencyText.setPosition(std::max(25.0f, 60.0f - currencyBounds.width), UIConstants::Battlefield::CURRENCY_Y);
    scoreText.setString("Score: " + std::to_string(player.score));
    levelText.setString("Level: " + std::to_string(player.level));
    
    cityPopulationText.setString("Citizens: " + std::to_string(gameController->getCityPopulation()));
    
    window.draw(scoreText);
    window.draw(levelText);
    window.draw(cityPopulationText);
    window.draw(pauseButton);
    
    DefenseSynth::EffectManager::getInstance().draw(window);
    window.setView(window.getDefaultView());
    
    cursor.draw(window);

    window.setView(gameplayView);
    sunFactory->draw(window);
    window.setView(window.getDefaultView());
    sh.draw(window, player, player.level);
    window.draw(currencyText);
    
    cursor.applyCursor(window);
    window.display();
}

}
