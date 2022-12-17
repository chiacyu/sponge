#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) : _capacity(capacity){}

size_t ByteStream::write(const string &data) {
    
    size_t writen_count = 0;
    size_t length = data.size();

    for(size_t i=0 ; i<length ; i++){
        if(_total_buff_size >= _capacity) break;
            
        _stream_buf.push_back(data[i]);
        _bytes_to_write++;
        _total_buff_size++;
        writen_count++;
    }

    return writen_count;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t length = len > _total_buff_size ? _total_buff_size : len;

    list<char>::const_iterator it = _stream_buf.begin();
    advance(it, length);
    return string(_stream_buf.begin(), it);;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    size_t length = len > _total_buff_size ? _total_buff_size : len;

    for(size_t i=0 ; i<length ; i++){
        if(_stream_buf.empty()) break;
        _stream_buf.pop_front();
        _total_buff_size--;
        _bytes_to_read++;
    }
    return;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string output = "";
    size_t length = len > _total_buff_size ? _total_buff_size : len;
    output = peek_output(length);
    pop_output(length);
    return output;
}

void ByteStream::end_input() { _is_input_end = true; }

bool ByteStream::input_ended() const { return _is_input_end; }

size_t ByteStream::buffer_size() const { return _total_buff_size; }

bool ByteStream::buffer_empty() const { return _stream_buf.empty(); }

bool ByteStream::eof() const { return (_is_input_end && _stream_buf.empty()); }

size_t ByteStream::bytes_written() const { return _bytes_to_write; }

size_t ByteStream::bytes_read() const { return _bytes_to_read; }

size_t ByteStream::remaining_capacity() const { return (_capacity - _total_buff_size); }
