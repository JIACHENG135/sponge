#include "byte_stream.hh"

// Constructor
ByteStream::ByteStream(const size_t capacity) : _buffer(), _capacity(capacity), _written_cnt(0), _read_cnt(0) {}

size_t ByteStream::write(const std::string &data) {
    size_t len = std::min(data.size(), remaining_capacity());
    for (size_t i = 0; i < len; ++i) {
        _buffer.push_back(data[i]);
    }
    _written_cnt += len;
    return len;
}

size_t ByteStream::remaining_capacity() const { return _capacity - _buffer.size(); }

void ByteStream::end_input() { _input_ended_flag = true; }

std::string ByteStream::peek_output(const size_t len) const {
    std::string ret;
    size_t n = std::min(len, _buffer.size());
    for (size_t i = 0; i < n; ++i) {
        ret.push_back(_buffer[i]);
    }
    return ret;
}

void ByteStream::pop_output(const size_t len) {
    size_t n = std::min(len, _buffer.size());
    for (size_t i = 0; i < n; ++i) {
        _buffer.pop_front();
    }
    _read_cnt += n;
}

std::string ByteStream::read(const size_t len) {
    std::string ret = peek_output(len);
    pop_output(len);
    return ret;
}

bool ByteStream::input_ended() const { return _input_ended_flag; }

size_t ByteStream::buffer_size() const { return _buffer.size(); }

bool ByteStream::buffer_empty() const { return _buffer.empty(); }

bool ByteStream::eof() const { return _input_ended_flag && _buffer.empty(); }

size_t ByteStream::bytes_written() const { return _written_cnt; }

size_t ByteStream::bytes_read() const { return _read_cnt; }
