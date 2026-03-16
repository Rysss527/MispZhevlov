#include "services/CombatManager.h"
#include <cstdlib>
#include <algorithm>

namespace DefenseSynth {

CombatManager::CombatManager() {
    combatClock.restart();
}

void CombatManager::resolveCombat(const CollisionService::CollisionResult& collision) {
    if (!collision.hasCollision) return;
    
    if (collision.type1 == "tower" && collision.type2 == "enemy") {
        Tower* tower = static_cast<Tower*>(collision.object1);
        Enemy* enemy = static_cast<Enemy*>(collision.object2);
        
        if (tower && enemy) {
            enemy->canAdvance = false;
            queueDamage(tower, "tower", 1, DamageType::Physical);
        }
    }
}

void CombatManager::applyDamage(Enemy* target, int damage, DamageType type) {
    if (!target || !target->enabled) return;
    
    int finalDamage = calculateDamage(damage, type, checkCriticalHit());
    target->hitpoints -= finalDamage;
    
    if (target->hitpoints <= 0) {
        target->triggerDeath();
    }
}

void CombatManager::applyDamage(Tower* target, int damage) {
    if (!target || !target->operational) return;
    
    target->vitality -= damage;
    
    if (target->vitality <= 0) {
        target->operational = false;
    }
}

void CombatManager::applyStatusEffect(Enemy* enemy, StatusEffect effect, float duration) {
    if (!enemy || !enemy->enabled) return;
    
    switch (effect) {
        case StatusEffect::Frozen:
            enemy->is_frozen = true;
            enemy->freeze_duration = duration;
            enemy->freeze_timer.restart();
            break;
            
        case StatusEffect::Poisoned:
            enemy->is_poisoned = true;
            enemy->poison_duration = duration;
            enemy->poison_timer.restart();
            break;
            
        case StatusEffect::Burning:
            enemy->is_burning = true;
            enemy->fire_duration = duration;
            enemy->fire_timer.restart();
            break;
            
        case StatusEffect::Slowed:
            enemy->velocity *= 0.5f;
            break;
            
        case StatusEffect::Stunned:
            enemy->canAdvance = false;
            break;
            
        default:
            break;
    }
}

void CombatManager::queueDamage(void* target, const std::string& targetType, int damage, 
                               DamageType type, StatusEffect effect, float effectDuration) {
    DamageEvent event;
    event.target = target;
    event.targetType = targetType;
    event.damage = damage;
    event.type = type;
    event.effect = effect;
    event.effectDuration = effectDuration;
    
    damageQueue.push(event);
}

void CombatManager::processDamageQueue() {
    while (!damageQueue.empty()) {
        DamageEvent event = damageQueue.front();
        damageQueue.pop();
        
        if (event.targetType == "enemy") {
            Enemy* enemy = static_cast<Enemy*>(event.target);
            applyDamage(enemy, event.damage, event.type);
            
            if (event.effect != StatusEffect::None) {
                applyStatusEffect(enemy, event.effect, event.effectDuration);
            }
        }
        else if (event.targetType == "tower") {
            Tower* tower = static_cast<Tower*>(event.target);
            applyDamage(tower, event.damage);
        }
    }
}

void CombatManager::updateCombatEffects(float deltaTime) {
    processDamageQueue();
}

void CombatManager::clearDamageQueue() {
    while (!damageQueue.empty()) {
        damageQueue.pop();
    }
}

int CombatManager::calculateDamage(int baseDamage, DamageType type, bool isCritical) const {
    int damage = baseDamage;
    
    if (isCritical) {
        damage = static_cast<int>(damage * CRITICAL_HIT_MULTIPLIER);
    }
    
    switch (type) {
        case DamageType::Fire:
            damage = static_cast<int>(damage * 1.2f);
            break;
        case DamageType::Explosive:
            damage = static_cast<int>(damage * 1.5f);
            break;
        case DamageType::Freeze:
            damage = static_cast<int>(damage * 0.8f);
            break;
        default:
            break;
    }
    
    return damage;
}

bool CombatManager::checkCriticalHit() const {
    return (rand() % 100) < (CRITICAL_HIT_CHANCE * 100);
}

}