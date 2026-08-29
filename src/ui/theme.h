#pragma once
#include <string>

// SOLAN's visual identity, mapped from the project site
// (solan.ujaanmukherjee.com): warm greys and one terracotta accent carrying
// every mark and stroke.
//
// Body text deliberately has no colour of its own -- it inherits the
// terminal's foreground, so the UI reads correctly on light and dark grounds
// alike, the same way the site swaps --text between themes.
namespace col
{
    constexpr const char* RESET = "\033[0m";
    constexpr const char* BOLD  = "\033[1m";
    constexpr const char* DIM   = "\033[2m";

    constexpr const char* ACCENT  = "\033[38;2;204;120;92m";  // #CC785C  --accent-line
    constexpr const char* MUTED   = "\033[38;2;140;136;127m"; // descriptions
    constexpr const char* FAINT   = "\033[38;2;168;164;155m"; // hairlines
    constexpr const char* SUCCESS = "\033[38;2;138;160;107m"; // sage
    constexpr const char* DANGER  = "\033[38;2;192;83;60m";   // brick
    constexpr const char* WARNING = "\033[38;2;217;149;74m";  // amber clay

    struct Rgb { int r, g, b; };

    constexpr Rgb ACCENT_DEEP_RGB {138, 75, 46};   // #8A4B2E
    constexpr Rgb ACCENT_RGB      {204, 120, 92};  // #CC785C
    constexpr Rgb ACCENT_SOFT_RGB {227, 156, 126}; // #E39C7E

    std::string fg(Rgb c);

    // Three-stop ramp: deep terracotta -> accent -> soft clay.
    std::string ramp(double t);
}

// Text styles. Each returns the text wrapped in its escape codes, so they
// compose inside a single output line.
namespace ui
{
    std::string help(const std::string &text);     // muted description
    std::string faint(const std::string &text);    // hairline / recessive
    std::string element(const std::string &text);  // terracotta accent
    std::string label(const std::string &text);    // uppercase meta label
    std::string bold(const std::string &text);
    std::string danger(const std::string &text);
    std::string warning(const std::string &text);
    std::string success(const std::string &text);

    // Letter-spaced wordmark, matching the site's 0.12em uppercase mark.
    std::string spaced(const std::string &text);
}
