
#ifndef GBEMU_SRC_APP_LOGWINDOW_H_
#define GBEMU_SRC_APP_LOGWINDOW_H_

#include <imgui.h>
#include <string>
#include <cstdint>


// copied and modified from the example in imgui_demo.cpp

// Usage:
//  static ExampleAppLog my_log;
//  my_log.AddLog("Hello %d world\n", 123);
//  my_log.Draw("title");
class SerialLogWindow {
public:

    SerialLogWindow()
    {
        mAutoScroll = true;
        mRawOutput = false;
        Clear();
    }

    void Clear()
    {
        mBuf.clear();
        mLineOffsets.clear();
        mLineOffsets.push_back(0);
    }

    void AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        int old_size = mBuf.size();
        va_list args;
        va_start(args, fmt);
        mBuf.appendfv(fmt, args);
        va_end(args);
        for (int new_size = mBuf.size(); old_size < new_size; old_size++)
            if (mBuf[old_size] == '\n')
                mLineOffsets.push_back(old_size + 1);
    }

    void Draw(const char* title, bool* p_open = NULL)
    {
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }

        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("Auto-scroll", &mAutoScroll);
            ImGui::EndPopup();
        }

        // Main window
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("Options");
        ImGui::SameLine();
        bool clear = ImGui::Button("Clear");
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        mFilter.Draw("Filter", -100.0f);
        ImGui::SameLine();
        ImGui::Checkbox("Raw", &mRawOutput);

        ImGui::Separator();

        if (ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (clear)
                Clear();
            if (copy)
                ImGui::LogToClipboard();

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            const char* buf = mBuf.begin();
            const char* buf_end = mBuf.end();
            if (mFilter.IsActive())
            {
                // In this example we don't use the clipper when mFilter is enabled.
                // This is because we don't have random access to the result of our filter.
                // A real application processing logs with ten of thousands of entries may want to store the result of
                // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
                for (int line_no = 0; line_no < mLineOffsets.Size; line_no++)
                {
                    const char* line_start = buf + mLineOffsets[line_no];
                    const char* line_end = (line_no + 1 < mLineOffsets.Size) ? (buf + mLineOffsets[line_no + 1] - 1) : buf_end;
                    if (mFilter.PassFilter(line_start, line_end))
                        ImGui::TextUnformatted(line_start, line_end);
                }
            }
            else
            {
                // The simplest and easy way to display the entire buffer:
                //   ImGui::TextUnformatted(buf_begin, buf_end);
                // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
                // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
                // within the visible area.
                // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
                // on your side is recommended. Using ImGuiListClipper requires
                // - A) random access into your data
                // - B) items all being the  same height,
                // both of which we can handle since we have an array pointing to the beginning of each line of text.
                // When using the filter (in the block of code above) we don't have random access into the data to display
                // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
                // it possible (and would be recommended if you want to search through tens of thousands of entries).
                ImGuiListClipper clipper;
                clipper.Begin(mLineOffsets.Size);
                while (clipper.Step())
                {
                    for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                    {
                        const char* line_start = buf + mLineOffsets[line_no];
                        const char* line_end = (line_no + 1 < mLineOffsets.Size) ? (buf + mLineOffsets[line_no + 1] - 1) : buf_end;
                        ImGui::TextUnformatted(line_start, line_end);
                    }
                }
                clipper.End();
            }
            ImGui::PopStyleVar();

            // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
            // Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (mAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();
        ImGui::End();
    }

    void OnSerialData(uint8_t data)
    {
        if (mRawOutput) {
            if (!mSerialBuf.empty()) {
                AddLog("[%1.f] - %s\n", ImGui::GetTime(), mSerialBuf.c_str());
                mSerialBuf.clear();
            }

            AddLog("[%1.f] - 0x%02x\n", ImGui::GetTime(), data);
        }
        else {
            if (data == '\n') {
                AddLog("[%1.f] - %s\n", ImGui::GetTime(), mSerialBuf.c_str());
                mSerialBuf.clear();
            }
            else {
                mSerialBuf.push_back(data);
            }
        }
    }


private:
    ImGuiTextBuffer mBuf;
    ImGuiTextFilter mFilter;
    ImVector<int> mLineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
    bool mAutoScroll;  // Keep scrolling if already at the bottom.

    std::string mSerialBuf;
    bool mRawOutput;
};


#endif // GBEMU_SRC_APP_LOGWINDOW_H_