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
    , _receiver_window_size(0)
    , _consecutive_retransmissions_counts(0)
    , _bytes_in_flight_counts(0)
    , _elapsed_time(0)
    , _timeout_counts(0)
    , _isn_flag(false)
    , _fin_flag(false)
    , _timer_liveness(false)
    {}

uint64_t TCPSender::bytes_in_flight() const { 
    return _bytes_in_flight_counts;
}

void TCPSender::fill_window() {

    TCPSegment segments;

    if( _receiver_window_size == 0 ){
        _receiver_window_size = 1;
    }

    if( _receiver_window_size < _bytes_in_flight_counts ){
        return;
    }

    // If isn segments haven't been sent
    if( !_isn_flag ){
        _isn_flag = true;
        segments.header().syn = true;
    }

    
    size_t payload_size = min(TCPConfig::MAX_PAYLOAD_SIZE, _stream.buffer_size());
    if( _receiver_window_size - _bytes_in_flight_counts < payload_size ){
        payload_size = _receiver_window_size - _bytes_in_flight_counts;
    }
    segments.header().seqno = wrap(_next_seqno, _isn);
    segments.payload() = Buffer(_stream.read(payload_size));

    // Set the Fin segments
    if (!_fin_flag && _stream.eof() && segments.payload().size() + _bytes_in_flight_counts < _receiver_window_size){
        _fin_flag = true;
        segments.header().fin = true;
    }

    if(_isn_flag && segments.length_in_sequence_space() == 0){
        return;
    }

    _bytes_in_flight_counts += segments.length_in_sequence_space();
    _next_seqno += segments.length_in_sequence_space();

    _segments_out.push(segments);
    _outstanding_segments.push(segments);

    // Everytime a message is sent, start the timer
    _timer_liveness = true;    

    if(_outstanding_segments.empty()){
        _timer_liveness = false;
    }

    return;
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) { 

    TCPSegment segments;

    size_t abs_seqno = unwrap(ackno, _isn, _next_seqno);

    if(abs_seqno > _next_seqno){
        return;
    }

    while( !_outstanding_segments.empty() ){

        segments = _outstanding_segments.front();
        size_t _outstanding_segments_seqno = unwrap(segments.header().seqno, _isn, _next_seqno);
        if( _outstanding_segments_seqno >= abs_seqno ){
            break;
        }

        _bytes_in_flight_counts -= segments.length_in_sequence_space();
        _consecutive_retransmissions_counts = 0;

        _outstanding_segments.pop();
        while ( _timeout_counts > 0 )
        {
            _timeout_counts--;
            _initial_retransmission_timeout >>= 1;
        }
    
    }
    _receiver_window_size = window_size;    
    return;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { 
    _elapsed_time += ms_since_last_tick;

    if(_elapsed_time >= _initial_retransmission_timeout){
        
        TCPSegment segments = _outstanding_segments.front();
        _segments_out.push(segments);
        
        _elapsed_time = 0;
        _timer_liveness =true;

        if( _receiver_window_size > 0 ){
            _consecutive_retransmissions_counts += 1;
            _initial_retransmission_timeout *= 2;
            _timeout_counts += 1;
        }
    }
    return;    
}

unsigned int TCPSender::consecutive_retransmissions() const { 
    return _consecutive_retransmissions_counts; 
}

void TCPSender::send_empty_segment() {
    TCPSegment segments;
    segments.header().seqno = next_seqno();
    segments_out().push(segments);
}
