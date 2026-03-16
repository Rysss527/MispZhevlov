#include "ResourceCache.h"
#include <iostream>

std::unique_ptr<ResourceManager> ResourceManager::instance = nullptr;
std::once_flag ResourceManager::initFlag;