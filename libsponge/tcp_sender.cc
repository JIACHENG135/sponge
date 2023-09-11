#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <algorithm>
#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _next_seqno(0)
    , _last_window_size(0)
    , _set_syn_flag(false)
    , _set_fin_flag(false)
    , _outgoing_bytes(0)
    , _timeout(retx_timeout)
    , _timecount(0) {}

uint64_t TCPSender::bytes_in_flight() const { return _outgoing_bytes; }

void TCPSender::fill_window() {
    size_t window_size = _last_window_size ? _last_window_size : 1;

    while (window_size > _outgoing_bytes) {
        // need to fill window untill outgoing bytes full
        TCPSegment segment;
        if (!_set_syn_flag) {
            segment.header().syn = true;
            _set_syn_flag = true;
        }
        segment.header().seqno = next_seqno();
        const size_t payload_size =
            std::min(TCPConfig::MAX_PAYLOAD_SIZE, window_size - _outgoing_bytes - segment.header().syn);

        string payload = _stream.read(payload_size);
        if (!_set_fin_flag && _stream.eof() && payload_size + _outgoing_bytes < window_size) {
            _set_fin_flag = segment.header().fin = true;
        }
        segment.payload() = Buffer(move(payload));
        if (segment.length_in_sequence_space() == 0)
            break;
        if (_outgoing_map.empty()) {
            _timeout = _initial_retransmission_timeout;
            _timecount = 0;
        }
        _segments_out.push(segment);
        _outgoing_bytes += segment.length_in_sequence_space();
        _outgoing_map.insert(make_pair(_next_seqno, segment));
        _next_seqno += segment.length_in_sequence_space();
        if (segment.header().fin)
            break;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    size_t abs_ackno = unwrap(ackno, _isn, _next_seqno);
    if (abs_ackno > _next_seqno) {
        return;
    }
    for (auto it = _outgoing_map.begin(); it != _outgoing_map.end();) {
        const TCPSegment &segment = it->second;
        if (it->first + segment.length_in_sequence_space() <= abs_ackno) {
            _outgoing_bytes -= segment.length_in_sequence_space();
            it = _outgoing_map.erase(it);
            _timeout = _initial_retransmission_timeout;
            _timecount = 0;
        } else {
            break;
        }
    }
    _consecutive_retransmissions_count = 0;
    _last_window_size = window_size;
    fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    _timecount += ms_since_last_tick;
    auto it = _outgoing_map.begin();
    if (it != _outgoing_map.end() && _timecount >= _timeout) {
        if (_last_window_size > 0) {
            _timeout *= 2;
        }
        _timecount = 0;
        _segments_out.push(it->second);
        ++_consecutive_retransmissions_count;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmissions_count; }

void TCPSender::send_empty_segment() {
    TCPSegment segment;
    segment.header().seqno = next_seqno();
    _segments_out.push(segment);
}
