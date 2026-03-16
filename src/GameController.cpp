#include "GameController.h"
#include "ITowerFactory.h"
#include "TowerFactory.h"
#include "Constants.h"
#include "CrystalManager.h"
#include <iostream>
#include <algorithm>

namespace DefenseSynth {

PlaceTowerCommand::PlaceTowerCommand(TowerFactoryManager* factoryManager, ::TowerFactory* factory, 
                                   const std::string& type, float posX, float posY, 
                                   int level, Player* gamePlayer)
    : towerFactoryManager(factoryManager), towerFactory(factory), towerType(type), 
      x(posX), y(posY), upgradeLevel(level), player(gamePlayer) {}

void PlaceTowerCommand::execute() {
    if (!canExecute()) return;
    
    float newX = static_cast<float>(static_cast<int>(x / 100) * 100);
    float newY = static_cast<float>(static_cast<int>(y / 100) * 100);
    
    int cost = towerFactoryManager->getTowerCost(towerType);
    CrystalManager& cm = CrystalManager::getInstance();
    
    if (cm.canAfford(cost)) {
        auto tower = towerFactoryManager->createTower(towerType, newX, newY, upgradeLevel);
        if (tower) {
            cm.spendCrystals(cost);
            
            if (player) {
                player->currency = std::max(0, player->currency - cost);
            }
            
            towerFactory->towers.push_back(std::move(tower));
            towerFactory->towers_created++;
            
            EventBus::getInstance()->publish(TowerPlacedEvent(towerType, newX, newY, cost));
            SoundManager::getInstance().playSound("plant1.wav");
        }
    }
}

void PlaceTowerCommand::undo() {
}

bool PlaceTowerCommand::canExecute() const {
    if (!towerFactoryManager || !towerFactory) return false;
    
    int cost = towerFactoryManager->getTowerCost(towerType);
    CrystalManager& cm = CrystalManager::getInstance();
    if (cost < 0 || !cm.canAfford(cost)) return false;
    
    return !towerFactory->isTowerThere(x, y);
}

RemoveTowerCommand::RemoveTowerCommand(::TowerFactory* factory, float posX, float posY)
    : towerFactory(factory), x(posX), y(posY) {}

void RemoveTowerCommand::execute() {
    if (!canExecute()) return;
    
    towerFactory->removeTower(x, y);
    EventBus::getInstance()->publish(TowerDestroyedEvent("unknown", x, y));
}

void RemoveTowerCommand::undo() {
}

bool RemoveTowerCommand::canExecute() const {
    return towerFactory && towerFactory->isTowerThere(x, y);
}

StartWaveCommand::StartWaveCommand(EnemyFactory* factory, int wave, const std::string& type)
    : enemyFactory(factory), waveNumber(wave), waveType(type) {}

void StartWaveCommand::execute() {
    if (!canExecute()) return;
    
    if (waveType == "boss") {
        enemyFactory->triggerBossWave();
        int bossesActivated = 0;
        int totalBosses = 0;
        for (auto &e : enemyFactory->enemies) {
            if (e && e->type.find("boss_") == 0) {
                totalBosses++;
            }
        }
        
        int maxBossesToActivate = 1 + static_cast<int>(enemyFactory->difficultyMultiplier);
        maxBossesToActivate = std::min(maxBossesToActivate, totalBosses);
        
        for (auto &e : enemyFactory->enemies) {
            if (bossesActivated >= maxBossesToActivate) break;
            if (e && e->type.find("boss_") == 0) {
                e->enabled = true;
                bossesActivated++;
            }
        }
    } else {
        enemyFactory->startWave1();
    }
    
    EventBus::getInstance()->publish(WaveStartedEvent(waveNumber, waveType));
}

bool StartWaveCommand::canExecute() const {
    return enemyFactory != nullptr;
}

UpdateGameStateCommand::UpdateGameStateCommand(std::function<void()> func)
    : updateFunction(func) {}

void UpdateGameStateCommand::execute() {
    if (updateFunction) {
        updateFunction();
    }
}

CommandManager::CommandManager(size_t maxHistory) : maxHistorySize(maxHistory) {}

void CommandManager::queueCommand(std::unique_ptr<ICommand> command) {
    if (command) {
        commandQueue.push(std::move(command));
    }
}

void CommandManager::executeQueuedCommands() {
    while (!commandQueue.empty()) {
        auto command = std::move(commandQueue.front());
        commandQueue.pop();
        
        if (command && command->canExecute()) {
            command->execute();
            
            commandHistory.push_back(std::move(command));
            
            if (commandHistory.size() > maxHistorySize) {
                commandHistory.erase(commandHistory.begin());
            }
        }
    }
}

void CommandManager::undoLastCommand() {
    if (!commandHistory.empty()) {
        auto& lastCommand = commandHistory.back();
        lastCommand->undo();
        commandHistory.pop_back();
    }
}

void CommandManager::clearQueue() {
    while (!commandQueue.empty()) {
        commandQueue.pop();
    }
}

void CommandManager::clearHistory() {
    commandHistory.clear();
}

bool CommandManager::hasQueuedCommands() const {
    return !commandQueue.empty();
}

size_t CommandManager::getQueueSize() const {
    return commandQueue.size();
}

GameController::GameController() 
    : towerFactoryManager(std::make_unique<TowerFactoryManager>()),
      commandManager(std::make_unique<CommandManager>()),
      eventSubscriptionManager(std::make_unique<EventSubscriptionManager>()),
      towerFactory(nullptr), enemyFactory(nullptr), sunFactory(nullptr),
      lawnMowerFactory(nullptr), collisionManager(nullptr), player(nullptr),
      gameStarted(false), currentWave(0), waveInProgress(false),
      wavePauseActive(false), paused(false), wavePauseDurationLeft(0.0f),
      cityPopulation(UIConstants::Battlefield::INITIAL_CITY_POPULATION) {
}

GameController::~GameController() {
}

void GameController::initialize(::TowerFactory* towers, EnemyFactory* enemies, SunFactory* suns,
                               LawnMowerFactory* lawnMowers, CollisionManager* collisions, Player* gamePlayer) {
    towerFactory = towers;
    enemyFactory = enemies;
    sunFactory = suns;
    lawnMowerFactory = lawnMowers;
    collisionManager = collisions;
    player = gamePlayer;
    
    if (enemyFactory && towerFactory) {
        enemyFactory->setTowerFactoryRef(towerFactory);
    }
    
    setupEventObservers();
    
    gameUpdateClock.restart();
}

void GameController::setupEventObservers() {
    scoreObserver = std::make_unique<ScoreObserver>(&player->score);
    audioObserver = std::make_unique<AudioObserver>();
    uiObserver = std::make_unique<UIObserver>(
        [this](const std::string& message) { onUIUpdate(message); }
    );
    
    eventSubscriptionManager->subscribe("EnemyDefeated", scoreObserver.get());
    eventSubscriptionManager->subscribe("TowerPlaced", scoreObserver.get());
    eventSubscriptionManager->subscribe("WaveCompleted", scoreObserver.get());
    
    eventSubscriptionManager->subscribe("EnemyDefeated", audioObserver.get());
    eventSubscriptionManager->subscribe("TowerPlaced", audioObserver.get());
    eventSubscriptionManager->subscribe("TowerDestroyed", audioObserver.get());
    eventSubscriptionManager->subscribe("WaveStarted", audioObserver.get());
    eventSubscriptionManager->subscribe("WaveCompleted", audioObserver.get());
    eventSubscriptionManager->subscribe("ResourceGenerated", audioObserver.get());
    eventSubscriptionManager->subscribe("GameOver", audioObserver.get());
    
    eventSubscriptionManager->subscribe("EnemyDefeated", uiObserver.get());
    eventSubscriptionManager->subscribe("TowerPlaced", uiObserver.get());
    eventSubscriptionManager->subscribe("WaveStarted", uiObserver.get());
    eventSubscriptionManager->subscribe("WaveCompleted", uiObserver.get());
    eventSubscriptionManager->subscribe("ResourceGenerated", uiObserver.get());
}

void GameController::prepareGame() {
    gameStarted = false;
    currentWave = 0;
    waveInProgress = false;
    wavePauseActive = false;
    paused = false;
    cityPopulation = 100;
    
    if (player) {
        player->currency = 500;
        player->score = 0;
    }
    
    commandManager->clearQueue();
    commandManager->clearHistory();
}

void GameController::startGame() {
    if (!gameStarted) {
        gameStarted = true;
        
        if (currentWave == 0) {
            currentWave = 1;
        }
        
        if (currentWave == 1 && !waveInProgress && wavePauseActive) {
            wavePauseActive = false;
            currentWave = 2;
        }

        if (currentWave == 2) {
            waveInProgress = true;
            startWave(currentWave, "boss");
        } else {
            waveInProgress = true;
            
            if (enemyFactory) {
                enemyFactory->startWave1();
            }
            
            EventBus::getInstance()->publish(WaveStartedEvent(currentWave, "normal"));
        }
        waveTimer.restart();
    }
}

void GameController::pauseGame() {
    if (paused) return;
    paused = true;
    if (wavePauseActive) {
        float elapsed = wavePauseTimer.getElapsedTime().asSeconds();
        float base = 3.0f;
        wavePauseDurationLeft = base - elapsed;
        if (wavePauseDurationLeft < 0.f) wavePauseDurationLeft = 0.f;
    }
}

void GameController::resumeGame() {
    if (!paused) return;
    paused = false;
    if (wavePauseActive) {
        wavePauseTimer.restart();
    }
}

void GameController::resetGame() {
    gameStarted = false;
    currentWave = 0;
    waveInProgress = false;
    wavePauseActive = false;
    paused = false;
    cityPopulation = UIConstants::Battlefield::INITIAL_CITY_POPULATION;
    
    commandManager->clearQueue();
    commandManager->clearHistory();
}

void GameController::startWave(int waveNumber, const std::string& waveType) {
    auto startWaveCmd = std::make_unique<StartWaveCommand>(enemyFactory, waveNumber, waveType);
    executeCommand(std::move(startWaveCmd));
}

void GameController::checkWaveCompletion() {
    if (wavePauseActive && currentWave == 1) {
        float target = (wavePauseDurationLeft > 0.f) ? wavePauseDurationLeft : 3.0f;
        if (wavePauseTimer.getElapsedTime().asSeconds() >= target) {
            wavePauseActive = false;
            wavePauseDurationLeft = 0.f;
            currentWave = 2;
            waveInProgress = false;
            gameStarted = false;
        }
        return;
    }
    
    if (waveInProgress && currentWave == 1) {
        bool allNormalEnemiesDead = true;
        for (int i = 0; i < enemyFactory->enemies_created; i++) {
            if (enemyFactory->enemies[i]->enabled && 
                enemyFactory->enemies[i]->type.find("boss_") != 0) {
                allNormalEnemiesDead = false;
                break;
            }
        }
        
        if (allNormalEnemiesDead) {
            EventBus::getInstance()->publish(WaveCompletedEvent(currentWave, 0));
            waveInProgress = false;
            gameStarted = false;
            wavePauseActive = true;
            wavePauseTimer.restart();
        }
    } else if (waveInProgress && currentWave == 2) {
        bool anyEnemyAlive = false;
        for (int i = 0; i < enemyFactory->enemies_created; i++) {
            if (enemyFactory->enemies[i]->enabled) {
                anyEnemyAlive = true;
                break;
            }
        }
        if (!anyEnemyAlive) {
            EventBus::getInstance()->publish(WaveCompletedEvent(currentWave, 0));
            waveInProgress = false;
            gameStarted = false;
        }
    }
}

bool GameController::isWaveComplete() const {
    return !waveInProgress && currentWave == 2;
}

bool GameController::placeTower(const std::string& type, float x, float y, int upgradeLevel) {
    auto placeTowerCmd = std::make_unique<PlaceTowerCommand>(
        towerFactoryManager.get(), towerFactory, type, x, y, upgradeLevel, player);
    
    if (placeTowerCmd->canExecute()) {
        executeCommand(std::move(placeTowerCmd));
        return true;
    }
    return false;
}

bool GameController::removeTower(float x, float y) {
    auto removeTowerCmd = std::make_unique<RemoveTowerCommand>(towerFactory, x, y);
    
    if (removeTowerCmd->canExecute()) {
        executeCommand(std::move(removeTowerCmd));
        return true;
    }
    return false;
}

bool GameController::canPlaceTower(const std::string& type, float x, float y) const {
    if (!towerFactoryManager || !towerFactory) return false;
    
    int cost = towerFactoryManager->getTowerCost(type);
    CrystalManager& cm = CrystalManager::getInstance();
    if (cost < 0 || !cm.canAfford(cost)) return false;
    
    return !towerFactory->isTowerThere(x, y);
}

int GameController::getTowerCost(const std::string& type) const {
    return towerFactoryManager ? towerFactoryManager->getTowerCost(type) : -1;
}

void GameController::update(float deltaTime) {
    processQueuedCommands();
    
    if (gameStarted && !paused) {
        if (collisionManager && towerFactory && enemyFactory && player) {
            collisionManager->checkCollisions(towerFactory, enemyFactory, *player);
        }

        updateGameObjects(deltaTime);

        if (collisionManager && lawnMowerFactory && enemyFactory && player) {
            collisionManager->checkLawnmowerCollisions(lawnMowerFactory, enemyFactory, *player);
        }
        checkGameEndConditions();
    }
    
    if (!paused) {
        updateWaveLogic();
    }
}

void GameController::updateGameObjects(float deltaTime) {
    if (enemyFactory) {
        enemyFactory->summonEnemies();
        enemyFactory->move(deltaTime);
    }
    
    if (sunFactory) {
        sunFactory->move();
    }
    
    if (lawnMowerFactory) {
        lawnMowerFactory->move();
    }
    
    if (towerFactory) {
        towerFactory->move();
        towerFactory->removeInactiveTowers();
    }
}

void GameController::checkCollisions() {
    if (collisionManager && towerFactory && enemyFactory && player && lawnMowerFactory) {
        collisionManager->checkCollisions(towerFactory, enemyFactory, *player);
        collisionManager->checkLawnmowerCollisions(lawnMowerFactory, enemyFactory, *player);
    }
}

void GameController::checkGameEndConditions() {
    if (cityPopulation <= 0) {
        EventBus::getInstance()->publish(GameOverEvent(false, player->score, player->level));
        gameStarted = false;
    }
    
    if (isWaveComplete()) {
        gameStarted = false;
    }
}

void GameController::executeCommand(std::unique_ptr<ICommand> command) {
    commandManager->queueCommand(std::move(command));
}

void GameController::undoLastCommand() {
    commandManager->undoLastCommand();
}

void GameController::onUIUpdate(const std::string& message) {
    std::cout << "UI Update: " << message << std::endl;
}

void GameController::processQueuedCommands() {
    commandManager->executeQueuedCommands();
}

void GameController::updateWaveLogic() {
    checkWaveCompletion();
}

void GameController::activateEnemiesForWave(int wave) {
    if (wave == 1) {
        int wave1EnemiesActivated = 0;
        int wave1TargetCount = enemyFactory->enemies_created / 2;
        
        for (int i = 0; i < enemyFactory->enemies_created && wave1EnemiesActivated < wave1TargetCount; i++) {
            if (!enemyFactory->enemies[i]->enabled && 
                enemyFactory->enemies[i]->type.find("boss_") != 0) {
                enemyFactory->enemies[i]->enabled = true;
                wave1EnemiesActivated++;
            }
        }
    }
}

}
