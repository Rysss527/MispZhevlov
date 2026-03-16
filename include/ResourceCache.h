#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <chrono>
#include <vector>
#include <atomic>
#include <functional>

template<typename ResourceType>
class ResourceCache {
public:
    struct CacheEntry {
        std::shared_ptr<ResourceType> resource;
        mutable std::chrono::steady_clock::time_point lastAccessed;
        size_t accessCount;
        size_t memorySize;
        
        CacheEntry(std::shared_ptr<ResourceType> res, size_t size = 0) 
            : resource(std::move(res))
            , lastAccessed(std::chrono::steady_clock::now())
            , accessCount(1)
            , memorySize(size) {}
    };
    
private:
    mutable std::shared_mutex cacheMutex;
    std::unordered_map<std::string, CacheEntry> cache;
    
    size_t maxCacheSize;
    size_t currentCacheSize;
    std::atomic<size_t> hitCount{0};
    std::atomic<size_t> missCount{0};
    
    std::function<std::shared_ptr<ResourceType>(const std::string&)> resourceLoader;
    
    void evictLeastRecentlyUsed() {
        if (cache.empty()) return;
        
        auto oldestEntry = cache.begin();
        for (auto it = cache.begin(); it != cache.end(); ++it) {
            if (it->second.lastAccessed < oldestEntry->second.lastAccessed) {
                oldestEntry = it;
            }
        }
        
        currentCacheSize -= oldestEntry->second.memorySize;
        cache.erase(oldestEntry);
    }
    
protected:
    virtual size_t estimateResourceSize(const ResourceType& resource) const {
        return sizeof(ResourceType);
    }
    
private:
    
public:
    explicit ResourceCache(size_t maxSize = 50 * 1024 * 1024)
        : maxCacheSize(maxSize), currentCacheSize(0) {}
        
    ~ResourceCache() = default;
    
    ResourceCache(const ResourceCache&) = delete;
    ResourceCache& operator=(const ResourceCache&) = delete;
    ResourceCache(ResourceCache&&) = default;
    ResourceCache& operator=(ResourceCache&&) = default;
    
    void setResourceLoader(std::function<std::shared_ptr<ResourceType>(const std::string&)> loader) {
        std::unique_lock<std::shared_mutex> lock(cacheMutex);
        resourceLoader = std::move(loader);
    }
    
    std::shared_ptr<ResourceType> get(const std::string& resourcePath) {
        {
            std::shared_lock<std::shared_mutex> readLock(cacheMutex);
            auto it = cache.find(resourcePath);
            if (it != cache.end()) {
                it->second.lastAccessed = std::chrono::steady_clock::now();
                it->second.accessCount++;
                hitCount++;
                return it->second.resource;
            }
        }
        
        std::unique_lock<std::shared_mutex> writeLock(cacheMutex);
        
        auto it = cache.find(resourcePath);
        if (it != cache.end()) {
            it->second.lastAccessed = std::chrono::steady_clock::now();
            it->second.accessCount++;
            hitCount++;
            return it->second.resource;
        }
        
        missCount++;
        if (!resourceLoader) {
            return nullptr;
        }
        
        auto resource = resourceLoader(resourcePath);
        if (!resource) {
            return nullptr;
        }
        
        size_t resourceSize = estimateResourceSize(*resource);
        
        while (currentCacheSize + resourceSize > maxCacheSize && !cache.empty()) {
            evictLeastRecentlyUsed();
        }
        
        currentCacheSize += resourceSize;
        cache.emplace(resourcePath, CacheEntry(resource, resourceSize));
        
        return resource;
    }
    
    void preload(const std::vector<std::string>& resourcePaths) {
        for (const auto& path : resourcePaths) {
            get(path);
        }
    }
    
    void clear() {
        std::unique_lock<std::shared_mutex> lock(cacheMutex);
        cache.clear();
        currentCacheSize = 0;
    }
    
    void remove(const std::string& resourcePath) {
        std::unique_lock<std::shared_mutex> lock(cacheMutex);
        auto it = cache.find(resourcePath);
        if (it != cache.end()) {
            currentCacheSize -= it->second.memorySize;
            cache.erase(it);
        }
    }
    
    struct CacheStats {
        size_t hitCount;
        size_t missCount;
        size_t totalEntries;
        size_t currentMemoryUsage;
        size_t maxMemoryUsage;
        double hitRatio;
    };
    
    CacheStats getStats() const {
        std::shared_lock<std::shared_mutex> lock(cacheMutex);
        size_t hits = hitCount.load();
        size_t misses = missCount.load();
        return {
            hits,
            misses,
            cache.size(),
            currentCacheSize,
            maxCacheSize,
            (hits + misses > 0) ? static_cast<double>(hits) / (hits + misses) : 0.0
        };
    }
    
    void setMaxCacheSize(size_t newMaxSize) {
        std::unique_lock<std::shared_mutex> lock(cacheMutex);
        maxCacheSize = newMaxSize;
        
        while (currentCacheSize > maxCacheSize && !cache.empty()) {
            evictLeastRecentlyUsed();
        }
    }
};

class TextureCache : public ResourceCache<sf::Texture> {
private:
    static std::shared_ptr<sf::Texture> loadTexture(const std::string& path) {
        auto texture = std::make_shared<sf::Texture>();
        if (texture->loadFromFile("assets/images/" + path)) {
            return texture;
        }
        return nullptr;
    }
    
protected:
    size_t estimateResourceSize(const sf::Texture& texture) const override {
        return texture.getSize().x * texture.getSize().y * 4;
    }
    
public:
    TextureCache(size_t maxSize = 30 * 1024 * 1024) : ResourceCache<sf::Texture>(maxSize) {
        setResourceLoader(loadTexture);
    }
};

class SoundBufferCache : public ResourceCache<sf::SoundBuffer> {
private:
    static std::shared_ptr<sf::SoundBuffer> loadSoundBuffer(const std::string& path) {
        auto buffer = std::make_shared<sf::SoundBuffer>();
        if (buffer->loadFromFile("assets/audio/" + path)) {
            return buffer;
        }
        return nullptr;
    }
    
protected:
    size_t estimateResourceSize(const sf::SoundBuffer& buffer) const override {
        return buffer.getSampleCount() * buffer.getChannelCount() * sizeof(sf::Int16);
    }
    
public:
    SoundBufferCache(size_t maxSize = 20 * 1024 * 1024) : ResourceCache<sf::SoundBuffer>(maxSize) {
        setResourceLoader(loadSoundBuffer);
    }
};

class ResourceManager {
private:
    static std::unique_ptr<ResourceManager> instance;
    static std::once_flag initFlag;
    
    TextureCache textureCache;
    SoundBufferCache soundCache;
    
    ResourceManager() = default;
    
public:
    static ResourceManager& getInstance() {
        std::call_once(initFlag, []() {
            instance = std::unique_ptr<ResourceManager>(new ResourceManager());
        });
        return *instance;
    }
    
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;
    
    std::shared_ptr<sf::Texture> getTexture(const std::string& path) {
        return textureCache.get(path);
    }
    
    std::shared_ptr<sf::SoundBuffer> getSoundBuffer(const std::string& path) {
        return soundCache.get(path);
    }
    
    void preloadTextures(const std::vector<std::string>& paths) {
        textureCache.preload(paths);
    }
    
    void preloadSounds(const std::vector<std::string>& paths) {
        soundCache.preload(paths);
    }
    
    void clearAllCaches() {
        textureCache.clear();
        soundCache.clear();
    }
    
    struct ResourceStats {
        TextureCache::CacheStats textureStats;
        SoundBufferCache::CacheStats soundStats;
    };
    
    ResourceStats getResourceStats() const {
        return {textureCache.getStats(), soundCache.getStats()};
    }
};
