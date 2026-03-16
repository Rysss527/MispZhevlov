#pragma once
#include <queue>
#include <vector>
#include <memory>
#include <functional>

namespace DefenseSynth {

template<typename T>
class ObjectPool {
private:
    struct PooledObject {
        T object;
        bool inUse;
        
        PooledObject() : inUse(false) {}
    };
    
    std::vector<std::unique_ptr<PooledObject>> pool;
    std::queue<PooledObject*> available;
    size_t maxSize;
    size_t currentSize;
    std::function<void(T&)> resetFunc;
    std::function<void(T&)> initFunc;
    
public:
    ObjectPool(size_t initialSize = 100, size_t maxPoolSize = 1000)
        : maxSize(maxPoolSize), currentSize(0) {
        preAllocate(initialSize);
    }
    
    void setResetFunction(std::function<void(T&)> func) {
        resetFunc = func;
    }
    
    void setInitFunction(std::function<void(T&)> func) {
        initFunc = func;
    }
    
    T* acquire() {
        if (available.empty()) {
            if (currentSize < maxSize) {
                expandPool(std::min(size_t(10), maxSize - currentSize));
            } else {
                for (auto& obj : pool) {
                    if (!obj->inUse) {
                        obj->inUse = true;
                        if (resetFunc) {
                            resetFunc(obj->object);
                        }
                        return &obj->object;
                    }
                }
                return nullptr;
            }
        }
        
        PooledObject* obj = available.front();
        available.pop();
        obj->inUse = true;
        
        if (resetFunc) {
            resetFunc(obj->object);
        }
        
        return &obj->object;
    }
    
    void release(T* object) {
        if (!object) return;
        
        for (auto& obj : pool) {
            if (&obj->object == object) {
                obj->inUse = false;
                available.push(obj.get());
                break;
            }
        }
    }
    
    void preAllocate(size_t count) {
        expandPool(count);
    }
    
    size_t getAvailableCount() const {
        return available.size();
    }
    
    size_t getTotalSize() const {
        return currentSize;
    }
    
    size_t getInUseCount() const {
        return currentSize - available.size();
    }
    
    void clear() {
        while (!available.empty()) {
            available.pop();
        }
        
        for (auto& obj : pool) {
            if (!obj->inUse) {
                available.push(obj.get());
            }
        }
    }
    
    template<typename Func>
    void forEach(Func func) {
        for (auto& obj : pool) {
            if (obj->inUse) {
                func(obj->object);
            }
        }
    }
    
    template<typename Func>
    void forEachActive(Func func) {
        for (auto& obj : pool) {
            if (obj->inUse) {
                func(obj->object);
            }
        }
    }
    
    void releaseAll() {
        for (auto& obj : pool) {
            if (obj->inUse) {
                obj->inUse = false;
                available.push(obj.get());
            }
        }
    }
    
private:
    void expandPool(size_t count) {
        size_t actualCount = std::min(count, maxSize - currentSize);
        
        for (size_t i = 0; i < actualCount; ++i) {
            auto obj = std::make_unique<PooledObject>();
            
            if (initFunc) {
                initFunc(obj->object);
            }
            
            available.push(obj.get());
            pool.push_back(std::move(obj));
        }
        
        currentSize += actualCount;
    }
};

template<typename T>
class ThreadSafeObjectPool {
private:
    ObjectPool<T> pool;
    mutable std::mutex poolMutex;
    
public:
    ThreadSafeObjectPool(size_t initialSize = 100, size_t maxPoolSize = 1000)
        : pool(initialSize, maxPoolSize) {}
    
    T* acquire() {
        std::lock_guard<std::mutex> lock(poolMutex);
        return pool.acquire();
    }
    
    void release(T* object) {
        std::lock_guard<std::mutex> lock(poolMutex);
        pool.release(object);
    }
    
    void preAllocate(size_t count) {
        std::lock_guard<std::mutex> lock(poolMutex);
        pool.preAllocate(count);
    }
    
    size_t getAvailableCount() const {
        std::lock_guard<std::mutex> lock(poolMutex);
        return pool.getAvailableCount();
    }
    
    size_t getTotalSize() const {
        std::lock_guard<std::mutex> lock(poolMutex);
        return pool.getTotalSize();
    }
    
    void clear() {
        std::lock_guard<std::mutex> lock(poolMutex);
        pool.clear();
    }
};

}
