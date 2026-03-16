#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <cmath>

namespace GameConstants {
    
    const std::string BASIC_ZOMBIE = "basic_zombie";
    const std::string BASIC_ZOMBIE1 = "basic_zombie1";
    const std::string ARMORED_ZOMBIE = "armored";
    const std::string FLYING_ZOMBIE = "flying";
    const std::string DANCING_ZOMBIE = "dancing";
    const std::string BOSS_PREFIX = "boss_";
    
    
    const std::string BOMB_TOWER = "BombTower";
    const std::string FREEZE_TOWER = "FreezeTower";
    const std::string AREA_ATTACK_TOWER = "AreaAttackTower";
    const std::string RAPID_SHOOTER = "RapidShooter";
    const std::string ATTACK_TOWER = "AttackTower";
    const std::string RESOURCE_GENERATOR = "ResourceGenerator";
    const int WINDOW_WIDTH = 1300;
    const int WINDOW_HEIGHT = 900;
    const int FPS = 60;
    
    const int BATTLEFIELD_ROWS = 5;
    const int BATTLEFIELD_COLS = 9;
    const int TILE_WIDTH = 92;
    const int TILE_HEIGHT = 107;
    const int TILE_SIZE = 100; 
    const int BATTLEFIELD_WIDTH = 1300; 
    const int BATTLEFIELD_HEIGHT = 900; 

    const int FIELD_START_X = 296;
    const int FIELD_START_Y = 162;
    const int X_OFFSET = 300;
    const int Y_OFFSET = 200;

    const int BASIC_SHOOTER_COST = 50;
    const int RAPID_SHOOTER_COST = 200;
    const int FREEZE_TOWER_COST = 100;
    const int AREA_ATTACK_TOWER_COST = 75;
    const int BARRIER_TOWER_COST = 50;
    const int BOMB_TOWER_COST = 150;
    const int RESOURCE_GENERATOR_COST = 100;

    const int BASIC_ZOMBIE_HEALTH = 10;
    const int ARMORED_ZOMBIE_HEALTH = 28;
    const int FLYING_ZOMBIE_HEALTH = 8;
    const int DANCING_ZOMBIE_HEALTH = 12;

    const int SUN_VALUE = 25;
    const int LAWNMOWER_SPEED = 15;

    const int BASE_TOWER_DAMAGE = 8;
    const int BASE_CITIZEN_DAMAGE = 8;
    const float BOSS_DAMAGE_MULTIPLIER = 1.5f;
    const float LEVEL_DAMAGE_SCALING = 1.25f;
    const int LEVEL_HEALTH_BASE_BONUS = 4;
    const int LEVEL_DAMAGE_BASE_BONUS = 3;
    const float DIFFICULTY_SCALING = 1.2f;
    const int CHERRY_BOMB_BASE_DAMAGE = 35;
    const int CHERRY_BOMB_RADIUS = 120;

    const int MAX_SELECTED_UNITS = 10;
    const int MIN_SELECTED_UNITS = 3;
    const int MAX_TOWERS = 15;

    const int MAX_SUNS_ON_SCREEN = 18;          
    const int MAX_STARS_ON_SCREEN = 12;         
    const int MAX_ZOMBIES_ON_SCREEN = 25;       
    const int RESOURCE_CLEANUP_THRESHOLD = 3;
    
    
    const float BASIC_ZOMBIE_SPEED = 30.0f;
    const float ARMORED_ZOMBIE_SPEED = 25.0f;
    const float FLYING_ZOMBIE_SPEED = 6400.0f;
    const float DANCING_ZOMBIE_SPEED = 35.0f;
    const float BOSS_ZOMBIE_SPEED = 20.0f;
    
    
    const float LEFT_BOUNDARY = 100.0f;
    const float RIGHT_BOUNDARY = 1300.0f;
    const float TOP_BOUNDARY = 200.0f;
    const float BOTTOM_BOUNDARY = 550.0f;
    const float ZOMBIE_SPAWN_X = 1300.0f;
    const float COLLISION_RANGE = 600.0f;
    
    
    const float BEHAVIOR_RESET_TIME = 5.0f;
    const float COLLISION_DAMAGE_DELAY = 0.4f;
    const float SUN_SPAWN_DELAY = 2.0f;
    const float FREEZE_BASE_DURATION = 2.0f;
    const float POISON_BASE_DURATION = 5.0f;
    const float FIRE_BASE_DURATION = 3.0f;
    
    
    const int SPRITE_SIZE = 100;
    const int SPRITE_WIDTH = 100;
    const int SPRITE_HEIGHT = 100;
    const int MAX_ANIMATION_FRAMES = 28;
    
    
    const float ZOMBIE_COLLISION_WIDTH = 50.0f;
    const float ZOMBIE_COLLISION_HEIGHT = 50.0f;
    const float BOSS_COLLISION_BONUS = 20.0f;
    const float SEPARATION_RADIUS = 60.0f;
    const float ZOMBIE_SEPARATION_PUSH = 2.0f;
    
    const float BOSS_SCALE_FACTOR = 1.4f;
    const float BOSS_Y_OFFSET_ADJUSTMENT = -25.0f;
    const float ZOMBIE_LANE_CENTER_OFFSET = -15.0f;
    const float BASIC_ZOMBIE_Y_OFFSET = -8.0f;
    const float ARMORED_ZOMBIE_Y_OFFSET = -12.0f;
    const float FLYING_ZOMBIE_Y_OFFSET = -20.0f;
    const float DANCING_ZOMBIE_Y_OFFSET = -10.0f;
}

namespace LaneSystem {
    constexpr int NUM_LANES = GameConstants::BATTLEFIELD_ROWS;
    constexpr float LANE_TOP_Y = GameConstants::FIELD_START_Y;
    constexpr float LANE_HEIGHT = GameConstants::TILE_SIZE;
    constexpr float LANE_WIDTH = GameConstants::BATTLEFIELD_WIDTH - GameConstants::X_OFFSET;
    constexpr float LANE_TOLERANCE = 15.0f;
    
    inline int getLane(float y) {
        if (y < LANE_TOP_Y - LANE_TOLERANCE) return -1;
        if (y > LANE_TOP_Y + (NUM_LANES * LANE_HEIGHT) + LANE_TOLERANCE) return -1;
        
        float adjustedY = y - LANE_TOP_Y;
        int lane = static_cast<int>(adjustedY / LANE_HEIGHT);
        
        if (lane < 0) lane = 0;
        if (lane >= NUM_LANES) lane = NUM_LANES - 1;
        
        return lane;
    }
    
    inline float getLaneY(int lane) {
        if (lane < 0 || lane >= NUM_LANES) return -1;

        return LANE_TOP_Y + (lane * LANE_HEIGHT);
    }
    
    inline float getLaneCenterY(int lane) {
        if (lane < 0 || lane >= NUM_LANES) return -1;

        return LANE_TOP_Y + (lane * LANE_HEIGHT) + (LANE_HEIGHT / 2.0f);
    }
    
    inline bool sameLane(float y1, float y2) {
        int lane1 = getLane(y1);
        int lane2 = getLane(y2);
        return (lane1 >= 0 && lane2 >= 0 && lane1 == lane2);
    }
    
    inline bool isInLaneBounds(float y, int lane) {
        if (lane < 0 || lane >= NUM_LANES) return false;
        float laneTop = LANE_TOP_Y + (lane * LANE_HEIGHT);
        float laneBottom = laneTop + LANE_HEIGHT;
        return (y >= laneTop - LANE_TOLERANCE && y <= laneBottom + LANE_TOLERANCE);
    }
}

namespace UIConstants {
    const float BASE_WINDOW_WIDTH = 1300.0f;
    const float BASE_WINDOW_HEIGHT = 900.0f;

    namespace MainMenu {
        const float PLAY_BUTTON_LEFT = 66.0f;
        const float PLAY_BUTTON_RIGHT = 635.0f;
        const float PLAY_BUTTON_TOP = 356.0f;
        const float PLAY_BUTTON_BOTTOM = 501.0f;
        
        const float TUTORIAL_BUTTON_TOP = 526.0f;
        const float TUTORIAL_BUTTON_BOTTOM = 669.0f;
        
        const float RANKINGS_BUTTON_TOP = 695.0f;
        const float RANKINGS_BUTTON_BOTTOM = 839.0f;
        
        const float EXIT_BUTTON_MARGIN = 20.0f;
        const float EXIT_BUTTON_SIZE = 100.0f;
        
        const int PLAY_BUTTON_FONT_SIZE = 36;
        const int EXIT_BUTTON_FONT_SIZE = 42;
    }

    namespace Navigation {
        const float BACK_BUTTON_LEFT = 927.0f;
        const float BACK_BUTTON_RIGHT = 1277.0f;
        const float BACK_BUTTON_TOP = 810.0f;
        const float BACK_BUTTON_BOTTOM = 889.0f;
    }

    namespace PauseScreen {
        const float RESUME_BUTTON_LEFT = 377.0f;
        const float RESUME_BUTTON_RIGHT = 923.0f;
        const float RESUME_BUTTON_TOP = 318.0f;
        const float RESUME_BUTTON_BOTTOM = 450.0f;
        
        const float MAIN_MENU_BUTTON_LEFT = 205.0f;
        const float MAIN_MENU_BUTTON_RIGHT = 1095.0f;
        const float MAIN_MENU_BUTTON_TOP = 440.0f;
        const float MAIN_MENU_BUTTON_BOTTOM = 617.0f;
    }

    namespace Rankings {
        const float HIGHSCORES_X = 450.0f;
        const float HIGHSCORES_Y = 300.0f;
        const int HIGHSCORES_FONT_SIZE = 32;
        const int MAX_DISPLAYED_SCORES = 10;
    }

    namespace GameOver {
        const int GAME_OVER_FONT_SIZE = 64;
        const int SCORE_FONT_SIZE = 48;
        const int INSTRUCTION_FONT_SIZE = 32;
        const float GAME_OVER_Y_OFFSET = -100.0f;
        const float SCORE_X = 50.0f;
        const float SCORE_Y_OFFSET = -20.0f;
        const float INSTRUCTION_Y_OFFSET = 50.0f;
    }

    namespace WinScreen {
        const int WIN_TEXT_FONT_SIZE = 64;
        const int INSTRUCTION_FONT_SIZE = 32;
        const float WIN_TEXT_Y_OFFSET = -50.0f;
        const float INSTRUCTION_Y_OFFSET = 50.0f;
    }

    namespace GameComplete {
        const int WIN_TEXT_FONT_SIZE = 72;
        const int NAME_PROMPT_FONT_SIZE = 36;
        const int INPUT_FONT_SIZE = 32;
        const int INSTRUCTION_FONT_SIZE = 24;
        
        const float WIN_TEXT_Y = 200.0f;
        const float NAME_PROMPT_X = 50.0f;
        const float NAME_PROMPT_Y = 400.0f;
        const float INPUT_X = 50.0f;
        const float INPUT_Y = 450.0f;
        const float INSTRUCTION_Y = 550.0f;
        
        const int MAX_NAME_LENGTH = 20;
    }

    namespace LevelUp {
        const int LEVEL_COMPLETE_FONT_SIZE = 48;
        const float DISPLAY_TIME_SECONDS = 3.0f;
    }

    namespace Battlefield {
        const int CURRENCY_FONT_SIZE = 24;
        const int SCORE_FONT_SIZE = 24;
        const int LEVEL_FONT_SIZE = 24;
        const int POPULATION_FONT_SIZE = 28;
        const int WAVE_TEXT_FONT_SIZE = 48;
        const int START_TEXT_FONT_SIZE = 36;
        
        const float CURRENCY_X = 45.0f;
        const float CURRENCY_Y = 70.0f;
        
        const float SCORE_X = 1100.0f;
        const float SCORE_Y = 850.0f;
        
        const float LEVEL_X = 20.0f;
        const float LEVEL_Y = 850.0f;
        
        const float POPULATION_X = 20.0f;
        const float POPULATION_Y = 820.0f;
        
        const float PAUSE_BUTTON_X = 1200.0f;
        const float PAUSE_BUTTON_Y = 0.0f;
        const int PAUSE_BUTTON_SIZE = 100;
        
        const int INITIAL_CITY_POPULATION = 100;
        const float WAVE_TEXT_DISPLAY_TIME = 5.0f;
        
        const float HEALTH_BAR_WIDTH = 8.0f;
        const float HEALTH_BAR_HEIGHT = 42.0f;
        const float HEALTH_BAR_INNER_WIDTH = 6.0f;
        const float HEALTH_BAR_INNER_HEIGHT = 40.0f;
        const float HEALTH_BAR_X_OFFSET = -16.0f;
        const float HEALTH_BAR_Y_OFFSET = 8.0f;
        const float HEALTH_BAR_INNER_X_OFFSET = -15.0f;
        const float HEALTH_BAR_INNER_Y_OFFSET = 9.0f;
        
        const float HEALTH_GOOD_THRESHOLD = 0.6f;
        const float HEALTH_WARNING_THRESHOLD = 0.3f;
    }

    namespace Audio {
        const std::vector<std::string> MENU_TRACKS = {
            "Look_up_at_the_Sky.wav",
            "UraniwaNi.wav",
            "Loonboon.wav",
            "Kitanai_Sekai.wav",
            "2.75.wav"
        };
        
        const std::vector<std::string> PAUSE_TRACKS = {
            "Mountains_1.wav",
            "Mountains_2.wav",
            "Mountains.wav"
        };
        
        const std::vector<std::string> BATTLE_TRACKS = {
            "Kitanai_Sekai.wav",
            "Look_up_at_the_Sky.wav",
            "UraniwaNi.wav", 
            "Loonboon.wav",
            "2.75.wav"
        };
    }
}