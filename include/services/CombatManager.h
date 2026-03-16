#pragma once

#include <queue>
#include <memory>
#include <SFML/Graphics.hpp>
#include "../Tower.h"
#include "../Enemy.h"
#include "CollisionService.h"

namespace DefenseSynth {

enum class DamageType {
    Physical,
    Freeze,
    Poison,
    Fire,
    Explosive
};

enum class StatusEffect {
    None,
    Frozen,
    Poisoned,
    Burning,
    Slowed,
    Stunned
};

struct DamageEvent {
    void* target;
    std::string targetType;
    int damage;
    DamageType type;
    StatusEffect effect;
    float effectDuration;
};

class CombatManager {
public:
    CombatManager();
    ~CombatManager() = default;
    
    void resolveCombat(const CollisionService::CollisionResult& collision);
    void applyDamage(Enemy* target, int damage, DamageType type = DamageType::Physical);
    void applyDamage(Tower* target, int damage);
    void applyStatusEffect(Enemy* enemy, StatusEffect effect, float duration = 3.0f);
    
    void queueDamage(void* target, const std::string& targetType, int damage, 
                    DamageType type = DamageType::Physical, 
                    StatusEffect effect = StatusEffect::None,
                    float effectDuration = 0.0f);
    void processDamageQueue();
    
    void updateCombatEffects(float deltaTime);
    void clearDamageQueue();
    
    int calculateDamage(int baseDamage, DamageType type, bool isCritical = false) const;
    bool checkCriticalHit() const;
    
private:
    std::queue<DamageEvent> damageQueue;
    sf::Clock combatClock;
    
    static constexpr float CRITICAL_HIT_CHANCE = 0.05f;
    static constexpr float CRITICAL_HIT_MULTIPLIER = 2.0f;
    static constexpr float FREEZE_DURATION = 3.0f;
    static constexpr float POISON_DURATION = 5.0f;
    static constexpr float BURN_DURATION = 4.0f;
    static constexpr int POISON_DAMAGE_PER_TICK = 2;
    static constexpr int BURN_DAMAGE_PER_TICK = 3;
};

}
