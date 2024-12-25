#include "server-app.h"

#include "../event-time-tag.h"
#include "../json.hpp"
#include "../logger.h"
#include "../packet-type-tag.h"
#include "../status-util.h"

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/random-variable-stream.h"

#include <sstream> // 添加头文件
using json = nlohmann::json;

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ServerApp");

TypeId ServerApp::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::ServerApp")
        .SetParent<Application>()
        .SetGroupName("Tutorial")
        .AddConstructor<ServerApp>()
        // 注册 TraceSource
        .AddTraceSource("ServerReponseRequestPacketTrace",
                        "Triggered when an request is processed and responded.",
                    MakeTraceSourceAccessor(&ServerApp::m_serverReponseRequestPacketTrace),
                    "ns3::Packet::TracedCallback")
        .AddTraceSource("ServerExecuteRequestPacketTrace",
                        "Triggered when executing an request is started.",
                        MakeTraceSourceAccessor(&ServerApp::m_serverExecRequestPacketTrace),
                        "ns3::Packet::TracedCallback")
        .AddTraceSource("ServerReceiveRequestPacketTrace",
                    "Triggered when executing an request is started.",
                    MakeTraceSourceAccessor(&ServerApp::m_serverReceiveRequestPacketTrace),
                    "ns3::Packet::TracedCallback")
        .AddTraceSource("ServerDropRequestPacketTrace",
                    "Triggered when dropped an request.",
                    MakeTraceSourceAccessor(&ServerApp::m_serverDropRequestPacketTrace),
                    "ns3::Packet::TracedCallback")
        .AddAttribute ("ServerQueue",
                   "The server queue.",
                   PointerValue (CreateObject<ServerQueue> ()),
                   MakePointerAccessor (&ServerApp::m_requestQueue),
                   MakePointerChecker<ServerQueue> ());
    return tid;
}


ServerApp::ServerApp() : m_socket(0), m_logger(ConfigManager::GetConfig()["dbPath"]), m_strategy(FCFS), m_queueCapacity(10)  {
    processSpeed = 2000; //2000 Cycles per Seconds
    m_requestQueue = CreateObject<ServerQueue>();
    m_requestQueue->set_max_capacity(m_queueCapacity);
    m_resourceTracker.UpdateSeats(m_queueCapacity);
}// 初始化 Logger

ServerApp::~ServerApp() { m_socket = 0; }
ServerResourceTracker::~ServerResourceTracker(){}

void ServerApp::Setup(Ipv4Address address, uint16_t port, QueueStrategy strategy, uint32_t queueCapacity)
{
    m_peerAddress = address;
    m_peerPort = port;
    m_strategy = strategy;
    m_queueCapacity = queueCapacity;
    useAgent = ConfigManager::GetConfig()["Agent"];
}
// 设置随机过程，使用 Ptr<RandomVariableStream>
void ServerApp::SetStochasticProcess(Ptr<RandomVariableStream> process)
{
    m_statusUpdateRnd = process;
}

// 获取随机值
Time  ServerApp::GetRandomStatusUpdateInterval()
{
    if (m_statusUpdateRnd)
    {
        double value = m_statusUpdateRnd->GetValue();

        return Seconds(value);
    }
    else
    {
        NS_ABORT_MSG("No stochastic process set.");
    }
}


void ServerApp::initializeRandomNumberGenerator(void)
{
    std::string process = ConfigManager::GetConfig()["server"]["stochasticProcess"];
    if (process == "PoisDist")  // 泊松分布
    {
        Ptr<ExponentialRandomVariable> poissonVar = CreateObject<ExponentialRandomVariable>();
        double lambda = ConfigManager::GetConfig()["server"]["PoisDist"]["statusUpdateRate"];
        double bound = ConfigManager::GetConfig()["server"]["PoisDist"]["bound"]; //0.0
        poissonVar->SetAttribute("Mean", DoubleValue(1.0/lambda));
        poissonVar->SetAttribute("Bound", DoubleValue(bound)); // bound=0 就是无边界。
        SetStochasticProcess(poissonVar);
    }
    else if (process == "UniformDist")  // 均匀分布
    {
        Ptr<UniformRandomVariable> uniformVar = CreateObject<UniformRandomVariable>();
        double min = ConfigManager::GetConfig()["server"]["UniformDist"]["min"];
        double max = ConfigManager::GetConfig()["server"]["UniformDist"]["max"];
        uniformVar->SetAttribute("Min", DoubleValue(min));
        uniformVar->SetAttribute("Max", DoubleValue(max));
        SetStochasticProcess(uniformVar);
    }
    else if (process == "NormalDist")  // 正态分布
    {
        Ptr<NormalRandomVariable> normalVar = CreateObject<NormalRandomVariable>();
        double mean = ConfigManager::GetConfig()["server"]["NormalDist"]["mean"];
        double variance = ConfigManager::GetConfig()["server"]["NormalDist"]["variance"];
        normalVar->SetAttribute("Mean", DoubleValue(mean));
        normalVar->SetAttribute("Variance", DoubleValue(variance));
        SetStochasticProcess(normalVar);
    }
    else if (process == "ExpDist")  // 指数分布
    {
        Ptr<ExponentialRandomVariable> expVar = CreateObject<ExponentialRandomVariable>();
        double mean = ConfigManager::GetConfig()["server"]["ExpDist"]["mean"];
        double bound = ConfigManager::GetConfig()["server"]["ExpDist"]["bound"];
        expVar->SetAttribute("Mean", DoubleValue(mean));
        expVar->SetAttribute("Bound", DoubleValue(bound));
        SetStochasticProcess(expVar);
    }
    else if (process == "Deterministic") //确定性
    {
        Ptr<ConstantRandomVariable> constantVar = CreateObject<ConstantRandomVariable>();
        double statusUpdateInterval = ConfigManager::GetConfig()["server"]["Deterministic"]["statusUpdateInterval"];
        constantVar->SetAttribute("Constant", DoubleValue(statusUpdateInterval));
        SetStochasticProcess(constantVar);
    }
    else
    {
        NS_ABORT_MSG("Unknown stochastic process: " << process);
    }
}

void ServerApp::StartApplication(void)
{

    initializeRandomNumberGenerator();
    m_socket = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::UdpSocketFactory"));
    m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_peerPort));
    m_socket->SetRecvCallback(MakeCallback(&ServerApp::HandleRead, this));
    
    // Start processing queue at startup if there are pending events
    if (!m_processEvent.IsPending() && !m_requestQueue->empty())
    {
        m_processEvent = Simulator::Schedule(Seconds(0.1), &ServerApp::ProcessQueue, this);
    }
    if (!useAgent)
    {
        Simulator::Schedule(Seconds(0.1), &ServerApp::SendServiceStatus, this);
    }
}
void ServerApp::SendResultPacket(Ptr<Packet> packet, Address to, Status request, Status response)
{
    EventTimeTag eventTimeTag;
    if (packet->PeekPacketTag(eventTimeTag))
    {
        eventTimeTag.SetRequestServerResponseTime(Simulator::Now());
        packet->ReplacePacketTag(eventTimeTag);
    }
    m_socket->SendTo(packet, 0, to);
    m_serverReponseRequestPacketTrace(packet);
    // m_logger.logStateObservation("ServerApp-Responded (Req.Resp)", request.seqid + "." +response.seqid);
    NS_LOG_INFO("[Server]: Processed and responded to request (PacketID=" << packet->GetUid() << ") from AP" << " at Time=" << Simulator::Now().As(Time::S));
    NS_LOG_DEBUG(eventTimeTag.prettyResults());

}
void ServerApp::freeSeats(uint32_t num)
{
    currentRunning = currentRunning - num;
}
void ServerApp::ProcessQueue()
{
    if (!m_requestQueue->empty())
    {
        std::pair<Address, Ptr<Packet>> adst;
        if (m_strategy == FCFS)
        {
            adst = m_requestQueue->front();
            m_requestQueue->pop_front();
        }
        else if (m_strategy == FCLS)
        {
            adst = m_requestQueue->back();
            m_requestQueue->pop_back();
        }
        else if (m_strategy == MDP)
        {
            
        }
        Ptr<Packet> packet = adst.second;
        //serving the request
        Status request = extractStatus(packet);
        Status response = generateMessage(StatusType::RESPONSE, request.jsonString);
        std::string responseStr = serialize(response);
        Ptr<Packet> responsePacket = Create<Packet>((uint8_t *)responseStr.c_str(), responseStr.size());

        PacketTypeTag packetTypeTag;
        packetTypeTag.SetPacketType(PacketType::SERVICE_RESPONSE);
        responsePacket->AddPacketTag(packetTypeTag);

        EventTimeTag eventTimeTag;
        if (packet->PeekPacketTag(eventTimeTag))
        {
            eventTimeTag.SetRequestServerProcessTime(Simulator::Now());
            m_serverExecRequestPacketTrace(packet);
            // packet->ReplacePacketTag(eventTimeTag);
        }
        uint32_t delayInMs = calcComputingDelay(request);
        eventTimeTag.SetRequestServerFinishTime(Simulator::Now() + MilliSeconds(delayInMs));
        responsePacket->AddPacketTag(eventTimeTag);

        // 发送pkts占据1ms
        m_processEvent = Simulator::Schedule(MilliSeconds(delayInMs), &ServerApp::SendResultPacket, this, responsePacket, adst.first, request, response);
        //执行完一个任务后，currentRunning--, 空出1个坑位
        Simulator::Schedule(MilliSeconds(delayInMs), &ServerApp::freeSeats, this, 1);
    }
}

Status
ServerApp::extractStatus(const Ptr<Packet> packet)
{
    uint32_t packetSize = packet->GetSize();
    uint8_t buffer[1024];

    std::string msg;

    if (packetSize <= sizeof(buffer))
    {
        // 如果数据包大小小于等于 1024 字节，使用固定大小的 buffer
        packet->CopyData(buffer, packetSize);
        msg = std::string(reinterpret_cast<char*>(buffer), packetSize);
    }
    else
    {
        // 如果数据包大小超过 1024 字节，使用动态分配的 buffer
        std::unique_ptr<uint8_t[]> largeBuffer(new uint8_t[packetSize]);
        packet->CopyData(largeBuffer.get(), packetSize);
        msg = std::string(reinterpret_cast<char *>(largeBuffer.get()), packetSize);
    }
    Status status = deserialize(msg);
    return status;
}
Ptr<ServerQueue>& ServerApp::GetRequestQueue()
{
    return m_requestQueue;
}
ServerResourceTracker ServerApp::GetServerResourceTracker()
{
    return m_resourceTracker;
}
// in ms
uint32_t ServerApp::calcComputingDelay(Status request)
{
    uint32_t delay = static_cast<uint32_t>(1000.0 * request.requirements / processSpeed);// in ms
    return delay;
}
void ServerApp::StopApplication(void)
{
    if (m_socket)
    {
        m_socket->Close();
    }
    if (m_processEvent.IsPending())
    {
        Simulator::Cancel(m_processEvent);
    }
}

void ServerApp::SendServiceStatus(void)
{
    statusCounter++;
    Status status = generateServiceStatus();
    status.capacity = m_queueCapacity - currentRunning; //server's available seats
    std::string message = serialize(status);
    Ptr<Packet> packet = Create<Packet>((uint8_t *)message.c_str(), message.size());

    EventTimeTag statusTimeTag;
    statusTimeTag.SetServerStatusGeneratedTime(Simulator::Now());
    packet->AddPacketTag(statusTimeTag); // 将时间标签添加到包上

    PacketTypeTag packetTypeTag;
    packetTypeTag.SetPacketType(PacketType::SERVICE_STATUS);
    packet->AddPacketTag(packetTypeTag);

    m_socket->SendTo(packet, 0, InetSocketAddress(m_peerAddress, m_peerPort));
    NS_LOG_DEBUG("[Server] Sent status (PacketID='" << packet->GetUid() << ") to AP" << ", at Time=" << Simulator::Now().As(Time::S));

    if (!useAgent)
    {
        Simulator::Schedule(GetRandomStatusUpdateInterval(), &ServerApp::SendServiceStatus, this); // Schedule next status
    }
}

void ServerApp::HandleRead(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom(from)))
    {
        PacketTypeTag packetTypeTag;
        if (packet->PeekPacketTag(packetTypeTag))
        {
            if (packetTypeTag.GetPacketType() == PacketType::USER_REQUEST)
            {
                EventTimeTag eventTimeTag;
                if (packet->PeekPacketTag(eventTimeTag))
                {
                    eventTimeTag.SetRequestServerReceivedTime(Simulator::Now());
                    packet->ReplacePacketTag(eventTimeTag);
                    m_serverReceiveRequestPacketTrace(packet);
                }
                // if (currentRunning < m_queueCapacity)
                 if (m_resourceTracker.GetAvailableSeats() > 0)
                {
                    currentRunning ++; // 正在执行的任务数加1
                    m_resourceTracker.UpdateSeats(m_queueCapacity - currentRunning);
                    m_requestQueue->push_back(std::make_pair(from, packet));
                    NS_LOG_DEBUG("[Server] Queued request from " << InetSocketAddress::ConvertFrom(from).GetIpv4() << " at Time=" << Simulator::Now().As(Time::S));

                    // Schedule processing the queue if not already scheduled
                    Simulator::Schedule(Seconds(0), &ServerApp::ProcessQueue, this);
                }
                else
                { //允许的并发任务数到达上限，执行失败
                    m_serverDropRequestPacketTrace(packet);
                    NS_LOG_INFO("[Server] Capacity full, dropped request PacketID=" << packet->GetUid() << " from " << InetSocketAddress::ConvertFrom(from).GetIpv4() << " at Time=" << Simulator::Now().As(Time::S));
                }
            }
        }
    }
}

} // namespace ns3