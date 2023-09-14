#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) { return WrappingInt32{static_cast<uint32_t>(n) + isn.raw_value()}; }

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    auto low32bit = static_cast<uint64_t>(n.raw_value() - isn.raw_value());
    auto high32bit1 = (checkpoint + (1ull << 31)) & 0xFFFFFFFF00000000;
    auto high32bit2 = (checkpoint - (1ull << 31)) & 0xFFFFFFFF00000000;

    uint64_t num1 = high32bit1 | low32bit;
    uint64_t num2 = high32bit2 | low32bit;

    uint64_t diff1 = (num1 > checkpoint) ? (num1 - checkpoint) : (checkpoint - num1);
    uint64_t diff2 = (num2 > checkpoint) ? (num2 - checkpoint) : (checkpoint - num2);

    return (diff1 < diff2) ? num1 : num2;
}
