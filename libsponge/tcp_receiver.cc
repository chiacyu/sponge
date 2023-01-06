#include "tcp_receiver.hh"
#include "tcp_header.hh"
#include "wrapping_integers.hh"
#include <cstddef>
#include <cstdint>
#include <type_traits>

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    const TCPHeader &header = seg.header();
    string data = seg.payload().copy();

    if (!_syn_flag) {
        if (!header.syn)
            return;
        _iseqno = header.seqno;
        _syn_flag = true;
    }

    uint64_t checkpoint_index = _reassembler.stream_out().bytes_written() + 1;
    uint64_t curr_seqno_num = unwrap(header.seqno, _iseqno, checkpoint_index);

    uint64_t stream_index = curr_seqno_num - 1 + header.syn;
    _reassembler.push_substring(data, stream_index, header.fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const { 

    if (!_syn_flag)
        return {};

    uint64_t ack_no = _reassembler.stream_out().bytes_written() + 1;

    if (_reassembler.stream_out().input_ended()){
        ack_no+=1;
    }
    return _iseqno + ack_no;
}

size_t TCPReceiver::window_size() const { 
    return _capacity - _reassembler.stream_out().buffer_size(); 
}
