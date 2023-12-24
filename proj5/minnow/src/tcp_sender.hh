#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include <queue>
class Timer
{
private:
  uint64_t ticket_ = 0;
  bool is_running_ = false;

public:
  // Timer():ticket_(0),is_running_(false){}
  void start()
  {
    ticket_ = 0;
    is_running_ = true;
  }
  void stop() { is_running_ = false; }
  bool is_running() const { return is_running_; }
  uint64_t get_ticket() const { return ticket_; }
  bool is_expired( uint64_t ms_since_last_tick, uint64_t RTO_ms )
  {
    ticket_ += ms_since_last_tick;
    return is_running_ && ( ticket_ >= RTO_ms );
  }
};

class TCPSender
{
private:
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;
  uint64_t RTO_ms_ = initial_RTO_ms_;
  uint64_t consecutive_retransmissions_ { 0 };
  uint64_t rcv_abs_ackno_ { 0 };
  uint64_t window_size_ { 1 };
  uint64_t abs_seqno_ { 0 };
  uint64_t bytes_in_flight_ { 0 };
  Timer timer_ {};
  bool sent_SYN_ { false };
  bool sent_FIN_ { false };
  // store the sent but not acked message
  std::queue<TCPSenderMessage> _outstanding_messages {};
  // store the message to be sent
  std::queue<TCPSenderMessage> _messages {};

public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( uint64_t ms_since_last_tick );

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
};
