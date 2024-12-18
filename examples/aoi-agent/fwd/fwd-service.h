#ifndef FWD_SERVICE_RATE_H
#define FWD_SERVICE_RATE_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/core-module.h"
#include "ns3/event-id.h"
#include "ns3/socket.h"

#include <queue>

namespace ns3 {

class FwdService : public Application
{
public:
    /**
     * \brief Get the type ID.
     * \return The object TypeId.
     */
    static TypeId GetTypeId();
    FwdService();
    virtual ~FwdService();
    
    void Setup(Ptr<Socket> recvSocket,
               Ptr<Socket> sendSocket,
               Address forwardAddress);
    void SetStochasticProcess(Ptr<RandomVariableStream> process);
    void initializeRandomNumberGenerator();
    Time GetRandomProcessDelay();

  protected:
    virtual void StartApplication() override;
    virtual void StopApplication() override;

private:
    void HandleRead(Ptr<Socket> socket);//接收数据包packet
    void Process();  // 处理数据包
    void Forward(Ptr<Packet> &packet); // 转发数据包
    // void ScheduleNextPacket(); // 调度下一个数据包出队

    Ptr<Socket> m_recvSocket; // 接收状态包的Socket
    Ptr<Socket> m_sendSocket; // 转发包的Socket
    Address m_forwardAddress; // 转发的目标地址
    Time m_serviceDelay;    // 服务处理的延迟
    EventId m_serviceEvent;   // 调度转发的事件

    std::queue<Ptr<Packet>> m_packetQueue; // 应用层的包队列
    Ptr<RandomVariableStream> m_processDelayRnd; //处理时延随机变量

    /// Tx TracedCallback
    // TracedCallback<Ptr<const Packet>> m_txTrace;
};

} // namespace ns3

#endif // FWD_SERVICE_RATE_H