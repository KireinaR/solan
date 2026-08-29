#include "ui/spinner.h"

namespace
{
    // portal: WaitingSpinner, FPS time.Second / 12
    const char* const WAITING_FRAMES[] = {
        "⠋ ", "⠙ ", "⠹ ", "⠸ ", "⠼ ", "⠴ ", "⠦ ", "⠧ ", "⠇ ", "⠏ "};

    // portal: CompressingSpinner, FPS time.Second / 3
    const char* const COMPRESSING_FRAMES[] = {
        "┉┉┉", "┅┅┅", "┄┄┄", "┉ ┉", "┅ ┅", "┄ ┄", " ┉ ", " ┉ ", " ┅ ", " ┅ ", " ┄ "};

    // portal: TransferSpinner, FPS 400ms
    const char* const TRANSFER_FRAMES[] = {
        "⇢┄┄", "┄⇢┄", "┄┄⇢", "┄┄┄"};

    // portal: ReceivingSpinner, FPS time.Second / 2
    const char* const RECEIVING_FRAMES[] = {
        "┄┄┄", "┄┄⇠", "┄⇠┄", "⇠┄┄"};
}

namespace spin
{
    const SpinnerStyle WAITING     {WAITING_FRAMES, 10, 83};
    const SpinnerStyle COMPRESSING {COMPRESSING_FRAMES, 11, 333};
    const SpinnerStyle TRANSFER    {TRANSFER_FRAMES, 4, 400};
    const SpinnerStyle RECEIVING   {RECEIVING_FRAMES, 4, 500};
}
