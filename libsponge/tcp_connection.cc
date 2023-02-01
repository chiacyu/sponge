#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { 
    return _sender.stream_in().remaining_capacity(); 
}

size_t TCPConnection::bytes_in_flight() const { 
    return _sender.bytes_in_flight();
}

size_t TCPConnection::unassembled_bytes() const { 
    return _receiver.unassembled_bytes();
 }

size_t TCPConnection::time_since_last_segment_received() const { 
    return _time_since_last_segment_received; 
}

void TCPConnection::segment_received(const TCPSegment &seg) { 

    if(!_active){
        return;
    }
    
    const TCPSegment segments = seg;

    // if reset flag is set
    if( segments.header().rst){
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _active = false;
    }

    // if ack flag is set
    if( segments.header().ack){
        _sender.ack_received(segments.header().ackno, segments.header().win);
    }
    
    _receiver.segment_received(segments);


}

void TCPConnection::send_segments(){
    // while the sender
    while ( !_sender.segments_out().empty() )
    {
        TCPSegment segment = _sender.segments_out().front();
        _sender.segments_out().pop();

        // optional<WrappingInt32> ack_no = _receiver.ackno();

        // if(ack_no.has_value()){
        //     segment.header().ack = true;
        //     segment.header().ackno = ack_no;
        // }

        segment.header().win = _receiver.window_size() <= numeric_limits<uint16_t>::max() 
                                                    ?   _receiver.window_size()
                                                    :   numeric_limits<uint16_t>::max();
        _segments_out.emplace(segment);
    }
    return;
}

bool TCPConnection::active() const { 
    return _active; 
}

size_t TCPConnection::write(const string &data) {
    if(!data.size() || !_active) return 0;

    size_t wc = _sender.stream_in().write(data);
    _sender.fill_window();
    return wc;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) { 
    DUMMY_CODE(ms_since_last_tick); 
}

void TCPConnection::end_input_stream() {}

void TCPConnection::connect() {}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
