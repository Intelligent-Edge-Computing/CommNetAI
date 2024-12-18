#include "fwd-service.h"

#include "../event-time-tag.h"
#include "../packet-type-tag.h"
#include "../status-util.h"

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("FwdService");
NS_OBJECT_ENSURE_REGISTERED(FwdService);

TypeId
FwdService::GetTypeId()
{
    static TypeId tid = TypeId("ns3::FwdService")
                            .SetParent<Application>()
                            .AddConstructor<FwdService>();
                            // .AddTraceSource("Tx",
                            //                 "A new packet is received --> serviced --> sent",
                            //                 MakeTraceSourceAccessor(&FwdService::m_txTrace),
                            //                 "ns3::Packet::TracedCallback");
    return tid;
}
FwdService::FwdService()
    : m_recvSocket(0), m_sendSocket(0), m_serviceDelay(0.0)
{
}

FwdService::~FwdService()
{
    m_recvSocket = nullptr;
    m_sendSocket = nullptr;
}

void FwdService::Setup(Ptr<Socket> recvSocket, Ptr<Socket> sendSocket, Address forwardAddress)
{
    m_recvSocket = recvSocket;
    m_sendSocket = sendSocket;
    m_forwardAddress = forwardAddress;
}
// 设置随机过程，使用 Ptr<RandomVariableStream>
void FwdService::SetStochasticProcess(Ptr<RandomVariableStream> process)
{
    m_processDelayRnd = process;
}

void FwdService::initializeRandomNumberGenerator(void)
{
    std::string process = ConfigManager::GetConfig()["forwarder"]["stochasticProcess"];
    if (process == "PoisDist")  // 泊松分布
    {
        Ptr<ExponentialRandomVariable> poissonVar = CreateObject<ExponentialRandomVariable>();
        double lambda = ConfigManager::GetConfig()["forwarder"]["PoisDist"]["serviceRate"];
        double bound = ConfigManager::GetConfig()["forwarder"]["PoisDist"]["bound"]; //0.0
        poissonVar->SetAttribute("Mean", DoubleValue(1.0/lambda));
        poissonVar->SetAttribute("Bound", DoubleValue(bound)); // bound=0 就是无边界。
        SetStochasticProcess(poissonVar);
    }
    else if (process == "UniformDist")  // 均匀分布
    {
        Ptr<UniformRandomVariable> uniformVar = CreateObject<UniformRandomVariable>();
        double min = ConfigManager::GetConfig()["forwarder"]["UniformDist"]["min"];
        double max = ConfigManager::GetConfig()["forwarder"]["UniformDist"]["max"];
        uniformVar->SetAttribute("Min", DoubleValue(min));
        uniformVar->SetAttribute("Max", DoubleValue(max));
        SetStochasticProcess(uniformVar);
    }
    else if (process == "NormalDist")  // 正态分布
    {
        Ptr<NormalRandomVariable> normalVar = CreateObject<NormalRandomVariable>();
        double mean = ConfigManager::GetConfig()["forwarder"]["NormalDist"]["mean"];
        double variance = ConfigManager::GetConfig()["forwarder"]["NormalDist"]["variance"];
        normalVar->SetAttribute("Mean", DoubleValue(mean));
        normalVar->SetAttribute("Variance", DoubleValue(variance));
        SetStochasticProcess(normalVar);
    }
    else if (process == "ExpDist")  // 指数分布
    {
        Ptr<ExponentialRandomVariable> expVar = CreateObject<ExponentialRandomVariable>();
        double mean = ConfigManager::GetConfig()["forwarder"]["ExpDist"]["mean"];
        double bound = ConfigManager::GetConfig()["forwarder"]["ExpDist"]["bound"];
        expVar->SetAttribute("Mean", DoubleValue(mean));
        expVar->SetAttribute("Bound", DoubleValue(bound));
        SetStochasticProcess(expVar);
    }
    else if (process == "Deterministic") //确定性
    {
        Ptr<ConstantRandomVariable> constantVar = CreateObject<ConstantRandomVariable>();
        double delay = ConfigManager::GetConfig()["forwarder"]["Deterministic"]["serviceDelay"];
        constantVar->SetAttribute("Constant", DoubleValue(delay));
        SetStochasticProcess(constantVar);
    }
    else
    {
        NS_ABORT_MSG("Unknown stochastic process: " << process);
    }
}
// 获取随机值
Time FwdService::GetRandomProcessDelay()
{
    if (m_processDelayRnd)
    {
        double value = m_processDelayRnd->GetValue();
        return Seconds(value);
    }
    else
    {
        NS_ABORT_MSG("No stochastic process set.");
    }
}

void FwdService::StartApplication()
{
    initializeRandomNumberGenerator();
    if (m_recvSocket)
    {
        m_recvSocket->SetRecvCallback(MakeCallback(&FwdService::HandleRead, this));
    }
}

void FwdService::StopApplication()
{
    if (m_recvSocket)
    {
        m_recvSocket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }

    if (m_serviceEvent.IsPending())
    {
        Simulator::Cancel(m_serviceEvent);
    }
    // 清空队列
    std::queue<Ptr<Packet>> emptyQueue;
    std::swap(m_packetQueue, emptyQueue);
}

void FwdService::HandleRead(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom(from)))
    {
        PacketTypeTag packetTypeTag;
        if (packet->PeekPacketTag(packetTypeTag))
        {
            if(packetTypeTag.GetPacketType() == PacketType::SERVICE_STATUS)
            {
                NS_LOG_DEBUG("[FWD] Received packet " << packet->GetUid() << " at " << Simulator::Now().GetSeconds() << " seconds, enqueuing...");
                EventTimeTag enqueueTimeTag;
                if (packet->PeekPacketTag(enqueueTimeTag))
                {
                    enqueueTimeTag.SetFwdServerStatusEnqueueTime(Simulator::Now());
                    packet->ReplacePacketTag(enqueueTimeTag); // 将时间标签添加到包上
                }

                m_packetQueue.push(packet); // 将包加入队列
                NS_LOG_DEBUG("[FWD] Packet " << packet->GetUid() << " was queued at " << Simulator::Now().GetSeconds() << "s.");
                // 如果队列中只有一个包，立即调度处理
                if (m_packetQueue.size() == 1 && !m_serviceEvent.IsPending())
                {
                    Process();
                }
            } else if (packetTypeTag.GetPacketType() == PacketType::SERVICE_RESPONSE)
            {//实际上结果直接通过P2P的形式在底层直接返回给了AP，没有经过引用层fwd-service
                NS_LOG_DEBUG("[FWD] Received service results " << packet->GetUid() << " at " << Simulator::Now().GetSeconds());
            }
        }
    }
}
void FwdService::Process()
{
    if (!m_packetQueue.empty())
    {
        Ptr<Packet> packet = m_packetQueue.front();
        m_packetQueue.pop();

        NS_LOG_DEBUG("[FWD] Processing (PacketID=" << packet->GetUid() << ") at " << Simulator::Now().GetSeconds() << "s.");
        EventTimeTag timeTag;
        if (packet->PeekPacketTag(timeTag))
        {
            timeTag.SetFwdServerStatusProcessTime(Simulator::Now());
            packet->ReplacePacketTag(timeTag);
        }
        m_serviceDelay = GetRandomProcessDelay();
        Simulator::Schedule(m_serviceDelay, &FwdService::Forward, this, packet);
    }
}
void FwdService::Forward(Ptr<Packet> &packet)
{
    PacketTypeTag packetTypeTag;
    if (packet->PeekPacketTag(packetTypeTag))
    {
        if (packetTypeTag.GetPacketType() == PacketType::SERVICE_STATUS)
        {
            EventTimeTag timeTag;
            if (packet->PeekPacketTag(timeTag))
            {
                // 转发包
                if (m_sendSocket)
                {
                    // 设置完成时间为当前模拟时间
                    timeTag.SetFwdServerStatusFinishTime(Simulator::Now());
                    packet->ReplacePacketTag(timeTag);

                    m_sendSocket->SendTo(packet, 0, m_forwardAddress);

                    // m_txTrace(packet);

                    NS_LOG_DEBUG("[FWD] Forwarded packet " << packet->GetUid() << " at " << Simulator::Now().As(Time::S));
                    // NS_LOG_INFO(timeTag.prettyResults());
                }
                // 调度下一个包的处理
                if (!m_packetQueue.empty())
                {
                    Simulator::Schedule(MilliSeconds(0), &FwdService::Process, this);
                }
            }
        }
    }
}

}// namespace ns3