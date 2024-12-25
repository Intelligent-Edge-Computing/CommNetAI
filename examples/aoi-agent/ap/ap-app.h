#ifndef AP_APP_H
#define AP_APP_H

#include "../logger.h"
#include "../status-util.h"
#include "ap-app.h"
#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/tag.h" // 用于 QueueProcessTimeTag
#include "ns3/trace-source-accessor.h"
#include "ap-running-info.h"

namespace ns3 {
  class ApApp : public Application
{
public:
    static TypeId GetTypeId();
    ApApp();
    virtual ~ApApp();

    void Setup(Address serverAddress, uint16_t serverPort, uint16_t clientPort);
    void UpdateRunningInfo(const Ptr<Packet> packet);
    // TraceSource：用于捕获包到达并解析 QueueProcessTimeTag 的事件

    /**
       * TracedCallback signature for DL scheduling events.
       *
       * \param [in] oldPacket 已存储的status对应的Packet
       * \param [in] newPacket 新收到的status对应的Packet
       */
    typedef void (*ApTraceCallBack)(const Ptr<Packet> oldPacket, const Ptr<Packet> newPacket);


    TracedCallback<Ptr<const Packet>, Ptr<const Packet>> m_APReceiveServerStatusPacketTrace; //Stage 1
    // TracedCallback<Ptr<Packet>> m_requestPacketTrace;
    TracedCallback<Ptr<const Packet>> m_APFowardRequestToServerTrace;
    TracedCallback<Ptr<const Packet>> m_APReceiveRequestFromUserTrace;
    TracedCallback<Ptr<const Packet>> m_APReceiveResponseFromServerTrace;
    TracedCallback<Ptr<const Packet>> m_APDropRequestPacketTrace;//True or False Negative, should judge with server capacity at same time.

private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);
    Status extractStatus(const Ptr<Packet> packet);

    void HandleRead(Ptr<Socket> socket);

    Ptr<Socket> m_socket;
    Address m_serverAddress;
    uint16_t m_serverPort;
    uint16_t m_clientPort;
    Ptr<Packet> m_storedStatusPacket;
    Status m_storedStatus;
    Ptr<ApActivityTracker> m_apActivityTracker;
    Logger m_logger; // 添加 Logger 成员
};
} // namespace ns3

#endif // AP_APP_H