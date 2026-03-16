#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include "GameCursor.h"
#include "Constants.h"
#include "CrystalManager.h"
#include "ResourceManager.h"
#include <fstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <random>


class Sun {
public:
	int value;
	bool exist;
	bool animating;
	bool willMove;
	sf::Clock lifeClock;
	coordinates position;
	sf::Sprite sprite;
	sf::Clock clock;
	float landingY;
	bool movingHorizontally;
	bool attractedToGenerator;
	Sun(float x = 0, float y = 0) {
		value = 25;
		auto& rm = DefenseSynth::ResourceManager::getInstance();
		bool ok = rm.loadTextureWithRetry("assets/images/Sun.gif", "sun_gif", 1);
		if (!ok) ok = rm.loadTextureWithRetry("assets/images/sun.png", "sun_png", 1);
		std::string texName;
		if (rm.isTextureLoaded("sun_gif")) texName = "sun_gif";
		else if (rm.isTextureLoaded("sun_png")) texName = "sun_png";
		else { rm.createPlaceholderTexture("sun_placeholder"); texName = "sun_placeholder"; }
		sprite.setTexture(rm.getTexture(texName));
		sprite.setOrigin(25, 25);
		sprite.setPosition(x, y);
		position.x = x;
		exist = true;
		animating = false;
		position.y = y;
		willMove = true;
		landingY = GameConstants::FIELD_START_Y + GameConstants::BATTLEFIELD_ROWS * GameConstants::TILE_SIZE;
		movingHorizontally = false;
		attractedToGenerator = false;
		lifeClock.restart();
	}
	Sun(const Sun& sun) {
		value = sun.value;
		sprite = sun.sprite;
		position.x = sun.position.x;
		position.y = sun.position.y;
		exist = sun.exist;
		animating = sun.animating;
		willMove = sun.willMove;
		landingY = sun.landingY;
		movingHorizontally = sun.movingHorizontally;
		attractedToGenerator = sun.attractedToGenerator;
		lifeClock = sun.lifeClock;
	}
	void draw(sf::RenderWindow& window) {
		if (exist || animating) {
			window.draw(sprite);
		}
	}
	void move() {
		CrystalManager& cm = CrystalManager::getInstance();
		float durationMultiplier = cm.getSunDurationMultiplier();
		float maxLifeTime = 60.f * durationMultiplier;

		if (exist && !willMove && lifeClock.getElapsedTime().asSeconds() >= maxLifeTime) {
			exist = false;
		}
		if (exist && willMove) {
			if (clock.getElapsedTime().asMilliseconds() >= 50) {
				clock.restart();
				animate();
				if (!movingHorizontally) {
					position.y += 1.0f;
					if (position.y >= landingY) {
						position.y = landingY;
						movingHorizontally = true;
					}
				} else {
					position.y += 0.5f;
					float fieldBottomY = GameConstants::FIELD_START_Y + GameConstants::BATTLEFIELD_ROWS * GameConstants::TILE_SIZE + 50;
					if (position.y >= fieldBottomY) {
						willMove = false;
						lifeClock.restart();
					}
				}
				sprite.setPosition(position.x, position.y);
			}
		}
		else if (animating){
			if (clock.getElapsedTime().asMilliseconds() >= 16) {
				clock.restart();
				float dx = 0 - position.x;
				float dy = 0 - position.y;
				float len = std::sqrt(dx * dx + dy * dy);
				if (len < 1.0f) {
					position.x = 0;
					position.y = 0;
					animating = false;
					exist = false;
				} else {
					float vx = dx / len;
					float vy = dy / len;
					float step = 6.0f;
					position.x += vx * step;
					position.y += vy * step;
					sprite.setPosition(position.x, position.y);
				}
			}
		}
	}
	void animate() {
		sprite.rotate(1);
	}

	void Serialize(std::ostream& stream) const {
		stream << value << std::endl;
		stream << exist << std::endl;
		stream << animating << std::endl;
		stream << willMove << std::endl;
		stream << position.x << std::endl;
		stream << position.y << std::endl;
	}

	void Deserialize(std::istream& stream) {
		stream >> value;
		stream >> exist;
		stream >> animating;
		stream >> willMove;
		stream >> position.x;
		stream >> position.y;
		sprite.setPosition(position.x, position.y);
		landingY = GameConstants::FIELD_START_Y + GameConstants::BATTLEFIELD_ROWS * GameConstants::TILE_SIZE;
		movingHorizontally = false;
		std::cout << "Sun Deserialized" << std::endl;
		std::cout<< "attributes: " << value << " " << exist << " " << animating << " " << willMove << " " << position.x << " " << position.y << std::endl;
	}
	

};

class SunFactory {
public:
	std::vector<std::unique_ptr<Sun>> suns;
	sf::Clock spawnClock;
	coordinates* generatorPosition;
	sf::Vector2f adjustPosition(float x, float y) {
		int col = static_cast<int>((x - GameConstants::FIELD_START_X) / GameConstants::TILE_SIZE);
		if (col < 0) col = 0;
		if (col >= GameConstants::BATTLEFIELD_COLS) col = GameConstants::BATTLEFIELD_COLS - 1;
		int row = static_cast<int>((y - GameConstants::FIELD_START_Y) / GameConstants::TILE_SIZE);
		if (row < 0) row = 0;
		if (row >= GameConstants::BATTLEFIELD_ROWS) row = GameConstants::BATTLEFIELD_ROWS - 1;

		auto centerAt = [&](int c) -> sf::Vector2f {
			int cl = c > 0 ? c - 1 : 0;
			float cx = GameConstants::FIELD_START_X + cl * GameConstants::TILE_SIZE + GameConstants::TILE_SIZE / 2.0f - 6.0f;
			float cy = GameConstants::FIELD_START_Y + row * GameConstants::TILE_SIZE + GameConstants::TILE_SIZE / 2.0f;
			return {cx, cy};
		};

		auto occupied = [&](const sf::Vector2f& p) -> bool {
			sf::FloatRect candidate(p.x - 30.f, p.y - 30.f, 60.f, 60.f);
			for (const auto& s : suns) {
				if ((s->exist || s->animating) && candidate.intersects(s->sprite.getGlobalBounds())) return true;
			}
			return false;
		};

		sf::Vector2f snapped = centerAt(col);
		if (!occupied(snapped)) return snapped;
		for (int dc = 1; dc < GameConstants::BATTLEFIELD_COLS; ++dc) {
			int c = (col + dc) % GameConstants::BATTLEFIELD_COLS;
			sf::Vector2f p = centerAt(c);
			if (!occupied(p)) return p;
		}
		return snapped;
	}
	
	SunFactory(const SunFactory&) = delete;
	SunFactory& operator=(const SunFactory&) = delete;
	
	SunFactory(SunFactory&&) = default;
	SunFactory& operator=(SunFactory&&) = default;
	SunFactory(int numSuns = 0) {
        CrystalManager& cm = CrystalManager::getInstance();
        float frequencyMultiplier = cm.getSunFrequencyMultiplier();
        int adjustedSuns = static_cast<int>(numSuns * frequencyMultiplier);
        
        generatorPosition = nullptr;
        		suns.reserve(adjustedSuns > 0 ? adjustedSuns : 10);
		std::vector<int> availableCols;
		for (int c = 0; c < GameConstants::BATTLEFIELD_COLS; c++) {
			availableCols.push_back(c);
		}
		std::shuffle(availableCols.begin(), availableCols.end(), std::default_random_engine());
		
		for (int i = 0; i < adjustedSuns; i++) {
			int col = availableCols[i % GameConstants::BATTLEFIELD_COLS];
            float newX = GameConstants::FIELD_START_X + col * GameConstants::TILE_SIZE + GameConstants::TILE_SIZE / 4.0f - GameConstants::TILE_SIZE;
            int j = (rand() % 17) - 8;
            newX += static_cast<float>(j);
            float newY = -GameConstants::TILE_SIZE;
            auto s = std::make_unique<Sun>(newX, newY);
            float sc = 0.5f + static_cast<float>(rand() % 5) * 0.02f;
			s->sprite.setScale(sc, sc);
			int row = rand() % GameConstants::BATTLEFIELD_ROWS;
			int jy0 = (rand() % 11) - 5;
			s->landingY = GameConstants::FIELD_START_Y + GameConstants::BATTLEFIELD_ROWS * GameConstants::TILE_SIZE;
			suns.push_back(std::move(s));
		}
	}
	void move() {
        for (const auto& sun : suns) {
            if (generatorPosition && !sun->attractedToGenerator && rand() % 100 < 30) {
                float dx = generatorPosition->x - sun->position.x;
                float dy = generatorPosition->y - sun->position.y;
                float distance = std::sqrt(dx * dx + dy * dy);
                if (distance < 150.0f && distance > 10.0f) {
                    sun->attractedToGenerator = true;
                }
            }
            
            if (sun->attractedToGenerator && generatorPosition) {
                float dx = generatorPosition->x - sun->position.x;
                float dy = generatorPosition->y - sun->position.y;
                float distance = std::sqrt(dx * dx + dy * dy);
                if (distance > 40.0f) {
                    float speed = 3.0f;
                    sun->position.x += (dx / distance) * speed;
                    sun->position.y += (dy / distance) * speed;
                    sun->sprite.setPosition(sun->position.x, sun->position.y);
                } else {
                    sun->exist = false;
                }
            } else {
                sun->move();
            }
        }
    }
	void draw(sf::RenderWindow& window) {
		for (const auto& sun : suns) {
			sun->draw(window);
		}
	}
	bool isSunThere(float x, float y) {
		for (const auto& sun : suns) {
			sf::FloatRect sunBounds = sun->sprite.getGlobalBounds();
			if (sunBounds.contains(x, y)) {
				return true;
			}
		}
		return false;
	}

	void moveSunToOrigin(float x, float y) {
		for (const auto& sun : suns) {
			sf::FloatRect sunBounds = sun->sprite.getGlobalBounds();
			if (sunBounds.contains(x, y)) {
				sun->animating = true;
				sun->exist = false;	
				return;
			}
		}
	}
	void restartClock() {
		for (const auto& sun : suns) {
			sun->clock.restart();
		}
	}

	void addSunFromPlant(float x, float y) {
        sf::Vector2f p = adjustPosition(x, y);
        auto newSun = std::make_unique<Sun>(p.x, p.y);
        float sc = 0.5f + static_cast<float>(rand() % 5) * 0.02f;
        newSun->sprite.setScale(sc, sc);
        newSun->willMove = false;
        newSun->landingY = GameConstants::FIELD_START_Y + GameConstants::BATTLEFIELD_ROWS * GameConstants::TILE_SIZE;
        suns.push_back(std::move(newSun));
    }

    void addFallingSun(float x, float y) {
        int j = (rand() % 17) - 8;
        int col = static_cast<int>((x - GameConstants::FIELD_START_X) / GameConstants::TILE_SIZE);
        if (col < 0) col = 0;
        if (col >= GameConstants::BATTLEFIELD_COLS) col = GameConstants::BATTLEFIELD_COLS - 1;
        float newX = GameConstants::FIELD_START_X + col * GameConstants::TILE_SIZE + GameConstants::TILE_SIZE / 2.0f + static_cast<float>(j) - GameConstants::TILE_SIZE;
        auto newSun = std::make_unique<Sun>(newX, y);
        float sc = 0.5f + static_cast<float>(rand() % 5) * 0.02f;
        newSun->sprite.setScale(sc, sc);
        newSun->willMove = true;
        int row = rand() % GameConstants::BATTLEFIELD_ROWS;
        int jy2 = (rand() % 11) - 5;
        newSun->landingY = GameConstants::FIELD_START_Y + GameConstants::BATTLEFIELD_ROWS * GameConstants::TILE_SIZE;
        suns.push_back(std::move(newSun));
    }

	void removeNonExistantSun() {
		suns.erase(
			std::remove_if(suns.begin(), suns.end(), 
				[](const std::unique_ptr<Sun>& sun) {
					return !sun->exist && !sun->animating;
				}), 
			suns.end()
		);
	}
	int getExistantSunCount() {
		return static_cast<int>(std::count_if(suns.begin(), suns.end(),
			[](const std::unique_ptr<Sun>& sun) {
				return sun->exist;
			}));
	}

	void removeSun(float x, float y) {
		auto it = std::find_if(suns.begin(), suns.end(),
			[x, y](const std::unique_ptr<Sun>& sun) {
				return sun->position.x == x && sun->position.y == y;
			});
		if (it != suns.end()) {
			suns.erase(it);
		}
	}

	~SunFactory() = default;
	void Serialize(std::ostream& stream) const {
		stream << suns.size() << std::endl;
		for (const auto& sun : suns) {
			sun->Serialize(stream);
		}
	}

	void Deserialize(std::istream& stream) {
		size_t temp_suns_created;
		stream >> temp_suns_created;
		suns.clear();
		suns.reserve(temp_suns_created);
		for (size_t i = 0; i < temp_suns_created; i++) {
			auto sun = std::make_unique<Sun>();
			sun->Deserialize(stream);
			suns.push_back(std::move(sun));
		}
	}
	
	void setGeneratorPosition(coordinates* pos) {
		generatorPosition = pos;
	}
	
	void resetForNextLevel(int numSuns)
	{
		suns.clear();
		generatorPosition = nullptr;
		
		CrystalManager& cm = CrystalManager::getInstance();
        float frequencyMultiplier = cm.getSunFrequencyMultiplier();
        int adjustedSuns = static_cast<int>(numSuns * frequencyMultiplier);
		
		                suns.reserve(adjustedSuns);
		std::vector<int> availableCols2;
		for (int c = 0; c < GameConstants::BATTLEFIELD_COLS; c++) {
			availableCols2.push_back(c);
		}
		std::shuffle(availableCols2.begin(), availableCols2.end(), std::default_random_engine());
		
                for (int i = 0; i < adjustedSuns; i++)
                {
            int col = availableCols2[i % GameConstants::BATTLEFIELD_COLS];
            float newX = GameConstants::FIELD_START_X + col * GameConstants::TILE_SIZE + GameConstants::TILE_SIZE / 4.0f - GameConstants::TILE_SIZE;
            int j = (rand() % 17) - 8;
            newX += static_cast<float>(j);
            float newY = -GameConstants::TILE_SIZE;
            auto s = std::make_unique<Sun>(newX, newY);
            float sc = 0.5f + static_cast<float>(rand() % 5) * 0.02f;
            s->sprite.setScale(sc, sc);
            int row = rand() % GameConstants::BATTLEFIELD_ROWS;
            int jy3 = (rand() % 11) - 5;
            s->landingY = GameConstants::FIELD_START_Y + GameConstants::BATTLEFIELD_ROWS * GameConstants::TILE_SIZE;
            suns.push_back(std::move(s));
        }
    }

};
