#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <limits>
#include <functional>
#include <SFML/Graphics.hpp>
#include "Constants.h"

namespace SafeOperations {
    
    template<size_t N>
    void safeCopy(char (&dest)[N], const std::string& src) {
        if (src.length() >= N) {
            throw std::invalid_argument("String too long for destination buffer");
        }
        std::copy(src.begin(), src.end(), dest);
        dest[src.length()] = '\0';
    }
    
    std::string safeFormat(const char* format, ...);
    
    template<typename T>
    T safeAdd(T a, T b) {
        if constexpr (std::is_signed_v<T>) {
            if ((b > 0 && a > std::numeric_limits<T>::max() - b) ||
                (b < 0 && a < std::numeric_limits<T>::min() - b)) {
                throw std::overflow_error("Addition overflow");
            }
        } else {
            if (a > std::numeric_limits<T>::max() - b) {
                throw std::overflow_error("Addition overflow");
            }
        }
        return a + b;
    }
    
    template<typename T>
    T safeMultiply(T a, T b) {
        if (a == 0 || b == 0) return 0;
        
        if constexpr (std::is_signed_v<T>) {
            if (a > std::numeric_limits<T>::max() / b ||
                a < std::numeric_limits<T>::min() / b) {
                throw std::overflow_error("Multiplication overflow");
            }
        } else {
            if (a > std::numeric_limits<T>::max() / b) {
                throw std::overflow_error("Multiplication overflow");
            }
        }
        return a * b;
    }
    
    template<typename T>
    T clamp(T value, T min, T max) {
        return std::clamp(value, min, max);
    }
    
    inline bool isValidArrayIndex(size_t index, size_t arraySize) {
        return index < arraySize;
    }
    
    inline bool isValidPosition(const sf::Vector2f& pos) {
        return pos.x >= 0 && pos.x <= GameConstants::BATTLEFIELD_WIDTH &&
               pos.y >= 0 && pos.y <= GameConstants::BATTLEFIELD_HEIGHT;
    }
    
    inline bool isValidGridPosition(int row, int col) {
        return row >= 0 && row < GameConstants::BATTLEFIELD_ROWS &&
               col >= 0 && col < GameConstants::BATTLEFIELD_COLS;
    }
    
    inline sf::Vector2f validatePosition(const sf::Vector2f& pos) {
        return sf::Vector2f(
            clamp(pos.x, 0.0f, static_cast<float>(GameConstants::BATTLEFIELD_WIDTH)),
            clamp(pos.y, 0.0f, static_cast<float>(GameConstants::BATTLEFIELD_HEIGHT))
        );
    }
    
    template<typename T>
    T safeDivide(T numerator, T denominator) {
        if (denominator == 0) {
            throw std::invalid_argument("Division by zero");
        }
        return numerator / denominator;
    }
    
    inline float safeSqrt(float value) {
        if (value < 0) {
            throw std::invalid_argument("Square root of negative number");
        }
        return std::sqrt(value);
    }
    
    inline float safeDistance(const sf::Vector2f& a, const sf::Vector2f& b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        
        if (std::abs(dx) > std::sqrt(std::numeric_limits<float>::max() / 2) ||
            std::abs(dy) > std::sqrt(std::numeric_limits<float>::max() / 2)) {
            throw std::overflow_error("Distance calculation overflow");
        }
        
        return safeSqrt(dx * dx + dy * dy);
    }
    
    inline bool isValidResourcePath(const std::string& path) {
        if (path.empty() || path.length() > 260) {
            return false;
        }
        
        const std::string dangerousChars = "<>:\"|?*";
        return path.find_first_of(dangerousChars) == std::string::npos &&
               path.find("..\\") == std::string::npos;
    }
    
    inline std::string sanitizeResourcePath(const std::string& path) {
        if (!isValidResourcePath(path)) {
            throw std::invalid_argument("Invalid resource path: " + path);
        }
        return path;
    }
    
    inline int validateHealth(int health) {
        return clamp(health, 0, 10000);
    }
    
    inline int validateDamage(int damage) {
        return clamp(damage, 0, 1000);
    }
    
    inline int validateLevel(int level) {
        return clamp(level, 1, 100);
    }
    
    inline int validateScore(int score) {
        return std::max(0, score);
    }
    
    template<typename Container, typename Index>
    auto safeAt(Container& container, Index index) -> decltype(container[index]) {
        if (index >= container.size()) {
            throw std::out_of_range("Index " + std::to_string(index) +
                                  " out of range [0, " + std::to_string(container.size()) + ")");
        }
        return container[index];
    }
    
    template<typename Container, typename Index>
    auto safeAt(const Container& container, Index index) -> decltype(container[index]) {
        if (index >= container.size()) {
            throw std::out_of_range("Index " + std::to_string(index) +
                                  " out of range [0, " + std::to_string(container.size()) + ")");
        }
        return container[index];
    }
    
    template<typename T>
    std::unique_ptr<T[]> safeAllocateArray(size_t count) {
        if (count == 0) {
            throw std::invalid_argument("Cannot allocate array of size 0");
        }
        
        if (count > std::numeric_limits<size_t>::max() / sizeof(T)) {
            throw std::overflow_error("Array allocation size overflow");
        }
        
        try {
            return std::make_unique<T[]>(count);
        } catch (const std::bad_alloc& e) {
            throw std::runtime_error("Failed to allocate memory for array of size " +
                                   std::to_string(count * sizeof(T)) + " bytes");
        }
    }
    
    inline bool isValidFileSize(size_t fileSize, size_t maxSize = 10 * 1024 * 1024) {
        return fileSize > 0 && fileSize <= maxSize;
    }
}

template<typename T>
class ResourceGuard {
private:
    T resource;
    std::function<void(T&)> cleanup;
    bool released = false;
    
public:
    ResourceGuard(T res, std::function<void(T&)> cleanupFunc) 
        : resource(std::move(res)), cleanup(std::move(cleanupFunc)) {}
    
    ~ResourceGuard() {
        if (!released && cleanup) {
            cleanup(resource);
        }
    }
    
    T& get() { return resource; }
    const T& get() const { return resource; }
    
    T release() {
        released = true;
        return std::move(resource);
    }
    
    ResourceGuard(const ResourceGuard&) = delete;
    ResourceGuard& operator=(const ResourceGuard&) = delete;
    ResourceGuard(ResourceGuard&&) = default;
    ResourceGuard& operator=(ResourceGuard&&) = default;
};
