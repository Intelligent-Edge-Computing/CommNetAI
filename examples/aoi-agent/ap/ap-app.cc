#include "ap-app.h"

#include "../event-time-tag.h"
#include "../json.hpp"
#include "../packet-type-tag.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
using json = nlohmann::json;

#include "../status-util.h"

#include <sstream> // 添加头文件

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ApApp");
NS_OBJECT_ENSURE_REGISTERED(ApApp);

ApApp::ApApp() : m_socket(0), m_logger(ConfigManager::GetConfig()["dbPath"]) {}

ApApp::~ApApp() { m_socket = 0; }
TypeId ApApp::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::ApApp")
        .SetParent<Application>()
        .SetGroupName("Tutorial")
        .AddConstructor<ApApp>()
        // 注册 TraceSource
        .AddTraceSource("APReceiveServerStatusPacketTrace",
                        "Triggered when a status packet is received and processed.",
                        MakeTraceSourceAccessor(&ApApp::m_APReceiveServerStatusPacketTrace),
                        "ns3::ApApp::ApTracedCallback")
        .AddTraceSource("APForwardRequestToServerTrace",
                        "Triggered when user's request packet is forwarded to server.",
                        MakeTraceSourceAccessor(&ApApp::m_APFowardRequestToServerTrace),
                        "ns3::Packet::TracedCallback")
        .AddTraceSource("APReceiveRequestFromUserTrace",
                        "Triggered when user's request packet being received.",
                        MakeTraceSourceAccessor(&ApApp::m_APReceiveRequestFromUserTrace),
                        "ns3::Packet::TracedCallback")
        .AddTraceSource("APReceiveResponseFromServerTrace",
                        "Triggered when user's request packet being responded.",
                        MakeTraceSourceAccessor(&ApApp::m_APReceiveResponseFromServerTrace),
                        "ns3::Packet::TracedCallback")
        .AddTraceSource("APDropRequestTrace",
                        "Triggered when user's request packet being dropped.",
                        MakeTraceSourceAccessor(&ApApp::m_APDropRequestPacketTrace),
                        "ns3::Packet::TracedCallback");
    ;
    return tid;
}

void ApApp::Setup(Address serverAddress, uint16_t serverPort, uint16_t clientPort)
{
    m_serverAddress = serverAddress;
    m_serverPort = serverPort;
    m_clientPort = clientPort;
}

void ApApp::StartApplication(void)
{
    m_socket = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::UdpSocketFactory"));
    m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_clientPort));
    m_socket->SetRecvCallback(MakeCallback(&ApApp::HandleRead, this));
}

void ApApp::StopApplication(void)
{
    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());

    }
}

bool statusUpdateDecision(const Status & oldStatus, const Status & newStatus)
{//该函数可基于价值实现。目前是时间。
    return newStatus.generatedTime > oldStatus.generatedTime;
}
void ApApp::HandleRead(Ptr<Socket> socket)
{
    Ptr<Packet> receivedNewPacket;
    Address from;
    while ((receivedNewPacket = socket->RecvFrom(from)))
    {
        PacketTypeTag packetTypeTag;
        if (receivedNewPacket->PeekPacketTag(packetTypeTag))
        {
            if(packetTypeTag.GetPacketType() == PacketType::SERVICE_STATUS)
            {
                EventTimeTag timeTag;
                if (receivedNewPacket->PeekPacketTag(timeTag))
                {
                    timeTag.SetServerStatusReceiveTime(Simulator::Now());
                    receivedNewPacket->ReplacePacketTag(timeTag); // 将时间标签添加到包上
                }
                Status newStatus = extractStatus(receivedNewPacket);
                if(statusUpdateDecision(m_storedStatus, newStatus)) //收到了更加新鲜的status。
                {
                    if(m_storedStatus.generatedTime != -1)
                    {
                        //Using callback to log Age.
                        //For the first status, we ignored. Because the timeline origin is started from the 1st status.
                        m_APReceiveServerStatusPacketTrace(m_storedStatusPacket,receivedNewPacket);
                    }
                    m_storedStatus = newStatus;
                    m_storedStatusPacket = receivedNewPacket;
                }
                NS_LOG_DEBUG("AP: Received Server Status, PacketID=" << m_storedStatusPacket->GetUid()
                    <<" at Time=" << Simulator::Now().As(Time::S));
            }
            else if(packetTypeTag.GetPacketType() == PacketType::USER_REQUEST)
            {
                m_APReceiveRequestFromUserTrace(receivedNewPacket);
                //当前AP节点已经有status且server有空闲资源
                if (m_storedStatus.generatedTime !=-1 && m_storedStatus.capacity > 0)
                {
                    //TODO: 读取status并判断server端是否可以被访问
                    //forwarding request to server.
                    // 立刻转发消息到Server
                    Status newStatus = extractStatus(receivedNewPacket);
                    NS_LOG_DEBUG("AP: Received request (PacketID=" << receivedNewPacket->GetUid()
                        << ") from client at Time=" << Simulator::Now().As(Time::S));
                    std::ostringstream oss;
                    oss << InetSocketAddress::ConvertFrom(from).GetIpv4();
                    newStatus.message =  "ip: " + oss.str();
                    std::string jsonString = serialize(newStatus);
                    Ptr<Packet> requestPacketToBeForward = Create<Packet>(reinterpret_cast<const uint8_t*> (jsonString.c_str()), jsonString.size());
                    requestPacketToBeForward->AddPacketTag(packetTypeTag);

                    EventTimeTag requestEventTimeTag;
                    if (receivedNewPacket->PeekPacketTag(requestEventTimeTag))
                    {
                        EventTimeTag storedProcessTimeTag;
                        if (m_storedStatusPacket->PeekPacketTag(storedProcessTimeTag))
                        {
                            storedProcessTimeTag.campareAndCopyTo(requestEventTimeTag);
                        }
                        requestEventTimeTag.SetRequestAPReceivedTime(Simulator::Now());
                        //Note：若AP有request调度功能，可设置request从AP被Forward的时间 `SetRequestAPForwardTime`
                        requestEventTimeTag.SetRequestAPForwardTime(Simulator::Now());
                        requestPacketToBeForward->AddPacketTag(requestEventTimeTag);
                    }
                    socket->SendTo(requestPacketToBeForward, 0, InetSocketAddress(Ipv4Address::ConvertFrom(m_serverAddress), m_serverPort));
                    NS_LOG_INFO("[AP] Request packet (PacketID=" << receivedNewPacket->GetUid() <<
                        ") transformed to packet (PacketID=" << requestPacketToBeForward->GetUid() <<
                        ") at Time=" << Simulator::Now().As(Time::S) <<". Status ref. PacketID=" <<  m_storedStatusPacket->GetUid() );
                    // 触发 TraceSource 回调，传递包
                    m_APFowardRequestToServerTrace(requestPacketToBeForward);
                }
                else
                {
                    m_APDropRequestPacketTrace(receivedNewPacket);
                    NS_LOG_INFO("[AP] Drop request (PacketID=" << receivedNewPacket->GetUid() << ") from "
                        << InetSocketAddress::ConvertFrom(from).GetIpv4() << " at Time=" << Simulator::Now().As(Time::S));
                }
            }
            else if(packetTypeTag.GetPacketType() == PacketType::SERVICE_RESPONSE)
            {
                // Status newStatus = extractStatus(receivedNewPacket);
                m_APReceiveResponseFromServerTrace(receivedNewPacket);
                NS_LOG_INFO("AP: Received response (PacketID=" << receivedNewPacket->GetUid() << ") from server at Time=" << Simulator::Now().As(Time::S));
            }
        }
    }
}
Status ApApp::extractStatus(const Ptr<Packet> packet)
{

    uint32_t packetSize = packet->GetSize();
    uint8_t buffer[1024];

    std::string msg;

    if (packetSize <= sizeof(buffer)) {
        // 如果数据包大小小于等于 1024 字节，使用固定大小的 buffer
        packet->CopyData(buffer, packetSize);
        msg = std::string(reinterpret_cast<char *>(buffer), packetSize);
    } else {
        // 如果数据包大小超过 1024 字节，使用动态分配的 buffer
        std::unique_ptr<uint8_t[]> largeBuffer(new uint8_t[packetSize]);
        packet->CopyData(largeBuffer.get(), packetSize);
        msg = std::string(reinterpret_cast<char *>(largeBuffer.get()), packetSize);
    }
    Status status = deserialize(msg);
    return status;
}
} // namespace ns3