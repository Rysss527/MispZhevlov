#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <typeindex>

namespace DefenseSynth {

class IEvent {
public:
    virtual ~IEvent() = default;
    virtual std::string getEventType() const = 0;
};

class IEventObserver {
public:
    virtual ~IEventObserver() = default;
    virtual void onEvent(const IEvent& event) = 0;
};

class EnemyDefeatedEvent : public IEvent {
public:
    int enemyId;
    std::string enemyType;
    int scoreValue;
    float positionX, positionY;

    EnemyDefeatedEvent(int id, const std::string& type, int score, float x, float y)
        : enemyId(id), enemyType(type), scoreValue(score), positionX(x), positionY(y) {}

    std::string getEventType() const override { return "EnemyDefeated"; }
};

class TowerPlacedEvent : public IEvent {
public:
    std::string towerType;
    float positionX, positionY;
    int cost;

    TowerPlacedEvent(const std::string& type, float x, float y, int towerCost)
        : towerType(type), positionX(x), positionY(y), cost(towerCost) {}

    std::string getEventType() const override { return "TowerPlaced"; }
};

class TowerDestroyedEvent : public IEvent {
public:
    std::string towerType;
    float positionX, positionY;

    TowerDestroyedEvent(const std::string& type, float x, float y)
        : towerType(type), positionX(x), positionY(y) {}

    std::string getEventType() const override { return "TowerDestroyed"; }
};

class WaveStartedEvent : public IEvent {
public:
    int waveNumber;
    std::string waveType;

    WaveStartedEvent(int wave, const std::string& type)
        : waveNumber(wave), waveType(type) {}

    std::string getEventType() const override { return "WaveStarted"; }
};

class WaveCompletedEvent : public IEvent {
public:
    int waveNumber;
    int enemiesDefeated;

    WaveCompletedEvent(int wave, int defeated)
        : waveNumber(wave), enemiesDefeated(defeated) {}

    std::string getEventType() const override { return "WaveCompleted"; }
};

class ResourceGeneratedEvent : public IEvent {
public:
    std::string resourceType;
    int amount;
    float positionX, positionY;

    ResourceGeneratedEvent(const std::string& type, int amt, float x, float y)
        : resourceType(type), amount(amt), positionX(x), positionY(y) {}

    std::string getEventType() const override { return "ResourceGenerated"; }
};

class GameOverEvent : public IEvent {
public:
    bool victory;
    int finalScore;
    int levelReached;

    GameOverEvent(bool win, int score, int level)
        : victory(win), finalScore(score), levelReached(level) {}

    std::string getEventType() const override { return "GameOver"; }
};

class EventBus {
private:
    std::unordered_map<std::string, std::vector<IEventObserver*>> observers;
    static std::unique_ptr<EventBus> instance;

public:
    static EventBus* getInstance();
    
    void subscribe(const std::string& eventType, IEventObserver* observer);
    void unsubscribe(const std::string& eventType, IEventObserver* observer);
    void publish(const IEvent& event);
    void clear();

private:
    EventBus() = default;
};

class EventSubscriptionManager {
private:
    std::vector<std::pair<std::string, IEventObserver*>> subscriptions;

public:
    void subscribe(const std::string& eventType, IEventObserver* observer);
    void unsubscribeAll();
    ~EventSubscriptionManager();
};

class ScoreObserver : public IEventObserver {
private:
    int* playerScore;

public:
    ScoreObserver(int* score) : playerScore(score) {}
    void onEvent(const IEvent& event) override;
};

class AudioObserver : public IEventObserver {
public:
    void onEvent(const IEvent& event) override;
};

class UIObserver : public IEventObserver {
private:
    std::function<void(const std::string&)> uiUpdateCallback;

public:
    UIObserver(std::function<void(const std::string&)> callback) 
        : uiUpdateCallback(callback) {}
    void onEvent(const IEvent& event) override;
};

}
