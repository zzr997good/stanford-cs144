#include "reassembler.hh"

using namespace std;
Reassembler::Reassembler()
  : unassembled_segments(), first_unassembled_index( 0 ), bytes_pended( 0 ), got_last_substring( false )
{}
void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // If this is the last substring, set got_lat_substring to true
  if ( is_last_substring )
    got_last_substring = true;
  // First get the first unacceptable index
  uint64_t first_unacceptable_index = first_unassembled_index + output.available_capacity();
  if ( first_index < first_unacceptable_index && data.size() > 0
       && first_index + data.size() > first_unassembled_index ) {
    // Cut the data
    uint64_t merged_data_start_index = max( first_index, first_unassembled_index );
    // Last index makes sure that the data could be stored in the reassembler buffer and will not exceed the
    // capacity
    uint64_t merged_data_last_index = min( first_unacceptable_index, first_index + data.size() );
    string merged_data
      = data.substr( merged_data_start_index - first_index, merged_data_last_index - merged_data_start_index );
    segment merged_segment = { merged_data_start_index, merged_data.size(), merged_data };
    // Merge the data
    auto insert_ok = unassembled_segments.insert( merged_segment );
    // If cur_data is already existed
    if ( insert_ok.second == false )
      return;
    // If cur_data is new data
    auto cur_data = insert_ok.first;
    bytes_pended += merged_data.size();
    cur_data = try_merge( cur_data );
    if ( cur_data->first_index == first_unassembled_index ) {
      // Write the data to the output
      output.push( cur_data->data );
      // Update the first unassembled index
      first_unassembled_index += cur_data->len;
      // Update the bytes pended
      bytes_pended -= cur_data->len;
      // Erase the data
      unassembled_segments.erase( cur_data );
    }
  }
  // If got the last substring and there is no bytes pending, close the stream
  if ( got_last_substring && bytes_pending() == 0 ) {
    // If the reassembler has got the last substring and the first unacceptable index is the first unassembled
    // index, then the reassembler should close the stream
    output.close();
    return;
  }
}

uint64_t Reassembler::bytes_pending() const
{ // Your code here.
  return bytes_pended;
}

set<Reassembler::segment>::iterator Reassembler::try_merge( set<Reassembler::segment>::iterator cur_data )
{
  // First try to merge with the previous segment
  bool overlap_with_prev = false;
  bool overlap_with_next = false;
  if ( cur_data != unassembled_segments.begin()
       && prev( cur_data )->first_index + prev( cur_data )->len >= cur_data->first_index ) {
    overlap_with_prev = true;
  }
  if ( cur_data != prev( unassembled_segments.end() )
       && next( cur_data )->first_index <= cur_data->first_index + cur_data->len ) {
    overlap_with_next = true;
  }
  if ( !overlap_with_prev && !overlap_with_next ) {
    return cur_data;
  }
  // If overlap with the previous segment, merge the data
  if ( overlap_with_prev ) {
    auto prev_seg = prev( cur_data );
    cur_data = merge_overlapped( prev_seg, cur_data );
  }
  // If overlap with the next segment, merge the data
  if ( overlap_with_next ) {
    auto next_seg = next( cur_data );
    cur_data = merge_overlapped( cur_data, next_seg );
  }
  // Recursively merge the data
  return try_merge( cur_data );
}

set<Reassembler::segment>::iterator Reassembler::merge_overlapped( set<Reassembler::segment>::iterator prev_seg,
                                                                   set<Reassembler::segment>::iterator cur_data )
{
  uint64_t merged_data_start_index = min( prev_seg->first_index, cur_data->first_index );
  uint64_t overlap_len = min( prev_seg->first_index + prev_seg->len, cur_data->first_index + cur_data->len )
                         - max( prev_seg->first_index, cur_data->first_index );
  string merged_data;
  if ( overlap_len == cur_data->len ) {
    merged_data = prev_seg->data;
  } else if ( overlap_len == prev_seg->len ) {
    merged_data = cur_data->data;
  } else {
    merged_data = prev_seg->data + cur_data->data.substr( overlap_len );
  }
  bytes_pended -= prev_seg->len;
  unassembled_segments.erase( prev_seg );
  bytes_pended -= cur_data->len;
  unassembled_segments.erase( cur_data );
  auto ret = unassembled_segments.insert( { merged_data_start_index, merged_data.size(), merged_data } );
  bytes_pended += merged_data.size();
  return ret.first;
}
