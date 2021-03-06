// Copyright 2015-2019 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_UTILS_LOG_HPP
#define OUZEL_UTILS_LOG_HPP

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>
#include "math/Matrix.hpp"
#include "math/Quaternion.hpp"
#include "math/Size.hpp"
#include "math/Vector.hpp"

namespace ouzel
{
    class Logger;

    class Log final
    {
    public:
        enum class Level
        {
            OFF,
            ERR,
            WARN,
            INFO,
            ALL
        };

        explicit Log(const Logger& initLogger, Level initLevel = Level::INFO):
            logger(initLogger), level(initLevel)
        {
        }

        Log(const Log& other):
            logger(other.logger),
            level(other.level),
            s(other.s)
        {
        }

        Log(Log&& other):
            logger(other.logger),
            level(other.level),
            s(std::move(other.s))
        {
            other.level = Level::INFO;
        }

        Log& operator=(const Log& other)
        {
            level = other.level;
            s = other.s;

            return *this;
        }

        Log& operator=(Log&& other)
        {
            if (&other != this)
            {
                level = other.level;
                other.level = Level::INFO;
                s = std::move(other.s);
            }

            return *this;
        }

        ~Log();

        template<typename T, typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value>::type* = nullptr>
        Log& operator<<(T val)
        {
            s += std::to_string(val);
            return *this;
        }

        template<typename T, typename std::enable_if<std::is_same<T, bool>::value>::type* = nullptr>
        Log& operator<<(T val)
        {
            s += val ? "true" : "false";
            return *this;
        }

        template<typename T, typename std::enable_if<std::is_same<T, std::string>::value>::type* = nullptr>
        Log& operator<<(const T& val)
        {
            s += val;
            return *this;
        }

        template<typename T, typename std::enable_if<std::is_same<T, char>::value>::type* = nullptr>
        Log& operator<<(const T* val)
        {
            s += val;
            return *this;
        }

        template<typename T, typename std::enable_if<!std::is_same<T, char>::value>::type* = nullptr>
        Log& operator<<(const T* val)
        {
            static constexpr const char* digits = "0123456789ABCDEF";

            std::string str(sizeof(val) * 2, '0');
            for (size_t i = 0; i < sizeof(val) * 2; ++i)
                str[i] = digits[(reinterpret_cast<uintptr_t>(val) >> (sizeof(val) * 2 - i - 1) * 4) & 0x0f];

            s += str;
            return *this;
        }

        template<typename T, typename std::enable_if<std::is_same<T, std::vector<uint8_t>>::value>::type* = nullptr>
        Log& operator<<(const T& val)
        {
            bool first = true;

            for (uint8_t b : val)
            {
                if (!first) s += ", ";
                first = false;

                static constexpr const char* digits = "0123456789ABCDEF";
                s += digits[(b >> 4) & 0x0f];
                s += digits[b & 0x0f];
            }

            return *this;
        }

        template<typename T, typename std::enable_if<std::is_same<T, std::vector<std::string>>::value>::type* = nullptr>
        Log& operator<<(const T& val)
        {
            bool first = true;

            for (const std::string& str : val)
            {
                if (!first) s += ", ";
                first = false;
                s += str;
            }

            return *this;
        }

        template<size_t N, size_t M, class T>
        Log& operator<<(const Matrix<N, M, T>& val)
        {
            bool first = true;

            for (T c : val.m)
            {
                if (!first) s += ",";
                first = false;
                s += std::to_string(c);
            }

            return *this;
        }

        template<class T>
        Log& operator<<(const Quaternion<T>& val)
        {
            s += std::to_string(val.v[0]) + "," + std::to_string(val.v[1]) + "," +
                std::to_string(val.v[2]) + "," + std::to_string(val.v[3]);
            return *this;
        }

        template<size_t N, class T>
        Log& operator<<(const Size<N, T>& val)
        {
            bool first = true;

            for (T c : val.v)
            {
                if (!first) s += ",";
                first = false;
                s += std::to_string(c);
            }
            return *this;
        }

        template<size_t N, class T>
        Log& operator<<(const Vector<N, T>& val)
        {
            bool first = true;

            for (T c : val.v)
            {
                if (!first) s += ",";
                first = false;
                s += std::to_string(c);
            }
            return *this;
        }

    private:
        const Logger& logger;
        Level level = Level::INFO;
        std::string s;
    };

    class Logger final
    {
    public:
        explicit Logger(Log::Level initThreshold = Log::Level::ALL):
            threshold(initThreshold)
        {
#if !defined(__EMSCRIPTEN__)
            logThread = std::thread(&Logger::logLoop, this);
#endif
        }

        ~Logger()
        {
#if !defined(__EMSCRIPTEN__)
            std::unique_lock<std::mutex> lock(queueMutex);
            running = false;
            lock.unlock();
            logCondition.notify_all();
            if (logThread.joinable()) logThread.join();
#endif
        }

        Log log(Log::Level level = Log::Level::INFO) const
        {
            return Log(*this, level);
        }

        void log(const std::string& str, Log::Level level = Log::Level::INFO) const
        {
            if (level <= threshold)
            {
#if defined(__EMSCRIPTEN__)
                logString(str, level);
#else
                std::unique_lock<std::mutex> lock(queueMutex);
                logQueue.push(std::make_pair(level, str));
                lock.unlock();
                logCondition.notify_all();
#endif
            }
        }

    private:
        static void logString(const std::string& str, Log::Level level = Log::Level::INFO);

#ifdef DEBUG
        std::atomic<Log::Level> threshold{Log::Level::ALL};
#else
        std::atomic<Log::Level> threshold{Log::Level::INFO};
#endif

#if !defined(__EMSCRIPTEN__)
        void logLoop()
        {
            for (;;)
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                while (running && logQueue.empty()) logCondition.wait(lock);
                if (!running) break;
                std::pair<Log::Level, std::string> str = std::move(logQueue.front());
                logQueue.pop();
                lock.unlock();

                logString(str.second, str.first);
            }
        }

        mutable std::condition_variable logCondition;
        mutable std::mutex queueMutex;
        mutable std::queue<std::pair<Log::Level, std::string>> logQueue;
        std::thread logThread;
        bool running = true;
#endif
    };
}

#endif // OUZEL_UTILS_LOG_HPP
