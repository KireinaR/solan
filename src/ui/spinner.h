#pragma once

// Spinner definitions copied verbatim from SpatiumPortae/portal
// (cmd/portal/tui/tui.go), including their frame rates.
struct SpinnerStyle
{
    const char* const* frames;
    int count;
    int interval_ms;
};

namespace spin
{
    extern const SpinnerStyle WAITING;      // awaiting the receiver's decision
    extern const SpinnerStyle COMPRESSING;  // packing the payload
    extern const SpinnerStyle TRANSFER;     // sending
    extern const SpinnerStyle RECEIVING;    // receiving
}
