#include "../include/ConsoleLog.h"

#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <mutex>
#include <utility>
#include <imgui.h>
using namespace palka;

std::vector<Console::Log>       Console::Buffer = {};
bool                       Console::ScrollToBottom = 0;
std::vector<std::string>   Console::current_input = {};
bool Console::newLog = false;
size_t Console::offset = 0;
std::shared_mutex Console::globalMutex;

bool palka::Console::hasNewLogByTyp(logType t)
{
    std::unique_lock<std::shared_mutex> lock{globalMutex};
    if(Buffer.empty())
        return false;
    bool prev = newLog;
    newLog = false;
    if(offset == 0) //if offset is zero, we look at the end of the vector
    {
        if(Buffer.back().type == t && prev)
        {
            offset = Buffer.size() - 1;
            //Just change the offset value from zero to something else, most likely there is more than one element in the vector
            return true;
        }
    } else
    {
        //We know the location of the previous log displayed on the screen,
        //just see if there are still logs that we might have missed (hasNewLogByTyp may not be called every application tick)
        if(auto it = std::find_if(Buffer.begin() + (offset + 1), Buffer.end(), [t](auto val)
            { return val.type == t; });
                it != Buffer.end())
        {
            offset = std::distance(Buffer.begin(), it);
            //if we find such a log(the first one we find), we calculate offset to it from the beginning of the vector
            return true;
        }
    }
    return false;
}

void palka::Console::addLog(Log log, skip_unique_check skip)
{
    std::unique_lock<std::shared_mutex> lock{globalMutex};
    newLog = true;
    if(!Buffer.empty())
    {
        if(skip.skip || Buffer.back().pervText != log.pervText)
            Buffer.emplace_back(log);
        else
        {
            newLog = false;
            offset = 0;
            //We added a copy of the log, so the next log to output will be exactly at the end of the vector
            Buffer.back().count_update(++Buffer.back().log_count);
        }
    } else
        Buffer.emplace_back(log);

}

//void Console::addLog_(std::string s, logType t, ...)
//{
//    va_list args;
//    const char* format = s.data();
//    char buffer[1024];
//    va_start (args, format);
//    int n = vsnprintf(buffer, 1024, format, args);
//    std::string str(buffer, n);
//    addLog(str, t);
//    va_end (args);
//}

void palka::Console::addLog(std::string s, logType t)
{
    addLog(Log(std::move(s), t));
}

void Console::saveLog(std::string_view path)
{
    std::ofstream out;
    out.open(path.data());
    if(out.is_open())
    {
        for(const auto& log: Buffer)
            out << log.text << "\n";
    }
}

void palka::Console::Log::count_update(int count)
{
    auto b = text.find_first_of(')');
    auto a = text.find_first_of('(');
    // auto offset = b - a - 1;
    text.erase(a + 1, b - a - 1);
    text.insert(a + 1, std::to_string(count));
}

void Console::Draw(const char* title, bool* p_open)
{
    if(*p_open)
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always, ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y * 0.3));
        ImGui::SetNextWindowBgAlpha(0.8f);
        ImGui::Begin(title, p_open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                                    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        if(ImGui::Button("Clear")) Clear();
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        static bool find = false;
        static int itemCount = 0;
        ImGui::Checkbox("Find By Type", &find);
        static std::string item_current = logType_s[0].data();
        ImGui::PushItemWidth(45);
        if(ImGui::BeginCombo("__", item_current.c_str(), ImGuiComboFlags_NoArrowButton))
        {
            for(auto& item: logType_s)
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
            ScrollToBottom = true;
            if(buff_input[0] == '/')
            {

            }
        }
        ImGui::PopItemWidth();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        if(copy) ImGui::LogToClipboard();
        if(value_changed)
        {
            std::string input = buff_search;
            for(auto item: Buffer)
            {
                if(item.text.find(input) != std::string::npos)
                {
                    ImGui::TextColored(item.color, item.text.c_str());
                }
//                std::string found = "";
//                std::size_t start_pos = -1;
//                start_pos = item.text.find_first_of(" ");
//                for(char i : input)
//                {
//                    std::size_t pos_begin = -1;
//
//                    pos_begin = item.text.find_first_of(i, start_pos);
//                    if(pos_begin != std::string::npos)
//                    {
//                        found += item.text.substr(pos_begin, 1);
//                    }
//                }
//                if(found == input)
//                {
//                    ImGui::TextColored(item.color, item.text.c_str());
//                }
            }
        } else if(find && item_current != "all")
        {
            for(const auto& item: Buffer)
            {
                if(logType_s[item.type].data() == item_current)
                    ImGui::TextColored(item.color, "%s", item.text.c_str());
            }
        } else
            for(const auto& i: Buffer)
                ImGui::TextColored(i.color, "%s", i.text.c_str());
        if(ScrollToBottom)
            ImGui::SetScrollHereY(1.f);
        ScrollToBottom = false;
        ImGui::EndChild();
        ImGui::End();
    }

}

bool Console::hasNewLog()
{
    bool prev = newLog;
    newLog = false;
    return prev;
}

std::string Console::lastLog()
{
    std::shared_lock<std::shared_mutex> lock{globalMutex};

    if(offset == 0)
        return Buffer.back().pervText;
    else
        return (Buffer.begin() + offset)->pervText;
}

void Console::Clear()
{
    Buffer.clear();
    Buffer.shrink_to_fit();
}

Console::Log::Log(std::string clearText, logType t)
{
    type = t;
    log_count = 1;
    double time = clock();
    std::string ti = std::to_string(std::round(time / 10) / 100);
    ti.erase(ti.find_first_of('.') + 3, ti.size());
    std::string FormattedText = "[sec:" + ti;
    switch(t)
    {
        case logType::error:
            color = ImVec4(1, 0.35f, 0, 1);
            FormattedText += " type:error";
            break;
        case logType::info:
            color = ImVec4(0, 1, 0.3f, 1);
            FormattedText += " type:info";
            break;
        case logType::fatal:
            color = ImVec4(1, 0, 0, 1);
            FormattedText += " type:fatal";
            break;
        case logType::system:
            color = ImVec4(1, 0, 0.8, 1);
            FormattedText += " type:system";
            break;
        case logType::script:
            color = ImVec4(0.1f, 0.5f, 0.1f, 1);
            FormattedText += " type:script";
            break;
        case logType::message:
            color = ImVec4(0.32f, 0.65f, 1.f, 1);
            FormattedText += " type:message";
            break;
    }
    FormattedText += " (1)]: ";
    FormattedText += clearText;
    FormattedText += " \n";
    text = FormattedText;
    pervText = clearText;
}
