#pragma once

#include<iostream>
#include<fstream>

class Player {

public:
	int level;
	int currency;
	int lives;
	int score;
	Player() {
	level = 1;
	currency = 100;
	lives=3;
	score = 0;
}
	void Serialize(std::ostream& stream) const {
		stream << level << std::endl;
		stream << currency << std::endl;
		stream << lives << std::endl;
		stream << score << std::endl;
	}

	void Deserialize(std::istream& stream) {
stream >> level;
		stream >> currency;
		stream >> lives;
stream >> score;
	}
	
	void reset() {
		level = 1;
		currency = 100;
		lives = 3;
		score = 0;
	}
};