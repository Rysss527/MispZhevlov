#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <random>
#include <algorithm>

namespace DefenseSynth {

enum class EffectType {
    EXPLOSION,
    FREEZE,
    POISON_CLOUD,
    FIRE,
    SMOKE,
    SPARKLE,
    HIT_IMPACT,
    HEAL,
    SHIELD_BREAK,
    TRAIL,
    BURNING_ZOMBIE,
    POISON_VAPOR,
    ICE_CRYSTALS,
    FIRE_EMBER,
    TOXIC_BUBBLE,
    ZOMBIE_DEATH_SPLATTER,
    ZOMBIE_DEATH_CHUNKS,
    BOSS_DEATH_EXPLOSION,
    BOSS_DEATH_SHOCKWAVE,
    BOSS_DEATH_ENERGY,
    PLANT_DEATH_PETALS,
    PLANT_DEATH_LEAVES,
    PLANT_DEATH_POLLEN,
    DISINTEGRATION,
    SOUL_ESCAPE,
    SPARKS,
    SPEED_BOOST,
    HEIGHT_ADJUSTMENT,
    FORCE_MOVEMENT,
    LANE_REALIGNMENT
};

class Particle : public sf::Transformable {
public:
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    sf::Color color;
    sf::Color targetColor;
    float lifeTime;
    float maxLifeTime;
    float size;
    float sizeGrowth;
    float alphaDecay;
    float rotation;
    float rotationSpeed;
    float turbulence;
    float gravity;
    bool isActive;
    bool usePhysics;
    bool fadeToTarget;
    
    Particle() : lifeTime(0), maxLifeTime(1.0f), size(5.0f), 
                 sizeGrowth(0), alphaDecay(255.0f), isActive(false),
                 rotation(0), rotationSpeed(0), turbulence(0),
                 gravity(50.0f), usePhysics(true), fadeToTarget(false) {}
    
    void init(const sf::Vector2f& pos, const sf::Vector2f& vel, 
              const sf::Color& col, float life, float initialSize) {
        setPosition(pos);
        velocity = vel;
        acceleration = sf::Vector2f(0, 0);
        color = col;
        targetColor = col;
        lifeTime = 0;
        maxLifeTime = life;
        size = initialSize;
        isActive = true;
        rotation = static_cast<float>(rand() % 360);
        rotationSpeed = static_cast<float>((rand() % 200) - 100);
    }
    
    void update(float deltaTime) {
        if (!isActive) return;
        
        lifeTime += deltaTime;
        if (lifeTime >= maxLifeTime) {
            isActive = false;
            return;
        }
        
        velocity += acceleration * deltaTime;
        
        if (turbulence > 0) {
            float turbX = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * turbulence;
            float turbY = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * turbulence;
            velocity.x += turbX * deltaTime;
            velocity.y += turbY * deltaTime;
        }
        
        move(velocity * deltaTime);
        
        if (usePhysics) {
            velocity.y += gravity * deltaTime;
        }
        
        size += sizeGrowth * deltaTime;
        if (size < 0.1f) size = 0.1f;
        
        rotation += rotationSpeed * deltaTime;
        setRotation(rotation);
        
        float lifeRatio = lifeTime / maxLifeTime;
        
        if (fadeToTarget) {
            float t = lifeRatio;
            color.r = static_cast<sf::Uint8>(color.r * (1.0f - t) + targetColor.r * t);
            color.g = static_cast<sf::Uint8>(color.g * (1.0f - t) + targetColor.g * t);
            color.b = static_cast<sf::Uint8>(color.b * (1.0f - t) + targetColor.b * t);
        }
        
        float alphaMult = 1.0f - lifeRatio;
        if (lifeRatio < 0.2f) {
            alphaMult = lifeRatio * 5.0f;
        }
        color.a = static_cast<sf::Uint8>(alphaMult * alphaDecay);
    }
};

class ParticleSystem : public sf::Drawable, public sf::Transformable {
private:
    std::vector<Particle> particles;
    sf::VertexArray vertices;
    EffectType type;
    sf::Vector2f emitterPosition;
    float emissionRate;
    float emissionTimer;
    int maxParticles;
    bool isEmitting;
    std::mt19937 rng;
    std::uniform_real_distribution<float> angleDist;
    std::uniform_real_distribution<float> speedDist;
    std::uniform_real_distribution<float> lifeDist;
    std::uniform_real_distribution<float> sizeDist;
    
public:
    ParticleSystem(EffectType effectType, int maxCount = 500) 
        : type(effectType), maxParticles(maxCount), isEmitting(false),
          emissionRate(100.0f), emissionTimer(0),
          rng(std::random_device{}()),
          angleDist(0, 360), speedDist(50, 150), 
          lifeDist(0.5f, 2.0f), sizeDist(2.0f, 8.0f) {
        
        particles.resize(maxParticles);
        vertices.setPrimitiveType(sf::Quads);
        vertices.resize(maxParticles * 4);
        
        configureForType();
    }
    
    void configureForType() {
        switch(type) {
            case EffectType::EXPLOSION:
                emissionRate = 500.0f;
                speedDist = std::uniform_real_distribution<float>(100, 300);
                lifeDist = std::uniform_real_distribution<float>(0.5f, 1.5f);
                sizeDist = std::uniform_real_distribution<float>(5.0f, 15.0f);
                break;
                
            case EffectType::FREEZE:
                emissionRate = 50.0f;
                speedDist = std::uniform_real_distribution<float>(10, 30);
                lifeDist = std::uniform_real_distribution<float>(1.0f, 3.0f);
                sizeDist = std::uniform_real_distribution<float>(3.0f, 6.0f);
                break;
                
            case EffectType::POISON_CLOUD:
                emissionRate = 30.0f;
                speedDist = std::uniform_real_distribution<float>(5, 20);
                lifeDist = std::uniform_real_distribution<float>(2.0f, 4.0f);
                sizeDist = std::uniform_real_distribution<float>(8.0f, 16.0f);
                break;
                
            case EffectType::FIRE:
                emissionRate = 80.0f;
                speedDist = std::uniform_real_distribution<float>(30, 80);
                lifeDist = std::uniform_real_distribution<float>(0.8f, 1.5f);
                sizeDist = std::uniform_real_distribution<float>(4.0f, 10.0f);
                angleDist = std::uniform_real_distribution<float>(-30, 30);
                break;
                
            case EffectType::SPARKLE:
                emissionRate = 20.0f;
                speedDist = std::uniform_real_distribution<float>(20, 60);
                lifeDist = std::uniform_real_distribution<float>(0.5f, 1.0f);
                sizeDist = std::uniform_real_distribution<float>(2.0f, 4.0f);
                break;
                
            case EffectType::SMOKE:
                emissionRate = 15.0f;
                speedDist = std::uniform_real_distribution<float>(10, 25);
                lifeDist = std::uniform_real_distribution<float>(3.0f, 5.0f);
                sizeDist = std::uniform_real_distribution<float>(10.0f, 20.0f);
                break;
                
            case EffectType::BURNING_ZOMBIE:
                emissionRate = 120.0f;
                speedDist = std::uniform_real_distribution<float>(20, 60);
                lifeDist = std::uniform_real_distribution<float>(1.2f, 2.2f);
                sizeDist = std::uniform_real_distribution<float>(3.0f, 8.0f);
                angleDist = std::uniform_real_distribution<float>(-30, 30);
                break;
                
            case EffectType::POISON_VAPOR:
                emissionRate = 45.0f;
                speedDist = std::uniform_real_distribution<float>(8, 25);
                lifeDist = std::uniform_real_distribution<float>(2.5f, 4.5f);
                sizeDist = std::uniform_real_distribution<float>(12.0f, 20.0f);
                angleDist = std::uniform_real_distribution<float>(0, 360);
                break;
                
            case EffectType::ICE_CRYSTALS:
                emissionRate = 60.0f;
                speedDist = std::uniform_real_distribution<float>(15, 40);
                lifeDist = std::uniform_real_distribution<float>(1.5f, 3.5f);
                sizeDist = std::uniform_real_distribution<float>(2.0f, 5.0f);
                angleDist = std::uniform_real_distribution<float>(0, 360);
                break;
                
            case EffectType::FIRE_EMBER:
                emissionRate = 150.0f;
                speedDist = std::uniform_real_distribution<float>(50, 120);
                lifeDist = std::uniform_real_distribution<float>(0.4f, 1.0f);
                sizeDist = std::uniform_real_distribution<float>(1.5f, 4.0f);
                angleDist = std::uniform_real_distribution<float>(-20, 20);
                break;
                
            case EffectType::TOXIC_BUBBLE:
                emissionRate = 25.0f;
                speedDist = std::uniform_real_distribution<float>(10, 30);
                lifeDist = std::uniform_real_distribution<float>(3.0f, 5.0f);
                sizeDist = std::uniform_real_distribution<float>(4.0f, 8.0f);
                angleDist = std::uniform_real_distribution<float>(0, 360);
                break;
                
            case EffectType::ZOMBIE_DEATH_SPLATTER:
                emissionRate = 300.0f;
                speedDist = std::uniform_real_distribution<float>(100, 250);
                lifeDist = std::uniform_real_distribution<float>(0.8f, 1.5f);
                sizeDist = std::uniform_real_distribution<float>(3.0f, 8.0f);
                angleDist = std::uniform_real_distribution<float>(0, 360);
                break;
                
            case EffectType::ZOMBIE_DEATH_CHUNKS:
                emissionRate = 50.0f;
                speedDist = std::uniform_real_distribution<float>(80, 180);
                lifeDist = std::uniform_real_distribution<float>(1.0f, 2.0f);
                sizeDist = std::uniform_real_distribution<float>(8.0f, 15.0f);
                angleDist = std::uniform_real_distribution<float>(-45, 45);
                break;
                
            case EffectType::BOSS_DEATH_EXPLOSION:
                emissionRate = 800.0f;
                speedDist = std::uniform_real_distribution<float>(200, 400);
                lifeDist = std::uniform_real_distribution<float>(1.0f, 2.5f);
                sizeDist = std::uniform_real_distribution<float>(10.0f, 25.0f);
                angleDist = std::uniform_real_distribution<float>(0, 360);
                break;
                
            case EffectType::BOSS_DEATH_SHOCKWAVE:
                emissionRate = 100.0f;
                speedDist = std::uniform_real_distribution<float>(300, 500);
                lifeDist = std::uniform_real_distribution<float>(0.5f, 1.0f);
                sizeDist = std::uniform_real_distribution<float>(15.0f, 30.0f);
                angleDist = std::uniform_real_distribution<float>(0, 360);
                break;
                
            case EffectType::BOSS_DEATH_ENERGY:
                emissionRate = 200.0f;
                speedDist = std::uniform_real_distribution<float>(50, 150);
                lifeDist = std::uniform_real_distribution<float>(2.0f, 4.0f);
                sizeDist = std::uniform_real_distribution<float>(5.0f, 12.0f);
                angleDist = std::uniform_real_distribution<float>(0, 360);
                break;
                
            case EffectType::PLANT_DEATH_PETALS:
                emissionRate = 80.0f;
                speedDist = std::uniform_real_distribution<float>(40, 120);
                lifeDist = std::uniform_real_distribution<float>(1.5f, 3.0f);
                sizeDist = std::uniform_real_distribution<float>(4.0f, 10.0f);
                angleDist = std::uniform_real_distribution<float>(-60, 60);
                break;
                
            case EffectType::PLANT_DEATH_LEAVES:
                emissionRate = 60.0f;
                speedDist = std::uniform_real_distribution<float>(30, 80);
                lifeDist = std::uniform_real_distribution<float>(2.0f, 4.0f);
                sizeDist = std::uniform_real_distribution<float>(6.0f, 12.0f);
                angleDist = std::uniform_real_distribution<float>(0, 360);
                break;
                
            case EffectType::PLANT_DEATH_POLLEN:
                emissionRate = 150.0f;
                speedDist = std::uniform_real_distribution<float>(20, 60);
                lifeDist = std::uniform_real_distribution<float>(1.0f, 2.5f);
                sizeDist = std::uniform_real_distribution<float>(1.0f, 3.0f);
                angleDist = std::uniform_real_distribution<float>(0, 360);
                break;
                
            case EffectType::DISINTEGRATION:
                emissionRate = 400.0f;
                speedDist = std::uniform_real_distribution<float>(10, 50);
                lifeDist = std::uniform_real_distribution<float>(1.5f, 3.0f);
                sizeDist = std::uniform_real_distribution<float>(2.0f, 6.0f);
                angleDist = std::uniform_real_distribution<float>(0, 360);
                break;
                
            case EffectType::SOUL_ESCAPE:
                emissionRate = 30.0f;
                speedDist = std::uniform_real_distribution<float>(40, 100);
                lifeDist = std::uniform_real_distribution<float>(2.0f, 4.0f);
                sizeDist = std::uniform_real_distribution<float>(8.0f, 16.0f);
                angleDist = std::uniform_real_distribution<float>(-10, 10);
                break;
                
            case EffectType::SPARKS:
                emissionRate = 100.0f;
                speedDist = std::uniform_real_distribution<float>(80, 200);
                lifeDist = std::uniform_real_distribution<float>(0.3f, 0.8f);
                sizeDist = std::uniform_real_distribution<float>(1.0f, 3.0f);
                angleDist = std::uniform_real_distribution<float>(0, 360);
                break;
                
            case EffectType::SPEED_BOOST:
                emissionRate = 60.0f;
                speedDist = std::uniform_real_distribution<float>(40, 120);
                lifeDist = std::uniform_real_distribution<float>(0.5f, 1.2f);
                sizeDist = std::uniform_real_distribution<float>(2.0f, 5.0f);
                angleDist = std::uniform_real_distribution<float>(-30, 30);
                break;
                
            case EffectType::HEIGHT_ADJUSTMENT:
                emissionRate = 50.0f;
                speedDist = std::uniform_real_distribution<float>(20, 80);
                lifeDist = std::uniform_real_distribution<float>(1.0f, 2.0f);
                sizeDist = std::uniform_real_distribution<float>(3.0f, 6.0f);
                angleDist = std::uniform_real_distribution<float>(-90, 90);
                break;
                
            case EffectType::FORCE_MOVEMENT:
                emissionRate = 80.0f;
                speedDist = std::uniform_real_distribution<float>(60, 150);
                lifeDist = std::uniform_real_distribution<float>(0.4f, 1.0f);
                sizeDist = std::uniform_real_distribution<float>(2.0f, 4.0f);
                angleDist = std::uniform_real_distribution<float>(160, 200);
                break;
                
            case EffectType::LANE_REALIGNMENT:
                emissionRate = 40.0f;
                speedDist = std::uniform_real_distribution<float>(30, 70);
                lifeDist = std::uniform_real_distribution<float>(0.8f, 1.5f);
                sizeDist = std::uniform_real_distribution<float>(4.0f, 8.0f);
                angleDist = std::uniform_real_distribution<float>(-45, 45);
                break;
                
            default:
                break;
        }
    }
    
    void emit(const sf::Vector2f& position, int count = -1) {
        emitterPosition = position;
        isEmitting = true;
        
        if (count > 0) {
            for (int i = 0; i < count && i < maxParticles; ++i) {
                emitParticle();
            }
            isEmitting = false;
        }
    }
    
    void stop() {
        isEmitting = false;
    }
    
    void update(float deltaTime) {
        if (isEmitting) {
            emissionTimer += deltaTime;
            float particlesToEmit = emissionRate * emissionTimer;
            while (particlesToEmit >= 1.0f) {
                emitParticle();
                particlesToEmit -= 1.0f;
                emissionTimer -= 1.0f / emissionRate;
            }
        }
        
        for (auto& particle : particles) {
            particle.update(deltaTime);
        }
        
        updateVertices();
    }
    
    bool isActive() const {
        if (isEmitting) return true;
        
        for (const auto& particle : particles) {
            if (particle.isActive) return true;
        }
        return false;
    }
    
    void setEmitterPosition(const sf::Vector2f& pos) {
        emitterPosition = pos;
    }
    
private:
    void emitParticle() {
        for (auto& particle : particles) {
            if (!particle.isActive) {
                float angle = angleDist(rng) * 3.14159f / 180.0f;
                float speed = speedDist(rng);
                sf::Vector2f velocity(std::cos(angle) * speed, std::sin(angle) * speed);
                
                sf::Color color = getColorForType();
                float lifeTime = lifeDist(rng);
                float size = sizeDist(rng);
                
                particle.init(emitterPosition, velocity, color, lifeTime, size);
                
                configureParticleForType(particle);
                break;
            }
        }
    }
    
    sf::Color getColorForType() {
        switch(type) {
            case EffectType::EXPLOSION:
                return sf::Color(255, rng() % 155 + 100, 0, 255);
            case EffectType::FREEZE:
                return sf::Color(150, 200, 255, 200);
            case EffectType::POISON_CLOUD:
                return sf::Color(50, 200, 50, 150);
            case EffectType::FIRE:
                return sf::Color(255, rng() % 100 + 100, 0, 255);
            case EffectType::SPARKLE:
                return sf::Color(255, 255, 200, 255);
            case EffectType::SMOKE:
                return sf::Color(100, 100, 100, 150);
            case EffectType::HIT_IMPACT:
                return sf::Color(255, 255, 255, 255);
            case EffectType::HEAL:
                return sf::Color(100, 255, 100, 200);
            case EffectType::SHIELD_BREAK:
                return sf::Color(200, 150, 255, 255);
            case EffectType::BURNING_ZOMBIE:
                {
                    int r = 255;
                    int g = rng() % 80 + 50;
                    int b = rng() % 30;
                    return sf::Color(r, g, b, 220);
                }
            case EffectType::POISON_VAPOR:
                {
                    int r = rng() % 50;
                    int g = 100 + rng() % 100;
                    int b = rng() % 80;
                    return sf::Color(r, g, b, 180);
                }
            case EffectType::ICE_CRYSTALS:
                {
                    int r = 180 + rng() % 40;
                    int g = 220 + rng() % 35;
                    int b = 255;
                    return sf::Color(r, g, b, 230);
                }
            case EffectType::FIRE_EMBER:
                {
                    int r = 255;
                    int g = 200 + rng() % 55;
                    int b = rng() % 50;
                    return sf::Color(r, g, b, 255);
                }
            case EffectType::TOXIC_BUBBLE:
                {
                    int r = rng() % 100;
                    int g = 150 + rng() % 105;
                    int b = rng() % 50;
                    return sf::Color(r, g, b, 160);
                }
            case EffectType::ZOMBIE_DEATH_SPLATTER:
                {
                    int r = 120 + rng() % 60;
                    int g = rng() % 40;
                    int b = rng() % 30;
                    return sf::Color(r, g, b, 255);
                }
            case EffectType::ZOMBIE_DEATH_CHUNKS:
                {
                    int r = 80 + rng() % 40;
                    int g = 60 + rng() % 30;
                    int b = 40 + rng() % 20;
                    return sf::Color(r, g, b, 230);
                }
            case EffectType::BOSS_DEATH_EXPLOSION:
                {
                    int r = 255;
                    int g = 100 + rng() % 155;
                    int b = rng() % 100;
                    return sf::Color(r, g, b, 255);
                }
            case EffectType::BOSS_DEATH_SHOCKWAVE:
                {
                    int r = 200 + rng() % 55;
                    int g = 150 + rng() % 105;
                    int b = 255;
                    return sf::Color(r, g, b, 200);
                }
            case EffectType::BOSS_DEATH_ENERGY:
                {
                    int r = 150 + rng() % 105;
                    int g = rng() % 150;
                    int b = 200 + rng() % 55;
                    return sf::Color(r, g, b, 220);
                }
            case EffectType::PLANT_DEATH_PETALS:
                {
                    int r = 200 + rng() % 55;
                    int g = 100 + rng() % 100;
                    int b = 150 + rng() % 50;
                    return sf::Color(r, g, b, 200);
                }
            case EffectType::PLANT_DEATH_LEAVES:
                {
                    int r = rng() % 100;
                    int g = 150 + rng() % 105;
                    int b = rng() % 80;
                    return sf::Color(r, g, b, 220);
                }
            case EffectType::PLANT_DEATH_POLLEN:
                {
                    int r = 255;
                    int g = 220 + rng() % 35;
                    int b = 100 + rng() % 100;
                    return sf::Color(r, g, b, 180);
                }
            case EffectType::DISINTEGRATION:
                {
                    int r = 100 + rng() % 155;
                    int g = 100 + rng() % 155;
                    int b = 200 + rng() % 55;
                    return sf::Color(r, g, b, 200);
                }
            case EffectType::SOUL_ESCAPE:
                {
                    int r = 150 + rng() % 50;
                    int g = 200 + rng() % 55;
                    int b = 255;
                    return sf::Color(r, g, b, 150);
                }
            case EffectType::SPARKS:
                {
                    int r = 255;
                    int g = 200 + rng() % 55;
                    int b = 100 + rng() % 100;
                    return sf::Color(r, g, b, 255);
                }
            case EffectType::SPEED_BOOST:
                {
                    int r = 100 + rng() % 155;
                    int g = 200 + rng() % 55;
                    int b = 255;
                    return sf::Color(r, g, b, 200);
                }
            case EffectType::HEIGHT_ADJUSTMENT:
                {
                    int r = 200 + rng() % 55;
                    int g = 150 + rng() % 105;
                    int b = 255;
                    return sf::Color(r, g, b, 180);
                }
            case EffectType::FORCE_MOVEMENT:
                {
                    int r = 255;
                    int g = 150 + rng() % 105;
                    int b = 100 + rng() % 100;
                    return sf::Color(r, g, b, 220);
                }
            case EffectType::LANE_REALIGNMENT:
                {
                    int r = 150 + rng() % 105;
                    int g = 255;
                    int b = 150 + rng() % 105;
                    return sf::Color(r, g, b, 190);
                }
            default:
                return sf::Color::White;
        }
    }
    
    void configureParticleForType(Particle& particle) {
        switch(type) {
            case EffectType::EXPLOSION:
                particle.sizeGrowth = 10.0f;
                particle.alphaDecay = 255.0f;
                particle.turbulence = 50.0f;
                break;
            case EffectType::SMOKE:
                particle.sizeGrowth = 5.0f;
                particle.alphaDecay = 150.0f;
                particle.velocity.y = -20.0f;
                particle.gravity = -10.0f;
                particle.turbulence = 30.0f;
                break;
            case EffectType::FIRE:
                particle.velocity.y = -50.0f;
                particle.sizeGrowth = -2.0f;
                particle.gravity = -80.0f;
                particle.turbulence = 40.0f;
                break;
            case EffectType::SPARKLE:
                particle.sizeGrowth = -1.0f;
                particle.rotationSpeed = 360.0f;
                break;
            case EffectType::BURNING_ZOMBIE:
                particle.velocity.y = -35.0f - (rng() % 20);
                particle.sizeGrowth = 1.0f;
                particle.gravity = -40.0f;
                particle.turbulence = 30.0f;
                particle.rotationSpeed = static_cast<float>((rng() % 200) - 100);
                particle.fadeToTarget = true;
                particle.targetColor = sf::Color(120, 60, 30, 160);
                break;
            case EffectType::POISON_VAPOR:
                particle.velocity.y = -15.0f - (rng() % 20);
                particle.velocity.x += (static_cast<float>(rng()) / RAND_MAX - 0.5f) * 40.0f;
                particle.sizeGrowth = 8.0f;
                particle.gravity = -5.0f;
                particle.turbulence = 80.0f;
                particle.rotationSpeed = static_cast<float>((rng() % 200) - 100);
                particle.usePhysics = false;
                particle.fadeToTarget = true;
                particle.targetColor = sf::Color(20, 60, 20, 50);
                break;
            case EffectType::ICE_CRYSTALS:
                particle.velocity.y = 10.0f + (rng() % 20);
                particle.velocity.x += (static_cast<float>(rng()) / RAND_MAX - 0.5f) * 60.0f;
                particle.sizeGrowth = -0.5f;
                particle.gravity = 15.0f;
                particle.turbulence = 20.0f;
                particle.rotationSpeed = static_cast<float>((rng() % 600) - 300);
                break;
            case EffectType::FIRE_EMBER:
                particle.velocity.y = -100.0f - (rng() % 50);
                particle.velocity.x += (static_cast<float>(rng()) / RAND_MAX - 0.5f) * 30.0f;
                particle.sizeGrowth = -3.0f;
                particle.gravity = -150.0f;
                particle.turbulence = 100.0f;
                particle.fadeToTarget = true;
                particle.targetColor = sf::Color(255, 100, 0, 0);
                break;
            case EffectType::TOXIC_BUBBLE:
                particle.velocity.y = -10.0f - (rng() % 15);
                particle.velocity.x += (static_cast<float>(rng()) / RAND_MAX - 0.5f) * 20.0f;
                particle.sizeGrowth = 3.0f;
                particle.gravity = -8.0f;
                particle.turbulence = 40.0f;
                particle.rotationSpeed = static_cast<float>((rng() % 100) - 50);
                particle.usePhysics = false;
                break;
            case EffectType::ZOMBIE_DEATH_SPLATTER:
                particle.velocity.y += (static_cast<float>(rng()) / RAND_MAX - 0.7f) * 100.0f;
                particle.sizeGrowth = -2.0f;
                particle.gravity = 200.0f;
                particle.turbulence = 10.0f;
                particle.rotationSpeed = static_cast<float>((rng() % 600) - 300);
                break;
            case EffectType::ZOMBIE_DEATH_CHUNKS:
                particle.velocity.y -= static_cast<float>(rng() % 100);
                particle.sizeGrowth = -1.0f;
                particle.gravity = 250.0f;
                particle.turbulence = 5.0f;
                particle.rotationSpeed = static_cast<float>((rng() % 400) - 200);
                particle.fadeToTarget = true;
                particle.targetColor = sf::Color(50, 30, 20, 100);
                break;
            case EffectType::BOSS_DEATH_EXPLOSION:
                particle.sizeGrowth = 15.0f;
                particle.gravity = 0.0f;
                particle.turbulence = 100.0f;
                particle.rotationSpeed = static_cast<float>((rng() % 800) - 400);
                particle.usePhysics = false;
                break;
            case EffectType::BOSS_DEATH_SHOCKWAVE:
                particle.sizeGrowth = 50.0f;
                particle.gravity = 0.0f;
                particle.turbulence = 0.0f;
                particle.usePhysics = false;
                particle.alphaDecay = 180.0f;
                break;
            case EffectType::BOSS_DEATH_ENERGY:
                particle.velocity.y -= 50.0f;
                particle.sizeGrowth = 5.0f;
                particle.gravity = -30.0f;
                particle.turbulence = 80.0f;
                particle.rotationSpeed = static_cast<float>((rng() % 1000) - 500);
                particle.fadeToTarget = true;
                particle.targetColor = sf::Color(255, 100, 255, 0);
                break;
            case EffectType::PLANT_DEATH_PETALS:
                particle.velocity.y -= static_cast<float>(rng() % 50);
                particle.velocity.x += (static_cast<float>(rng()) / RAND_MAX - 0.5f) * 100.0f;
                particle.sizeGrowth = -0.5f;
                particle.gravity = 30.0f;
                particle.turbulence = 50.0f;
                particle.rotationSpeed = static_cast<float>((rng() % 360) - 180);
                break;
            case EffectType::PLANT_DEATH_LEAVES:
                particle.velocity.y -= static_cast<float>(rng() % 30);
                particle.sizeGrowth = 0.0f;
                particle.gravity = 20.0f;
                particle.turbulence = 60.0f;
                particle.rotationSpeed = static_cast<float>((rng() % 200) - 100);
                particle.fadeToTarget = true;
                particle.targetColor = sf::Color(100, 80, 50, 50);
                break;
            case EffectType::PLANT_DEATH_POLLEN:
                particle.velocity.y -= 20.0f + static_cast<float>(rng() % 40);
                particle.velocity.x += (static_cast<float>(rng()) / RAND_MAX - 0.5f) * 60.0f;
                particle.sizeGrowth = 1.0f;
                particle.gravity = -5.0f;
                particle.turbulence = 100.0f;
                particle.rotationSpeed = 0.0f;
                particle.alphaDecay = 180.0f;
                break;
            case EffectType::DISINTEGRATION:
                particle.velocity *= 0.3f;
                particle.sizeGrowth = -1.0f;
                particle.gravity = -10.0f;
                particle.turbulence = 150.0f;
                particle.rotationSpeed = static_cast<float>((rng() % 1200) - 600);
                particle.fadeToTarget = true;
                particle.targetColor = sf::Color(255, 255, 255, 0);
                break;
            case EffectType::SOUL_ESCAPE:
                particle.velocity.y = -100.0f - static_cast<float>(rng() % 50);
                particle.velocity.x = (static_cast<float>(rng()) / RAND_MAX - 0.5f) * 20.0f;
                particle.sizeGrowth = 2.0f;
                particle.gravity = -50.0f;
                particle.turbulence = 30.0f;
                particle.rotationSpeed = 0.0f;
                particle.fadeToTarget = true;
                particle.targetColor = sf::Color(255, 255, 255, 0);
                particle.usePhysics = false;
                break;
            case EffectType::SPARKS:
                particle.velocity *= 1.5f;
                particle.sizeGrowth = -2.0f;
                particle.gravity = 100.0f;
                particle.turbulence = 20.0f;
                particle.rotationSpeed = static_cast<float>((rng() % 1000) - 500);
                particle.fadeToTarget = true;
                particle.targetColor = sf::Color(255, 100, 0, 0);
                break;
            case EffectType::SPEED_BOOST:
                particle.velocity.y = -80.0f - static_cast<float>(rng() % 40);
                particle.velocity.x *= 0.5f;
                particle.sizeGrowth = 3.0f;
                particle.gravity = -60.0f;
                particle.turbulence = 60.0f;
                particle.rotationSpeed = static_cast<float>((rng() % 400) - 200);
                particle.fadeToTarget = true;
                particle.targetColor = sf::Color(200, 255, 255, 50);
                break;
            case EffectType::HEIGHT_ADJUSTMENT:
                particle.velocity.y = static_cast<float>((rng() % 100) - 50);
                particle.velocity.x += (static_cast<float>(rng()) / RAND_MAX - 0.5f) * 40.0f;
                particle.sizeGrowth = 1.0f;
                particle.gravity = 20.0f;
                particle.turbulence = 80.0f;
                particle.rotationSpeed = static_cast<float>((rng() % 600) - 300);
                particle.usePhysics = false;
                break;
            case EffectType::FORCE_MOVEMENT:
                particle.velocity.x = -120.0f - static_cast<float>(rng() % 80);
                particle.velocity.y += (static_cast<float>(rng()) / RAND_MAX - 0.5f) * 60.0f;
                particle.sizeGrowth = -1.0f;
                particle.gravity = 30.0f;
                particle.turbulence = 40.0f;
                particle.rotationSpeed = static_cast<float>((rng() % 800) - 400);
                particle.fadeToTarget = true;
                particle.targetColor = sf::Color(255, 0, 0, 100);
                break;
            case EffectType::LANE_REALIGNMENT:
                particle.velocity.y = 0.0f;
                particle.velocity.x += (static_cast<float>(rng()) / RAND_MAX - 0.5f) * 80.0f;
                particle.sizeGrowth = 2.0f;
                particle.gravity = 10.0f;
                particle.turbulence = 50.0f;
                particle.rotationSpeed = static_cast<float>((rng() % 300) - 150);
                particle.fadeToTarget = true;
                particle.targetColor = sf::Color(100, 255, 100, 80);
                break;
            default:
                break;
        }
    }
    
    void updateVertices() {
        vertices.clear();
        
        for (const auto& particle : particles) {
            if (!particle.isActive) continue;
            
            float size = particle.size;
            sf::Vector2f pos = particle.getPosition();
            
            sf::Vertex quad[4];
            
            quad[0].position = sf::Vector2f(pos.x - size, pos.y - size);
            quad[1].position = sf::Vector2f(pos.x + size, pos.y - size);
            quad[2].position = sf::Vector2f(pos.x + size, pos.y + size);
            quad[3].position = sf::Vector2f(pos.x - size, pos.y + size);
            
            for (int i = 0; i < 4; ++i) {
                quad[i].color = particle.color;
                vertices.append(quad[i]);
            }
        }
    }
    
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        states.transform *= getTransform();
        states.blendMode = sf::BlendAdd;
        target.draw(vertices, states);
    }
};

class ScreenEffect {
public:
    enum Type {
        FLASH,
        FADE,
        SLOW_MOTION
    };
    
    Type type;
    float duration;
    float intensity;
    float timer;
    bool isActive;
    
    ScreenEffect(Type t, float dur, float intens) 
        : type(t), duration(dur), intensity(intens), timer(0), isActive(true) {}
    
    void update(float deltaTime) {
        timer += deltaTime;
        if (timer >= duration) {
            isActive = false;
        }
    }
    
    float getProgress() const {
        return timer / duration;
    }
};

class EffectManager {
private:
    std::vector<std::unique_ptr<ParticleSystem>> particleSystems;
    std::vector<std::unique_ptr<ScreenEffect>> screenEffects;
    sf::View* gameView;
    sf::Vector2f originalViewCenter;
    float timeScale;
    static EffectManager instance;
    
public:
    EffectManager() : gameView(nullptr), timeScale(1.0f) {}
    
    static EffectManager& getInstance() {
        return instance;
    }
    
    void setView(sf::View* view) {
        gameView = view;
        if (view) {
            originalViewCenter = view->getCenter();
        }
    }
    
    void addExplosion(const sf::Vector2f& position, float radius = 100.0f) {
        auto explosion = std::make_unique<ParticleSystem>(EffectType::EXPLOSION);
        explosion->emit(position, static_cast<int>(radius * 2));
        particleSystems.push_back(std::move(explosion));
    }
    
    void addFreezeEffect(const sf::Vector2f& position) {
        auto freeze = std::make_unique<ParticleSystem>(EffectType::FREEZE);
        freeze->emit(position, 30);
        particleSystems.push_back(std::move(freeze));
    }
    
    void addPoisonCloud(const sf::Vector2f& position, float duration = 3.0f) {
        auto poison = std::make_unique<ParticleSystem>(EffectType::POISON_CLOUD);
        poison->emit(position);
        particleSystems.push_back(std::move(poison));
    }
    
    void addFireEffect(const sf::Vector2f& position) {
        auto fire = std::make_unique<ParticleSystem>(EffectType::FIRE);
        fire->emit(position);
        particleSystems.push_back(std::move(fire));
    }
    
    void addHitImpact(const sf::Vector2f& position) {
        auto impact = std::make_unique<ParticleSystem>(EffectType::HIT_IMPACT);
        impact->emit(position, 10);
        particleSystems.push_back(std::move(impact));
    }
    
    void addHealEffect(const sf::Vector2f& position) {
        auto heal = std::make_unique<ParticleSystem>(EffectType::HEAL);
        heal->emit(position, 20);
        particleSystems.push_back(std::move(heal));
    }
    
    void addBurningZombieEffect(const sf::Vector2f& position) {
        sf::Vector2f pos = sf::Vector2f(position.x + 15.f, position.y + 35.f);
        auto burning = std::make_unique<ParticleSystem>(EffectType::BURNING_ZOMBIE);
        burning->emit(pos, 40);
        particleSystems.push_back(std::move(burning));
        
        auto embers = std::make_unique<ParticleSystem>(EffectType::FIRE_EMBER);
        embers->emit(pos, 20);
        particleSystems.push_back(std::move(embers));
    }
    
    void addPoisonVaporEffect(const sf::Vector2f& position) {
        auto vapor = std::make_unique<ParticleSystem>(EffectType::POISON_VAPOR);
        vapor->emit(position);
        particleSystems.push_back(std::move(vapor));
        
        auto bubbles = std::make_unique<ParticleSystem>(EffectType::TOXIC_BUBBLE);
        bubbles->emit(position);
        particleSystems.push_back(std::move(bubbles));
    }
    
    void addIceCrystalsEffect(const sf::Vector2f& position) {
        auto crystals = std::make_unique<ParticleSystem>(EffectType::ICE_CRYSTALS);
        crystals->emit(position);
        particleSystems.push_back(std::move(crystals));
        
        auto sparkle = std::make_unique<ParticleSystem>(EffectType::SPARKLE);
        sparkle->emit(position, 15);
        particleSystems.push_back(std::move(sparkle));
    }
    
    ParticleSystem* createContinuousEffect(EffectType type, const sf::Vector2f& position) {
        auto system = std::make_unique<ParticleSystem>(type);
        system->emit(position);
        ParticleSystem* ptr = system.get();
        particleSystems.push_back(std::move(system));
        return ptr;
    }
    
    void addZombieDeath(const sf::Vector2f& position) {
        auto splatter = std::make_unique<ParticleSystem>(EffectType::ZOMBIE_DEATH_SPLATTER);
        splatter->emit(position, 30);
        particleSystems.push_back(std::move(splatter));
        
        auto chunks = std::make_unique<ParticleSystem>(EffectType::ZOMBIE_DEATH_CHUNKS);
        chunks->emit(position, 8);
        particleSystems.push_back(std::move(chunks));
    }
    
    void addBossDeath(const sf::Vector2f& position) {
        auto explosion = std::make_unique<ParticleSystem>(EffectType::BOSS_DEATH_EXPLOSION);
        explosion->emit(position, 100);
        particleSystems.push_back(std::move(explosion));
        
        auto shockwave = std::make_unique<ParticleSystem>(EffectType::BOSS_DEATH_SHOCKWAVE);
        shockwave->emit(position, 20);
        particleSystems.push_back(std::move(shockwave));
        
        auto energy = std::make_unique<ParticleSystem>(EffectType::BOSS_DEATH_ENERGY);
        energy->emit(position, 50);
        particleSystems.push_back(std::move(energy));
        
        auto soul = std::make_unique<ParticleSystem>(EffectType::SOUL_ESCAPE);
        soul->emit(position, 5);
        particleSystems.push_back(std::move(soul));
        addScreenFlash(0.5f, 0.8f);
        addSlowMotion(0.5f, 0.3f);
    }
    
    void addPlantDeath(const sf::Vector2f& position) {
        auto petals = std::make_unique<ParticleSystem>(EffectType::PLANT_DEATH_PETALS);
        petals->emit(position, 15);
        particleSystems.push_back(std::move(petals));
        
        auto leaves = std::make_unique<ParticleSystem>(EffectType::PLANT_DEATH_LEAVES);
        leaves->emit(position, 10);
        particleSystems.push_back(std::move(leaves));
        
        auto pollen = std::make_unique<ParticleSystem>(EffectType::PLANT_DEATH_POLLEN);
        pollen->emit(position, 30);
        particleSystems.push_back(std::move(pollen));
    }
    
    void addDisintegrationEffect(const sf::Vector2f& position) {
        auto disintegrate = std::make_unique<ParticleSystem>(EffectType::DISINTEGRATION);
        disintegrate->emit(position, 80);
        particleSystems.push_back(std::move(disintegrate));
    }
    
    void addSparks(const sf::Vector2f& position, int count = 15) {
        auto sparks = std::make_unique<ParticleSystem>(EffectType::SPARKS);
        sparks->emit(position, count);
        particleSystems.push_back(std::move(sparks));
    }
    
    void addSpeedBoostEffect(const sf::Vector2f& position) {
        auto speedBoost = std::make_unique<ParticleSystem>(EffectType::SPEED_BOOST);
        speedBoost->emit(position, 25);
        particleSystems.push_back(std::move(speedBoost));
    }
    
    void addHeightAdjustmentEffect(const sf::Vector2f& position) {
        auto heightAdj = std::make_unique<ParticleSystem>(EffectType::HEIGHT_ADJUSTMENT);
        heightAdj->emit(position, 20);
        particleSystems.push_back(std::move(heightAdj));
    }
    
    void addForceMovementEffect(const sf::Vector2f& position) {
        auto forceMove = std::make_unique<ParticleSystem>(EffectType::FORCE_MOVEMENT);
        forceMove->emit(position, 30);
        particleSystems.push_back(std::move(forceMove));
    }
    
    void addLaneRealignmentEffect(const sf::Vector2f& position) {
        auto laneAlign = std::make_unique<ParticleSystem>(EffectType::LANE_REALIGNMENT);
        laneAlign->emit(position, 18);
        particleSystems.push_back(std::move(laneAlign));
    }
    
    void addHit(const sf::Vector2f& position) {
        addHitImpact(position);
    }
    
    void addScreenFlash(float duration, float intensity = 1.0f) {
        screenEffects.push_back(
            std::make_unique<ScreenEffect>(ScreenEffect::FLASH, duration, intensity)
        );
    }
    
    void addSlowMotion(float duration, float factor = 0.5f) {
        screenEffects.push_back(
            std::make_unique<ScreenEffect>(ScreenEffect::SLOW_MOTION, duration, factor)
        );
        timeScale = factor;
    }
    
    void update(float deltaTime) {
        deltaTime *= timeScale;
        
        for (auto& system : particleSystems) {
            system->update(deltaTime);
        }
        
        particleSystems.erase(
            std::remove_if(particleSystems.begin(), particleSystems.end(),
                [](const std::unique_ptr<ParticleSystem>& system) {
                    return !system->isActive();
                }),
            particleSystems.end()
        );
        
        timeScale = 1.0f;
        
        for (auto& effect : screenEffects) {
            effect->update(deltaTime);
            if (effect->type == ScreenEffect::SLOW_MOTION) {
                timeScale = effect->intensity;
            }
        }
        
        screenEffects.erase(
            std::remove_if(screenEffects.begin(), screenEffects.end(),
                [](const std::unique_ptr<ScreenEffect>& effect) {
                    return !effect->isActive;
                }),
            screenEffects.end()
        );
        
        if (screenEffects.empty() && gameView) {
            gameView->setCenter(originalViewCenter);
        }
    }
    
    void draw(sf::RenderWindow& window) {
        for (const auto& system : particleSystems) {
            window.draw(*system);
        }
        
        for (const auto& effect : screenEffects) {
            if (effect->type == ScreenEffect::FLASH) {
                applyScreenFlash(window, effect.get());
            }
        }
    }
    
    float getTimeScale() const {
        return timeScale;
    }
    
    void clear() {
        particleSystems.clear();
        screenEffects.clear();
        timeScale = 1.0f;
    }
    
private:
    void applyScreenFlash(sf::RenderWindow& window, ScreenEffect* flash) {
        float alpha = (1.0f - flash->getProgress()) * flash->intensity * 255;
        
        sf::RectangleShape flashOverlay(sf::Vector2f(window.getSize()));
        flashOverlay.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(alpha)));
        
        sf::View currentView = window.getView();
        window.setView(window.getDefaultView());
        window.draw(flashOverlay);
        window.setView(currentView);
    }
};

}
