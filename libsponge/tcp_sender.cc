#include "tcp_sender.hh"

#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <cstddef>
#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _outstanding_segments()
    , _receiver_window_size(0)
    , _consecutive_retransmissions_counts(0)
    , _bytes_in_flight_counts(0)
    , _elapsed_time(0)
    , _timeout_counts(0)
    , _isn_flag(false)
    , _fin_flag(false)
    {}

uint64_t TCPSender::bytes_in_flight() const { 
    return _bytes_in_flight_counts;
}

void TCPSender::fill_window() {


    TCPSegment segments;
    // If isn segments haven't been sent
    if( !_isn_flag ){
        _isn_flag = true;
        segments.header().syn = true;
        segments.header().seqno = _isn;
        size_t abs_seqno = unwrap(_isn, _isn, _next_seqno);
        _outstanding_segments.insert(pair<size_t,TCPSegment>(abs_seqno, segments));
        _bytes_in_flight_counts += 1;
        segments_out().push(segments);
        return;
    }

    if ( 0 == _receiver_window_size ){
        _receiver_window_size = 1;
    }



    const size_t payload = segments.payload().size(); 


    segments.payload() = Buffer(_stream.read(payload));

    _segments_out.push(segments);


}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) { 
    //size_t abs_seqno = unwrap(ackno, _isn, _next_seqno);

    
    

    
    DUMMY_CODE(ackno, window_size); 

}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { 
    _elapsed_time += ms_since_last_tick;

    if(_receiver_window_size > 0){
        _consecutive_retransmissions_counts += 1;
        _initial_retransmission_timeout *= 2;
        _timeout_counts += 1;
        _elapsed_time = 0;
    }
    
}

unsigned int TCPSender::consecutive_retransmissions() const { 
    return _consecutive_retransmissions_counts; 
}

void TCPSender::send_empty_segment() {
    TCPSegment segments;
    segments.header().seqno = next_seqno();
    segments_out().push(segments);
}
