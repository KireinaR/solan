#pragma once
#include "ui/spinner.h"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// A block of terminal lines repainted in place.
class LiveView
{
public:
    void render(const std::vector<std::string> &lines);
    void clear();

    // Leaves whatever is on screen alone and forgets it, so a later clear()
    // cannot erase it.
    void keep();

private:
    int painted_ = 0;
};

// An animated status block: a spinner plus an optional transfer progress bar
// with transferred bytes, speed and ETA, repainted from its own thread.
//
// When stdout is not a terminal the animation is skipped entirely and the
// status is emitted once as a plain line.
class StatusView
{
public:
    StatusView(const SpinnerStyle &style, std::string status);
    ~StatusView();

    StatusView(const StatusView &) = delete;
    StatusView &operator=(const StatusView &) = delete;

    void set_status(std::string status);

    // Switches the block into progress mode and starts the clock.
    void track_bytes(uint64_t total_bytes);
    void set_bytes(uint64_t transferred);

    // Stops the animation and erases the block. Called by the destructor, so
    // an abandoned transfer leaves no half-drawn block behind.
    void stop();

    // Stops the animation and leaves the block on screen for good, headed by
    // `summary` and footed by the totals: bytes moved, average speed and
    // elapsed time. Use this on success, so the terminal keeps a record of
    // what was transferred.
    void finish(std::string summary);

private:
    void loop();
    std::vector<std::string> compose() const;

    const SpinnerStyle *style_;
    std::string status_;
    int frame_ = 0;

    bool tracking_ = false;
    bool done_ = false;
    uint64_t total_bytes_ = 0;
    uint64_t transferred_ = 0;
    std::chrono::steady_clock::time_point started_;

    bool interactive_;
    LiveView live_;
    mutable std::mutex mutex_;
    std::atomic<bool> running_{true};
    std::thread worker_;
};
