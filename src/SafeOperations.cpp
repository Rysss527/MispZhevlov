#include "SafeOperations.h"
#include <cstdarg>
#include <memory>
#include <cstdio>

namespace SafeOperations {
    
std::string safeFormat(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    va_list args_copy;
    va_copy(args_copy, args);
    int size = std::vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);
    
    if (size < 0) {
        va_end(args);
        throw std::runtime_error("String formatting error");
    }
    
    std::string result(size, '\0');
    std::vsnprintf(&result[0], size + 1, format, args);
    va_end(args);
    
    return result;
}

}
