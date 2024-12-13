#include "user-app.h"

#include "event-time-tag.h"
#include "logger.h"
#include "packet-type-tag.h"
#include "status-util.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("UserApp");

UserApp::UserApp() : m_socket(0), m_logger(ConfigManager::GetConfig()["dbPath"]) {}

UserApp::~UserApp() { m_socket = 0; }

void UserApp::Setup(Address apAddress, uint16_t apPort)
{
    m_peerAddress = apAddress;
    m_peerPort = apPort;
}

void UserApp::StartApplication(void)
{
    m_socket = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::UdpSocketFactory"));
    m_socket->Bind();
    m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
    m_socket->SetRecvCallback(MakeCallback(&UserApp::HandleRead, this));
    // double lambda = 0.004; // λ表示请求的频率 (次/毫秒), 例如设置 λ = 0.004 表示每秒大约 4 次请求
    // expRand->SetAttribute("Mean", DoubleValue(1.0 / lambda)); // 平均时间间隔为 1/λ
    accessBehavior->SetAttribute("Mean", DoubleValue(250)); // 设置均值为250毫秒
    Simulator::Schedule(Seconds(1.0), &UserApp::SendRequest, this);
}

void UserApp::StopApplication(void)
{
    if (m_socket)
    {
        m_socket->Close();
    }
}

uint32_t generateRequirements(void)
{
    // 创建一个UniformRandomVariable对象
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
    // 设置随机数范围
    rand->SetAttribute("Min", DoubleValue(1000));  // 最小值
    rand->SetAttribute("Max", DoubleValue(1500));  // 最大值
    // 生成一个Min~Max之间的随机数
    uint32_t randomValue = rand->GetValue();
    return randomValue;
}

void UserApp::SendRequest(void)
{
    Status userRequest = generateMessage(StatusType::REQUEST, "Request");
    userRequest.requirements = generateRequirements();
    
    std::string message = serialize(userRequest);
    Ptr<Packet> packet = Create<Packet>((uint8_t *)message.c_str(), message.size());

    PacketTypeTag packetTypeTag;
    packetTypeTag.SetPacketType(PacketType::USER_REQUEST);
    packet->AddPacketTag(packetTypeTag);


    EventTimeTag requestGenTimeTag;
    requestGenTimeTag.SetRequestGeneratedTime(Simulator::Now());
    packet->AddPacketTag(requestGenTimeTag);

    m_socket->Send(packet);
    
    // m_logger.logStateObservation("UserApp-Sent-Req", userRequest.seqid); //TODO: 需要存储status seqid
    // m_logger.Log("UserApp", "Sent request to AP"); // 记录日志
    Simulator::Schedule(MilliSeconds(accessBehavior->GetValue()), &UserApp::SendRequest, this); // 使用指数分布的值作为下一次调用SendRequest的延迟。注意，此处是面向NS3调度系统的编程，因此采用类似递归的写法。
    NS_LOG_INFO("User: Sent request (PacketID=" << packet->GetUid() << ") to AP at " << requestGenTimeTag.GetRequestGeneratedTime().As(Time::S));
}

void UserApp::HandleRead(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom(from)))
    {
        uint8_t buffer[1024];
        packet->CopyData(buffer, 1024);
        std::string msg = std::string(reinterpret_cast<char *>(buffer), packet->GetSize());
        NS_LOG_INFO("User: Received response: " << msg << " at Time=" << Simulator::Now().As(Time::S));
        // m_logger.Log("UserApp", "Received response: " + msg); // 记录日志
    }
}

} // namespace ns3