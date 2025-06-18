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
  ndn::StrategyChoiceHelper::InstallAll("/prefix", "/localhost/nfd/strategy/best-route/%FD%01");

  // Installing applications

  // Consumer 1 - Low DataRate, starts early
  ndn::AppHelper app1("ns3::ndn::Subscriber");
  app1.SetPrefix("/icens/powerlevel");
  app1.SetAttribute("TxTimer", StringValue("20")); // resend subscription interest every 20 seconds
  app1.SetAttribute("DataRate", IntegerValue(5)); // Low rate - 5 packets per second
  app1.SetAttribute("Subscription", IntegerValue(2)); // Hard subscription
  ApplicationContainer consumer1App = app1.Install(nodes.Get(2));
  consumer1App.Start(Seconds(1.0)); // Start at t=1s
  consumer1App.Stop(Seconds(20.0));

/*
  // Log when consumer1 starts and stops
  Simulator::Schedule(Seconds(1.0), [](){
    std::cout << "[" << Simulator::Now().GetSeconds() << "s] *** CONSUMER1 STARTED *** Node=2, DataRate=5 pps" << std::endl;
  });
  Simulator::Schedule(Seconds(20.0), [](){
    std::cout << "[" << Simulator::Now().GetSeconds() << "s] *** CONSUMER1 STOPPED *** Node=2" << std::endl;
  });
*/

  // Consumer 2 - High DataRate, starts later
  ndn::AppHelper app2("ns3::ndn::Subscriber");
  app2.SetPrefix("/icens/powerlevel");
  app2.SetAttribute("TxTimer", StringValue("20"));
  app2.SetAttribute("DataRate", IntegerValue(15)); // High rate - 15 packets per second
  app2.SetAttribute("Subscription", IntegerValue(2)); // Hard subscription
  ApplicationContainer consumer2App = app2.Install(nodes.Get(3));
  consumer2App.Start(Seconds(5.0)); // Start at t=5s (should trigger rate increase)
  consumer2App.Stop(Seconds(20.0));

/*
  // Log when consumer2 starts and stops
  Simulator::Schedule(Seconds(5.0), [](){
    std::cout << "[" << Simulator::Now().GetSeconds() << "s] *** CONSUMER2 STARTED *** Node=3, DataRate=15 pps" << std::endl;
  });
  Simulator::Schedule(Seconds(20.0), [](){
    std::cout << "[" << Simulator::Now().GetSeconds() << "s] *** CONSUMER2 STOPPED *** Node=3" << std::endl;
  });
*/

  // Consumer 3 - Medium DataRate, starts even later
  ndn::AppHelper app3("ns3::ndn::Subscriber");
  app3.SetPrefix("/icens/powerlevel");
  app3.SetAttribute("TxTimer", StringValue("20"));
  app3.SetAttribute("DataRate", IntegerValue(10)); // Medium rate - 10 packets per second
  app3.SetAttribute("Subscription", IntegerValue(2)); // Hard subscription
  ApplicationContainer consumer3App = app3.Install(nodes.Get(4));
  consumer3App.Start(Seconds(8.0)); // Start at t=8s (should NOT trigger rate change, 10 < 15)
  consumer3App.Stop(Seconds(20.0));
/*
  // Log when consumer3 starts and stops
  Simulator::Schedule(Seconds(8.0), [](){
    std::cout << "[" << Simulator::Now().GetSeconds() << "s] *** CONSUMER3 STARTED *** Node=4, DataRate=10 pps" << std::endl;
  });
  Simulator::Schedule(Seconds(20.0), [](){
    std::cout << "[" << Simulator::Now().GetSeconds() << "s] *** CONSUMER3 STOPPED *** Node=4" << std::endl;
  });
*/

  // TEST CASE: Two consumers starting almost simultaneously (uncomment to test race condition)
  /*
  ndn::AppHelper app4("ns3::ndn::Subscriber");
  app4.SetPrefix("/icens/powerlevel");
  app4.SetAttribute("TxTimer", StringValue("20"));
  app4.SetAttribute("DataRate", IntegerValue(20)); // Very high rate
  app4.SetAttribute("Subscription", IntegerValue(2));
  ApplicationContainer consumer4App = app4.Install(nodes.Get(2)); // Same node as consumer1
  consumer4App.Start(Seconds(12.000)); // Almost same time as potential consumer5
  consumer4App.Stop(Seconds(20.0));

  ndn::AppHelper app5("ns3::ndn::Subscriber");
  app5.SetPrefix("/icens/powerlevel");
  app5.SetAttribute("TxTimer", StringValue("20"));
  app5.SetAttribute("DataRate", IntegerValue(25)); // Even higher rate
  app5.SetAttribute("Subscription", IntegerValue(2));
  ApplicationContainer consumer5App = app5.Install(nodes.Get(3)); // Same node as consumer2
  consumer5App.Start(Seconds(12.001)); // 1ms later - test race condition
  consumer5App.Stop(Seconds(20.0));
  */

  // Producer
  ndn::AppHelper producerApp("ns3::ndn::SpontaneousProducer");
  producerApp.SetPrefix("/icens/powerlevel");
  producerApp.SetAttribute("Frequency", StringValue("1")); // Initial frequency (will be overridden by DataRate)
  producerApp.SetAttribute("PayloadSize", StringValue("1024"));
  ApplicationContainer producer = producerApp.Install(nodes.Get(0));
  producer.Start(Seconds(0.0)); // Producer starts immediately
  producer.Stop(Seconds(20.0));

  // Set up NetAnim node positions for visualization
  AnimationInterface::SetConstantPosition(nodes.Get(2), 50, 20);    // Consumer 1 position
  AnimationInterface::SetConstantPosition(nodes.Get(3), 140, 20);   // Consumer 2 position
  AnimationInterface::SetConstantPosition(nodes.Get(4), 230, 20);   // Consumer 3 position
  AnimationInterface::SetConstantPosition(nodes.Get(1), 140, 110);  // Router position  
  AnimationInterface::SetConstantPosition(nodes.Get(0), 140, 200);  // Producer position

  // Create animation interface - this will generate the XML file for NetAnim
  AnimationInterface anim("christmastree.xml"); 

  // Enable logging for debugging
  LogComponentEnable("ndn.Subscriber", LOG_LEVEL_INFO);
  LogComponentEnable("ndn.SpontaneousProducer", LOG_LEVEL_INFO);
  // LogComponentEnable("nfd.Forwarder", LOG_LEVEL_DEBUG);

 // std::cout << "=== Starting Simulation ===" << std::endl;

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
