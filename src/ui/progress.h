#pragma once
#include <string>

// Progress bar: solid blocks filling along a terracotta ramp
// (#8A4B2E -> #CC785C -> #E39C7E) over a faint shaded track.
std::string progress_bar(double ratio, int total_width);
