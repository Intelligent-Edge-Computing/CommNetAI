#include "test-tag.h"

#include "ProcessTimeTag.h"
#include "process-tag.h"

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
using namespace ns3;

// 定义日志组件
NS_LOG_COMPONENT_DEFINE("MySimulation");

void ReceivePacketCallback(Ptr<Socket> socket)
{
    Ptr<Packet> packet = socket->Recv();
    // 检查包上是否有 MyFlowIdTag
    // MyFlowIdTag tag;
    // if (packet->PeekPacketTag(tag))
    // {
    //     NS_LOG_UNCOND("Received packet with FlowId: " << tag.GetFlowId());
    // }
    // else
    // {
    //     NS_LOG_UNCOND("No MyFlowIdTag found in the packet.");
    // }
    // 尝试读取标签
    QueueProcessTimeTag timeTag;
    if (packet->PeekPacketTag(timeTag))
    {
        timeTag.SetFinishTime(Simulator::Now());
        ns3::Time enqueueTime = timeTag.GetEnqueueTime();
        ns3::Time finishTime = timeTag.GetFinishTime();
        NS_LOG_INFO("Enqueue Time: " << enqueueTime.GetSeconds() << ", Finish Time: " << finishTime.GetSeconds());

        packet->ReplacePacketTag(timeTag);
        packet->PeekPacketTag(timeTag);

        enqueueTime = timeTag.GetEnqueueTime();
        finishTime = timeTag.GetFinishTime();
        NS_LOG_INFO("Enqueue Time: " << enqueueTime.GetSeconds() << ", Finish Time: " << finishTime.GetSeconds());

    }
    else
    {
        NS_LOG_WARN("No QueueProcessTimeTag found in packet.");
    }

}

int main1(int argc, char *argv[])
{
    // 启用日志
    LogComponentEnable("MySimulation", LOG_LEVEL_INFO);

    // 创建两个节点
    NodeContainer nodes;
    nodes.Create(2);

    // 设置 Point-to-Point 链路
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    // 安装设备和协议栈
    NetDeviceContainer devices = pointToPoint.Install(nodes);
    InternetStackHelper stack;
    stack.Install(nodes);
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // 创建服务器 Socket
    Ptr<Socket> serverSocket = Socket::CreateSocket(nodes.Get(1), TypeId::LookupByName("ns3::UdpSocketFactory"));
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), 9);  // 绑定到端口 9
    serverSocket->Bind(local);
    serverSocket->SetRecvCallback(MakeCallback(&ReceivePacketCallback));
    NS_LOG_UNCOND("Server socket created and bound to port 9.");

    // 创建客户端 Socket
    Ptr<Socket> clientSocket = Socket::CreateSocket(nodes.Get(0), TypeId::LookupByName("ns3::UdpSocketFactory"));
    InetSocketAddress remote = InetSocketAddress(interfaces.GetAddress(1), 9);  // 连接到服务器的端口 9
    clientSocket->Connect(remote);

    // 发送数据包
    Simulator::Schedule(Seconds(2.0), [&]() {
        Ptr<Packet> packet = Create<Packet>(1024);
        // MyFlowIdTag flowTag;
        // flowTag.SetFlowId(12345);  // 设置 FlowId
        // 创建并设置标签
       EventTimeTag timeTag;
       timeTag.SetEnqueueTime(Simulator::Now());  // 记录当前时间
       packet->AddPacketTag(timeTag);  // 添加标签到包上
        // packet->AddPacketTag(flowTag);   // 将 Tag 添加到数据包上
       clientSocket->Send(packet);
        NS_LOG_UNCOND("Client sent packet to server.");
    });

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}