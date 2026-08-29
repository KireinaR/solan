#pragma once
#include <string>

// Page furniture, borrowed from the SOLAN site: generous margins, hairline
// rules, and label-left / value-right metadata rows. Everything here prints.
namespace ui
{
    constexpr int MARGIN    = 2;
    constexpr int MAX_WIDTH = 78;

    extern const std::string PAD;

    // min(MAX_WIDTH, terminal width - 2 * MARGIN)
    int content_width();

    // A hairline of the given width, or the content width by default.
    std::string rule(int width = 0);

    void separator();
    void line(const std::string &text);
    void row(const std::string &left, const std::string &right, int width = 0);
    void blank();

    // Status lines. All three sit at the same indent with no leading glyph --
    // the sentence colour alone carries the outcome.
    void task(const std::string &text);       // terminal default foreground
    void task_ok(const std::string &text);    // whole sentence in green
    void task_fail(const std::string &text);  // whole sentence in red
}
