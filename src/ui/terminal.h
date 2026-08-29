#pragma once

// Terminal capability detection. Everything here is platform-specific and
// deliberately kept away from the palette and layout code.

// Enables ANSI escape processing (and UTF-8 output) on Windows consoles.
// A no-op elsewhere. Call once, before any styled output.
void enable_ansi_support();

// Usable columns, or 80 when the width cannot be determined.
int terminal_width();

// False when stdout is a pipe or a file, in which case the animated status
// blocks fall back to plain one-shot lines.
bool stdout_is_tty();
