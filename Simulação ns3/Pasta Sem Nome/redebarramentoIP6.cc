/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
//################################
#include "ns3/csma-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include <fstream>


#include "ns3/internet-apps-module.h"

// Default Network Topology
//
//    n1   n2   n3   n4
//     |    |    |    |
//     ================
//       LAN 10.1.2.0

using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("BarramentoIPv6");

int
main (int argc, char *argv[])
{
  
  bool verbose = false;
  uint32_t nCsma = 4;
  uint32_t erroRate = 0.00015;
  double interval = 0.001;
  uint32_t maxPacket = 5;//10000000;/*maximum number of packets sent in the simulation time.*/
  uint32_t payloadSizeVoip1 = 65;//default VoIP payload size.


  CommandLine cmd;
  cmd.AddValue ("nPacket", "Number of \"extra\" CSMA nodes/devices", maxPacket);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc, argv);

  if (verbose)
  {
   
      //LogComponentEnable("Ipv6Interface", LOG_LEVEL_ALL);
      LogComponentEnable("Ping6Application", LOG_LEVEL_INFO);
      //LogComponentEnable ("BarramentoIPv6", LOG_LEVEL_INFO);
      //LogComponentEnable ("Ipv6EndPointDemux", LOG_LEVEL_ALL);
      //LogComponentEnable ("Ipv6L3Protocol", LOG_LEVEL_ALL);
      //LogComponentEnable ("Ipv6StaticRouting", LOG_LEVEL_ALL);
      //LogComponentEnable ("Ipv6ListRouting", LOG_LEVEL_ALL);
      //LogComponentEnable ("Ipv6Interface", LOG_LEVEL_INFO);
      //LogComponentEnable ("Icmpv6L4Protocol", LOG_LEVEL_INFO);
     //LogComponentEnable ("Ping6Application", LOG_LEVEL_ALL);
     //LogComponentEnable ("NdiscCache", LOG_LEVEL_ALL);*/
  }

  nCsma = nCsma == 0 ? 1 : nCsma;

  //declaramos outro NodeContainer para manter os nós que serão parte da rede em barramento (CSMA)
  NS_LOG_INFO ("Creating Topology");
  NodeContainer csmaNodes;
  csmaNodes.Create (nCsma);
  
  //instanciamos um NetDeviceContainer para gerenciar os dispositivos ponto-a-ponto e então       Instalamos os dispositivos nos nós ponto-a-
  //ponto.

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("1Gbps"));
  csma.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);
  
  Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  em->SetAttribute ("ErrorRate", DoubleValue (erroRate));
  csmaDevices.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

  /* Install IPv4/IPv6 stack */
  
  InternetStackHelper internetv6;
  internetv6.SetIpv4StackInstall (false);
  internetv6.Install (csmaNodes);
 
  NS_LOG_INFO ("Assign IPv6 Addresses.");
  Ipv6AddressHelper ipv6;
  Ipv6InterfaceContainer csmaInterfaces = ipv6.Assign (csmaDevices);

  NS_LOG_INFO ("Create Applications.");

  //Create a Ping6 application to send ICMPv6 echo request from node zero to all-nodes (ff02::1).
  
  Time interPacketInterval = Seconds (interval);
  Ping6Helper ping6;
  
  ping6.SetLocal (csmaInterfaces.GetAddress (0, 1)); 
  ping6.SetRemote (csmaInterfaces.GetAddress (nCsma-1, 1));

  /*
  ping6.SetIfIndex (csmaInterfaces.GetInterfaceIndex (0));
  ping6.SetRemote (Ipv6Address::GetAllNodesMulticast ());
  */

  ping6.SetAttribute ("MaxPackets", UintegerValue (maxPacket));
  ping6.SetAttribute ("Interval", TimeValue (interPacketInterval));
  ping6.SetAttribute ("PacketSize", UintegerValue (payloadSizeVoip1));
  ApplicationContainer apps = ping6.Install (csmaNodes.Get (0));
  apps.Start (Seconds (2.0));
  apps.Stop (Seconds (10.0));

  //salvando dados do trafego no ping6.tr

  AsciiTraceHelper ascii;
  csma.EnableAscii (ascii.CreateFileStream ("./tests/ping6.tr"),csmaNodes.Get (nCsma-1)->GetId (), 0);    
  csma.EnablePcap ("ping6", csmaNodes.Get (nCsma-1)->GetId (), 0);
  csma.EnablePcap ("ping6", csmaNodes.Get (0)->GetId (), 0);

  //csma.EnablePcapAll (std::string ("./tests/ping6"), true);
  //csma.EnablePcap ("ping6", csmaNodes.Get (0)->GetId (), 0, false);
  //csma.EnablePcapAll (std::string ("./tests/ping6"), true);
  

 

  
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
