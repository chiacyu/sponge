#include "stream_reassembler.hh"
#include <cstddef>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : 
    _buffer_char_bitmap(),
    _next_reassemble_char_index(0),
    _current_buf_sizes(0),
    _eof_flag(false),
    _output(capacity), 
    _capacity(capacity)
    {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {

    if(eof && (index + data.size() <= _next_reassemble_char_index + _output.remaining_capacity())){
        _eof_flag = true;
    }
    //flush reveived data into bytestreams
    if( _next_reassemble_char_index >= index ){
        string flush_data = "";
        size_t start_index = _next_reassemble_char_index - index;
        size_t remain_capacity = _output.remaining_capacity();
        
        while( (start_index < data.size()) && (remain_capacity != 0) ){
            if( _buffer_char_bitmap.count(_next_reassemble_char_index)){
                _current_buf_sizes--;
                _buffer_char_bitmap.erase(_next_reassemble_char_index);
            }

            flush_data += data[start_index];
            start_index++;
	        _next_reassemble_char_index++;
            remain_capacity--;
        }
        _output.write(flush_data);
    }
    else{
        //save unassemble string into buffer
        size_t start_index = index;
        for( char c : data){
            if( start_index > _next_reassemble_char_index + _output.remaining_capacity()){
                break;
            }

            if(_buffer_char_bitmap.count(start_index)>0){
                start_index+=1;
                continue;
            }
            _buffer_char_bitmap.insert(pair<size_t, char>(start_index, c));
            _current_buf_sizes+=1;
	        start_index+=1;
	    }
    }

    string flush_data = "";

    
    while(_buffer_char_bitmap.count(_next_reassemble_char_index) >0 &&(_output.remaining_capacity() > 0)){
        flush_data += _buffer_char_bitmap[_next_reassemble_char_index];
        _buffer_char_bitmap.erase(_next_reassemble_char_index);
        _next_reassemble_char_index++;
        _current_buf_sizes--;
        if(_output.remaining_capacity() == 0){
            break;
        }
    }
    _output.write(flush_data);

    if(empty() && _eof_flag){
	    _output.end_input();
    }
    return;
}

size_t StreamReassembler::unassembled_bytes() const { 
    return _current_buf_sizes; 
}

bool StreamReassembler::empty() const { 
    return (_current_buf_sizes == 0); 
}
