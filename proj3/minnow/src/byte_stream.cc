#include "byte_stream.hh"
#include <stdexcept>

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity ), buffer_(), status_( StatusType::Normal ), bytes_pushed_( 0 ), bytes_popped_( 0 )
{}

void Writer::push( string data )
{
  // Your code here.
  // 把buffer_当作一个循环队列来维护capacity的stream
  if ( status_ != StatusType::Normal ) {
    return;
  }
  uint64_t push_len = min( available_capacity(), data.size() );
  buffer_ += data.substr( 0, push_len );
  bytes_pushed_ += push_len;
}

void Writer::close()
{
  // Your code here.
  if ( is_closed() ) {
    return;
  }
  status_ = StatusType::Closed;
}

void Writer::set_error()
{
  // Your code here.
  if ( is_closed() ) {
    return;
  }
  status_ = StatusType::Error;
}

bool Writer::is_closed() const
{
  // Your code here.
  return status_ == StatusType::Closed;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - ( bytes_pushed_ - bytes_popped_ );
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return bytes_pushed_;
}

string_view Reader::peek() const
{
  // Your code here.
  if ( is_finished() ) {
    return string_view();
  }
  if ( has_error() ) {
    return string_view();
  }
  if ( buffer_.empty() ) {
    return string_view();
  }
  return string_view( buffer_ );
}

bool Reader::is_finished() const
{
  // Your code here.
  return status_ == StatusType::Closed && buffer_.empty();
}

bool Reader::has_error() const
{
  // Your code here.
  return status_ == StatusType::Error;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  uint64_t pop_len = min( len, bytes_buffered() );
  buffer_ = buffer_.substr( pop_len );
  bytes_popped_ += pop_len;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return bytes_pushed_ - bytes_popped_;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return bytes_popped_;
}
