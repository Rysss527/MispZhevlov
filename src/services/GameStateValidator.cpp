#include "services/GameStateValidator.h"
#include <algorithm>
#include "SoundManager.h"

namespace DefenseSynth {

GameStateValidator::GameStateValidator() 
    : currentState(GameState::InProgress),
      cityPopulation(INITIAL_CITY_POPULATION),
      maxCityPopulation(INITIAL_CITY_POPULATION),
      waveInProgress(false) {
    currentWave.waveNumber = 0;
    currentWave.totalEnemies = 0;
    currentWave.enemiesSpawned = 0;
    currentWave.enemiesKilled = 0;
    currentWave.isComplete = false;
}

bool GameStateValidator::hasPlayerWon() const {
    return currentState == GameState::Won;
}

bool GameStateValidator::hasPlayerLost() const {
    return currentState == GameState::Lost;
}

void GameStateValidator::checkEndConditions(Player& player, EnemyFactory* enemies) {
    if (cityPopulation <= 0 || player.lives <= 0) {
        currentState = GameState::Lost;
        return;
    }
    
    if (hasEnemyReachedEnd(enemies)) {
        decreaseCityPopulation(10);
        player.lives--;
        SoundManager::getInstance().playSound("scream.wav");
    }
    
    if (currentWave.waveNumber >= 10 && areAllEnemiesDefeated(enemies)) {
        currentState = GameState::Won;
    }
    
    updateWaveProgress(enemies);
}

bool GameStateValidator::isWaveComplete(EnemyFactory* enemies) const {
    if (!enemies || !waveInProgress) return false;
    
    return currentWave.enemiesSpawned >= currentWave.totalEnemies && 
           areAllEnemiesDefeated(enemies);
}

bool GameStateValidator::areAllEnemiesDefeated(EnemyFactory* enemies) const {
    if (!enemies) return true;
    
    for (const auto& enemy : enemies->enemies) {
        if (enemy && enemy->enabled) {
            return false;
        }
    }
    return true;
}

bool GameStateValidator::hasEnemyReachedEnd(EnemyFactory* enemies) const {
    if (!enemies) return false;
    
    bool enemyReached = false;
    for (auto& enemy : enemies->enemies) {
        if (enemy && enemy->enabled && enemy->coords.x <= LEFT_BOUNDARY) {
            enemy->enabled = false;
            enemyReached = true;
        }
    }
    return enemyReached;
}

void GameStateValidator::startWave(int waveNumber, int enemyCount) {
    currentWave.waveNumber = waveNumber;
    currentWave.totalEnemies = enemyCount;
    currentWave.enemiesSpawned = 0;
    currentWave.enemiesKilled = 0;
    currentWave.isComplete = false;
    waveInProgress = true;
}

void GameStateValidator::endWave() {
    currentWave.isComplete = true;
    waveInProgress = false;
    waveEndClock.restart();
}

void GameStateValidator::decreaseCityPopulation(int amount) {
    cityPopulation = std::max(0, cityPopulation - amount);
}

bool GameStateValidator::shouldSpawnNextWave() const {
    return !waveInProgress && 
           currentWave.isComplete && 
           getTimeSinceWaveEnd() >= WAVE_INTERVAL;
}

float GameStateValidator::getTimeSinceWaveEnd() const {
    if (waveInProgress) return 0.0f;
    return waveEndClock.getElapsedTime().asSeconds();
}

void GameStateValidator::updateWaveProgress(EnemyFactory* enemies) {
    if (!enemies || !waveInProgress) return;
    
    int activeEnemies = 0;
    for (const auto& enemy : enemies->enemies) {
        if (enemy && enemy->enabled) {
            activeEnemies++;
        }
    }
    
    currentWave.enemiesKilled = currentWave.enemiesSpawned - activeEnemies;
    
    if (isWaveComplete(enemies)) {
        endWave();
    }
}

void GameStateValidator::resetForNewGame() {
    currentState = GameState::InProgress;
    cityPopulation = INITIAL_CITY_POPULATION;
    currentWave.waveNumber = 0;
    currentWave.totalEnemies = 0;
    currentWave.enemiesSpawned = 0;
    currentWave.enemiesKilled = 0;
    currentWave.isComplete = false;
    waveInProgress = false;
}

}