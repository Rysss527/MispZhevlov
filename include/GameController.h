#pragma once

#include <memory>
#include <vector>
#include <queue>
#include <functional>
#include "ITowerFactory.h"
#include "EnemyFactory.h"
#include "EventBus.h"
#include "GamePlayer.h"
#include "Resource.h"
#include "DefenseUnitFactory.h"
#include "CollisionManager.h"
#include "TowerFactory.h"

namespace DefenseSynth {

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual void undo() {}
    virtual bool canExecute() const { return true; }
};

class PlaceTowerCommand : public ICommand {
private:
    TowerFactoryManager* towerFactoryManager;
    ::TowerFactory* towerFactory;
    std::string towerType;
    float x, y;
    int upgradeLevel;
    Player* player;
    std::unique_ptr<Tower> placedTower;

public:
    PlaceTowerCommand(TowerFactoryManager* factoryManager, ::TowerFactory* factory, 
                     const std::string& type, float posX, float posY, 
                     int level, Player* gamePlayer);
    void execute() override;
    void undo() override;
    bool canExecute() const override;
};

class RemoveTowerCommand : public ICommand {
private:
    ::TowerFactory* towerFactory;
    float x, y;
    std::unique_ptr<Tower> removedTower;

public:
    RemoveTowerCommand(::TowerFactory* factory, float posX, float posY);
    void execute() override;
    void undo() override;
    bool canExecute() const override;
};

class StartWaveCommand : public ICommand {
private:
    EnemyFactory* enemyFactory;
    int waveNumber;
    std::string waveType;

public:
    StartWaveCommand(EnemyFactory* factory, int wave, const std::string& type);
    void execute() override;
    bool canExecute() const override;
};

class UpdateGameStateCommand : public ICommand {
private:
    std::function<void()> updateFunction;

public:
    UpdateGameStateCommand(std::function<void()> func);
    void execute() override;
};

class CommandManager {
private:
    std::queue<std::unique_ptr<ICommand>> commandQueue;
    std::vector<std::unique_ptr<ICommand>> commandHistory;
    size_t maxHistorySize;

public:
    CommandManager(size_t maxHistory = 100);
    
    void queueCommand(std::unique_ptr<ICommand> command);
    void executeQueuedCommands();
    void undoLastCommand();
    void clearQueue();
    void clearHistory();
    
    bool hasQueuedCommands() const;
    size_t getQueueSize() const;
};

class GameController {
private:
    std::unique_ptr<TowerFactoryManager> towerFactoryManager;
    std::unique_ptr<CommandManager> commandManager;
    std::unique_ptr<EventSubscriptionManager> eventSubscriptionManager;
    
    ::TowerFactory* towerFactory;
    EnemyFactory* enemyFactory;
    SunFactory* sunFactory;
    LawnMowerFactory* lawnMowerFactory;
    CollisionManager* collisionManager;
    Player* player;
    
    bool gameStarted;
    int currentWave;
    bool waveInProgress;
    sf::Clock waveTimer;
    sf::Clock gameUpdateClock;
    sf::Clock wavePauseTimer;
    bool wavePauseActive;
    bool paused;
    float wavePauseDurationLeft;
    int cityPopulation;
    
    std::unique_ptr<ScoreObserver> scoreObserver;
    std::unique_ptr<AudioObserver> audioObserver;
    std::unique_ptr<UIObserver> uiObserver;

public:
    GameController();
    ~GameController();
    
    void initialize(::TowerFactory* towers, EnemyFactory* enemies, SunFactory* suns,
                   LawnMowerFactory* lawnMowers, CollisionManager* collisions, Player* gamePlayer);
    void setupEventObservers();
    
    void prepareGame();
    void startGame();
    void pauseGame();
    void resumeGame();
    bool isPaused() const { return paused; }
    void resetGame();
    
    void startWave(int waveNumber, const std::string& waveType);
    void checkWaveCompletion();
    bool isWaveComplete() const;
    
    bool placeTower(const std::string& type, float x, float y, int upgradeLevel = 1);
    bool removeTower(float x, float y);
    bool canPlaceTower(const std::string& type, float x, float y) const;
    int getTowerCost(const std::string& type) const;
    
    void update(float deltaTime);
    void updateGameObjects(float deltaTime);
    void checkCollisions();
    void checkGameEndConditions();
    
    void executeCommand(std::unique_ptr<ICommand> command);
    void undoLastCommand();
    
    bool isGameStarted() const { return gameStarted; }
    int getCurrentWave() const { return currentWave; }
    bool isWaveInProgress() const { return waveInProgress; }
    int getCityPopulation() const { return cityPopulation; }
    void setCityPopulation(int population) { cityPopulation = population; }
    
    void onUIUpdate(const std::string& message);
    
private:
    void processQueuedCommands();
    void updateWaveLogic();
    void activateEnemiesForWave(int wave);
};

}
