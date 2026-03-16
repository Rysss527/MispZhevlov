#pragma once
#include <SFML/Audio.hpp>
#include <map>
#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <random>
#include "ResourceManager.h"

class SoundManager {
private:
	static std::unique_ptr<SoundManager> instance;
	static std::once_flag initFlag;
	
	mutable std::mutex soundMutex;
	std::map<std::string, sf::SoundBuffer> soundBuffers;
	std::map<std::string, sf::Sound> sounds;
	sf::Music music;
        std::vector<std::string> playlist;
        size_t currentTrackIdx = 0;
        std::string currentTrackName;
        bool shufflePlaylist = false;

	SoundManager() {} 

public:
	static SoundManager& getInstance()
	{
		std::call_once(initFlag, []() {
			instance = std::unique_ptr<SoundManager>(new SoundManager());
		});
		return *instance;
	}
	
	SoundManager(const SoundManager&) = delete;
	SoundManager& operator=(const SoundManager&) = delete;
	SoundManager(SoundManager&&) = delete;
	SoundManager& operator=(SoundManager&&) = delete;

	void loadSound(const std::string& name, const std::string& path)
	{
		std::lock_guard<std::mutex> lock(soundMutex);
		auto& rm = DefenseSynth::ResourceManager::getInstance();
		if (rm.loadSound("assets/audio/" + path, name)) {
			soundBuffers[name] = rm.getSound(name);
			sounds[name].setBuffer(soundBuffers[name]);
		}
	}

	void playSound(const std::string& name)
	{
		std::unique_lock<std::mutex> lock(soundMutex);
		if (soundBuffers.find(name) == soundBuffers.end())
		{
			lock.unlock();
			loadSound(name, name);
			lock.lock();
		}
		if (sounds.find(name) != sounds.end())
		{
			sounds[name].play();
		}
	}
	
	void initialize()
	{
		loadSound("splat1.wav", "assets/audio/splat1.wav");
		loadSound("splat2.wav", "assets/audio/splat2.wav");
		loadSound("splat3.wav", "assets/audio/splat3.wav");
		loadSound("plant1.wav", "assets/audio/plant1.wav");
		loadSound("plant2.wav", "assets/audio/plant2.wav");
		loadSound("points.wav", "assets/audio/points.wav");
		loadSound("shovel.wav", "assets/audio/shovel.wav");
		loadSound("lawnmower.wav", "assets/audio/lawnmower.wav");
		loadSound("buttonclick.wav", "assets/audio/buttonclick.wav");
		loadSound("groan1.wav", "assets/audio/groan1.wav");
		loadSound("groan2.wav", "assets/audio/groan2.wav");
		loadSound("groan3.wav", "assets/audio/groan3.wav");
		loadSound("seedlift.wav", "assets/audio/seedlift.wav");
		loadSound("frozen.wav", "assets/audio/frozen.wav");
		loadSound("cherrybomb.wav", "assets/audio/cherrybomb.wav");
		loadSound("explosion.wav", "assets/audio/explosion.wav");
		loadSound("pause.wav", "assets/audio/pause.wav");
		loadSound("scream.wav", "assets/audio/scream.wav");
		loadSound("losemusic.wav", "assets/audio/losemusic.wav");
		loadSound("winmusic.wav", "assets/audio/winmusic.wav");
		loadSound("fume.wav", "assets/audio/fume.wav");
		loadSound("plastichit.wav", "assets/audio/plastichit.wav");
		loadSound("dancer.wav", "assets/audio/dancer.wav");
		loadSound("gargantuar_thump.wav", "assets/audio/gargantuar_thump.wav");
	}

    void setPlaylist(const std::vector<std::string>& tracks, bool shuffle = false) {
        playlist = tracks;
        currentTrackIdx = 0;
        shufflePlaylist = shuffle;
        if (!playlist.empty()) {
            if (shufflePlaylist && playlist.size() > 1) {
                static thread_local std::mt19937 rng{std::random_device{}()};
                std::uniform_int_distribution<size_t> dist(0, playlist.size() - 1);
                currentTrackIdx = dist(rng);
            }
            playMusicInternal(playlist[currentTrackIdx]);
        }
    }

    void updateMusic();

private:
    void playMusicInternal(const std::string& path);
public:
    void playMusic(const std::string& path, bool loop = true)
    {
        auto& rm = DefenseSynth::ResourceManager::getInstance();
        stopMusic();
        if (!rm.loadMusic("assets/audio/" + path, path)) return;
        if (auto* m = rm.getMusic(path)) {
            m->setLoop(loop);
            m->play();
            currentTrackName = path;
        }
    }

    bool isMusicFinished() const {
        auto* m = DefenseSynth::ResourceManager::getInstance().getMusic(currentTrackName);
        if (!m) return true;
        return m->getStatus() != sf::Music::Playing;
    }

    void stopMusic()
    {
        if (auto* m = DefenseSynth::ResourceManager::getInstance().getMusic(currentTrackName)) {
            m->stop();
        }
    }

    std::string getCurrentTrack() const {
        return currentTrackName;
    }

    float getCurrentOffsetSeconds() const {
        auto* m = DefenseSynth::ResourceManager::getInstance().getMusic(currentTrackName);
        if (!m) return 0.0f;
        return m->getPlayingOffset().asSeconds();
    }

    void setPlaylistStartingAt(const std::vector<std::string>& tracks, const std::string& startTrack, float offsetSeconds);

    void loopSound(const std::string& name, bool loop)
    {
        std::unique_lock<std::mutex> lock(soundMutex);
        if (soundBuffers.find(name) == soundBuffers.end())
        {
            lock.unlock();
            loadSound(name, name);
            lock.lock();
        }
        auto it = sounds.find(name);
        if (it == sounds.end()) return;
        if (name == "dancer.wav") {
            it->second.setVolume(70.0f);
        }
        it->second.setLoop(loop);
        if (loop) {
            if (it->second.getStatus() != sf::Sound::Playing) {
                it->second.play();
            }
        } else {
            it->second.stop();
        }
    }

    bool isSoundPlaying(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(soundMutex);
        auto it = sounds.find(name);
        if (it == sounds.end()) return false;
        return it->second.getStatus() == sf::Sound::Playing;
    }

    void stopSound(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(soundMutex);
        auto it = sounds.find(name);
        if (it == sounds.end()) return;
        it->second.stop();
    }
};
