#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : 
    _buffer_bitmap(capacity, false),
    _unassemble_buffer_(capacity, 0),
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

    _eof_flag = eof;

    if(_next_reassemble_char_index >= _capacity){
        return;
    }

    //flush reveived data into bytestreams
    if( _next_reassemble_char_index >= index ){
        string flush_data = "";
        size_t start_index = _next_reassemble_char_index - index;
        
        while( start_index < data.size() ){
            flush_data += data[start_index];
	    if(_buffer_bitmap[start_index + index] == true){
	    	_current_buf_sizes--;
	    }
             start_index++;
	    _next_reassemble_char_index++;
            if((_next_reassemble_char_index >= _capacity)){
                break;
            }
        }
        _output.write(flush_data);
    }
    else{
        //save unassemble string into buffer
        for( char c : data){
        size_t start_index = index;
        if(_buffer_bitmap[start_index] == true){
            continue;
        }
        _unassemble_buffer_[start_index] = c;
	_buffer_bitmap[start_index] = true;
        _current_buf_sizes+=1;
	start_index+=1;
        if(( _current_buf_sizes >= (_capacity - _next_reassemble_char_index)) ||
		(start_index >= _capacity)){
		break;
        	}
	}
    }

    string flush_data = "";
    //flush ready data into bytestreams
    while( _buffer_bitmap[_next_reassemble_char_index] == true){
        flush_data += _unassemble_buffer_[_next_reassemble_char_index];
        _next_reassemble_char_index++;
        _current_buf_sizes--;
        if( _next_reassemble_char_index == _capacity){
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
    size_t bytes_count = 0;
    size_t start_index = _next_reassemble_char_index;
    while ( start_index != _capacity)
    {
        if((_unassemble_buffer_[start_index] != 0) && 
            (_buffer_bitmap[start_index]!=true)){
            bytes_count++;    
        }
    }
    return bytes_count; 
}

bool StreamReassembler::empty() const { 
    return (_current_buf_sizes == 0); 
}
