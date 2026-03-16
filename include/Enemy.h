#pragma once
#include <SFML/Graphics.hpp>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include "GameCursor.h"
#include <fstream>
#include <memory>

#include "Constants.h"
#include "MovementStrategy.h"
#include "IEnemyBehavior.h"
#include "GameContext.h"
#include "EffectManager.h"
#include "TowerFactory.h"
#include "ResourceManager.h"
#include "StuckDetector.h"

inline int calculateZombieLevel(int gameLevel, float difficulty) {
	float levelProbability = (gameLevel - 1) * 0.15f + difficulty * 0.2f;
	int randomValue = rand() % 100;
	
	if (randomValue < 50 - levelProbability * 8) return 0;
	else if (randomValue < 70 - levelProbability * 6) return 1;
	else if (randomValue < 85 - levelProbability * 4) return 2;
	else if (randomValue < 95 - levelProbability * 2) return 3;
	else if (randomValue < 98) return 4;
	else return 5;
}

class Enemy
{
public:
	int hitpoints;
	float velocity;
	bool enabled;
	coordinates coords;
	sf::Sprite display;
	sf::Texture texture;
	sf::Clock moveClock;
	sf::Clock behaviorClock;
	bool summon;
	int points;
	sf::Clock clock2;
	int numframes;
	int frame;
	bool canAdvance;
	std::string type;
	bool hasSpawnedAllies;
	float targetY;
	int scoreValue;
	float freeze_duration;
	sf::Clock freeze_timer;
	bool is_frozen;
	float poison_duration;
	sf::Clock poison_timer;
	bool is_poisoned;
	int poison_damage;
	float fire_duration;
	sf::Clock fire_timer;
	bool is_burning;
	int fire_damage;
	sf::Texture shotTexture;
	sf::Sprite shotSprite;
	bool hasShotActive;
	coordinates shotPosition;
	sf::Clock shotClock;
	sf::Clock animationClock;
	bool hasExploded;
	int level;
	sf::Font upgradeFont;
	sf::Text upgradeLevelText;
	bool hasUpgradeIndicator;
	sf::Font debugFont;
	sf::Text debugLabel;
	bool showDebugLabel;
	bool isAttacking;
	int attackDamage;
	sf::Clock attackTimer;
	bool reachedEnd;
	
	std::unique_ptr<IMovementStrategy> movementStrategy;
	static std::vector<Enemy*> allEnemiesRef;
	
	std::unique_ptr<DefenseSynth::EnemyBehaviorContext> behaviorContext;
	Enemy(float x = 10, float y = 10)
	{
		numframes = 9;
		frame = 0;
		hitpoints = GameConstants::BASIC_ZOMBIE_HEALTH;
		coords.x = x;
		coords.y = y;
		velocity = 15.0f;
		freeze_duration = 0;
		is_frozen = false;
		poison_duration = 0;
		is_poisoned = false;
		poison_damage = 2;
		fire_duration = 0;
		is_burning = false;
		fire_damage = 3;
		enabled = true;
		isAttacking = false;
		attackDamage = 1;
		attackTimer.restart();
		reachedEnd = false;
		auto* context = DefenseSynth::GameContext::getInstance();
		level = calculateZombieLevel(context->getCurrentLevel(), context->getDifficulty());
		hasUpgradeIndicator = false;
		int zombieType = rand() % 2;
		if (zombieType == 0) {
			type = GameConstants::BASIC_ZOMBIE;
			velocity = 12.0f;
			display.setTexture(DefenseSynth::ResourceManager::getInstance().getTexture("zombie"));
		} else {
			type = GameConstants::BASIC_ZOMBIE1;
			velocity = 10.0f;
			display.setTexture(DefenseSynth::ResourceManager::getInstance().getTexture("zombie1"));
		}
		display.setTextureRect(sf::IntRect(0, 0, GameConstants::SPRITE_WIDTH, GameConstants::SPRITE_HEIGHT));
		display.setOrigin(GameConstants::SPRITE_WIDTH / 2.0f, GameConstants::SPRITE_HEIGHT / 2.0f);
		coords.x = x + GameConstants::SPRITE_WIDTH / 2.0f;
		coords.y = y + GameConstants::SPRITE_HEIGHT / 2.0f;
		display.setPosition(coords.x, coords.y);
		canAdvance = true;
		targetY = 0;
		scoreValue = 65;
		points = 65;
		hasSpawnedAllies = false;
		freeze_duration = 0.0f;
		animationClock.restart();
		shotSprite.setTexture(DefenseSynth::ResourceManager::getInstance().getTexture("shot"));
		shotSprite.setTextureRect(sf::IntRect(0, 0, 30, 30));
		hasShotActive = false;
		shotPosition.x = 0;
		shotPosition.y = 0;
		hasExploded = false;
		
		upgradeLevelText.setFont(DefenseSynth::ResourceManager::getInstance().getFont("arial"));
		upgradeLevelText.setCharacterSize(20);
		upgradeLevelText.setFillColor(sf::Color::Red);
		upgradeLevelText.setStyle(sf::Text::Bold);
		upgradeLevelText.setOutlineThickness(2);
		upgradeLevelText.setOutlineColor(sf::Color::Black);
		updateUpgradeIndicator();
		
		showDebugLabel = false;
		debugLabel.setFont(DefenseSynth::ResourceManager::getInstance().getFont("arial"));
		debugLabel.setCharacterSize(16);
		debugLabel.setFillColor(sf::Color::White);
		debugLabel.setStyle(sf::Text::Bold);
		debugLabel.setString(type);
		debugLabel.setPosition(coords.x, coords.y - 35);
		
		if (level > 0) {
			auto* context = DefenseSynth::GameContext::getInstance();
			int hitpointsBonus = static_cast<int>(level * GameConstants::LEVEL_HEALTH_BASE_BONUS * (1.0f + context->getDifficulty() * 0.4f));
			hitpoints += hitpointsBonus;
			float maxVelocityIncrease = velocity * 0.5f;
			float velocityIncrease = std::min(level * (0.5f + context->getDifficulty() * 0.3f), maxVelocityIncrease);
			velocity = velocity + velocityIncrease;
			int scoreBonus = (level - 1) * 15;
			scoreValue += scoreBonus;
			points += scoreBonus;
			poison_damage += (level - 1);
			fire_damage += (level - 1) * 2;
			attackDamage += level * GameConstants::LEVEL_DAMAGE_BASE_BONUS;
		}
		
		setMovementStrategy(std::make_unique<LinearMovementStrategy>(1.2f));
		
		behaviorContext = std::make_unique<DefenseSynth::EnemyBehaviorContext>(this);
		auto behavior = DefenseSynth::BehaviorFactory::createBehaviorForEnemyType(type);
		behaviorContext->setBehavior(std::move(behavior));
		
		DefenseSynth::StuckDetector::getInstance().registerEnemy(this, sf::Vector2f(coords.x, coords.y));
	}
	
	void updateUpgradeIndicator() {
		if (level > 0) {
			hasUpgradeIndicator = true;
			std::string indicator = "Lvl: ";
			for (int i = 0; i < level; i++) {
				indicator += "*";
			}
			upgradeLevelText.setString(indicator);
			upgradeLevelText.setPosition(coords.x, coords.y + 55);
		} else {
			hasUpgradeIndicator = false;
		}
	}
	Enemy(const Enemy &e)
	{
		hitpoints = e.hitpoints;
		coords.x = e.coords.x;
		coords.y = e.coords.y;
		velocity = e.velocity;
		enabled = e.enabled;
		texture = e.texture;
		display = e.display;
		canAdvance = e.canAdvance;
		type = e.type;
		frame = e.frame;
		numframes = e.numframes;
		scoreValue = e.scoreValue;
		points = e.points;
		hasSpawnedAllies = e.hasSpawnedAllies;
		targetY = e.targetY;
		level = e.level;
		hasUpgradeIndicator = e.hasUpgradeIndicator;
		shotTexture = e.shotTexture;
		shotSprite = e.shotSprite;
		hasShotActive = e.hasShotActive;
		shotPosition = e.shotPosition;
		shotClock = e.shotClock;
		hasExploded = e.hasExploded;
		isAttacking = e.isAttacking;
		attackDamage = e.attackDamage;
		attackTimer.restart();
		reachedEnd = e.reachedEnd;
	}
	
	int getLane(float y) {
		return LaneSystem::getLane(y);
	}
	
	void checkPlantCollision(TowerFactory* plants) {
		if (!enabled || !plants) return;
		
		bool isFlying = (type == GameConstants::FLYING_ZOMBIE || type == "boss_flying");
		int myLane = LaneSystem::getLane(coords.y);
		canAdvance = true;
		
		bool foundPlantToAttack = false;
		for (auto& plant : plants->towers) {
			if (!plant || !plant->operational) continue;
			
			int plantLane = LaneSystem::getLane(plant->location.y);
			if (myLane != plantLane) continue;
			
			float distance = std::abs(coords.x - plant->location.x);
			float attackRange = isFlying ? 120.0f : 100.0f;
			float attackOffset = isFlying ? -30.0f : -20.0f;
			
			if (distance < attackRange && coords.x >= plant->location.x + attackOffset) {
				foundPlantToAttack = true;
				canAdvance = false;
				if (!isAttacking) {
					attackTimer.restart();
					clock2.restart();
					
					if (isFlying) {
						DefenseSynth::EffectManager::getInstance().addSparks(sf::Vector2f(coords.x, coords.y + 30), 15);
					}
				}
				isAttacking = true;
				
				bool usesShotSprite = (type != GameConstants::BASIC_ZOMBIE && type != GameConstants::BASIC_ZOMBIE1 && type.find("boss_basic") == std::string::npos);
				
				if (usesShotSprite) {
					hasShotActive = true;
					shotPosition.x = coords.x - 25;
					shotPosition.y = isFlying ? coords.y + 35 : coords.y + 25;
					shotSprite.setPosition(shotPosition.x, shotPosition.y);
				} else {
					hasShotActive = false;
				}
				
				float attackCooldown = isFlying ? 0.8f : 1.0f;
				if (attackTimer.getElapsedTime().asSeconds() > attackCooldown) {
					plant->vitality -= attackDamage;
					attackTimer.restart();
					
					DefenseSynth::EffectManager::getInstance().addHit(sf::Vector2f(plant->location.x, plant->location.y));
					
					if (plant->vitality <= 0) {
						plant->operational = false;
						foundPlantToAttack = false;
						canAdvance = true;
						isAttacking = false;
						hasShotActive = false;
						clock2.restart();
						
						DefenseSynth::EffectManager::getInstance().addPlantDeath(sf::Vector2f(plant->location.x, plant->location.y));
					}
				}
				break;
			}
		}
		
		if (!foundPlantToAttack) {
			isAttacking = false;
			hasShotActive = false;
		}
	}
	
	virtual void move(float deltaTime = 0.016f)
	{
		if (behaviorClock.getElapsedTime().asSeconds() > 5)
		{
			canAdvance = true;
			behaviorClock.restart();
		}

		if (enabled)
		{
			bool isBoss = (type.find("boss") != std::string::npos);
			if (isBoss && coords.x > 1200) {
				static sf::Clock offScreenTimer;
				static float lastX = 0;
				if (lastX != coords.x) {
					offScreenTimer.restart();
					lastX = coords.x;
				}
				float timeOffScreen = offScreenTimer.getElapsedTime().asSeconds();
				if (timeOffScreen > 70.0f) {
					hitpoints -= 100;
					if (hitpoints <= 0) {
						hitpoints = 0;
						triggerDeath();
						return;
					}
				}
			}
			
			if (freeze_duration > 0.0f)
			{
				if (freeze_timer.getElapsedTime().asSeconds() >= freeze_duration)
				{
					is_frozen = false;
					freeze_duration = 0.0f;
				}
			}

			if (poison_duration > 0.0f)
			{
				if (poison_timer.getElapsedTime().asSeconds() >= poison_duration)
				{
					is_poisoned = false;
					poison_duration = 0.0f;
				}
				else if (poison_timer.getElapsedTime().asSeconds() >= 1.0f)
				{
					hitpoints -= poison_damage;
					poison_timer.restart();
				}
			}

			if (fire_duration > 0.0f)
			{
				if (fire_timer.getElapsedTime().asSeconds() >= fire_duration)
				{
					is_burning = false;
					fire_duration = 0.0f;
				}
				else if (fire_timer.getElapsedTime().asSeconds() >= 0.5f)
				{
					hitpoints -= fire_damage;
					fire_timer.restart();
				}
			}
			
			if (hitpoints <= 0)
			{
				triggerDeath();
				return;
			}

			sf::Color currentColor = sf::Color(255, 255, 255, 255);
			if (isBoss) {
				currentColor = sf::Color(200, 150, 255);
			} else {
				if (is_frozen) currentColor = sf::Color(150, 150, 255, 255);
				else if (is_poisoned) currentColor = sf::Color(50, 100, 50, 255);
				else if (is_burning) currentColor = sf::Color(255, 100, 100, 255);
			}
			display.setColor(currentColor);
			
			if (!is_frozen)
			{
				if (animationClock.getElapsedTime().asSeconds() >= 0.1f) {
					frame = (frame + 1) % numframes;
					animationClock.restart();
				}
				if (!canAdvance && clock2.getElapsedTime().asSeconds() >= 5.0f) {
					canAdvance = true;
				}
				
				DefenseSynth::StuckDetector::getInstance().detectAndCorrectStuck(
					this, coords, velocity, type, isAttacking, is_frozen, canAdvance, deltaTime
				);
				
				if (isAttacking) {
					bool usesShotSprite = (type != GameConstants::BASIC_ZOMBIE && type != GameConstants::BASIC_ZOMBIE1 && type.find("boss_basic") == std::string::npos);
					
					if (usesShotSprite) {
						display.setTextureRect(sf::IntRect(100 * (frame), 0, 100, 100));
					} else {
						display.setTextureRect(sf::IntRect(100 * (frame), 100, 100, 100));
					}
				} else if (!isAttacking) {
					canAdvance = true;
					coords.x -= velocity * deltaTime;
					if (coords.x < GameConstants::LEFT_BOUNDARY) {
						reachedEnd = true;
						enabled = false;
					}
					display.setTextureRect(sf::IntRect(GameConstants::SPRITE_SIZE * (frame), 0, GameConstants::SPRITE_WIDTH, GameConstants::SPRITE_HEIGHT));
				}
				display.setPosition(coords.x, coords.y);
			}
		}
		
		if (showDebugLabel) {
			debugLabel.setPosition(coords.x, coords.y - 35);
		}
	}
	virtual void draw(sf::RenderWindow &window)
	{
		if (enabled)
		{
			window.draw(display);
			
			if (hasShotActive && type != GameConstants::BASIC_ZOMBIE && type != GameConstants::BASIC_ZOMBIE1)
			{
				window.draw(shotSprite);
			}
			
			if (showDebugLabel) {
				window.draw(debugLabel);
			}
		}
	}

	void Serialize(std::ostream &stream) const
	{
		stream << type << std::endl;
		stream << hitpoints << std::endl;
		stream << velocity << std::endl;
		stream << enabled << std::endl;
		stream << coords.x << std::endl;
		stream << coords.y << std::endl;
		stream << numframes << std::endl;
		stream << frame << std::endl;
		stream << canAdvance << std::endl;
		stream << hasSpawnedAllies << std::endl;
		stream << targetY << std::endl;
		stream << scoreValue << std::endl;
	}

	void Deserialize(std::istream &stream)
	{
		stream >> hitpoints;
		stream >> velocity;
		stream >> enabled;
		stream >> coords.x;
		stream >> coords.y;
		stream >> numframes;
		stream >> frame;
		stream >> canAdvance;
		stream >> hasSpawnedAllies;
		stream >> targetY;
		stream >> scoreValue;
		display.setPosition(coords.x, coords.y);
		display.setTextureRect(sf::IntRect(GameConstants::SPRITE_SIZE * (frame), 0, GameConstants::SPRITE_WIDTH, GameConstants::SPRITE_HEIGHT));
		std::cout << "Deserialized" << std::endl;
		std::cout << "the enemy has the following attributes: " << std::endl;
		std::cout << "hitpoints: " << hitpoints << std::endl;
		std::cout << "velocity: " << velocity << std::endl;
		std::cout << "enabled: " << enabled << std::endl;
		std::cout << "coords.x: " << coords.x << std::endl;
		std::cout << "coords.y: " << coords.y << std::endl;
		std::cout << "numframes: " << numframes << std::endl;
		std::cout << "frame: " << frame << std::endl;
		std::cout << "canAdvance: " << canAdvance << std::endl;
		std::cout << "type: " << type << std::endl;
		std::cout << "hasSpawnedAllies: " << hasSpawnedAllies << std::endl;
		std::cout << "targetY: " << targetY << std::endl;
	}
	
	void setMovementStrategy(std::unique_ptr<IMovementStrategy> strategy) {
		movementStrategy = std::move(strategy);
	}
	
	IMovementStrategy* getMovementStrategy() const {
		return movementStrategy.get();
	}
	
	static void setAllEnemiesReference(std::vector<Enemy*>& enemies) {
		allEnemiesRef = enemies;
	}
	
	void executeBehavior(float deltaTime, const TowerFactory* towers) {
		if (behaviorContext) {
			behaviorContext->executeBehavior(deltaTime, allEnemiesRef, towers);
		}
	}
	
	void triggerDeath() {
		if (!enabled) return;
		
		enabled = false;
		
		DefenseSynth::StuckDetector::getInstance().unregisterEnemy(this);
		
		bool isBoss = (type.find("boss") != std::string::npos);
		if (isBoss) {
			DefenseSynth::EffectManager::getInstance().addBossDeath(sf::Vector2f(coords.x, coords.y));
		} else {
			DefenseSynth::EffectManager::getInstance().addZombieDeath(sf::Vector2f(coords.x, coords.y));
		}
	}
	
	virtual ~Enemy() {
		DefenseSynth::StuckDetector::getInstance().unregisterEnemy(this);
	}
};

class ArmoredEnemy : public Enemy
{
public:
	ArmoredEnemy(float x = 0, float y = 0)
	{
		scoreValue = 75;
		hitpoints = GameConstants::ARMORED_ZOMBIE_HEALTH;
		coords.x = x;
		coords.y = y;
		velocity = 8.0f;
		enabled = true;
		isAttacking = false;
		attackDamage = 2;
		attackTimer.restart();
		reachedEnd = false;
		
		auto& rm = DefenseSynth::ResourceManager::getInstance();
		rm.loadTextureWithRetry("assets/images/footballzombie.png", "footballzombie", 1);
		numframes = 28;
		frame = 0;
		display.setTexture(rm.getTexture("footballzombie"));
		display.setTextureRect(sf::IntRect(0, 0, GameConstants::SPRITE_WIDTH, GameConstants::SPRITE_HEIGHT));
		display.setOrigin(GameConstants::SPRITE_WIDTH / 2.0f, GameConstants::SPRITE_HEIGHT / 2.0f);
		coords.x = x + GameConstants::SPRITE_WIDTH / 2.0f;
		coords.y = y + GameConstants::SPRITE_HEIGHT / 2.0f;
		display.setPosition(coords.x, coords.y);
		canAdvance = true;
		type = GameConstants::ARMORED_ZOMBIE;
		targetY = coords.y;
		
		setMovementStrategy(std::make_unique<LinearMovementStrategy>(2.0f));
		
		behaviorContext = std::make_unique<DefenseSynth::EnemyBehaviorContext>(this);
		auto behavior = DefenseSynth::BehaviorFactory::createBehaviorForEnemyType(type);
		behaviorContext->setBehavior(std::move(behavior));
		
		rm.loadTextureWithRetry("assets/images/shot.png", "shot", 1);
		shotSprite.setTexture(rm.getTexture("shot"));
		shotSprite.setTextureRect(sf::IntRect(0, 0, 30, 30));
		hasShotActive = false;
		shotPosition.x = 0;
		shotPosition.y = 0;
		hasExploded = false;
		
		auto* context = DefenseSynth::GameContext::getInstance();
		level = calculateZombieLevel(context->getCurrentLevel(), context->getDifficulty());
		hasUpgradeIndicator = false;
		
		if (!rm.loadFont("assets/fonts/arial.ttf", "arial")) {
			rm.loadFont("assets/fonts/new.ttf", "arial");
		}
		upgradeFont = rm.getFont("arial");
		upgradeLevelText.setFont(upgradeFont);
		upgradeLevelText.setCharacterSize(20);
		upgradeLevelText.setFillColor(sf::Color::Red);
		upgradeLevelText.setStyle(sf::Text::Bold);
		upgradeLevelText.setOutlineThickness(2);
		upgradeLevelText.setOutlineColor(sf::Color::Black);
		updateUpgradeIndicator();
		
		if (level > 0) {
			auto* context = DefenseSynth::GameContext::getInstance();
			int hitpointsBonus = static_cast<int>(level * GameConstants::LEVEL_HEALTH_BASE_BONUS * 2 * (1.0f + context->getDifficulty() * 0.4f));
			hitpoints += hitpointsBonus;
			float maxVelocityIncrease = velocity * 0.4f;
			float velocityIncrease = std::min(level * (0.4f + context->getDifficulty() * 0.25f), maxVelocityIncrease);
			velocity = velocity + velocityIncrease;
			int scoreBonus = (level - 1) * 20;
			scoreValue += scoreBonus;
			attackDamage += level * GameConstants::LEVEL_DAMAGE_BASE_BONUS;
		}
	}
	
	void updateUpgradeIndicator() {
		if (level > 0) {
			hasUpgradeIndicator = true;
			std::string indicator = "Lvl: ";
			for (int i = 0; i < level; i++) {
				indicator += "*";
			}
			upgradeLevelText.setString(indicator);
			upgradeLevelText.setPosition(coords.x, coords.y + 55);
		} else {
			hasUpgradeIndicator = false;
		}
	}

	virtual void move(float deltaTime = 0.016f)
	{
		if (behaviorClock.getElapsedTime().asSeconds() > 5)
		{
			canAdvance = true;
			behaviorClock.restart();
		}

		if (enabled)
		{
			if (freeze_duration > 0.0f)
			{
				if (freeze_timer.getElapsedTime().asSeconds() >= freeze_duration)
				{
					is_frozen = false;
					freeze_duration = 0.0f;
				}
			}

			if (poison_duration > 0.0f)
			{
				if (poison_timer.getElapsedTime().asSeconds() >= poison_duration)
				{
					is_poisoned = false;
					poison_duration = 0.0f;
				}
				else if (poison_timer.getElapsedTime().asSeconds() >= 1.0f)
				{
					hitpoints -= poison_damage;
					poison_timer.restart();
				}
			}

			if (fire_duration > 0.0f)
			{
				if (fire_timer.getElapsedTime().asSeconds() >= fire_duration)
				{
					is_burning = false;
					fire_duration = 0.0f;
				}
				else if (fire_timer.getElapsedTime().asSeconds() >= 0.5f)
				{
					hitpoints -= fire_damage;
					fire_timer.restart();
				}
			}
			
			if (hitpoints <= 0)
			{
				triggerDeath();
				return;
			}

			bool isBoss = (type.find("boss") != std::string::npos);
			sf::Color currentColor = sf::Color(255, 255, 255, 255);
			if (isBoss) {
				currentColor = sf::Color(200, 150, 255);
			} else {
				if (is_frozen) currentColor = sf::Color(150, 150, 255, 255);
				else if (is_poisoned) currentColor = sf::Color(50, 100, 50, 255);
				else if (is_burning) currentColor = sf::Color(255, 100, 100, 255);
			}
			display.setColor(currentColor);

			if (!is_frozen)
			{
				if (animationClock.getElapsedTime().asSeconds() >= 0.1f) {
					frame = (frame + 1) % numframes;
					animationClock.restart();
				}
				if (!canAdvance && clock2.getElapsedTime().asSeconds() >= 5.0f) {
					canAdvance = true;
				}
				if (isAttacking) {
					bool usesShotSprite = (type != GameConstants::BASIC_ZOMBIE && type != GameConstants::BASIC_ZOMBIE1 && type.find("boss_basic") == std::string::npos);
					
					if (usesShotSprite) {
						display.setTextureRect(sf::IntRect(100 * (frame), 0, 100, 100));
					} else {
						display.setTextureRect(sf::IntRect(100 * (frame), 100, 100, 100));
					}
				} else if (!isAttacking) {
					canAdvance = true;
					coords.x -= velocity * deltaTime;
					display.setTextureRect(sf::IntRect(GameConstants::SPRITE_SIZE * (frame), 0, GameConstants::SPRITE_WIDTH, GameConstants::SPRITE_HEIGHT));
				}
				display.setPosition(coords.x, coords.y);
				
				if (coords.x < GameConstants::LEFT_BOUNDARY)
				{
					reachedEnd = true;
					enabled = false;
				}
			}
		}
	}

	void draw(sf::RenderWindow &window)
	{
		if (enabled)
		{
			window.draw(display);
			
			if (showDebugLabel) {
				window.draw(debugLabel);
			}
		}
	}
};

class FlyingEnemy : public Enemy
{
public:
    FlyingEnemy(float x = 0, float y = 0)
    {
        scoreValue = 100;
        numframes = 11;
        frame = 0;
        hitpoints = GameConstants::FLYING_ZOMBIE_HEALTH;
        coords.x = x;
        coords.y = y;
        velocity = 55.0f;
        enabled = true;
        isAttacking = false;
        attackDamage = 1;
        attackTimer.restart();
        reachedEnd = false;
        
        auto& rm = DefenseSynth::ResourceManager::getInstance();
        rm.loadTextureWithRetry("assets/images/flyingzombie.png", "flyingzombie", 1);
        display.setTexture(rm.getTexture("flyingzombie"));
        display.setTextureRect(sf::IntRect(0, 0, GameConstants::SPRITE_WIDTH, GameConstants::SPRITE_HEIGHT));
        display.setOrigin(GameConstants::SPRITE_WIDTH / 2.0f, GameConstants::SPRITE_HEIGHT / 2.0f);
        coords.x = x + GameConstants::SPRITE_WIDTH / 2.0f;
        coords.y = y + GameConstants::SPRITE_HEIGHT / 2.0f;
        display.setPosition(coords.x, coords.y);
        canAdvance = true;
        type = GameConstants::FLYING_ZOMBIE;
        targetY = coords.y;
        
        setMovementStrategy(std::make_unique<LinearMovementStrategy>(256.0f));
        
        behaviorContext = std::make_unique<DefenseSynth::EnemyBehaviorContext>(this);
        auto behavior = DefenseSynth::BehaviorFactory::createBehaviorForEnemyType(type);
        behaviorContext->setBehavior(std::move(behavior));
        
        rm.loadTextureWithRetry("assets/images/shot.png", "shot", 1);
        shotSprite.setTexture(rm.getTexture("shot"));
        shotSprite.setTextureRect(sf::IntRect(0, 0, 30, 30));
        hasShotActive = false;
        shotPosition.x = 0;
        shotPosition.y = 0;
        hasExploded = false;
        
        auto* context = DefenseSynth::GameContext::getInstance();
        level = calculateZombieLevel(context->getCurrentLevel(), context->getDifficulty());
        hasUpgradeIndicator = false;
        
        if (!rm.loadFont("assets/fonts/arial.ttf", "arial")) {
            rm.loadFont("assets/fonts/new.ttf", "arial");
        }
        upgradeFont = rm.getFont("arial");
        upgradeLevelText.setFont(upgradeFont);
        upgradeLevelText.setCharacterSize(20);
        upgradeLevelText.setFillColor(sf::Color::Red);
        upgradeLevelText.setStyle(sf::Text::Bold);
        upgradeLevelText.setOutlineThickness(2);
        upgradeLevelText.setOutlineColor(sf::Color::Black);
        updateUpgradeIndicator();
        
        if (level > 0) {
            auto* context = DefenseSynth::GameContext::getInstance();
            int hitpointsBonus = static_cast<int>(level * GameConstants::LEVEL_HEALTH_BASE_BONUS / 2 * (1.0f + context->getDifficulty() * 0.4f));
            hitpoints += hitpointsBonus;
            float maxVelocityIncrease = velocity * 0.6f;
            float velocityIncrease = std::min(level * (0.8f + context->getDifficulty() * 0.4f), maxVelocityIncrease);
            velocity = velocity + velocityIncrease;
            int scoreBonus = (level - 1) * 25;
            scoreValue += scoreBonus;
            attackDamage += level * GameConstants::LEVEL_DAMAGE_BASE_BONUS;
        }
    }
    
    void updateUpgradeIndicator() {
        if (level > 0) {
            hasUpgradeIndicator = true;
            std::string indicator = "Lvl: ";
            for (int i = 0; i < level; i++) {
                indicator += "*";
            }
            upgradeLevelText.setString(indicator);
            upgradeLevelText.setPosition(coords.x, coords.y + 55);
        } else {
            hasUpgradeIndicator = false;
        }
    }
    
    virtual void move(float deltaTime = 0.016f)
    {
        if (behaviorClock.getElapsedTime().asSeconds() > 5)
        {
            canAdvance = true;
            behaviorClock.restart();
        }

        if (enabled)
        {
            if (freeze_duration > 0.0f)
            {
                if (freeze_timer.getElapsedTime().asSeconds() >= freeze_duration)
                {
                    is_frozen = false;
                    freeze_duration = 0.0f;
                }
            }

            if (poison_duration > 0.0f)
            {
                if (poison_timer.getElapsedTime().asSeconds() >= poison_duration)
                {
                    is_poisoned = false;
                    poison_duration = 0.0f;
                }
                else if (poison_timer.getElapsedTime().asSeconds() >= 1.0f)
                {
                    hitpoints -= poison_damage;
                    poison_timer.restart();
                }
            }

            if (fire_duration > 0.0f)
            {
                if (fire_timer.getElapsedTime().asSeconds() >= fire_duration)
                {
                    is_burning = false;
                    fire_duration = 0.0f;
                }
                else if (fire_timer.getElapsedTime().asSeconds() >= 0.5f)
                {
                    hitpoints -= fire_damage;
                    fire_timer.restart();
                }
            }
            
            if (hitpoints <= 0)
            {
                triggerDeath();
                return;
            }

            bool isBoss = (type.find("boss") != std::string::npos);
            bool isFlying = (type == "flying");
            sf::Color currentColor = sf::Color(255, 255, 255, 255);
            if (isBoss) {
                currentColor = sf::Color(200, 150, 255);
            }
            if (is_frozen) currentColor = sf::Color(150, 150, 255, 255);
            else if (is_poisoned) currentColor = sf::Color(50, 100, 50, 255);
            else if (is_burning) currentColor = sf::Color(255, 100, 100, 255);
            display.setColor(currentColor);
            
            

            if (!is_frozen)
            {
                if (animationClock.getElapsedTime().asSeconds() >= 0.1f) {
                    frame = (frame + 1) % numframes;
                    animationClock.restart();
                }
                if (!canAdvance && clock2.getElapsedTime().asSeconds() >= 5.0f) {
                    canAdvance = true;
                }
                if (isAttacking) {
                    display.setTextureRect(sf::IntRect(100 * (frame), 0, 100, 100));
                } else if (!isAttacking) {
                    canAdvance = true;
                    coords.x -= velocity * deltaTime;
                    
                    display.setTextureRect(sf::IntRect(GameConstants::SPRITE_SIZE * (frame), 0, GameConstants::SPRITE_WIDTH, GameConstants::SPRITE_HEIGHT));
                }
                display.setPosition(coords.x, coords.y);
                
                if (coords.x < GameConstants::LEFT_BOUNDARY)
                {
                    reachedEnd = true;
                    enabled = false;
                }
            }
        }
    }
    void draw(sf::RenderWindow &window)
    {
        if (enabled)
        {
            window.draw(display);
            
            if (hasShotActive)
            {
                window.draw(shotSprite);
            }
        }
    }
};

class DancingEnemy : public Enemy
{
private:
    std::vector<sf::CircleShape> fireParticles;
    sf::Clock fireAnimationClock;
    
public:
    DancingEnemy(float x = 0, float y = 0)
    {
        scoreValue = 100;
        hitpoints = GameConstants::DANCING_ZOMBIE_HEALTH;
        coords.x = x;
        coords.y = y;
        velocity = 22.0f;
        numframes = 8;
        frame = 0;
        enabled = true;
        isAttacking = false;
        attackDamage = 1;
        attackTimer.restart();
        reachedEnd = false;
        
        auto& rm = DefenseSynth::ResourceManager::getInstance();
        rm.loadTextureWithRetry("assets/images/dancingzombie.png", "dancingzombie", 1);
        display.setTexture(rm.getTexture("dancingzombie"));
        display.setTextureRect(sf::IntRect(0, 0, GameConstants::SPRITE_WIDTH, GameConstants::SPRITE_HEIGHT));
        display.setOrigin(GameConstants::SPRITE_WIDTH / 2.0f, GameConstants::SPRITE_HEIGHT / 2.0f);
        coords.x = x + GameConstants::SPRITE_WIDTH / 2.0f;
        coords.y = y + GameConstants::SPRITE_HEIGHT / 2.0f;
        display.setPosition(coords.x, coords.y);
        canAdvance = true;
        type = GameConstants::DANCING_ZOMBIE;
        
        setMovementStrategy(std::make_unique<WanderingMovementStrategy>(3.0f, 0.02f, 1.0f));
        
        behaviorContext = std::make_unique<DefenseSynth::EnemyBehaviorContext>(this);
        auto behavior = DefenseSynth::BehaviorFactory::createBehaviorForEnemyType(type);
        behaviorContext->setBehavior(std::move(behavior));
        
        rm.loadTextureWithRetry("assets/images/shot.png", "shot", 1);
        shotSprite.setTexture(rm.getTexture("shot"));
        shotSprite.setTextureRect(sf::IntRect(0, 0, 30, 30));
        hasShotActive = false;
        shotPosition.x = 0;
        shotPosition.y = 0;
        hasExploded = false;
        
        auto* context = DefenseSynth::GameContext::getInstance();
        level = calculateZombieLevel(context->getCurrentLevel(), context->getDifficulty());
        hasUpgradeIndicator = false;
        
        if (!rm.loadFont("assets/fonts/arial.ttf", "arial")) {
            rm.loadFont("assets/fonts/new.ttf", "arial");
        }
        upgradeFont = rm.getFont("arial");
        upgradeLevelText.setFont(upgradeFont);
        upgradeLevelText.setCharacterSize(20);
        upgradeLevelText.setFillColor(sf::Color::Red);
        upgradeLevelText.setStyle(sf::Text::Bold);
        upgradeLevelText.setOutlineThickness(2);
        upgradeLevelText.setOutlineColor(sf::Color::Black);
        updateUpgradeIndicator();
        
        if (level > 0) {
            int hitpointsBonus = level * GameConstants::LEVEL_HEALTH_BASE_BONUS;
            hitpoints += hitpointsBonus;
            float maxVelocityIncrease = velocity * 0.45f;
            float velocityIncrease = std::min(level * (0.6f + context->getDifficulty() * 0.35f), maxVelocityIncrease);
            velocity = velocity + velocityIncrease;
            int scoreBonus = (level - 1) * 18;
            scoreValue += scoreBonus;
            attackDamage += level * GameConstants::LEVEL_DAMAGE_BASE_BONUS;
        }
        
        fireParticles.resize(6);
        for (size_t i = 0; i < fireParticles.size(); i++) {
            fireParticles[i].setRadius(1.5f + static_cast<float>(rand() % 3));
            fireParticles[i].setFillColor(sf::Color(255, 150 + static_cast<sf::Uint8>(rand() % 105), static_cast<sf::Uint8>(rand() % 100), 200));
        }
    }
    
    void updateUpgradeIndicator() {
        if (level > 0) {
            hasUpgradeIndicator = true;
            std::string indicator = "Lvl: ";
            for (int i = 0; i < level; i++) {
                indicator += "*";
            }
            upgradeLevelText.setString(indicator);
            upgradeLevelText.setPosition(coords.x, coords.y + 55);
        } else {
            hasUpgradeIndicator = false;
        }
    }

    virtual void move(float deltaTime = 0.016f)
    {
        if (behaviorClock.getElapsedTime().asSeconds() > 5)
        {
            canAdvance = true;
            behaviorClock.restart();
        }

        if (enabled)
        {
            if (freeze_duration > 0.0f)
            {
                if (freeze_timer.getElapsedTime().asSeconds() >= freeze_duration)
                {
                    is_frozen = false;
                    freeze_duration = 0.0f;
                }
            }

            if (poison_duration > 0.0f)
            {
                if (poison_timer.getElapsedTime().asSeconds() >= poison_duration)
                {
                    is_poisoned = false;
                    poison_duration = 0.0f;
                }
                else if (poison_timer.getElapsedTime().asSeconds() >= 1.0f)
                {
                    hitpoints -= poison_damage;
                    poison_timer.restart();
                }
            }

            if (fire_duration > 0.0f)
            {
                if (fire_timer.getElapsedTime().asSeconds() >= fire_duration)
                {
                    is_burning = false;
                    fire_duration = 0.0f;
                }
                else if (fire_timer.getElapsedTime().asSeconds() >= 0.5f)
                {
                    hitpoints -= fire_damage;
                    fire_timer.restart();
                }
            }
            
            if (hitpoints <= 0)
            {
                triggerDeath();
                return;
            }

            bool isBoss = (type.find("boss") != std::string::npos);
            sf::Color currentColor = sf::Color(255, 255, 255, 255);
            if (isBoss) {
                currentColor = sf::Color(200, 150, 255);
            } else {
                if (is_frozen) currentColor = sf::Color(150, 150, 255, 255);
                else if (is_poisoned) {
                    float rem = (std::max)(0.0f, poison_duration - poison_timer.getElapsedTime().asSeconds());
                    float ratio = poison_duration > 0.0f ? rem / poison_duration : 1.0f;
                    sf::Uint8 a = static_cast<sf::Uint8>((std::max)(0.0f, (std::min)(255.0f, 255.0f * ratio)));
                    currentColor = sf::Color(50, 100, 50, a);
                }
                else if (is_burning) currentColor = sf::Color(255, 100, 100, 255);
            }
            
            if (isAttacking && type == GameConstants::DANCING_ZOMBIE) {
                float time = animationClock.getElapsedTime().asSeconds();
                float fireIntensity = 0.5f + 0.5f * sin(time * 15.0f);
                float flicker = 0.8f + 0.2f * sin(time * 25.0f);
                currentColor = sf::Color(
                    255, 
                    static_cast<sf::Uint8>(100 + 100 * fireIntensity * flicker), 
                    static_cast<sf::Uint8>(30 * fireIntensity), 
                    static_cast<sf::Uint8>(200 + 55 * fireIntensity)
                );
                
                static sf::Clock fireEffectClock;
                if (fireEffectClock.getElapsedTime().asSeconds() > 0.1f) {
                    DefenseSynth::EffectManager::getInstance().addBurningZombieEffect(sf::Vector2f(coords.x + 25, coords.y + 30));
                    fireEffectClock.restart();
                }
            }
            
            display.setColor(currentColor);


            if (!is_frozen)
            {
                if (animationClock.getElapsedTime().asSeconds() >= 0.1f) {
                    frame = (frame + 1) % numframes;
                    animationClock.restart();
                }
                if (!canAdvance && clock2.getElapsedTime().asSeconds() >= 5.0f) {
                    canAdvance = true;
                }
                if (isAttacking) {
                    bool usesShotSprite = (type != GameConstants::BASIC_ZOMBIE && type != GameConstants::BASIC_ZOMBIE1 && type.find("boss_basic") == std::string::npos);
                    
                    if (usesShotSprite) {
                        display.setTextureRect(sf::IntRect(100 * (frame), 0, 100, 100));
                    } else {
                        display.setTextureRect(sf::IntRect(100 * (frame), 100, 100, 100));
                    }
                } else if (!isAttacking) {
                    canAdvance = true;
                    coords.x -= velocity * deltaTime;
                    
                    if (rand() % 100 < 2)
                    {
                        coords.y += (rand() % 2 ? 1 : -1);
                        if (coords.y < GameConstants::TOP_BOUNDARY) coords.y = GameConstants::TOP_BOUNDARY;
                        if (coords.y > 550) coords.y = 550;
                    }
                    
                    display.setTextureRect(sf::IntRect(GameConstants::SPRITE_SIZE * (frame), 0, GameConstants::SPRITE_WIDTH, GameConstants::SPRITE_HEIGHT));
                }
                display.setPosition(coords.x, coords.y);
                
                if (coords.x < GameConstants::LEFT_BOUNDARY)
                {
                    reachedEnd = true;
                    enabled = false;
                }
            }
        }
        
        if (isAttacking && fireAnimationClock.getElapsedTime().asSeconds() > 0.05f) {
            for (size_t i = 0; i < fireParticles.size(); i++) {
                float offsetX = -10.0f + static_cast<float>(rand() % 21);
                float offsetY = -15.0f + static_cast<float>(rand() % 11);
                fireParticles[i].setPosition(coords.x + 25 + offsetX, coords.y + 20 + offsetY);
                
                sf::Uint8 alpha = 150 + rand() % 105;
                sf::Uint8 red = 200 + rand() % 55;
                sf::Uint8 green = 50 + rand() % 155;
                sf::Uint8 blue = rand() % 80;
                fireParticles[i].setFillColor(sf::Color(red, green, blue, alpha));
            }
            fireAnimationClock.restart();
        }
    }

    void draw(sf::RenderWindow &window)
    {
        if (enabled)
        {
            window.draw(display);
            
            if (isAttacking) {
                for (const auto& particle : fireParticles) {
                    window.draw(particle);
                }
            }
            
            if (hasShotActive)
            {
                window.draw(shotSprite);
            }
        }
    }
};
