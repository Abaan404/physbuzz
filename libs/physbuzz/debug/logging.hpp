#pragma once

#include <cstdlib>
#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace Physbuzz {

class Logger {
  public:
    static void init() {
        m_Console->set_level(spdlog::level::debug);
        m_Console->set_pattern("[%H:%M:%S] %^[%g%l] %v%$");
    }

    template <typename... Args>
    static inline void DEBUG(spdlog::format_string_t<Args...> fmt, Args &&...args) {
        static std::shared_ptr<spdlog::logger> logger = spdlog::get(m_Active);
        logger->debug(fmt, std::forward<Args>(args)...);
    }

    template <typename T>
    static inline void DEBUG(const T &msg) {
        static std::shared_ptr<spdlog::logger> logger = spdlog::get(m_Active);
        logger->debug(msg);
    }

    template <typename... Args>
    static inline void INFO(spdlog::format_string_t<Args...> fmt, Args &&...args) {
        static std::shared_ptr<spdlog::logger> logger = spdlog::get(m_Active);
        logger->info(fmt, std::forward<Args>(args)...);
    }

    template <typename T>
    static inline void INFO(const T &msg) {
        static std::shared_ptr<spdlog::logger> logger = spdlog::get(m_Active);
        logger->info(msg);
    }

    template <typename... Args>
    static inline void WARNING(spdlog::format_string_t<Args...> fmt, Args &&...args) {
        static std::shared_ptr<spdlog::logger> logger = spdlog::get(m_Active);
        logger->warn(fmt, std::forward<Args>(args)...);
    }

    template <typename T>
    static inline void WARNING(const T &msg) {
        static std::shared_ptr<spdlog::logger> logger = spdlog::get(m_Active);
        logger->warn(msg);
    }

    template <typename... Args>
    static inline void ERROR(spdlog::format_string_t<Args...> fmt, Args &&...args) {
        static std::shared_ptr<spdlog::logger> logger = spdlog::get(m_Active);
        logger->error(fmt, std::forward<Args>(args)...);
    }

    template <typename T>
    static inline void ERROR(const T &msg) {
        static std::shared_ptr<spdlog::logger> logger = spdlog::get(m_Active);
        logger->error(msg);
    }

    template <typename... Args>
    static inline void CRITICAL(spdlog::format_string_t<Args...> fmt, Args &&...args) {
        static std::shared_ptr<spdlog::logger> logger = spdlog::get(m_Active);
        logger->critical(fmt, std::forward<Args>(args)...);
        std::quick_exit(1);
    }

    template <typename T>
    static inline void CRITICAL(const T &msg) {
        static std::shared_ptr<spdlog::logger> logger = spdlog::get(m_Active);
        logger->critical(msg);
        std::quick_exit(1);
    }

  private:
    inline static std::string m_Active = "color";
    inline static std::shared_ptr<spdlog::logger> m_Console = spdlog::stdout_color_mt("color");
};

} // namespace Physbuzz
