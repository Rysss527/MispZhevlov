#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include "Enemy.h"
#include "BossZombies.h"
#include "GamePlayer.h"
#include <fstream>
#include "SoundManager.h"

class TowerFactory;

class EnemyFactory
{
public:
	std::vector<std::unique_ptr<Enemy>> enemies;
	int enemies_created;
	bool isBossWave = false;
	sf::Clock spawnClock;
	float difficultyMultiplier;
	int currentGameLevel;
	TowerFactory* towerFactoryRef;
	EnemyFactory(int numEnemies = 0, int level = 1, float difficulty = 1.0f)
	{
		currentGameLevel = level;
		difficultyMultiplier = difficulty;
		auto* context = DefenseSynth::GameContext::getInstance();
		context->setLevel(level);
		context->setDifficulty(difficulty);
		towerFactoryRef = nullptr;
		
		int adjustedEnemies = static_cast<int>(numEnemies * difficulty);
		if (adjustedEnemies < 4) adjustedEnemies = 4;
		enemies_created = adjustedEnemies;
		enemies.reserve(enemies_created);
		spawnClock.restart();
		isBossWave = false;

		int numBasicEnemies = 0;
		int numArmoredEnemies = 0;
		int numFlyingEnemies = 0;
		int numDancingEnemies = 0;
		int wave1Enemies = std::max(2, adjustedEnemies / 2);
		
		float basicRatio = std::max(0.2f, 1.0f - (level - 1) * 0.12f);
		float armoredRatio = std::min(0.4f, (level - 1) * 0.08f);
		float flyingRatio = std::min(0.25f, std::max(0.0f, (level - 2) * 0.06f));
		float dancingRatio = std::min(0.15f, std::max(0.0f, (level - 3) * 0.04f));
		
		float totalRatio = basicRatio + armoredRatio + flyingRatio + dancingRatio;
		basicRatio /= totalRatio;
		armoredRatio /= totalRatio;
		flyingRatio /= totalRatio;
		dancingRatio /= totalRatio;
		
		numBasicEnemies = static_cast<int>(wave1Enemies * basicRatio);
		numArmoredEnemies = static_cast<int>(wave1Enemies * armoredRatio);
		numFlyingEnemies = static_cast<int>(wave1Enemies * flyingRatio);
		numDancingEnemies = wave1Enemies - numBasicEnemies - numArmoredEnemies - numFlyingEnemies;


		int wave2Enemies = adjustedEnemies - wave1Enemies;
		int wave2BasicEnemies = static_cast<int>(wave2Enemies * basicRatio);
		int wave2ArmoredEnemies = static_cast<int>(wave2Enemies * armoredRatio);
		int wave2FlyingEnemies = static_cast<int>(wave2Enemies * flyingRatio);
		int wave2DancingEnemies = wave2Enemies - wave2BasicEnemies - wave2ArmoredEnemies - wave2FlyingEnemies;

		enemies_created = numBasicEnemies + numArmoredEnemies + numFlyingEnemies + numDancingEnemies + wave2BasicEnemies + wave2ArmoredEnemies + wave2FlyingEnemies + wave2DancingEnemies;
		enemies.clear();
		enemies.reserve(enemies_created);

		int currentIndex = 0;
		
		for (int i = 0; i < numBasicEnemies; i++)
		{
			float newX = static_cast<float>((rand() % 1001) + 1300);
			int lane = rand() % GameConstants::BATTLEFIELD_ROWS;
			float newY = LaneSystem::getLaneCenterY(lane) + GameConstants::BASIC_ZOMBIE_Y_OFFSET;
			newX = static_cast<float>(static_cast<int>(newX) / 100 * 100);
			auto enemy = std::make_unique<Enemy>(newX, newY);
			enemy->enabled = false;
			enemies.push_back(std::move(enemy));
			currentIndex++;
		}
		for (int i = 0; i < numArmoredEnemies; i++)
		{
			float newX = static_cast<float>((rand() % 1001) + 1300);
			int lane = rand() % GameConstants::BATTLEFIELD_ROWS;
			float newY = LaneSystem::getLaneCenterY(lane) + GameConstants::ARMORED_ZOMBIE_Y_OFFSET;
			newX = static_cast<float>(static_cast<int>(newX) / 100 * 100);
			auto enemy = std::make_unique<ArmoredEnemy>(newX, newY);
			enemy->enabled = false;
			enemies.push_back(std::move(enemy));
			currentIndex++;
		}
		for (int i = 0; i < numFlyingEnemies; i++)
		{
			float newX = static_cast<float>((rand() % 1001) + 1300);
			int lane = rand() % GameConstants::BATTLEFIELD_ROWS;
			float newY = LaneSystem::getLaneCenterY(lane) + GameConstants::FLYING_ZOMBIE_Y_OFFSET;
			newX = static_cast<float>(static_cast<int>(newX) / 100 * 100);
			auto enemy = std::make_unique<FlyingEnemy>(newX, newY);
			enemy->enabled = false;
			enemies.push_back(std::move(enemy));
			currentIndex++;
		}
		for (int i = 0; i < numDancingEnemies; i++)
		{
			float newX = static_cast<float>((rand() % 1001) + 1300);
			int lane = rand() % GameConstants::BATTLEFIELD_ROWS;
			float newY = LaneSystem::getLaneCenterY(lane) + GameConstants::DANCING_ZOMBIE_Y_OFFSET;
			newX = static_cast<float>(static_cast<int>(newX) / 100 * 100);
			auto enemy = std::make_unique<DancingEnemy>(newX, newY);
			enemy->enabled = false;
			enemies.push_back(std::move(enemy));
			currentIndex++;
		}
		
		for (int i = 0; i < wave2BasicEnemies; i++)
		{
			float newX = static_cast<float>((rand() % 1001) + 1300);
			int lane = rand() % GameConstants::BATTLEFIELD_ROWS;
			float newY = LaneSystem::getLaneCenterY(lane);
			newX = static_cast<float>(static_cast<int>(newX) / 100 * 100);
			auto enemy = std::make_unique<Enemy>(newX, newY);
			enemy->enabled = false;
			enemies.push_back(std::move(enemy));
			currentIndex++;
		}
		for (int i = 0; i < wave2ArmoredEnemies; i++)
		{
			float newX = static_cast<float>((rand() % 1001) + 1300);
			int lane = rand() % GameConstants::BATTLEFIELD_ROWS;
			float newY = LaneSystem::getLaneCenterY(lane);
			newX = static_cast<float>(static_cast<int>(newX) / 100 * 100);
			auto enemy = std::make_unique<ArmoredEnemy>(newX, newY);
			enemy->enabled = false;
			enemies.push_back(std::move(enemy));
			currentIndex++;
		}
		for (int i = 0; i < wave2FlyingEnemies; i++)
		{
			float newX = static_cast<float>((rand() % 1001) + 1300);
			int lane = rand() % GameConstants::BATTLEFIELD_ROWS;
			float newY = LaneSystem::getLaneCenterY(lane);
			newX = static_cast<float>(static_cast<int>(newX) / 100 * 100);
			auto enemy = std::make_unique<FlyingEnemy>(newX, newY);
			enemy->enabled = false;
			enemies.push_back(std::move(enemy));
			currentIndex++;
		}
		for (int i = 0; i < wave2DancingEnemies; i++)
		{
			float newX = static_cast<float>((rand() % 1001) + 1300);
			int lane = rand() % GameConstants::BATTLEFIELD_ROWS;
			float newY = LaneSystem::getLaneCenterY(lane);
			newX = static_cast<float>(static_cast<int>(newX) / 100 * 100);
			auto enemy = std::make_unique<DancingEnemy>(newX, newY);
			enemy->enabled = false;
			enemies.push_back(std::move(enemy));
			currentIndex++;
		}

	}

	EnemyFactory(EnemyFactory&& other) noexcept
		: enemies(std::move(other.enemies)), enemies_created(other.enemies_created),
		  isBossWave(other.isBossWave), spawnClock(other.spawnClock),
		  difficultyMultiplier(other.difficultyMultiplier), currentGameLevel(other.currentGameLevel)
	{
		other.enemies_created = 0;
		other.isBossWave = false;
		other.difficultyMultiplier = 1.0f;
		other.currentGameLevel = 1;
	}
	
	EnemyFactory& operator=(EnemyFactory&& other) noexcept
	{
		if (this != &other)
		{
			enemies = std::move(other.enemies);
			enemies_created = other.enemies_created;
			isBossWave = other.isBossWave;
			spawnClock = other.spawnClock;
			difficultyMultiplier = other.difficultyMultiplier;
			currentGameLevel = other.currentGameLevel;
			
			other.enemies_created = 0;
			other.isBossWave = false;
			other.difficultyMultiplier = 1.0f;
			other.currentGameLevel = 1;
		}
		return *this;
	}
	
	EnemyFactory(const EnemyFactory&) = delete;
	EnemyFactory& operator=(const EnemyFactory&) = delete;
	void createEnemy(float x = 0, float y = 0)
	{
		auto newEnemy = std::make_unique<Enemy>(x, y);
		enemies.push_back(std::move(newEnemy));
		enemies_created++;
	}
	void draw(sf::RenderWindow &window)
	{
		drawSprites(window);
		drawLabels(window);
	}
	
	void drawSprites(sf::RenderWindow &window)
	{
		drawGroundEnemies(window);
	}
	
	void drawGroundEnemies(sf::RenderWindow &window)
	{
		std::vector<Enemy*> toDraw;
		toDraw.reserve(enemies.size());
		for (const auto& enemy : enemies)
		{
			if (enemy && enemy->enabled && enemy->type != "flying" && enemy->type.find("boss_flying") == std::string::npos)
			{
				toDraw.push_back(enemy.get());
			}
		}
		std::sort(toDraw.begin(), toDraw.end(), [](const Enemy* a, const Enemy* b){ return a->coords.y < b->coords.y; });
		for (auto* e : toDraw) { e->draw(window); }
	}

	void drawFlyingEnemies(sf::RenderWindow &window)
	{
		for (const auto& enemy : enemies)
		{
			if (enemy && enemy->enabled && (enemy->type == "flying" || enemy->type.find("boss_flying") != std::string::npos))
			{
				enemy->draw(window);
			}
		}
	}

	void drawLabels(sf::RenderWindow &window)
	{
		std::vector<Enemy*> labelEnemies;
		labelEnemies.reserve(enemies.size());
		for (const auto& enemy : enemies)
		{
			if (enemy && enemy->enabled)
			{
				if (enemy->hasUpgradeIndicator && enemy->level > 0 && enemy->type.find("boss_") != 0) {
					labelEnemies.push_back(enemy.get());
				}
			}
		}
		std::sort(labelEnemies.begin(), labelEnemies.end(), [](const Enemy* a, const Enemy* b){ return a->coords.y < b->coords.y; });
		std::vector<sf::FloatRect> placed;
		placed.reserve(labelEnemies.size());
		for (auto* enemy : labelEnemies)
		{
			enemy->upgradeLevelText.setPosition(enemy->coords.x, enemy->coords.y + 55);
			auto pos = enemy->upgradeLevelText.getPosition();
			sf::FloatRect rect = enemy->upgradeLevelText.getGlobalBounds();
			bool overlap = true;
			while (overlap)
			{
				overlap = false;
				for (const auto& r : placed)
				{
					if (rect.intersects(r))
					{
						pos.y -= 18.f;
						enemy->upgradeLevelText.setPosition(pos);
						rect = enemy->upgradeLevelText.getGlobalBounds();
						overlap = true;
						break;
					}
				}
			}
			window.draw(enemy->upgradeLevelText);
			placed.push_back(rect);
		}

		std::vector<Enemy*> bosses;
		bosses.reserve(enemies.size());
		for (const auto& enemy : enemies)
		{
			if (enemy && enemy->enabled && enemy->type.find("boss_") == 0) {
				bosses.push_back(enemy.get());
			}
		}
		std::sort(bosses.begin(), bosses.end(), [](const Enemy* a, const Enemy* b){ return a->coords.y < b->coords.y; });
		for (auto* enemy : bosses) {
			if (enemy->type == "boss_basic") {
				BossEnemy* boss = static_cast<BossEnemy*>(enemy);
				boss->draw(window);
				auto& rm = DefenseSynth::ResourceManager::getInstance();
				sf::Text t;
				t.setFont(rm.getFont("arial"));
				t.setCharacterSize(20);
				t.setFillColor(sf::Color::Red);
				t.setStyle(sf::Text::Bold);
				t.setOutlineThickness(2);
				t.setOutlineColor(sf::Color::Black);
				std::string stars;
				for (int i = 0; i < enemy->level; ++i) stars += "*";
				if (enemy->level > 0) {
					t.setString(std::string("Lvl: ") + stars);
					t.setPosition(enemy->coords.x + 45, enemy->coords.y - 5);
					window.draw(t);
				}
			} else if (enemy->type == "boss_armored") {
				BossArmoredEnemy* boss = static_cast<BossArmoredEnemy*>(enemy);
				boss->draw(window);
				auto& rm = DefenseSynth::ResourceManager::getInstance();
				sf::Text t;
				t.setFont(rm.getFont("arial"));
				t.setCharacterSize(20);
				t.setFillColor(sf::Color::Red);
				t.setStyle(sf::Text::Bold);
				t.setOutlineThickness(2);
				t.setOutlineColor(sf::Color::Black);
				if (enemy->level > 0) {
					std::string stars;
					for (int i = 0; i < enemy->level; ++i) stars += "*";
					t.setString(std::string("Lvl: ") + stars);
					t.setPosition(enemy->coords.x + 45, enemy->coords.y - 5);
					window.draw(t);
				}
			} else if (enemy->type == "boss_flying") {
				BossFlyingEnemy* boss = static_cast<BossFlyingEnemy*>(enemy);
				boss->draw(window);
				auto& rm = DefenseSynth::ResourceManager::getInstance();
				sf::Text t;
				t.setFont(rm.getFont("arial"));
				t.setCharacterSize(20);
				t.setFillColor(sf::Color::Red);
				t.setStyle(sf::Text::Bold);
				t.setOutlineThickness(2);
				t.setOutlineColor(sf::Color::Black);
				std::string stars;
				for (int i = 0; i < enemy->level; ++i) stars += "*";
				if (enemy->level > 0) {
					t.setString(std::string("Lvl: ") + stars);
					t.setPosition(enemy->coords.x + 45, enemy->coords.y - 5);
					window.draw(t);
				}
			} else if (enemy->type == "boss_dancing") {
				BossDancingEnemy* boss = static_cast<BossDancingEnemy*>(enemy);
				boss->draw(window);
				auto& rm = DefenseSynth::ResourceManager::getInstance();
				sf::Text t;
				t.setFont(rm.getFont("arial"));
				t.setCharacterSize(20);
				t.setFillColor(sf::Color::Red);
				t.setStyle(sf::Text::Bold);
				t.setOutlineThickness(2);
				t.setOutlineColor(sf::Color::Black);
				std::string stars;
				for (int i = 0; i < enemy->level; ++i) stars += "*";
				if (enemy->level > 0) {
					t.setString(std::string("Lvl: ") + stars);
					t.setPosition(enemy->coords.x + 45, enemy->coords.y - 5);
					window.draw(t);
				}
			}
		}
	}
	void move(float deltaTime = 0.016f)
	{
		std::vector<Enemy*> enemyRefs;
		for (const auto& enemy : enemies) {
			if (enemy && enemy->enabled) {
				enemyRefs.push_back(enemy.get());
			}
		}
		Enemy::setAllEnemiesReference(enemyRefs);
		
		for (auto& currentEnemy : enemies)
		{
			if (!currentEnemy || !currentEnemy->enabled) continue;
			
			currentEnemy->checkPlantCollision(towerFactoryRef);
			currentEnemy->executeBehavior(deltaTime, towerFactoryRef);
			
			if (currentEnemy->type == "flying" || currentEnemy->type.find("boss_flying") != std::string::npos) {
				currentEnemy->move(deltaTime);
			} else {
				bool blocked = false;
				for (const auto& otherEnemy : enemies) {
					if (currentEnemy.get() != otherEnemy.get() && otherEnemy && otherEnemy->enabled && 
						otherEnemy->type != "flying" && otherEnemy->type.find("boss_flying") == std::string::npos) {
						float dx = currentEnemy->coords.x - otherEnemy->coords.x;
						float dy = abs(currentEnemy->coords.y - otherEnemy->coords.y);
						bool isBoss = (currentEnemy->type.find("boss_") == 0);
						bool otherIsBoss = (otherEnemy->type.find("boss_") == 0);
						float collisionDistance = (isBoss || otherIsBoss) ? 80.0f : 50.0f;
						float verticalTolerance = (isBoss || otherIsBoss) ? 40.0f : 30.0f;
						if (dx > 0 && dx < collisionDistance && dy < verticalTolerance && !currentEnemy->isAttacking) {
							blocked = true;
							float pushDistance = (isBoss || otherIsBoss) ? 5.0f : 2.0f;
							currentEnemy->coords.x += pushDistance;
							currentEnemy->display.setPosition(currentEnemy->coords.x, currentEnemy->coords.y);
							break;
						}
					}
				}
				currentEnemy->move(deltaTime);
			}
			currentEnemy->display.setPosition(currentEnemy->coords.x, currentEnemy->coords.y);
		}
		bool hasBossDancer = false;
		for (const auto& e : enemies) {
			if (e && e->enabled && e->type == "boss_dancing") { hasBossDancer = true; break; }
		}
		SoundManager::getInstance().loopSound("dancer.wav", hasBossDancer);
	}
	void summonEnemies()
	{
		if (spawnClock.getElapsedTime().asSeconds() > 1.0f) {
			for (auto& enemy : enemies)
			{
				if (enemy && !enemy->enabled) {
					bool isBoss = enemy->type.find("boss_") == 0;
					if (!isBoss || isBossWave) {
						float newX = static_cast<float>((rand() % 1001) + 1300);
						int lane = rand() % GameConstants::BATTLEFIELD_ROWS;
						float newY = LaneSystem::getLaneCenterY(lane);
						newX = static_cast<float>(static_cast<int>(newX) / 100 * 100);
						enemy->coords.x = newX;
						enemy->coords.y = newY;
						enemy->display.setPosition(newX, newY);
						enemy->enabled = true;
						spawnClock.restart();
						break;
					}
				}
			}
		}
		
	}
	
	void startWave1() {
		isBossWave = false;
		for (auto& enemy : enemies) {
			if (enemy && enemy->type.find("boss_") == 0) {
				enemy->enabled = false;
			}
		}
		
		std::vector<std::string> wave1Types = {"basic", "armored", "flying", "dancing"};
		int target = std::min(enemies_created, std::max(1, enemies_created / 2));
		int activated = 0;
		
		for (const auto& targetType : wave1Types) {
			for (auto& enemy : enemies) {
				if (activated >= target) break;
				if (enemy && !enemy->enabled && enemy->type == targetType) {
					enemy->enabled = true;
					activated++;
				}
			}
			if (activated >= target) break;
		}
		
		if (activated == 0 && enemies_created > 0) {
			for (auto& enemy : enemies) {
				if (enemy && !enemy->enabled && enemy->type.find("boss_") != 0) {
					enemy->enabled = true;
					activated++;
					if (activated >= 1) break;
				}
			}
		}
	}
	bool areEnemiesDead()
	{
		for (const auto& enemy : enemies)
		{
			if (enemy && enemy->enabled)
			{
				return false;
			}
		}
		return true;
	}

	void freezeEnemies(int x, int y)
	{
		int rightEdge = x + 200;
		int bottomEdge = y + 200;
		for (const auto& enemy : enemies)
		{
			if (enemy && enemy->coords.x >= x && enemy->coords.x <= rightEdge &&
				enemy->coords.y >= y && enemy->coords.y <= bottomEdge)
			{
				enemy->freeze_duration = 5.0f;
				enemy->is_frozen = true;
				enemy->freeze_timer.restart();
			}
		}
	}
	
	void freezeEnemiesInLine(int x, int y, int range)
	{
		for (const auto& enemy : enemies)
		{
			if (enemy && enemy->coords.x >= x && enemy->coords.x <= x + range &&
				abs(enemy->coords.y - y) <= 50)
			{
				bool isBoss = (enemy->type.find("boss") != std::string::npos);
				int freezeChance = isBoss ? 30 : 50;
				if (rand() % 100 < freezeChance) {
					enemy->freeze_duration = 5.0f;
					enemy->is_frozen = true;
					enemy->freeze_timer.restart();
				}
			}
		}
	}

	void deleteEnemiesInRect(float x, float y, Player &player)
	{
		float x2 = x + 300;
		float y2 = y + 300;

		auto it = std::remove_if(enemies.begin(), enemies.end(),
			[x, y, x2, y2, &player](const std::unique_ptr<Enemy>& enemy) {
				if (!enemy) return true;
				sf::FloatRect enemyBounds = enemy->display.getGlobalBounds();
				if (enemyBounds.left >= x && enemyBounds.left <= x2 &&
					enemyBounds.top >= y && enemyBounds.top <= y2)
				{
					player.score += enemy->points;
					return true;
				}
				return false;
			});
		
		enemies.erase(it, enemies.end());
		enemies_created = static_cast<int>(enemies.size());
	}
	~EnemyFactory() = default;
	void Serialize(std::ostream &stream) const
	{
		stream << enemies_created << std::endl;
		for (const auto& enemy : enemies)
		{
			if (enemy)
			{
				enemy->Serialize(stream);
			}
		}
	}
	void Deserialize(std::istream &stream)
	{
		int numEnemies;
		stream >> numEnemies;

		enemies.clear();
		enemies_created = numEnemies;
		enemies.reserve(numEnemies);

		for (int i = 0; i < numEnemies; i++)
		{
			std::string enemyType;
			stream >> enemyType;

			std::unique_ptr<Enemy> enemy;
			if (enemyType == "ArmoredEnemy")
			{
				enemy = std::make_unique<ArmoredEnemy>();
			}
			else if (enemyType == "FlyingEnemy")
			{
				enemy = std::make_unique<FlyingEnemy>();
			}
			else if (enemyType == "DancingEnemy")
			{
				enemy = std::make_unique<DancingEnemy>();
			}
			else
			{
				enemy = std::make_unique<Enemy>();
			}
			enemy->Deserialize(stream);
			enemies.push_back(std::move(enemy));
		}
	}
	
	void resetForNextLevel(int numEnemies, int level, float difficulty = 1.0f)
	{
		currentGameLevel = level;
		difficultyMultiplier = difficulty;
		auto* context = DefenseSynth::GameContext::getInstance();
		context->setLevel(level);
		context->setDifficulty(difficulty);
		
		enemies.clear();
		spawnClock.restart();
		isBossWave = false;

		int adjustedEnemies = static_cast<int>(numEnemies * difficulty);
		if (adjustedEnemies < 4) adjustedEnemies = 4;
		
		float basicRatio = std::max(0.2f, 1.0f - (level - 1) * 0.12f);
		float armoredRatio = std::min(0.4f, (level - 1) * 0.08f);
		float flyingRatio = std::min(0.25f, std::max(0.0f, (level - 2) * 0.06f));
		float dancingRatio = std::min(0.15f, std::max(0.0f, (level - 3) * 0.04f));
		
		float totalRatio = basicRatio + armoredRatio + flyingRatio + dancingRatio;
		basicRatio /= totalRatio;
		armoredRatio /= totalRatio;
		flyingRatio /= totalRatio;
		dancingRatio /= totalRatio;
		
		int wave1Enemies = std::max(2, adjustedEnemies / 2);
		
		int numBasicEnemies = static_cast<int>(wave1Enemies * basicRatio);
		int numArmoredEnemies = static_cast<int>(wave1Enemies * armoredRatio);
		int numFlyingEnemies = static_cast<int>(wave1Enemies * flyingRatio);
		int numDancingEnemies = wave1Enemies - numBasicEnemies - numArmoredEnemies - numFlyingEnemies;

		int wave2Enemies = adjustedEnemies - wave1Enemies;
		int wave2BasicEnemies = static_cast<int>(wave2Enemies * basicRatio);
		int wave2ArmoredEnemies = static_cast<int>(wave2Enemies * armoredRatio);
		int wave2FlyingEnemies = static_cast<int>(wave2Enemies * flyingRatio);
		int wave2DancingEnemies = wave2Enemies - wave2BasicEnemies - wave2ArmoredEnemies - wave2FlyingEnemies;


		enemies_created = numBasicEnemies + numArmoredEnemies + numFlyingEnemies + numDancingEnemies + wave2BasicEnemies + wave2ArmoredEnemies + wave2FlyingEnemies + wave2DancingEnemies;
		enemies.reserve(enemies_created);

		int currentIndex = 0;
		for (int i = 0; i < numBasicEnemies; i++)
		{
			float newX = static_cast<float>((rand() % 1001) + 1300);
			float newY = static_cast<float>((rand() % 499) + 200);
			newX = static_cast<float>(static_cast<int>(newX) / 100 * 100);
			newY = static_cast<float>(static_cast<int>(newY) / 100 * 100);
			auto enemy = std::make_unique<Enemy>(newX, newY);
			enemy->enabled = false;
			enemies.push_back(std::move(enemy));
			currentIndex++;
		}
		for (int i = 0; i < numArmoredEnemies; i++)
		{
			float newX = static_cast<float>((rand() % 1001) + 1300);
			float newY = static_cast<float>((rand() % 499) + 200);
			newX = static_cast<float>(static_cast<int>(newX) / 100 * 100);
			newY = static_cast<float>(static_cast<int>(newY) / 100 * 100);
			enemies.push_back(std::make_unique<ArmoredEnemy>(newX, newY));
			currentIndex++;
		}
		for (int i = 0; i < numFlyingEnemies; i++)
		{
			float newX = static_cast<float>((rand() % 1001) + 1300);
			float newY = static_cast<float>((rand() % 499) + 200);
			newX = static_cast<float>(static_cast<int>(newX) / 100 * 100);
			newY = static_cast<float>(static_cast<int>(newY) / 100 * 100);
			enemies.push_back(std::make_unique<FlyingEnemy>(newX, newY));
			currentIndex++;
		}
		for (int i = 0; i < numDancingEnemies; i++)
		{
			float newX = static_cast<float>((rand() % 1001) + 1300);
			float newY = static_cast<float>((rand() % 499) + 200);
			newX = static_cast<float>(static_cast<int>(newX) / 100 * 100);
			newY = static_cast<float>(static_cast<int>(newY) / 100 * 100);
			enemies.push_back(std::make_unique<DancingEnemy>(newX, newY));
			currentIndex++;
		}

	}
	
	void triggerBossWave() {
		isBossWave = true;
		
		int numBossZombies = 0;
		if (currentGameLevel >= 1) {
			int baseBosses = 1 + (currentGameLevel - 1) / 2;
			float difficultyBonus = (difficultyMultiplier >= 3.0f) ? difficultyMultiplier * 1.5f : difficultyMultiplier;
			numBossZombies = std::min(6, static_cast<int>(baseBosses * difficultyBonus));
			if (numBossZombies == 0 && currentGameLevel >= 1) numBossZombies = 1;
		}
		
		if (numBossZombies > 0) {
			bool bossTypes[4] = {false, false, false, false};
			bool lanesUsed[GameConstants::BATTLEFIELD_ROWS] = {false};
			
			for (int b = 0; b < numBossZombies; b++) {
				float bossX = static_cast<float>(1300 + b * 250);
				
				int lane;
				if (numBossZombies <= GameConstants::BATTLEFIELD_ROWS) {
					do {
						lane = rand() % GameConstants::BATTLEFIELD_ROWS;
					} while (lanesUsed[lane]);
					lanesUsed[lane] = true;
				} else {
					lane = b % GameConstants::BATTLEFIELD_ROWS;
					bossX = static_cast<float>(1300 + (b / GameConstants::BATTLEFIELD_ROWS) * 400 + (b % GameConstants::BATTLEFIELD_ROWS) * 150);
				}
				
				float bossY = LaneSystem::getLaneCenterY(lane) + GameConstants::BOSS_Y_OFFSET_ADJUSTMENT;
				
				int bossType;
				if (numBossZombies > 4) {
					bossType = rand() % 4;
				} else {
					do {
						bossType = rand() % 4;
					} while (bossTypes[bossType] && b < 4);
					
					bossTypes[bossType] = true;
				}
				
				std::unique_ptr<Enemy> bossEnemy;
				switch(bossType) {
					case 0: bossEnemy = std::make_unique<BossEnemy>(bossX, bossY); break;
					case 1: bossEnemy = std::make_unique<BossArmoredEnemy>(bossX, bossY); break;
					case 2: bossEnemy = std::make_unique<BossFlyingEnemy>(bossX, bossY); break;
					case 3: bossEnemy = std::make_unique<BossDancingEnemy>(bossX, bossY); break;
				}
				bossEnemy->enabled = false;
				enemies.push_back(std::move(bossEnemy));
				enemies_created++;
			}
		}
	}
	
	void setTowerFactoryRef(TowerFactory* towers) {
		towerFactoryRef = towers;
	}
};
