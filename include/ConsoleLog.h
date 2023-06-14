//
// Created by NePutin on 6/30/2021.
//

#ifndef PALKA_CONSOLELOG_H
#define PALKA_CONSOLELOG_H

#include <imgui.h>
#include <string>
#include <iostream>
#include <vector>
#include <shared_mutex>
#include <fmt/core.h>

namespace palka
{
    namespace
    {
        constexpr std::string_view logType_s[] =
                {
                        "all", "error", "info", "fatal", "system", "script", "message"
                };


    }

    class Console
    {
    public:
        struct skip_unique_check
        {
            bool skip;
        };
        enum logType
        {
            error = 1,
            info,
            fatal,
            system,
            script,
            message
        };

        struct Log
        {
            Log(std::string s, logType t);

            std::string text;
            std::string pervText;

            void count_update(int count);

            int log_count;
            ImVec4 color;
            logType type;
        };

        static void Clear();

        static bool hasNewLog();

        static bool hasNewLogByTyp(logType t);

        static void addLog(Log log, skip_unique_check skip = {false});

        static void addLog(std::string s, logType t);

        // static void addLog_(std::string s, logType t, ...);

        template<typename... Args>
        static void fmt_log(const std::string& rt_fmt_str, logType type = info, Args&& ... args)
        {
            addLog(Log(vformat(rt_fmt_str, fmt::make_format_args(args...)), type));
        }

        template<typename... Args>
        static void fmt_log_skip_uniq(const std::string& rt_fmt_str, logType type = info, Args&& ... args)
        {
            addLog(Log(vformat(rt_fmt_str, fmt::make_format_args(args...)), type), skip_unique_check{true});
        }

        static void saveLog(std::string_view path);

        static void Draw(const char* title, bool* p_open);

        static std::string lastLog();

    private:
        static bool ScrollToBottom;
        static std::vector<std::string> current_input;
        static std::vector<Log> Buffer;
        static bool newLog;
        static size_t offset;
        static std::shared_mutex globalMutex;
    };

}

#endif //PALKA_CONSOLELOG_H
