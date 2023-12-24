#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Did not reverive SYN yet, then we should not receive any message
  if ( !zero_point.has_value() && !message.SYN )
    return;
  // If this is the SYN message, then we should set the zero_point
  if ( message.SYN )
    zero_point = message.seqno;
  uint64_t checkpoint = 1 + inbound_stream.bytes_pushed();
  // If the message is SYN massage, absolute sequence number is 0 and the first_index is also 0;
  // Otherwise the abotolute sequence number is the seqno minus 1
  uint64_t first_index = message.seqno.unwrap( zero_point.value(), checkpoint ) - ( message.SYN ? 0 : 1 );
  reassembler.insert( first_index, message.payload, message.FIN, inbound_stream );
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  TCPReceiverMessage message;
  // If zero_point has the value, the we alrady get the SYN
  if ( zero_point )
    message.ackno = Wrap32::wrap( ( zero_point.has_value() ? 1 : 0 ) + inbound_stream.bytes_pushed()
                                    + ( inbound_stream.is_closed() ? 1 : 0 ),
                                  zero_point.value() );
  else
    message.ackno = std::nullopt;
  message.window_size = inbound_stream.available_capacity() > 0xFFFF ? 0xFFFF : inbound_stream.available_capacity();
  return message;
}
