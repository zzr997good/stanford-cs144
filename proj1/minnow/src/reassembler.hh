#pragma once

#include "byte_stream.hh"

#include <set>
#include <string>
class Reassembler
{
public:
  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  Reassembler();
  void insert( uint64_t first_index, std::string data, bool is_last_substring, Writer& output );

  // How many bytes are stored in the Reassembler itself?
  uint64_t bytes_pending() const;

private:
  struct segment
  {
    uint64_t first_index;
    uint64_t len;
    std::string data;
    bool operator<( const segment& other ) const
    {
      return first_index == other.first_index ? len < other.len : first_index < other.first_index;
    }
  };
  std::set<segment> unassembled_segments;
  uint64_t first_unassembled_index;
  uint64_t bytes_pended;
  bool got_last_substring;

  std::set<segment>::iterator try_merge( std::set<segment>::iterator cur_data );
  std::set<segment>::iterator merge_overlapped( std::set<segment>::iterator prev_seg,
                                                std::set<segment>::iterator cur_data );
};
