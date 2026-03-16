#include "SoundManager.h"
#include "ResourceManager.h"
#include <random>

std::unique_ptr<SoundManager> SoundManager::instance = nullptr;
std::once_flag SoundManager::initFlag;

void SoundManager::updateMusic() {
    if (playlist.empty()) return;
    auto* m = DefenseSynth::ResourceManager::getInstance().getMusic(currentTrackName);
    if (!m) {
        playMusicInternal(playlist[currentTrackIdx]);
        return;
    }
    if (m->getStatus() != sf::Music::Playing) {
        if (shufflePlaylist) {
            if (playlist.size() == 1) {
                currentTrackIdx = 0;
            } else {
                static thread_local std::mt19937 rng{std::random_device{}()};
                std::uniform_int_distribution<size_t> dist(0, playlist.size() - 1);
                size_t nextIdx = currentTrackIdx;
                while (nextIdx == currentTrackIdx) {
                    nextIdx = dist(rng);
                }
                currentTrackIdx = nextIdx;
            }
        } else {
            currentTrackIdx = (currentTrackIdx + 1) % playlist.size();
        }
        playMusicInternal(playlist[currentTrackIdx]);
    }
}

void SoundManager::playMusicInternal(const std::string& path) {
    auto& rm = DefenseSynth::ResourceManager::getInstance();
    stopMusic();
    if (!rm.loadMusic("assets/audio/" + path, path)) return;
    if (auto* m = rm.getMusic(path)) {
        m->setLoop(false);
        m->play();
        currentTrackName = path;
    }
}

void SoundManager::setPlaylistStartingAt(const std::vector<std::string>& tracks, const std::string& startTrack, float offsetSeconds) {
    playlist = tracks;
    currentTrackIdx = 0;
    for (size_t i = 0; i < playlist.size(); ++i) {
        if (playlist[i] == startTrack) { currentTrackIdx = i; break; }
    }
    auto& rm = DefenseSynth::ResourceManager::getInstance();
    stopMusic();
    if (!playlist.empty()) {
        const std::string& path = playlist[currentTrackIdx];
        if (!rm.loadMusic("assets/audio/" + path, path)) return;
        if (auto* m = rm.getMusic(path)) {
            m->setLoop(false);
            if (offsetSeconds > 0.f) {
                m->setPlayingOffset(sf::seconds(offsetSeconds));
            }
            m->play();
            currentTrackName = path;
        }
    }
}
