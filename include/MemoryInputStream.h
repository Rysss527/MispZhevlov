#pragma once

#include <SFML/System/InputStream.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace DefenseSynth {

class MemoryInputStream : public sf::InputStream {
private:
    const std::uint8_t* base = nullptr;
    std::size_t length = 0;
    std::size_t pos = 0;
public:
    MemoryInputStream() = default;
    MemoryInputStream(const std::uint8_t* data, std::size_t size) { open(data, size); }
    void open(const std::uint8_t* data, std::size_t size) { base = data; length = size; pos = 0; }
    virtual ~MemoryInputStream() = default;

    sf::Int64 read(void* data, sf::Int64 size) override {
        if (!base || size <= 0) return -1;
        std::size_t avail = length - pos;
        std::size_t toRead = static_cast<std::size_t>(size);
        if (toRead > avail) toRead = avail;
        if (toRead == 0) return 0;
        std::memcpy(data, base + pos, toRead);
        pos += toRead;
        return static_cast<sf::Int64>(toRead);
    }

    sf::Int64 seek(sf::Int64 position) override {
        if (position < 0) return pos;
        std::size_t p = static_cast<std::size_t>(position);
        if (p > length) p = length;
        pos = p;
        return static_cast<sf::Int64>(pos);
    }

    sf::Int64 tell() override {
        return static_cast<sf::Int64>(pos);
    }

    sf::Int64 getSize() override {
        return static_cast<sf::Int64>(length);
    }
};

}
