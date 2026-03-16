#pragma once

#include "ScreenBase.h"
#include "LoadoutScreen.h"
#include "CardCollection.h"
#include "StateManager.h"
#include "CrystalManager.h"
#include <vector>
#include <map>

namespace DefenseSynth {

class LoadoutScreenWrapper : public Screen
{
private:
    LoadoutScreen* loadoutScreen;
    CardCollection cardCollection;
    std::vector<std::string> cachedSelection;
    std::map<std::string, int> cachedUpgradeLevels;
    
public:
    LoadoutScreenWrapper(sf::RenderWindow &window);
    ~LoadoutScreenWrapper();
    
    void refreshFromStateManager(sf::RenderWindow &window);
    void renderScreen(sf::RenderWindow &window, std::string &currentScreen) override;
    const std::vector<std::string> getSelectedUnits() const;
    std::map<std::string, int> getUpgradeLevels() const;
    int getUpgradeLevel(const std::string& unitType) const;
    float getDifficultyValue() const;
};

}
