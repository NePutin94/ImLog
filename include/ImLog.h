//
// Created by NePutin on 6/14/2023.
//

#ifndef IMLOG_IMLOG_H
#define IMLOG_IMLOG_H

#include <utility>
#include <vector>
#include <experimental/string>
#include <imgui.h>
#include <map>
#include <fmt/core.h>

enum ImLogType
{
    info = 1,
    error,
    fatal,
    script,
    message
};

namespace util
{
    struct LogHelper
    {
        inline static std::map<ImLogType, ImColor> colors = {{ImLogType::info,    ImColor(255, 0, 255, 255)},
                                                             {ImLogType::error,   ImColor(255, 0, 255, 255)},
                                                             {ImLogType::fatal,   ImColor(255, 0, 255, 255)},
                                                             {ImLogType::script,  ImColor(255, 0, 255, 255)},
                                                             {ImLogType::message, ImColor(255, 0, 255, 255)}};

        constexpr static std::string_view logType[] =
                {
                        "all", "info", "error", "fatal", "script", "message"
                };
    };
}
struct Log
{
    Log(std::string txt, ImLogType type = ImLogType::info) : log(std::move(txt)), type(type)
    {
        log_head = util::LogHelper::logType[type];
    }

    ImLogType type;
    std::string log_head;
    std::string log;
};

class ImLog
{
public:
    static void log(Log&& log)
    {
        buf.emplace_back(log);
    }

    static void log(Log& log)
    {
        buf.push_back(log);
    }

    static void log(const std::string& txt, ImLogType type = info)
    {
        buf.emplace_back(txt, type);
    }

    template<typename... Args>
    static void fmt_log(const std::string& rt_fmt_str, ImLogType type, Args&& ... args)
    {
        log(Log(vformat(rt_fmt_str, fmt::make_format_args(args...)), type));
    }

    static void draw(bool p_open)
    {
        if(p_open)
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always, ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y * 0.3));
            ImGui::SetNextWindowBgAlpha(0.8f);
            ImGui::Begin("ImLog", &p_open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                                           ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
            // if(ImGui::Button("Clear")) Clear();
            ImGui::SameLine();
            bool copy = ImGui::Button("Copy");
            ImGui::SameLine();
            static bool find = false;
            static int itemCount = 0;
            ImGui::Checkbox("Find By Type", &find);
            static std::string item_current = util::LogHelper::logType[0].data();
            ImGui::PushItemWidth(45);
            if(ImGui::BeginCombo("__", item_current.c_str(), ImGuiComboFlags_NoArrowButton))
            {
                for(auto& item: util::LogHelper::logType)
                {
                    bool is_selected = (item_current == item);
                    if(ImGui::Selectable(item.data(), is_selected))
                        item_current = item;
                    if(is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            ImGui::SameLine();
            char buff_search[256] = {0};
            char buff_input[256] = {0};

            ImGui::PushItemWidth(150);
            bool value_changed = ImGui::InputText("Filter", buff_search, IM_ARRAYSIZE(buff_search));
            ImGui::PopItemWidth();
            ImGui::PushItemWidth(300);
            bool input_change = ImGui::InputText("Input", buff_input, IM_ARRAYSIZE(buff_input), ImGuiInputTextFlags_EnterReturnsTrue);
            std::string last_result;
            if(input_change)
            {
                //ScrollToBottom = true;
                if(buff_input[0] == '/')
                {

                }
            }
            ImGui::PopItemWidth();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
            if(copy) ImGui::LogToClipboard();
            for(const auto& item: buf)
                ImGui::Text("%s", item.log.c_str());

            //   if(ScrollToBottom)
            //       ImGui::SetScrollHereY(1.f);
            //  ScrollToBottom = false;
            ImGui::EndChild();
            ImGui::End();
        }
    }

private:
    inline static std::vector<Log> buf = {};
};

#endif //IMLOG_IMLOG_H
