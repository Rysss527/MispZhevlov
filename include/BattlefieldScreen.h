#pragma once

#include "ScreenBase.h"
#include "GameContext.h"
#include "TowerFactory.h"
#include "EnemyFactory.h"
#include "Resource.h"
#include "GameCursor.h"
#include "Store.h"
#include "GameMonitor.h"
#include "DefenseUnitFactory.h"
#include "CollisionManager.h"
#include "SoundManager.h"
#include "StateManager.h"
#include "CrystalManager.h"
#include "Constants.h"
#include "GameCursor.h"
#include "GameController.h"
#include "EventBus.h"
#include "EffectManager.h"
#include "services/PlayerInteractionService.h"
#include <map>
#include <vector>
#include <memory>

namespace DefenseSynth {

class BattlefieldScreen : public Screen
{
public:
    sf::Texture pauseButtonTexture;
    sf::Sprite pauseButton;
    GameCursor cursor;
    Shop sh;
    sf::Text currencyText;
    sf::Font font;
    sf::Text scoreText;
    sf::Text levelText;
    std::unique_ptr<::TowerFactory> towerFactory;
    std::unique_ptr<EnemyFactory> enemyFactory;
    std::unique_ptr<SunFactory> sunFactory;
    std::unique_ptr<LawnMowerFactory> lawnMowerFactory;
    DefenseSynth::GameInspector inspector;
    CollisionManager collisionManager;
    std::map<std::string, int> unitUpgradeLevels;
    float gameDifficulty;
    sf::Text waveText;
    bool showWaveText = false;
    sf::Clock waveTextTimer;
    bool bossWaveTextShown = false;
    sf::Text cityPopulationText;
    bool isPaused = false;
    sf::View gameplayView;
    
    std::unique_ptr<DefenseSynth::GameController> gameController;
    std::unique_ptr<DefenseSynth::EventSubscriptionManager> eventSubscriptionManager;
    DefenseSynth::PlayerInteractionService sunService;

    BattlefieldScreen(sf::RenderWindow &window, const std::vector<std::string>& allowedUnits, 
                     const std::map<std::string, int>& upgradeLevels, float difficulty = 1.0f);
    ~BattlefieldScreen();

    void startNextLevel();
    void Serialize();
    void Deserialize();
    void showGameOverScreen(sf::RenderWindow &window);
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen) override;
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen, float deltaTime);
};

}
