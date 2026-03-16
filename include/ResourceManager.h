#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>
#include <future>
#include <vector>
#include <thread>
#include <mutex>
#include "MemoryInputStream.h"

namespace DefenseSynth {

class ResourceLoadException : public std::runtime_error {
public:
    ResourceLoadException(const std::string& resource) 
        : std::runtime_error("Failed to load: " + resource) {}
};

class ResourceManager {
private:
    static ResourceManager* instance;
    std::unordered_map<std::string, sf::Texture> textures;
    std::unordered_map<std::string, sf::SoundBuffer> sounds;
    std::unordered_map<std::string, sf::Font> fonts;
    std::unordered_map<std::string, std::unique_ptr<sf::Music>> music;
    std::unordered_map<std::string, std::shared_ptr<MemoryInputStream>> musicStreams;
    mutable std::mutex soundMutex;
    mutable std::mutex musicMutex;
    
    std::vector<std::uint8_t> bundleData;
    std::unordered_map<std::string, std::pair<std::size_t, std::size_t>> bundleIndex;
    bool bundleReady = false;
    
    ResourceManager();
    std::string getExecutableDirectory() const;
    std::string executableDir;
    
public:
    static ResourceManager& getInstance() {
        if (instance == nullptr) {
            instance = new ResourceManager();
        }
        return *instance;
    }
    
    static void cleanup() {
        if (instance != nullptr) {
            delete instance;
            instance = nullptr;
        }
    }
    
    bool preloadAllAssets();
    bool preloadTextures();
    bool preloadSounds();
    bool preloadFonts();
    
    const sf::Texture& getTexture(const std::string& name);
    const sf::SoundBuffer& getSound(const std::string& name);
    const sf::Font& getFont(const std::string& name);
    
    bool loadTexture(const std::string& path, const std::string& name);
    bool loadTextureWithRetry(const std::string& path, const std::string& name, int maxRetries = 3);
    bool loadSound(const std::string& path, const std::string& name);
    bool loadFont(const std::string& path, const std::string& name);
    bool loadMusic(const std::string& path, const std::string& name);
    bool loadSoundAsync(const std::string& path, const std::string& name);
    void createPlaceholderTexture(const std::string& name);
    
    void clearTextures() { textures.clear(); }
    void clearSounds() { sounds.clear(); }
    void clearFonts() { fonts.clear(); }
    void clearAll() {
        clearTextures();
        clearSounds();
        clearFonts();
    }
    
    bool isTextureLoaded(const std::string& name) const {
        return textures.find(name) != textures.end();
    }
    
    bool isSoundLoaded(const std::string& name) const {
        return sounds.find(name) != sounds.end();
    }
    
    bool isFontLoaded(const std::string& name) const {
        return fonts.find(name) != fonts.end();
    }
    
    sf::Music* getMusic(const std::string& name) {
        std::lock_guard<std::mutex> lock(musicMutex);
        auto it = music.find(name);
        if (it == music.end()) {
            return nullptr;
        }
        return it->second.get();
    }
    
    bool loadBundle(const std::string& path);
    bool hasInBundle(const std::string& path) const;
    bool getFromBundle(const std::string& path, const std::uint8_t*& ptr, std::size_t& size) const;
    
private:
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
};

}