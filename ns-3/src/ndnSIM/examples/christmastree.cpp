/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/netanim-module.h"  // Add NetAnim module

namespace ns3 {


int
main(int argc, char* argv[])
{
  // setting default parameters for PointToPoint links and channels
  Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
  Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
  Config::SetDefault("ns3::DropTailQueue::MaxPackets", StringValue("20"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse(argc, argv);

  // Creating nodes
  NodeContainer nodes;
  nodes.Create(5);

  // Connecting nodes using two links
  PointToPointHelper p2p;
  p2p.Install(nodes.Get(0), nodes.Get(1));
  p2p.Install(nodes.Get(1), nodes.Get(2));
  p2p.Install(nodes.Get(1), nodes.Get(3));
  p2p.Install(nodes.Get(1), nodes.Get(4));



  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetDefaultRoutes(true);
  ndnHelper.InstallAll();

  // Choosing forwarding strategy
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/multicast");

  // Installing applications

  // Consumer
  ndn::AppHelper consumerHelper("ns3::ndn::Subscriber");
  // Consumer will request /prefix/0, /prefix/1, ...
  consumerHelper.SetPrefix("/icens/powerlevel");
  consumerHelper.SetAttribute("TxTimer", StringValue("20")); //resend subscription interest every 20 seconds
  consumerHelper.SetAttribute("DataRate", IntegerValue(10)); // Set data rate to 10 packets per second
  consumerHelper.Install(nodes.Get(2));                        // first node
  consumerHelper.Install(nodes.Get(3));                        
  consumerHelper.Install(nodes.Get(4));                        
  // Producer
  ndn::AppHelper producerHelper("ns3::ndn::SpontaneousProducer");
  // Producer will reply to all requests starting with /prefix
  producerHelper.SetPrefix("/icens/powerlevel");
  producerHelper.SetAttribute("Frequency", StringValue("1")); //how many seconds to wait before sending data for subscription interest
  producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
  producerHelper.Install(nodes.Get(0)); // last node

  // Set up NetAnim node positions for visualization
  AnimationInterface::SetConstantPosition(nodes.Get(2), 50, 20);   // Consumer position
  AnimationInterface::SetConstantPosition(nodes.Get(3), 140, 20);   // Consumer position
  AnimationInterface::SetConstantPosition(nodes.Get(4), 230, 20);   // Consumer position
  AnimationInterface::SetConstantPosition(nodes.Get(1), 140, 110);  // Router position  
  AnimationInterface::SetConstantPosition(nodes.Get(0), 140, 200);  // Producer position

  // Create animation interface - this will generate the XML file for NetAnim
  AnimationInterface anim("christmastree.xml"); 

  Simulator::Stop(Seconds(20.0));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
