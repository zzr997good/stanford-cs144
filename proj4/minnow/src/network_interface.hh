#pragma once

#include "address.hh"
#include "ethernet_frame.hh"
#include "ipv4_datagram.hh"

#include <iostream>
#include <list>
#include <optional>
#include <queue>
#include <unordered_map>
#include <utility>
// A "network interface" that connects IP (the internet layer, or network layer)
// with Ethernet (the network access layer, or link layer).

// This module is the lowest layer of a TCP/IP stack
// (connecting IP with the lower-layer network protocol,
// e.g. Ethernet). But the same module is also used repeatedly
// as part of a router: a router generally has many network
// interfaces, and the router's job is to route Internet datagrams
// between the different interfaces.

// The network interface translates datagrams (coming from the
// "customer," e.g. a TCP/IP stack or router) into Ethernet
// frames. To fill in the Ethernet destination address, it looks up
// the Ethernet address of the next IP hop of each datagram, making
// requests with the [Address Resolution Protocol](\ref rfc::rfc826).
// In the opposite direction, the network interface accepts Ethernet
// frames, checks if they are intended for it, and if so, processes
// the the payload depending on its type. If it's an IPv4 datagram,
// the network interface passes it up the stack. If it's an ARP
// request or reply, the network interface processes the frame
// and learns or replies as necessary.
using IPAddr32 = uint32_t;

struct ARP_query
{
  IPAddr32 ip;
  EthernetFrame frame;
  uint64_t time;
};

class NetworkInterface
{
private:
  // Ethernet (known as hardware, network-access, or link-layer) address of the interface
  EthernetAddress ethernet_address_;

  // IP (known as Internet-layer or network-layer) address of the interface
  Address ip_address_;

  // Timer
  uint64_t cur_ms { 0 };

  static constexpr uint64_t UPDATE_DICTIONARY_TIMER = 30000; // arp_list updated time
  static constexpr uint64_t RETRANSMIT_ARP_TIMER = 5000;     // ARP request retransmit time
  // Map of IP addresses to datagrams waiting for ARP replies
  std::unordered_map<IPAddr32, std::pair<EthernetAddress, uint64_t>> ARP_dictionary_ {};

  // Queue of Ethernet frames awaiting transmission
  std::queue<EthernetFrame> ready_to_send_ {};

  // Queue of ARP frames awaiting reply
  std::queue<ARP_query> wait_for_reply_ {};

  // IP data waiting for ARP reply to be sent
  std::unordered_map<IPAddr32, InternetDatagram> data_wait_for_ARP_reply_ {};

public:
  // Construct a network interface with given Ethernet (network-access-layer) and IP (internet-layer)
  // addresses
  NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address );

  // Access queue of Ethernet frames awaiting transmission
  std::optional<EthernetFrame> maybe_send();

  // Sends an IPv4 datagram, encapsulated in an Ethernet frame (if it knows the Ethernet destination
  // address). Will need to use [ARP](\ref rfc::rfc826) to look up the Ethernet destination address
  // for the next hop.
  // ("Sending" is accomplished by making sure maybe_send() will release the frame when next called,
  // but please consider the frame sent as soon as it is generated.)
  void send_datagram( const InternetDatagram& dgram, const Address& next_hop );

  // Receives an Ethernet frame and responds appropriately.
  // If type is IPv4, returns the datagram.
  // If type is ARP request, learn a mapping from the "sender" fields, and send an ARP reply.
  // If type is ARP reply, learn a mapping from the "sender" fields.
  std::optional<InternetDatagram> recv_frame( const EthernetFrame& frame );

  // Called periodically when time elapses
  void tick( size_t ms_since_last_tick );

  void send_ARP_request( const InternetDatagram& dgram, const IPAddr32& next_hop_ip );
  void send_Ethernet_frame( const InternetDatagram& dgram, const IPAddr32& next_hop_ip );
  void send_ARP_reply( const IPAddr32& target_ip, const EthernetAddress& target_ethernet_address );
};
