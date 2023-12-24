#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address ), ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  // If the destination Ethernet address is already known, send it right away. Create
  // an Ethernet frame (with type = EthernetHeader::TYPE IPv4), set the payload to
  // be the serialized datagram, and set the source and destination addresses.
  IPAddr32 next_hop_ip = next_hop.ipv4_numeric();
  if ( ARP_dictionary_.find( next_hop_ip ) != ARP_dictionary_.end()
       && cur_ms - ARP_dictionary_[next_hop_ip].second <= UPDATE_DICTIONARY_TIMER ) {
    send_Ethernet_frame( dgram, next_hop_ip );
  }
  // If the destination Ethernet address is unknown, broadcast an ARP request for the
  // next hop’s Ethernet address, and queue the IP datagram so it can be sent after
  // the ARP reply is received.
  else {
    send_ARP_request( dgram, next_hop_ip );
  }
}

void NetworkInterface::send_Ethernet_frame( const InternetDatagram& dgram, const IPAddr32& next_hop_ip )
{
  EthernetFrame frame;
  frame.header.type = EthernetHeader::TYPE_IPv4;
  frame.header.src = ethernet_address_;
  frame.header.dst = ARP_dictionary_[next_hop_ip].first;
  frame.payload = serialize( dgram );
  ready_to_send_.push( frame );
}

void NetworkInterface::send_ARP_request( const InternetDatagram& dgram, const IPAddr32& next_hop_ip )
{
  // If already sent an ARP request, which means the data is waiting for ARP reply, do nothing
  if ( data_wait_for_ARP_reply_.find( next_hop_ip ) != data_wait_for_ARP_reply_.end() ) {
    return;
  }
  EthernetFrame frame;
  frame.header.type = EthernetHeader::TYPE_ARP;
  frame.header.src = ethernet_address_;
  frame.header.dst = ETHERNET_BROADCAST;
  ARPMessage arp;
  arp.opcode = ARPMessage::OPCODE_REQUEST;
  arp.sender_ethernet_address = ethernet_address_;
  arp.target_ethernet_address = { 0, 0, 0, 0, 0, 0 };
  arp.sender_ip_address = ip_address_.ipv4_numeric();
  arp.target_ip_address = next_hop_ip;
  frame.payload = serialize( arp );
  ready_to_send_.push( frame );
  wait_for_reply_.push( ARP_query { next_hop_ip, frame, cur_ms } );
  data_wait_for_ARP_reply_[next_hop_ip] = dgram;
}

// frame: the incoming Ethernet frame
// This method is called when an Ethernet frame arrives from the network. The code
// should ignore any frames not destined for the network interface (meaning, the Ethernet
// destination is either the broadcast address or the interface’s own Ethernet address
// stored in the ethernet address member variable).
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  //   If the inbound frame is IPv4, parse the payload as an InternetDatagram and,
  // if successful (meaning the parse() method returned true), return the resulting
  // InternetDatagram to the caller.
  if ( frame.header.type == EthernetHeader::TYPE_IPv4 ) {
    InternetDatagram dgram;
    if ( frame.header.dst == ethernet_address_ && parse( dgram, frame.payload ) ) {
      return dgram;
    }
  }
  //   If the inbound frame is ARP, parse the payload as an ARPMessage and, if successful,
  // remember the mapping between the sender’s IP address and Ethernet address for
  // 30 seconds. (Learn mappings from both requests and replies.) In addition, if it’s
  // an ARP request asking for our IP address, send an appropriate ARP reply
  else if ( frame.header.type == EthernetHeader::TYPE_ARP ) {
    ARPMessage arp;
    if ( parse( arp, frame.payload ) && arp.target_ip_address == ip_address_.ipv4_numeric() ) {
      ARP_dictionary_[arp.sender_ip_address] = make_pair( arp.sender_ethernet_address, cur_ms );
      if ( arp.opcode == ARPMessage::OPCODE_REQUEST ) {
        send_ARP_reply( arp.sender_ip_address, arp.sender_ethernet_address );
      } else if ( arp.opcode == ARPMessage::OPCODE_REPLY ) {
        IPAddr32 target_ip = arp.sender_ip_address;
        if ( data_wait_for_ARP_reply_.find( target_ip ) != data_wait_for_ARP_reply_.end() ) {
          send_Ethernet_frame( data_wait_for_ARP_reply_[target_ip], target_ip );
          data_wait_for_ARP_reply_.erase( target_ip );
        }
      }
    }
  }
  return nullopt;
}

void NetworkInterface::send_ARP_reply( const IPAddr32& target_ip, const EthernetAddress& target_ethernet_address )
{
  EthernetFrame frame;
  frame.header.type = EthernetHeader::TYPE_ARP;
  frame.header.src = ethernet_address_;
  frame.header.dst = target_ethernet_address;
  ARPMessage arp;
  arp.opcode = ARPMessage::OPCODE_REPLY;
  arp.sender_ethernet_address = ethernet_address_;
  arp.target_ethernet_address = target_ethernet_address;
  arp.sender_ip_address = ip_address_.ipv4_numeric();
  arp.target_ip_address = target_ip;
  frame.payload = serialize( arp );
  ready_to_send_.push( frame );
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
// This is called as time passes. Expire any IP-to-Ethernet mappings that have expired
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  cur_ms += ms_since_last_tick;
  while ( !wait_for_reply_.empty() && cur_ms - wait_for_reply_.front().time > RETRANSMIT_ARP_TIMER ) {
    ARP_query query = wait_for_reply_.front();
    wait_for_reply_.pop();
    auto query_ip = query.ip;
    auto query_frame = query.frame;
    if ( ARP_dictionary_.find( query_ip ) != ARP_dictionary_.end()
         && cur_ms - ARP_dictionary_[query_ip].second <= UPDATE_DICTIONARY_TIMER ) {
      continue;
    } else {
      ready_to_send_.push( query_frame );
      wait_for_reply_.push( ARP_query { query_ip, query_frame, cur_ms } );
    }
  }
}

optional<EthernetFrame> NetworkInterface::maybe_send()
{
  if ( ready_to_send_.empty() ) {
    return nullopt;
  }
  EthernetFrame frame = ready_to_send_.front();
  ready_to_send_.pop();
  return frame;
}
