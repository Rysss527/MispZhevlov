#include "EventBus.h"
#include "SoundManager.h"
#include <algorithm>
#include <iostream>

namespace DefenseSynth {

std::unique_ptr<EventBus> EventBus::instance = nullptr;

EventBus* EventBus::getInstance() {
    if (!instance) {
        instance = std::unique_ptr<EventBus>(new EventBus());
    }
    return instance.get();
}

void EventBus::subscribe(const std::string& eventType, IEventObserver* observer) {
    if (observer) {
        observers[eventType].push_back(observer);
    }
}

void EventBus::unsubscribe(const std::string& eventType, IEventObserver* observer) {
    auto it = observers.find(eventType);
    if (it != observers.end()) {
        auto& observerList = it->second;
        observerList.erase(
            std::remove(observerList.begin(), observerList.end(), observer),
            observerList.end()
        );
    }
}

void EventBus::publish(const IEvent& event) {
    const std::string eventType = event.getEventType();
    auto it = observers.find(eventType);
    if (it != observers.end()) {
        for (auto* observer : it->second) {
            if (observer) {
                observer->onEvent(event);
            }
        }
    }
}

void EventBus::clear() {
    observers.clear();
}

void EventSubscriptionManager::subscribe(const std::string& eventType, IEventObserver* observer) {
    EventBus::getInstance()->subscribe(eventType, observer);
    subscriptions.emplace_back(eventType, observer);
}

void EventSubscriptionManager::unsubscribeAll() {
    EventBus* eventBus = EventBus::getInstance();
    for (const auto& subscription : subscriptions) {
        eventBus->unsubscribe(subscription.first, subscription.second);
    }
    subscriptions.clear();
}

EventSubscriptionManager::~EventSubscriptionManager() {
    unsubscribeAll();
}

void ScoreObserver::onEvent(const IEvent& event) {
    if (!playerScore) return;

    if (event.getEventType() == "EnemyDefeated") {
        const auto& enemyEvent = static_cast<const EnemyDefeatedEvent&>(event);
        *playerScore += enemyEvent.scoreValue;
    }
    else if (event.getEventType() == "TowerPlaced") {
        const auto& towerEvent = static_cast<const TowerPlacedEvent&>(event);
        *playerScore -= towerEvent.cost;
    }
    else if (event.getEventType() == "WaveCompleted") {
        const auto& waveEvent = static_cast<const WaveCompletedEvent&>(event);
        *playerScore += waveEvent.waveNumber * 50;
    }
}

void AudioObserver::onEvent(const IEvent& event) {
    SoundManager& soundManager = SoundManager::getInstance();
    
    if (event.getEventType() == "EnemyDefeated") {
        const auto& enemyEvent = static_cast<const EnemyDefeatedEvent&>(event);
        if (enemyEvent.enemyType.find("boss") != std::string::npos) {
            soundManager.playSound("explosion.wav");
        } else {
            soundManager.playSound("splat1.wav");
        }
    }
    else if (event.getEventType() == "TowerPlaced") {
        soundManager.playSound("plant1.wav");
    }
    else if (event.getEventType() == "TowerDestroyed") {
        soundManager.playSound("explosion.wav");
    }
    else if (event.getEventType() == "WaveStarted") {
        const auto& waveEvent = static_cast<const WaveStartedEvent&>(event);
        if (waveEvent.waveType == "boss") {
            soundManager.playSound("siren.wav");
        } else {
            soundManager.playSound("readysetplant.wav");
        }
    }
    else if (event.getEventType() == "WaveCompleted") {
        soundManager.playSound("winmusic.wav");
    }
    else if (event.getEventType() == "ResourceGenerated") {
        soundManager.playSound("points.wav");
    }
    else if (event.getEventType() == "GameOver") {
        const auto& gameOverEvent = static_cast<const GameOverEvent&>(event);
        if (gameOverEvent.victory) {
            soundManager.playSound("winmusic.wav");
        }
    }
}

void UIObserver::onEvent(const IEvent& event) {
    if (uiUpdateCallback) {
        std::string message;
        
        if (event.getEventType() == "EnemyDefeated") {
            const auto& enemyEvent = static_cast<const EnemyDefeatedEvent&>(event);
            message = "Enemy defeated: +" + std::to_string(enemyEvent.scoreValue) + " points";
        }
        else if (event.getEventType() == "TowerPlaced") {
            const auto& towerEvent = static_cast<const TowerPlacedEvent&>(event);
            message = "Tower placed: " + towerEvent.towerType;
        }
        else if (event.getEventType() == "WaveStarted") {
            const auto& waveEvent = static_cast<const WaveStartedEvent&>(event);
            message = "Wave " + std::to_string(waveEvent.waveNumber) + " started!";
        }
        else if (event.getEventType() == "WaveCompleted") {
            const auto& waveEvent = static_cast<const WaveCompletedEvent&>(event);
            message = "Wave " + std::to_string(waveEvent.waveNumber) + " completed!";
        }
        else if (event.getEventType() == "ResourceGenerated") {
            const auto& resourceEvent = static_cast<const ResourceGeneratedEvent&>(event);
            message = "+" + std::to_string(resourceEvent.amount) + " " + resourceEvent.resourceType;
        }
        
        if (!message.empty()) {
            uiUpdateCallback(message);
        }
    }
}

}
