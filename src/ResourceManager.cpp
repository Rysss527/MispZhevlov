#include "ResourceManager.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <fstream>
#include "MemoryInputStream.h"
#ifdef _WIN32
#include <windows.h>
#endif

namespace DefenseSynth {

ResourceManager* ResourceManager::instance = nullptr;

ResourceManager::ResourceManager() {
    executableDir = getExecutableDirectory();
    std::vector<std::string> candidates = {
        "assets.pak",
        "assets.pack",
        "assets.bin"
    };
    for (const auto& f : candidates) {
        std::string p1 = executableDir + "/" + f;
        if (loadBundle(p1)) break;
        std::string p2 = executableDir + "/../" + f;
        if (loadBundle(p2)) break;
        std::string p3 = executableDir + "/../../defen_synth(pvz_like)/" + f;
        if (loadBundle(p3)) break;
    }
}

std::string ResourceManager::getExecutableDirectory() const {
    std::string exePath;
    
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    exePath = std::string(buffer);
    size_t lastSlash = exePath.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        exePath = exePath.substr(0, lastSlash);
    }
#else
    exePath = ".";
#endif
    
    return exePath;
}

bool ResourceManager::preloadAllAssets() {
    bool success = true;
    success &= preloadTextures();
    success &= preloadSounds();
    success &= preloadFonts();
    return success;
}

bool ResourceManager::preloadTextures() {
    struct TextureLoadTask {
        std::string path;
        std::string name;
    };
    
    std::vector<TextureLoadTask> tasks = {
        {"assets/images/zombie.png", "zombie"},
        {"assets/images/zombie1.png", "zombie1"},
        {"assets/images/footballzombie.png", "footballzombie"},
        {"assets/images/flyingzombie.png", "flyingzombie"},
        {"assets/images/dancingzombie.png", "dancingzombie"},
        {"assets/images/peeshooter.png", "peeshooter"},
        {"assets/images/sunflower.png", "sunflower"},
        {"assets/images/cherrybomb.png", "cherrybomb"},
        {"assets/images/wallnut.png", "wallnut"},
        {"assets/images/snowpea.png", "snowpea"},
        {"assets/images/repeater.png", "repeater"},
        {"assets/images/sun.png", "sun"},
        {"assets/images/shot.png", "shot"},
        {"assets/images/ProjectilePea.png", "ProjectilePea"},
        {"assets/images/ProjectileSnowPea.png", "ProjectileSnowPea"},
        {"assets/images/background.png", "background"},
        {"assets/images/pause.png", "pause"},
        {"assets/images/22.png", "app_icon"},
        {"assets/images/lawnmover.png", "lawnmower"},
        {"assets/images/endscreen.png", "endscreen"},
        {"assets/images/homescreen.png", "mainmenu"},
        {"assets/images/shop.png", "shop"},
        {"assets/images/instructions.png", "tutorial"},
        {"assets/images/leaderboard.png", "rankings"},
        {"assets/images/pausesccreen.png", "pausescreen"},
        {"assets/images/levelup.png", "levelup"},
        {"assets/images/grid.png", "grid"},
        {"assets/images/shovel.png", "shovel"},
        {"assets/images/plant.png", "plant"},
        {"assets/images/disabler.png", "disabler"},
        {"assets/images/fumeshroom.png", "fumeshroom"},
        {"assets/images/peeshoot.png", "peeshoot"},
        {"assets/images/zombieAnimation.png", "zombieAnimation"},
        {"assets/images/zombieOnPlace.png", "zombieOnPlace"},
        {"assets/images/spriteballoon.png", "spriteballoon"},
        {"assets/images/spritefootball.png", "spritefootball"},
        {"assets/images/crystal.png", "crystal"}
    };
    
    size_t numThreads = std::min(static_cast<size_t>(4), tasks.size());
    size_t tasksPerThread = tasks.size() / numThreads;
    std::vector<std::future<bool>> futures;
    
    for (size_t i = 0; i < numThreads; ++i) {
        size_t start = i * tasksPerThread;
        size_t end = (i == numThreads - 1) ? tasks.size() : (i + 1) * tasksPerThread;
        
        futures.push_back(std::async(std::launch::async, [this, &tasks, start, end]() {
            bool threadSuccess = true;
            for (size_t j = start; j < end; ++j) {
                threadSuccess &= loadTextureWithRetry(tasks[j].path, tasks[j].name, 1);
            }
            return threadSuccess;
        }));
    }
    
    bool success = true;
    for (auto& future : futures) {
        success &= future.get();
    }
    
    return success;
}

bool ResourceManager::preloadSounds() {
    bool success = true;
    
    std::vector<std::pair<std::string, std::string>> smallSounds = {
        {"assets/audio/puff.wav", "puff"},
        {"assets/audio/points.wav", "points"},
        {"assets/audio/plant1.wav", "plant"},
        {"assets/audio/splat1.wav", "splat"},
        {"assets/audio/explosion.wav", "explosion"}
    };
    
    std::vector<std::future<bool>> futures;
    for (const auto& [path, name] : smallSounds) {
        futures.push_back(std::async(std::launch::async, [this, path, name]() {
            return loadSoundAsync(path, name);
        }));
    }
    
    loadMusic("assets/audio/winmusic.wav", "winmusic");
    loadMusic("assets/audio/losemusic.wav", "losemusic");
    loadMusic("assets/audio/Look_up_at_the_Sky.wav", "menu_music_1");
    loadMusic("assets/audio/UraniwaNi.wav", "menu_music_2");
    loadMusic("assets/audio/Loonboon.wav", "menu_music_3");
    loadMusic("assets/audio/Kitanai_Sekai.wav", "menu_music_4");
    loadMusic("assets/audio/2.75.wav", "menu_music_5");
    
    for (auto& future : futures) {
        success &= future.get();
    }
    
    return success;
}

bool ResourceManager::preloadFonts() {
    bool success = true;
    
    success &= loadFont("assets/fonts/arial.ttf", "arial");
    if (!success) {
        success = loadFont("assets/fonts/new.ttf", "default");
    }
    if (!success) {
        success = loadFont("assets/fonts/arial.ttf", "arial");
    }
    
    return success;
}

const sf::Texture& ResourceManager::getTexture(const std::string& name) {
    auto it = textures.find(name);
    if (it == textures.end()) {
        static sf::Texture errorTexture;
        if (errorTexture.getSize().x == 0) {
            errorTexture.create(50, 50);
        }
        return errorTexture;
    }
    return it->second;
}

const sf::SoundBuffer& ResourceManager::getSound(const std::string& name) {
    auto it = sounds.find(name);
    if (it == sounds.end()) {
        throw ResourceLoadException(name);
    }
    return it->second;
}

const sf::Font& ResourceManager::getFont(const std::string& name) {
    auto it = fonts.find(name);
    if (it == fonts.end()) {
        static sf::Font defaultFont;
        static bool fontLoaded = false;
        if (!fontLoaded) {
            if (!defaultFont.loadFromFile("assets/fonts/arial.ttf")) {
                if (!defaultFont.loadFromFile(executableDir + "/assets/fonts/arial.ttf")) {
                    if (!defaultFont.loadFromFile(executableDir + "/../assets/fonts/arial.ttf")) {
                        defaultFont.loadFromFile(executableDir + "/../../defen_synth(pvz_like)/assets/fonts/arial.ttf");
                    }
                }
            }
            fontLoaded = true;
        }
        return defaultFont;
    }
    return it->second;
}

bool ResourceManager::loadTexture(const std::string& path, const std::string& name) {
    return loadTextureWithRetry(path, name, 1);
}

bool ResourceManager::loadTextureWithRetry(const std::string& path, const std::string& name, int maxRetries) {
    if (textures.find(name) != textures.end()) {
        return true;
    }
    
    sf::Texture texture;
    const std::uint8_t* memPtr = nullptr;
    std::size_t memSize = 0;
    if (hasInBundle(path) && getFromBundle(path, memPtr, memSize)) {
        if (texture.loadFromMemory(memPtr, memSize)) {
            texture.setSmooth(true);
            textures[name] = std::move(texture);
            return true;
        }
    }

    if (texture.loadFromFile(path)) {
        texture.setSmooth(true);
        textures[name] = std::move(texture);
        return true;
    }
    
    std::string exeDirPath = executableDir + "/" + path;
    if (texture.loadFromFile(exeDirPath)) {
        texture.setSmooth(true);
        textures[name] = std::move(texture);
        return true;
    }
    
    std::string parentPath = executableDir + "/../" + path;
    if (texture.loadFromFile(parentPath)) {
        texture.setSmooth(true);
        textures[name] = std::move(texture);
        return true;
    }
    
    std::string projectPath = executableDir + "/../../defen_synth(pvz_like)/" + path;
    if (texture.loadFromFile(projectPath)) {
        texture.setSmooth(true);
        textures[name] = std::move(texture);
        return true;
    }
    
    createPlaceholderTexture(name);
    return false;
}

void ResourceManager::createPlaceholderTexture(const std::string& name) {
    sf::Texture placeholder;
    placeholder.create(100, 100);
    
    std::hash<std::string> hasher;
    size_t hashValue = hasher(name);
    
    sf::Image img;
    img.create(100, 100, sf::Color(
        static_cast<sf::Uint8>(hashValue % 256),
        static_cast<sf::Uint8>((hashValue >> 8) % 256),
        static_cast<sf::Uint8>((hashValue >> 16) % 256)
    ));
    
    placeholder.loadFromImage(img);
    textures[name] = placeholder;
}

bool ResourceManager::loadSound(const std::string& path, const std::string& name) {
    if (sounds.find(name) != sounds.end()) {
        return true;
    }
    
    sf::SoundBuffer buffer;
    const std::uint8_t* memPtr = nullptr;
    std::size_t memSize = 0;
    if (hasInBundle(path) && getFromBundle(path, memPtr, memSize)) {
        if (buffer.loadFromMemory(memPtr, memSize)) {
            sounds[name] = buffer;
            return true;
        }
    }

    if (buffer.loadFromFile(path)) {
        sounds[name] = buffer;
        return true;
    }
    
    std::string exeDirPath = executableDir + "/" + path;
    if (buffer.loadFromFile(exeDirPath)) {
        sounds[name] = buffer;
        return true;
    }
    
    std::string parentPath = executableDir + "/../" + path;
    if (buffer.loadFromFile(parentPath)) {
        sounds[name] = buffer;
        return true;
    }
    
    std::string projectPath = executableDir + "/../../defen_synth(pvz_like)/" + path;
    if (buffer.loadFromFile(projectPath)) {
        sounds[name] = buffer;
        return true;
    }
    
    return false;
}

bool ResourceManager::loadSoundAsync(const std::string& path, const std::string& name) {
    {
        std::lock_guard<std::mutex> lock(soundMutex);
        if (sounds.find(name) != sounds.end()) {
            return true;
        }
    }
    
    sf::SoundBuffer buffer;
    bool loaded = false;
    const std::uint8_t* memPtr = nullptr;
    std::size_t memSize = 0;
    if (hasInBundle(path) && getFromBundle(path, memPtr, memSize)) {
        if (buffer.loadFromMemory(memPtr, memSize)) {
            loaded = true;
        }
    }

    if (!loaded && buffer.loadFromFile(path)) {
        loaded = true;
    } else if (!loaded) {
        std::string exeDirPath = executableDir + "/" + path;
        if (buffer.loadFromFile(exeDirPath)) {
            loaded = true;
        } else {
            std::string parentPath = executableDir + "/../" + path;
            if (buffer.loadFromFile(parentPath)) {
                loaded = true;
            } else {
                std::string projectPath = executableDir + "/../../defen_synth(pvz_like)/" + path;
                if (buffer.loadFromFile(projectPath)) {
                    loaded = true;
                }
            }
        }
    }
    
    if (loaded) {
        std::lock_guard<std::mutex> lock(soundMutex);
        sounds[name] = std::move(buffer);
        return true;
    }
    
    return false;
}

bool ResourceManager::loadMusic(const std::string& path, const std::string& name) {
    {
        std::lock_guard<std::mutex> lock(musicMutex);
        if (music.find(name) != music.end()) {
            return true;
        }
    }
    
    auto musicPtr = std::make_unique<sf::Music>();
    bool loaded = false;
    const std::uint8_t* memPtr = nullptr;
    std::size_t memSize = 0;
    if (hasInBundle(path) && getFromBundle(path, memPtr, memSize)) {
        if (musicPtr->openFromMemory(memPtr, memSize)) {
            loaded = true;
        } else {
            auto stream = std::make_shared<MemoryInputStream>(memPtr, memSize);
            if (musicPtr->openFromStream(*stream)) {
                std::lock_guard<std::mutex> lock(musicMutex);
                musicStreams[name] = std::move(stream);
                loaded = true;
            }
        }
    }
    
    if (!loaded && musicPtr->openFromFile(path)) {
        loaded = true;
    } else if (!loaded) {
        std::string exeDirPath = executableDir + "/" + path;
        if (musicPtr->openFromFile(exeDirPath)) {
            loaded = true;
        } else {
            std::string parentPath = executableDir + "/../" + path;
            if (musicPtr->openFromFile(parentPath)) {
                loaded = true;
            } else {
                std::string projectPath = executableDir + "/../../defen_synth(pvz_like)/" + path;
                if (musicPtr->openFromFile(projectPath)) {
                    loaded = true;
                }
            }
        }
    }
    
    if (loaded) {
        std::lock_guard<std::mutex> lock(musicMutex);
        music[name] = std::move(musicPtr);
        return true;
    }
    
    return false;
}

bool ResourceManager::loadFont(const std::string& path, const std::string& name) {
    if (fonts.find(name) != fonts.end()) {
        return true;
    }
    
    sf::Font font;
    const std::uint8_t* memPtr = nullptr;
    std::size_t memSize = 0;
    if (hasInBundle(path) && getFromBundle(path, memPtr, memSize)) {
        if (font.loadFromMemory(memPtr, memSize)) {
            fonts[name] = font;
            return true;
        }
    }

    if (font.loadFromFile(path)) {
        fonts[name] = font;
        return true;
    }
    
    std::string exeDirPath = executableDir + "/" + path;
    if (font.loadFromFile(exeDirPath)) {
        fonts[name] = font;
        return true;
    }
    
    std::string parentPath = executableDir + "/../" + path;
    if (font.loadFromFile(parentPath)) {
        fonts[name] = font;
        return true;
    }
    
    std::string projectPath = executableDir + "/../../defen_synth(pvz_like)/" + path;
    if (font.loadFromFile(projectPath)) {
        fonts[name] = font;
        return true;
    }
    
    return false;
}

bool ResourceManager::loadBundle(const std::string& path) {
    bundleData.clear();
    bundleIndex.clear();
    bundleReady = false;

    std::ifstream f(path, std::ios::binary);
    if (!f) return false;

    f.seekg(0, std::ios::end);
    std::streamoff size = f.tellg();
    if (size <= 0) return false;
    f.seekg(0, std::ios::beg);

    bundleData.resize(static_cast<std::size_t>(size));
    if (!f.read(reinterpret_cast<char*>(bundleData.data()), size)) return false;

    bundleReady = true;
    return true;
}

bool ResourceManager::hasInBundle(const std::string& path) const {
    return bundleIndex.find(path) != bundleIndex.end();
}

bool ResourceManager::getFromBundle(const std::string& path, const std::uint8_t*& ptr, std::size_t& size) const {
    auto it = bundleIndex.find(path);
    if (it == bundleIndex.end()) return false;
    ptr = bundleData.data() + it->second.first;
    size = it->second.second;
    return true;
}

}
