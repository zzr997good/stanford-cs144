#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  table_.push_back( tableEntry( route_prefix, prefix_length, next_hop, interface_num ) );
}

void Router::match_then_send( InternetDatagram& datagram )
{
  // Find the route with the longest prefix that matches the datagram's destination address
  // TTL is less than 1, drop the packet
  if ( datagram.header.ttl <= 1 )
    return;
  // Start matching process
  bool matched = false;
  size_t matched_idx = 0;
  for ( size_t i = 0; i < table_.size(); i++ ) {
    // Matched
    if ( ( ( datagram.header.dst & table_[i].mask_ ) == table_[i].route_prefix_ )
         && ( ( !matched ) || ( table_[i].prefix_length_ > table_[matched_idx].prefix_length_ ) ) ) {
      matched = true;
      matched_idx = i;
    }
  }
  // Not matched, drop the packet
  if ( !matched )
    return;
  datagram.header.ttl--;
  datagram.header.compute_checksum();
  Address dst_addr = table_[matched_idx].next_hop_.has_value() ? table_[matched_idx].next_hop_.value()
                                                               : Address::from_ipv4_numeric( datagram.header.dst );
  interface( table_[matched_idx].interface_num_ ).send_datagram( datagram, dst_addr );
}

void Router::route()
{
  for ( auto& interface : interfaces_ ) {
    while ( true ) {
      auto datagram = interface.maybe_receive();
      if ( !datagram.has_value() ) {
        break;
      }
      match_then_send( datagram.value() );
    }
  }
}
