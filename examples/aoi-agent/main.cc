/*
 * Copyright (c) 2009 INRIA
 *
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
 *
 * Author: Mohamed Amine Ismail <amine.ismail@sophia.inria.fr>
 */

// Network topology
/*
*               [ Server ] -------------- (  f  ) -------------- [ AP ]
*                                                              /  |  \
*                                                             /   |   \  ...
*                                                             /    |    \
*                                                          [u₁]   [u₂]   [u₃]   [u₄]   [u₅]
*/

#include "ap-app.h"
#include "fwd-service.h" // 引入自定义队列
#include "server-app.h"
#include "status-util.h"
#include "user-app.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ssid.h"
#include "ns3/stats-module.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/yans-wifi-helper.h"

#include <cstdlib>
#include <ctime>
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CustomNetworkSimulation");

void setupNetwork(json& config)
{
    double stopTime = config["simulation"]["stopTime"];
    NodeContainer serverNode; // 0
    serverNode.Create(1);
    NodeContainer fNode; // 1
    fNode.Create(1);
    NodeContainer apNode; // 2
    apNode.Create(1);
    NodeContainer userNodes; // 3~7
    userNodes.Create(5);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
    // pointToPoint.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("50p"));  // 增大队列容量
    
    NetDeviceContainer serverTofDevices;
    serverTofDevices = pointToPoint.Install(serverNode.Get(0), fNode.Get(0));

    NetDeviceContainer fToApDevices;
    fToApDevices = pointToPoint.Install(fNode.Get(0), apNode.Get(0));
    InternetStackHelper stack;
    stack.Install(serverNode);
    stack.Install(fNode);
    stack.Install(apNode);
    stack.Install(userNodes);
    
         
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer serverTofInterfaces;
    serverTofInterfaces = address.Assign(serverTofDevices);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer fToApInterfaces;
    fToApInterfaces = address.Assign(fToApDevices);

    Ipv4Address serverAddress =serverTofInterfaces.GetAddress(0);
    Ipv4Address fNodeAddress =serverTofInterfaces.GetAddress(1);
    Ipv4Address apAddress =fToApInterfaces.GetAddress(1);

    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::MinstrelHtWifiManager");

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiMacHelper mac;
    Ssid ssid = Ssid("ns3-ssid");
    mac.SetType("ns3::StaWifiMac",
                "Ssid", SsidValue(ssid),
                "ActiveProbing", BooleanValue(false));

    NetDeviceContainer userDevices;
    userDevices = wifi.Install(phy, mac, userNodes);

    mac.SetType("ns3::ApWifiMac",
                "Ssid", SsidValue(ssid));

    NetDeviceContainer apDevice;
    apDevice = wifi.Install(phy, mac, apNode);

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer apInterface = address.Assign(apDevice);
    Ipv4InterfaceContainer userInterfaces = address.Assign(userDevices);

    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(5.0),
                                  "DeltaY", DoubleValue(10.0),
                                  "GridWidth", UintegerValue(3),
                                  "LayoutType", StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    mobility.Install(serverNode);
    mobility.Install(fNode);
    mobility.Install(apNode);
    mobility.Install(userNodes);
    
    uint16_t port = 9;
    uint32_t serverProcessingQueueCapacity = 2; // 设置队列容量

//     //创建背景流量--------------
// // Ptr<PoissonRandomVariable> packetInterval = CreateObject<PoissonRandomVariable>();
// // packetInterval->SetAttribute("Mean", DoubleValue(2.0)); // 平均到达率
// Ptr<NormalRandomVariable> packetInterval = CreateObject<NormalRandomVariable>();
// packetInterval->SetAttribute("Mean", DoubleValue(100)); // 平均大小
// packetInterval->SetAttribute("Variance", DoubleValue(50)); // 方差
// // Ptr<GammaRandomVariable> packetInterval = CreateObject<GammaRandomVariable>();
// // packetInterval->SetAttribute("Alpha", DoubleValue(2.0)); // 形状参数
// // packetInterval->SetAttribute("Beta", DoubleValue(1.0));  // 比例参数

    //--------------------- 创建 OnOffApplication 来模拟背景流量
    // uint16_t backgroundTrafficPort = 10;
    // OnOffHelper onoff("ns3::UdpSocketFactory", 
    //                 InetSocketAddress(apInterface.GetAddress(0), backgroundTrafficPort)); // 目标为 apNode 的IP地址和端口
    // onoff.SetAttribute("DataRate", StringValue("1Mbps")); // 设置发送速率
    // onoff.SetAttribute("PacketSize", UintegerValue(1024)); // 设置包大小
    // // 使用指数分布设置 OnTime 和 OffTime
    // onoff.SetAttribute("OnTime", StringValue("ns3::ExponentialRandomVariable[Mean=1]")); // 平均开启时间为1秒
    // onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=1]")); // 固定关闭时间1秒
    // // 将 OnOff 应用安装到 serverNode
    // ApplicationContainer apps = onoff.Install(serverNode.Get(0));
    // apps.Start(Seconds(1.0)); // 应用程序启动时间
    // apps.Stop(Seconds(stopTime));   // 应用程序停止时间
    // // 创建 PacketSink 应用来接收流量（相当于服务器）
    // PacketSinkHelper sink("ns3::UdpSocketFactory", 
    //                     InetSocketAddress(Ipv4Address::GetAny(), backgroundTrafficPort)); // 监听所有地址的指定端口
    // ApplicationContainer sinkApps = sink.Install(apNode.Get(0));
    // sinkApps.Start(Seconds(1.0)); // 接收器启动时间
    // sinkApps.Stop(Seconds(stopTime));   // 接收器停止时间
    //---------------------
    // 安装服务器应用程序，选择策略 FCFS 或 FCLS
    Ptr<ServerApp> serverApp = CreateObject<ServerApp>();
    serverApp->Setup(fNodeAddress, port, ServerApp::FCFS, serverProcessingQueueCapacity); // 选择 FCFS 或 FCLS 并设置队列容量
    serverNode.Get(0)->AddApplication(serverApp);
    serverApp->SetStartTime(Seconds(1.0));
    serverApp->SetStopTime(Seconds(stopTime));

    // 在fNode上安装FwdServiceRate应用程序
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");

    Ptr<Socket> recvSocket = Socket::CreateSocket(fNode.Get(0), tid);
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), port);
    recvSocket->Bind(local);

    Ptr<Socket> sendSocket = Socket::CreateSocket(fNode.Get(0), tid);
    InetSocketAddress remote = InetSocketAddress(apAddress, port); // Forward to Ap

    Ptr<FwdService> fwdApp = CreateObject<FwdService>();
    fwdApp->Setup(recvSocket, sendSocket, remote); // 服务处理延迟
    fNode.Get(0)->AddApplication(fwdApp);
    fwdApp->SetStartTime(Seconds(1.0));
    fwdApp->SetStopTime(Seconds(stopTime));

     // 在apNode上安装AP应用程序
    Ptr<ApApp> apApp = CreateObject<ApApp>();
    apApp->Setup(serverAddress, port, port);
    apNode.Get(0)->AddApplication(apApp);
    apApp->SetStartTime(Seconds(1.0));
    apApp->SetStopTime(Seconds(stopTime));

    for (uint32_t i = 0; i < userNodes.GetN(); ++i)
    {
        Ptr<UserApp> userApp = CreateObject<UserApp>();
        userApp->Setup(apInterface.GetAddress(0), port);
        userNodes.Get(i)->AddApplication(userApp);
        userApp->SetStartTime(Seconds(2.0 + rand() % 5));
        userApp->SetStopTime(Seconds(stopTime));
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();


}

void
fdwPacketProbeTrace(std::string context, double oldVal, double newVal)
{
    std::cout << "context: " << context << " old " << oldVal << " new " << newVal << std::endl;
}
int main(int argc, char *argv[])
{
    CommandLine cmd;
    cmd.Parse(argc, argv);

    LogComponentEnable("CustomNetworkSimulation", LOG_LEVEL_INFO);
    LogComponentEnable("ServerApp", LOG_LEVEL_DEBUG);
    LogComponentEnable("ApApp", LOG_LEVEL_INFO);
    LogComponentEnable("UserApp", LOG_LEVEL_INFO);
    // LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO); // 启用 OnOffApplication 的日志
    LogComponentEnable("FwdService", LOG_LEVEL_INFO);
    // LogComponentEnable("Logger", LOG_LEVEL_DEBUG);
    json& config = ConfigManager::GetConfig();
    
    uint32_t numRuns = config["simulation"]["numRuns"];
    uint32_t startRunId = config["simulation"]["startRunId"];
    uint32_t seed = 12345;

    for (uint32_t i = startRunId; i < numRuns; ++i)
    {
        // 设置种子和运行编号
        SeedManager::SetSeed(seed);
        SeedManager::SetRun(i);

        // 每次运行前重新设置网络
        setupNetwork(config);
        Simulator::Stop(Seconds(config["simulation"]["stopTime"]));
        NS_LOG_INFO("Results for run " << SeedManager::GetRun() << ". ");
        // double statusUpdateRate = config["server"]["PoisDist"]["statusUpdateRate"];
        // statusUpdateRate += 0.01;
        // config["server"]["PoisDist"]["statusUpdateRate"] = statusUpdateRate;


        DataCollector data;
        std::string updateProcess = config["server"]["stochasticProcess"];
        std::string processParameters = config["server"][updateProcess].dump();
        // std::string strategy = std::format("ServerStatusUpdateProcess={}({})", updateProcess, processParameters);
        data.DescribeRun("Test-" + std::to_string(i), "strategy", "input", std::to_string(i));
        data.AddMetadata("author", "jianpeng");
        // Ptr<PacketSizeMinMaxAvgTotalCalculator> appTxPkts = CreateObject<PacketSizeMinMaxAvgTotalCalculator>();
        // appTxPkts->SetKey("tx-pkt-size");
        // appTxPkts->SetContext("FWD");
        // Config::Connect("/NodeList/1/ApplicationList/0/$ns3::FwdService/Tx",
        //                 MakeCallback(&PacketSizeMinMaxAvgTotalCalculator::PacketUpdate, appTxPkts));
        // data.AddDataCalculator(appTxPkts);
        //AP接收到用户request的总数量
        Ptr<PacketCounterCalculator> appTxRequest = CreateObject<PacketCounterCalculator>();
        appTxRequest->SetKey("received-request-packets");
        appTxRequest->SetContext("Ap");
        Config::Connect("/NodeList/2/ApplicationList/0/$ns3::ApApp/APReceiveRequestFromUserTrace",
        MakeCallback(&PacketCounterCalculator::PacketUpdate, appTxRequest));
        data.AddDataCalculator(appTxRequest);
        //AP转发用户request的总数量
        Ptr<PacketCounterCalculator> apFwRequest = CreateObject<PacketCounterCalculator>();
        apFwRequest->SetKey("forwarded-request-packets");
        apFwRequest->SetContext("Ap");
        Config::Connect("/NodeList/2/ApplicationList/0/$ns3::ApApp/APForwardRequestToServerTrace",
        MakeCallback(&PacketCounterCalculator::PacketUpdate, apFwRequest));
        data.AddDataCalculator(apFwRequest);
        //服务端执行完的request总数量
        Ptr<PacketCounterCalculator> appRxResponse = CreateObject<PacketCounterCalculator>();
        appRxResponse->SetKey("responded-request-packets");
        appRxResponse->SetContext("Ap");
        Config::Connect("/NodeList/0/ApplicationList/0/$ns3::ServerApp/ServerReponseRequestPacketTrace",
        MakeCallback(&PacketCounterCalculator::PacketUpdate, appRxResponse));
        data.AddDataCalculator(appRxResponse);

        //
        // data.AddDataCalculator(appTx);
        Logger logger(ConfigManager::GetConfig()["dbPath"]);
        //Stage 1. Receive service status packet.
        Config::ConnectWithoutContext ("/NodeList/2/ApplicationList/0/$ns3::ApApp/APReceiveServerStatusPacketTrace",
                 MakeCallback (&Logger::APReceiveServerStatusPacketTrace, &logger));
        // Stage 2. Receive user's request and forward it to server.
        Config::ConnectWithoutContext ("/NodeList/2/ApplicationList/0/$ns3::ApApp/APForwardRequestToServerTrace",
                         MakeCallback (&Logger::APForwardRequestToServerTrace, &logger));
        //Stage 3. Server receives request.
        Config::ConnectWithoutContext ("/NodeList/0/ApplicationList/0/$ns3::ServerApp/ServerReceiveRequestPacketTrace",
                         MakeCallback (&Logger::ServerReceiveRequestPacketTrace, &logger));
        // Stage 4. Execute finished and sent the result.
        Config::ConnectWithoutContext ("/NodeList/0/ApplicationList/0/$ns3::ServerApp/ServerReponseRequestPacketTrace",
                         MakeCallback (&Logger::ServerReponseRequestPacketTrace, &logger));


        //results check

        Config::ConnectWithoutContext ("/NodeList/0/ApplicationList/0/$ns3::ServerApp/ServerReceiveRequestPacketTrace",
        MakeCallback (&Logger::CheckResultServerReceiveRequestPacketTrace,  &logger));//TP
        Config::ConnectWithoutContext ("/NodeList/0/ApplicationList/0/$ns3::ServerApp/ServerDropRequestPacketTrace",
                         MakeCallback (&Logger::CheckResultServerDropRequestPacketTrace, &logger));//FP

        Config::ConnectWithoutContext ("/NodeList/2/ApplicationList/0/$ns3::ApApp/APDropRequestTrace",
                         MakeCallback (&Logger::CheckResultAPDropRequestTrace,  &logger));//TN + FN

        // Config::ConnectWithoutContext ("/NodeList/2/ApplicationList/0/$ns3::ApApp/APReceiveResponseFromServerTrace",
        //                  MakeCallback (&Logger::APReceiveResponseFromServerTrace, &logger));

        Simulator::Run();

        Ptr<DataOutputInterface> output = nullptr;
        output = CreateObject<SqliteDataOutput>();
        if (output)
        {
            output->SetFilePrefix("/workspace/data/logs");
            output->Output(data);
        }
        Simulator::Destroy();
        NS_LOG_INFO("Results: TP=" << logger.TP << ", FP=" << logger.FP << ", TN=" << logger.TN << ", FN=" << logger.FN <<", Accuracy=" << 1.0*(logger.TP +logger.TN) / (logger.TP + logger.FP + logger.TN + logger.FN));
        logger.logAccuracy();
        logger.clear();

        double mean = config["server"]["PoisDist"]["statusUpdateRate"];
        double step =  config["server"]["PoisDist"]["step"];
        mean = mean + step;
        config["server"]["PoisDist"]["statusUpdateRate"] = mean;
        NS_LOG_INFO("Update Rate: " << config["server"]["PoisDist"]["statusUpdateRate"] << " times/s. ");

    }
    return 0;
}
