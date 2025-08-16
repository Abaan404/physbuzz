#pragma once

#include "../debug/logging.hpp"

#define PBZ_GET_MACRO(_1, _2, NAME, ...) NAME

#if defined(_MSC_VER)
#define BREAKPOINT() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
#define BREAKPOINT() __builtin_trap()
#else
#include <csignal>
#define BREAKPOINT() std::raise(SIGTRAP)
#endif

#if defined(NDEBUG)
#define PBZ_ASSERT
#define PBZ_UNREACHABLE

#define PBZ_VK_CHECK(...) PBZ_GET_MACRO(__VA_ARGS__, PBZ_VK_CHECK_IMPL, PBZ_VK_CHECK_IMPL_DEFAULT)(__VA_ARGS__)

#define PBZ_VK_CHECK_IMPL_DEFAULT(resval) PBZ_VK_CHECK_IMPL(resval, "")
#define PBZ_VK_CHECK_IMPL(resval, msg)                                                                       \
    ([&]() {                                                                                                 \
        auto [result, value] = (resval);                                                                     \
        if (result != ::vk::Result::eSuccess) {                                                              \
            ::Physbuzz::Logger::WARNING("[vk_check] {} -> {} (in {}:{})", msg, #resval, __FILE__, __LINE__); \
        }                                                                                                    \
        return value;                                                                                        \
    }())

#else

#define PBZ_ASSERT(condition, message)                                                                          \
    do {                                                                                                        \
        if (!(condition)) {                                                                                     \
            ::Physbuzz::Logger::ERROR("[assert] {} -> {} (in {}:{})", #condition, message, __FILE__, __LINE__); \
            BREAKPOINT();                                                                                       \
        }                                                                                                       \
    } while (false)

#define PBZ_UNREACHABLE(message)                                                               \
    do {                                                                                       \
        ::Physbuzz::Logger::ERROR("[unreachable] {} (in {}:{})", message, __FILE__, __LINE__); \
        __builtin_unreachable();                                                               \
    } while (false)

#define PBZ_VK_CHECK(...) PBZ_GET_MACRO(__VA_ARGS__, PBZ_VK_CHECK_IMPL, PBZ_VK_CHECK_IMPL_DEFAULT)(__VA_ARGS__)

#define PBZ_VK_CHECK_IMPL_DEFAULT(resval) PBZ_VK_CHECK_IMPL(resval, "")
#define PBZ_VK_CHECK_IMPL(resval, msg)                                                                                                    \
    ([&]() {                                                                                                                              \
        auto [result, value] = (resval);                                                                                                  \
        if (result != ::vk::Result::eSuccess) {                                                                                           \
            ::Physbuzz::Logger::CRITICAL("[vk_check] ({} -> {}) {} (in {}:{})", vk::to_string(result), msg, #resval, __FILE__, __LINE__); \
        }                                                                                                                                 \
        return value;                                                                                                                     \
    }())

#endif
