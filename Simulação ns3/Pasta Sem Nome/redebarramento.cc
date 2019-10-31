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

// Default Network Topology
//
//    n1   n2   n3   n4
//     |    |    |    |
//     ================
//       LAN 10.1.2.0

using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("BarramentoIPv4");

int
main (int argc, char *argv[])
{
 //voip 6553 
  bool verbose = false;
  uint32_t nCsma = 4;
  uint32_t erroRate = 0.00015;
  double interval = 0.001;
  uint32_t maxPacket = 5;//10000000;/*maximum number of packets sent in the simulation time.*/
  uint32_t payloadSizeVoip1 = 65;//default VoIP payload size.

  CommandLine cmd;
  cmd.AddValue ("nPacket", "Number of \"extra\" CSMA nodes/devices", maxPacket);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("payloadSize", "size of application packet sent", payloadSizeVoip1);


  cmd.Parse (argc, argv);

  if (verbose)
  {
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
  }

  nCsma = nCsma == 0 ? 1 : nCsma;  
 
  //declaramos outro NodeContainer para manter os nós que serão parte da rede em barramento (CSMA)
  
  NS_LOG_INFO ("Creating Topology");
  NodeContainer csmaNodes;
  csmaNodes.Create (nCsma);

  //instanciamos um NetDeviceContainer para gerenciar os dispositivos ponto-a-ponto e então Instalamos os dispositivos nos nós ponto-a-ponto.
  
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("1Gbps"));
  csma.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  em->SetAttribute ("ErrorRate", DoubleValue (erroRate));
  csmaDevices.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

  //Assim como criamos um NetDeviceContainer para manter os dispositivos criados pelo PointToPointHelper, criamos um NetDeviceContainer para 
  //gerenciar os dispositivos criados pelo nosso CsmaHelper. Chamamos o método Install do CsmaHelper para instalar os dispositivos nos nós do 
  //csmaNodes NodeContainer.
  
  InternetStackHelper stack;
  stack.Install (csmaNodes);
  
  //usar o Ipv4AddressHelper para atribuir endereços IP para as interfaces de nossos dispositivos.

  Ipv4AddressHelper address;

  //Os dispositivos CSMA serão associados com endereços IP da rede 10.1.2.0
  
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);
  
  //vamos instanciar o servidor em um dos nós que tem um dispositivo CSMA e o cliente em um nó que tem apenas um 
  //dispositivo ponto-a-ponto. Primeiro, vamos configurar o servidor de eco.
  
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma-1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
  
  //Dizemos ao cliente para enviar pacotes para o servidor. Instalamos o cliente no nó ponto-a-ponto mais
  // à esquerda visto na ilustração da topologia.
  
  Time interPacketInterval = Seconds (interval);
  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma-1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (maxPacket));
  echoClient.SetAttribute ("Interval", TimeValue (interPacketInterval));
  echoClient.SetAttribute ("PacketSize", UintegerValue (payloadSizeVoip1));

  ApplicationContainer clientApps = echoClient.Install (csmaNodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
  


  //salvando dados do trafego no ping6.tr
  
  AsciiTraceHelper ascii;
  csma.EnableAscii (ascii.CreateFileStream ("./tests/ping4.tr"),csmaNodes.Get (nCsma-1)->GetId (), 0);    
  csma.EnablePcap ("ping4", csmaNodes.Get (nCsma-1)->GetId (), 0);
  csma.EnablePcap ("ping4", csmaNodes.Get (0)->GetId (), 0);

  //csma.EnablePcapAll (std::string ("./tests/ping6"), true);
  //csma.EnablePcap ("ping6", csmaNodes.Get (0)->GetId (), 0, false);
  //csma.EnablePcapAll (std::string ("./tests/ping6"), true);

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
