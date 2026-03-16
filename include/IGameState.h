#pragma once
#include <SFML/Graphics.hpp>
#include <memory>


class IGameState {
public:
    virtual ~IGameState() = default;
    virtual void update(sf::RenderWindow& window, float deltaTime) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
    virtual void handleInput(const sf::Event& event) = 0;
    virtual bool isFinished() const = 0;
    virtual std::string getNextState() const = 0;
};
