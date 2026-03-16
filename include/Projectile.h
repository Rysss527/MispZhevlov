#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <vector>
#include <memory>
#include <algorithm>
#include "GameCursor.h"
#include <fstream>
#include "Constants.h"
#include "ResourceManager.h"


class Bullet
{
public:
    int damage;
    int speed;
    bool exist;
    bool direction; 
    coordinates position;
    sf::Sprite sprite;
    sf::Clock clock;
    Bullet(float x = 0, float y = 0)
    {
        damage = GameConstants::BASE_TOWER_DAMAGE;
        speed = 8;
        exist = false;
        direction = false;

        auto& rm = DefenseSynth::ResourceManager::getInstance();
        const sf::Texture& t = rm.getTexture("ProjectilePea");
        sprite.setTexture(t);
        sprite.setOrigin(static_cast<float>(t.getSize().x) / 2.0f, static_cast<float>(t.getSize().y) / 2.0f);
        sprite.rotate(90);
        sprite.setPosition(x, y);
        position.x = x;
        position.y = y;
    }
    Bullet(const Bullet &bullet)
    {
        damage = bullet.damage;
        speed = bullet.speed;
        exist = bullet.exist;
        direction = bullet.direction;
        auto& rm = DefenseSynth::ResourceManager::getInstance();
        const sf::Texture& t = rm.getTexture("ProjectilePea");
        sprite.setTexture(t);
        sprite.setOrigin(static_cast<float>(t.getSize().x) / 2.0f, static_cast<float>(t.getSize().y) / 2.0f);
        sprite.rotate(90);
        position.x = bullet.position.x;
        position.y = bullet.position.y;
    }

    void Serialize(std::ostream& stream) const {
        stream << damage << std::endl;
        stream << speed << std::endl;
        stream << exist << std::endl;
        stream << direction << std::endl;
        stream << position.x << std::endl;
        stream << position.y << std::endl;
    }
    void Deserialize(std::istream& stream) {
        stream >> damage;
        stream >> speed;
        stream >> exist;
        stream >> direction;
        stream >> position.x;
        stream >> position.y;
        sprite.setPosition(position.x, position.y);
    }

    void move()
    {
        if (clock.getElapsedTime().asMicroseconds() > 2)
        {
            clock.restart();

            if (direction)
            {

                position.x -= speed;
            }
            else
            {

                position.x += speed;
            }
            sprite.setPosition(position.x, position.y);
        }
    }
    void draw(sf::RenderWindow &window)
    {
        window.draw(sprite);
    }
    bool reachedRightEdge(int windowWidth)
    {
        return position.x >= windowWidth;
    }
};

class Bomb : public Bullet
{
public:
    Bomb(float x = 0, float y = 0) : Bullet(x, y)
    {
        direction = false;
        auto& rm = DefenseSynth::ResourceManager::getInstance();
        const sf::Texture& t2 = rm.getTexture("ProjectileSnowPea");
        sprite.setTexture(t2);
        sprite.rotate(180);
    }
    Bomb(const Bomb &bomb)
    {
        damage = bomb.damage;
        speed = bomb.speed;
        exist = bomb.exist;
        direction = bomb.direction;
        auto& rm = DefenseSynth::ResourceManager::getInstance();
        const sf::Texture& t2 = rm.getTexture("ProjectileSnowPea");
        sprite.setTexture(t2);
        sprite.rotate(180);
        position.x = bomb.position.x;
        position.y = bomb.position.y;
    }
    void move()
    {
        position.y += speed;
        sprite.setPosition(position.x, position.y);
    }
    void draw(sf::RenderWindow &window)
    {
        window.draw(sprite);
    }
    bool reachedRightEdge(int windowWidth)
    {
        return position.y >= windowWidth;
    }
};

class BulletFactory
{
public:
    std::vector<std::unique_ptr<Bullet>> bullets;
    int bulletCount;
    BulletFactory(int count = 0)
    {
        bulletCount = 0;
        bullets.reserve(count > 0 ? count : 50);
    }
    BulletFactory(BulletFactory&& other) noexcept
        : bullets(std::move(other.bullets)), bulletCount(other.bulletCount)
    {
        other.bulletCount = 0;
    }
    
    BulletFactory& operator=(BulletFactory&& other) noexcept
    {
        if (this != &other)
        {
            bullets = std::move(other.bullets);
            bulletCount = other.bulletCount;
            other.bulletCount = 0;
        }
        return *this;
    }
    
    BulletFactory(const BulletFactory&) = delete;
    BulletFactory& operator=(const BulletFactory&) = delete;
    void addBullet(float x, float y, std::string bulletType = "normal")
    {
        if (bulletCount >= 4) {
            return;
        }
        auto bullet = std::make_unique<Bullet>(x, y);
        bullet->exist = true;
        bullet->direction = false;
        if (bulletType == "freeze") {
            const sf::Texture& t3 = DefenseSynth::ResourceManager::getInstance().getTexture("ProjectileSnowPea");
            bullet->sprite.setTexture(t3);
            bullet->sprite.setOrigin(static_cast<float>(t3.getSize().x) / 2.0f, static_cast<float>(t3.getSize().y) / 2.0f);
            bullet->sprite.rotate(90);
            bullet->damage = GameConstants::BASE_TOWER_DAMAGE - 2;
        } else if (bulletType == "poison") {
            bullet->sprite.setColor(sf::Color(50, 100, 50, 255));
            bullet->damage = GameConstants::BASE_TOWER_DAMAGE - 1;
        } else if (bulletType == "fire") {
            bullet->sprite.setColor(sf::Color(255, 100, 100, 255));
            bullet->damage = GameConstants::BASE_TOWER_DAMAGE + 4;
        }
        bullets.push_back(std::move(bullet));
        bulletCount++;
        if (bulletCount > 4) {
            bullets.resize(4);
            bulletCount = 4;
        }
    }
    void moveBullets()
    {
        for (const auto& bullet : bullets)
        {
            if (bullet && bullet->exist)
            {
                bullet->move();
            }
        }
    }
    void drawBullets(sf::RenderWindow &window)
    {
        for (const auto& bullet : bullets)
        {
            if (bullet && bullet->exist)
            {
                bullet->draw(window);
            }
        }
    }
    void removeNonExistantBullets()
    {
        auto it = std::remove_if(bullets.begin(), bullets.end(),
            [](const std::unique_ptr<Bullet>& bullet) {
                return !bullet || !bullet->exist;
            });
        bullets.erase(it, bullets.end());
        bulletCount = static_cast<int>(bullets.size());
        if (bulletCount > 4) {
            bullets.resize(4);
            bulletCount = 4;
        }
    }


    void Serialize(std::ostream& stream) const {
        stream << bulletCount << std::endl;
        for (const auto& bullet : bullets)
        {
            if (bullet)
            {
                bullet->Serialize(stream);
            }
        }
    }

    void Deserialize(std::istream& stream) {
        int temp_bulletCount;
        stream >> temp_bulletCount;
        bullets.clear();
        bullets.reserve(temp_bulletCount);
        bulletCount = temp_bulletCount;
        
        for (int i = 0; i < temp_bulletCount; i++) {
            auto bullet = std::make_unique<Bullet>();
            bullet->Deserialize(stream);
            bullets.push_back(std::move(bullet));
        }
        if (static_cast<int>(bullets.size()) > 4) {
            bullets.resize(4);
        }
        bulletCount = static_cast<int>(bullets.size());
    }

    ~BulletFactory() = default;
};
