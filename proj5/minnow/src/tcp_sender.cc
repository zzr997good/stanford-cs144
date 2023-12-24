#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return bytes_in_flight_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return consecutive_retransmissions_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // If there is no message to send, return an empty optional
  if ( _messages.empty() ) {
    return {};
  }
  // If I am goint to retransmit a message, I should resend it in the reset timer, which means timer_.ticket=0
  if ( consecutive_retransmissions_ && timer_.is_running() && timer_.get_ticket() > 0 ) {
    return {};
  }
  auto msg = _messages.front();
  _messages.pop();
  return msg;
}

void TCPSender::push( Reader& outbound_stream )
{
  // Your code here.
  // If TCP is closed, return
  if ( sent_FIN_ )
    return;
  auto available_space = rcv_abs_ackno_ + ( window_size_ ? window_size_ : 1 ) - abs_seqno_;
  while ( available_space > 0 && !sent_FIN_ ) {
    TCPSenderMessage msg;
    // If SYN has not been sent, send SYN
    msg.seqno = Wrap32::wrap( abs_seqno_, isn_ );
    if ( !sent_SYN_ ) {
      msg.SYN = true;
      sent_SYN_ = true;
      available_space--;
    }
    // Send the payload
    read( outbound_stream, std::min( available_space, TCPConfig::MAX_PAYLOAD_SIZE ), msg.payload );
    available_space -= msg.payload.size();
    // If the outbound_stream is finished, send FIN
    if ( available_space > 0 && outbound_stream.is_finished() ) {
      msg.FIN = true;
      sent_FIN_ = true;
      available_space--;
    }
    auto occupied = msg.sequence_length();
    // Could not read anything from the outbound_stream
    if ( occupied == 0 )
      return;
    // Push the message to _messages so that it could be sent later
    _messages.push( msg );
    // Push the message to _outstanding_messages because it is not acked yet
    _outstanding_messages.push( msg );
    bytes_in_flight_ += occupied;
    abs_seqno_ += occupied;
    if ( !timer_.is_running() ) {
      timer_.start();
    }
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  TCPSenderMessage msg;
  msg.seqno = Wrap32::wrap( abs_seqno_, isn_ );
  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  if ( msg.ackno ) {
    rcv_abs_ackno_ = msg.ackno->unwrap( isn_, abs_seqno_ );
  }
  // If the ackno is greater than abs_seqno_, the receiver needs future data, error happens
  if ( rcv_abs_ackno_ > abs_seqno_ ) {
    return;
  }
  window_size_ = msg.window_size;
  bool getAck = false;
  while ( !_outstanding_messages.empty() ) {
    // Try to remove the acked messages from _outstanding_messages
    auto& ealy_msg = _outstanding_messages.front();
    // This message is not completely acked,just partially acked
    if ( ealy_msg.seqno.unwrap( isn_, abs_seqno_ ) + ealy_msg.sequence_length() > rcv_abs_ackno_ ) {
      break;
    }
    bytes_in_flight_ -= ealy_msg.sequence_length();
    _outstanding_messages.pop();
    getAck = true;
  }
  if ( getAck ) {
    RTO_ms_ = initial_RTO_ms_;
    if ( !_outstanding_messages.empty() ) {
      timer_.start();
    } else {
      timer_.stop();
    }
    consecutive_retransmissions_ = 0;
  }
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  if ( !timer_.is_expired( ms_since_last_tick, RTO_ms_ ) || _outstanding_messages.empty() )
    return;
  // Start retransmission
  _messages.push( _outstanding_messages.front() );
  if ( window_size_ > 0 ) {
    RTO_ms_ *= 2;
  }
  consecutive_retransmissions_++;
  // maybe_send();
  timer_.start();
}
