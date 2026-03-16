#include "BossState.h"
#include "BossZombies.h"
#include "SoundManager.h"
#include "MovementStrategy.h"
#include "Constants.h"
#include <cmath>
#include <cstdlib>

const float BOSS_MOVEMENT_SPEED = 18.0f;  
const float BOSS_ATTACK_SPEED = 12.0f;
const float RIGHT_BOUNDARY = 600.0f;
const float LEFT_BOUNDARY = 0.0f;
const int BOSS_SHOT_PROBABILITY = 8;
const float SHOT_X_OFFSET = 30.0f;
const float SHOT_Y_OFFSET = 40.0f;
const float ATTACK_COOLDOWN = 1.0f;

void ChasePlayerState::enter(BossEnemy& boss) {
    stateTimer.restart();
    boss.setMovementStrategy(std::make_unique<PathfindingMovementStrategy>(BOSS_MOVEMENT_SPEED));
}

void ChasePlayerState::execute(BossEnemy& boss, float dt) {
    float prevX = boss.coords.x;
    if (!boss.isAttacking) {
        boss.canAdvance = true;
        boss.coords.x -= boss.velocity * dt;
        if (boss.coords.x < LEFT_BOUNDARY) {
            boss.reachedEnd = true;
            boss.enabled = false;
        }
    }
    boss.display.setPosition(boss.coords.x, boss.coords.y);
}

void ChasePlayerState::exit(BossEnemy& boss) {
}

void SpecialAttackState::enter(BossEnemy& boss) {
    stateTimer.restart();
    attackTimer.restart();
    hasAttacked = false;
    boss.canAdvance = true;
    boss.setMovementStrategy(std::make_unique<LinearMovementStrategy>(BOSS_ATTACK_SPEED));
}

void SpecialAttackState::execute(BossEnemy& boss, float dt) {
    
    float prevX = boss.coords.x;
    if (!boss.isAttacking) {
        boss.canAdvance = true;
        boss.coords.x -= boss.velocity * dt * 0.3f;
        if (boss.coords.x < LEFT_BOUNDARY) {
            boss.reachedEnd = true;
            boss.enabled = false;
        }
    }
    boss.display.setPosition(boss.coords.x, boss.coords.y);
}

void SpecialAttackState::exit(BossEnemy& boss) {
    boss.canAdvance = true;
    hasAttacked = false;
}

void IdleState::enter(BossEnemy& boss) {
    stateTimer.restart();
    hasSummoned = false;
    boss.setMovementStrategy(std::make_unique<WanderingMovementStrategy>(1.0f, 0.05f, 2.0f));
}

void IdleState::execute(BossEnemy& boss, float dt) {
    float prevX = boss.coords.x;
    if (!boss.isAttacking) {
        boss.canAdvance = true;
        boss.coords.x -= boss.velocity * dt * 0.2f;
        if (boss.coords.x < LEFT_BOUNDARY) {
            boss.reachedEnd = true;
            boss.enabled = false;
        }
    }
    boss.display.setPosition(boss.coords.x, boss.coords.y);
    
    if (!hasSummoned && stateTimer.getElapsedTime().asSeconds() > 1.0f) {
        hasSummoned = true;
        boss.hasSpawnedAllies = true;
    }
}

void IdleState::exit(BossEnemy& boss) {
    hasSummoned = false;
}

void RetreatState::enter(BossEnemy& boss) {
    stateTimer.restart();
    retreatTarget.x = boss.coords.x + 100.0f;
    retreatTarget.y = boss.coords.y;
    
    if (retreatTarget.x > 700.0f) retreatTarget.x = 700.0f;
    float minY = LaneSystem::getLaneY(0);
    float maxY = LaneSystem::getLaneY(LaneSystem::NUM_LANES - 1) + LaneSystem::LANE_HEIGHT;
    if (retreatTarget.y < minY) retreatTarget.y = minY;
    if (retreatTarget.y > maxY) retreatTarget.y = maxY;
    
    boss.setMovementStrategy(std::make_unique<PathfindingMovementStrategy>(6.0f));
}

void RetreatState::execute(BossEnemy& boss, float dt) {
    float prevX = boss.coords.x;
    sf::Vector2f direction;
    direction.x = retreatTarget.x - boss.coords.x;
    direction.y = retreatTarget.y - boss.coords.y;
    
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    
    if (distance > 10.0f) {
        direction /= distance;
        boss.coords.x += direction.x * 240.0f * dt;
        boss.coords.y += direction.y * 240.0f * dt;
        int lane = LaneSystem::getLane(boss.coords.y);
        if (lane >= 0) {
            boss.coords.y = LaneSystem::getLaneCenterY(lane) + 8.0f;
        }
        boss.display.setPosition(boss.coords.x, boss.coords.y);
    }
    
}

void RetreatState::exit(BossEnemy& boss) {
    boss.canAdvance = true;
}
