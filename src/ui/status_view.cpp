#include "ui/status_view.h"
#include "ui/layout.h"
#include "ui/progress.h"
#include "ui/terminal.h"
#include "ui/text.h"
#include "ui/theme.h"
#include <algorithm>
#include <iostream>

namespace
{
    // Repaint cadence. The spinner advances on its own frame rate, but bytes
    // and ETA keep moving between frames.
    constexpr int PAINT_INTERVAL_MS = 40;
}

void LiveView::render(const std::vector<std::string> &lines)
{
    const int width = terminal_width();
    std::string out;

    if (painted_ > 0) out += "\033[" + std::to_string(painted_) + "A";
    out += "\r";

    for (const auto &l : lines)
    {
        out += "\033[2K";
        out += ui::clip(l, static_cast<size_t>(std::max(1, width - 1)));
        out += "\n";
    }

    const int extra = painted_ - static_cast<int>(lines.size());
    for (int i = 0; i < extra; ++i) out += "\033[2K\n";
    if (extra > 0) out += "\033[" + std::to_string(extra) + "A";

    std::cout << out << std::flush;
    painted_ = static_cast<int>(lines.size());
}

void LiveView::clear()
{
    if (painted_ == 0) return;

    std::string out = "\033[" + std::to_string(painted_) + "A\r";
    for (int i = 0; i < painted_; ++i) out += "\033[2K\n";
    out += "\033[" + std::to_string(painted_) + "A";

    std::cout << out << std::flush;
    painted_ = 0;
}

void LiveView::keep()
{
    painted_ = 0;
}

StatusView::StatusView(const SpinnerStyle &style, std::string status)
    : style_(&style),
      status_(std::move(status)),
      started_(std::chrono::steady_clock::now()),
      interactive_(stdout_is_tty())
{
    if (interactive_)
    {
        worker_ = std::thread(&StatusView::loop, this);
    }
    else
    {
        ui::task(status_);
    }
}

StatusView::~StatusView()
{
    stop();
}

void StatusView::set_status(std::string status)
{
    if (!interactive_) ui::task(status);

    std::lock_guard<std::mutex> lock(mutex_);
    status_ = std::move(status);
}

void StatusView::track_bytes(uint64_t total_bytes)
{
    std::lock_guard<std::mutex> lock(mutex_);
    tracking_ = true;
    total_bytes_ = total_bytes;
    transferred_ = 0;
    started_ = std::chrono::steady_clock::now();
}

void StatusView::set_bytes(uint64_t transferred)
{
    std::lock_guard<std::mutex> lock(mutex_);
    transferred_ = transferred;
}

void StatusView::stop()
{
    if (!running_.exchange(false)) return;

    if (worker_.joinable()) worker_.join();

    std::lock_guard<std::mutex> lock(mutex_);
    live_.clear();
}

void StatusView::finish(std::string summary)
{
    if (!running_.exchange(false)) return;

    if (worker_.joinable()) worker_.join();

    std::lock_guard<std::mutex> lock(mutex_);
    done_ = true;
    status_ = std::move(summary);

    const std::vector<std::string> lines = compose();

    if (interactive_)
    {
        live_.render(lines);
        live_.keep();
    }
    else
    {
        // Nothing was painted, so write the record out once.
        for (const auto &l : lines) std::cout << l << "\n";
    }
}

void StatusView::loop()
{
    auto last_frame = std::chrono::steady_clock::now();

    while (running_.load())
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            const auto now = std::chrono::steady_clock::now();
            const auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(now - last_frame).count();

            if (elapsed >= style_->interval_ms)
            {
                frame_ = (frame_ + 1) % style_->count;
                last_frame = now;
            }

            live_.render(compose());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(PAINT_INTERVAL_MS));
    }
}

// Caller holds mutex_.
std::vector<std::string> StatusView::compose() const
{
    std::vector<std::string> lines;

    if (done_)
        lines.push_back(ui::PAD + ui::success(status_));
    else
        lines.push_back(ui::PAD + col::ACCENT + style_->frames[frame_] + col::RESET + " " + status_);

    if (!tracking_) return lines;

    const double seconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - started_).count();

    // Left: how much of what has moved, and how fast.
    std::string moved = format_bytes(transferred_) + " / " + format_bytes(total_bytes_);
    if (transferred_ > 0 && seconds > 0.0)
    {
        const auto speed = static_cast<uint64_t>(transferred_ / seconds);
        if (speed > 0)
        {
            moved += "  ·  " + format_bytes(speed) + "/s";
            if (done_) moved += " average";
        }
    }

    // Right: time still to go, or time it took.
    std::string timing;
    if (done_)
    {
        const std::string spent = format_duration(seconds);
        timing = (spent.empty() ? "<1s" : spent) + " elapsed";
    }
    else if (transferred_ > 0 && seconds > 0.0)
    {
        const uint64_t remaining = total_bytes_ > transferred_ ? total_bytes_ - transferred_ : 0;
        const std::string left = format_duration(static_cast<double>(remaining) * seconds /
                                                 static_cast<double>(transferred_));
        if (!left.empty()) timing = left + " remaining";
    }

    const int width = ui::content_width();
    const double ratio = total_bytes_ > 0
                             ? static_cast<double>(transferred_) / static_cast<double>(total_bytes_)
                             : 0.0;

    const size_t used = ui::visible_width(moved) + ui::visible_width(timing);
    const std::string gap(used < static_cast<size_t>(width) ? width - used : 1, ' ');

    lines.push_back("");
    lines.push_back(ui::PAD + progress_bar(ratio, width));
    lines.push_back(ui::PAD + ui::help(moved) + gap + ui::help(timing));

    return lines;
}
