#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return zero_point + static_cast<uint32_t>( n );
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Step1: wrap the checkpoint and find the offset in uint32_t range
  uint32_t offset = raw_value_ - wrap( checkpoint, zero_point ).raw_value_;
  // Step2: get the potential result
  uint64_t result = checkpoint + offset;
  // Why this is a potential result?
  // 1. rwo_value_ could be smaller than wrapped checkpoint and in that case offset is wrap to a bigger dis
  // 2. offset could be bigger than 2^31 which cause offset-2^32 is a closer result
  if ( offset > ( 1u << 31 ) && result >= ( 1ull << 32 ) ) {
    result -= ( 1ull << 32 );
  }
  return result;
}
