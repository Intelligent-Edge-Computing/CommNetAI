#ifndef SERVER_APP_H
#define SERVER_APP_H

#include "../logger.h"
#include "../status-util.h"
#include "server-queue.h"

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/socket.h"

#include <utility> // For std::pair

namespace ns3 {

class ServerResourceTracker {
public:
    ServerResourceTracker() : avaliableSeats(0), lastUpdateTime(Seconds(0)) {}
    virtual ~ServerResourceTracker();
    void UpdateSeats(uint32_t newSeats) {
        avaliableSeats = newSeats;
        lastUpdateTime = Simulator::Now();
    }

    uint32_t GetAvailableSeats() const {
        return avaliableSeats;
    }

    Time GetDuration() const {
        return Simulator::Now() - lastUpdateTime;
    }

private:
    uint32_t avaliableSeats; // 当前可用资源数量
    Time lastUpdateTime;                  // 上次更新的时间
};
class ServerApp : public Application
{
public:
    static TypeId GetTypeId();

    enum QueueStrategy
    {
        FCFS, // 先来先服务，静态决策
        FCLS,  // 先来后服务，静态决策
        MDP //马尔可夫决策过程，动态决策
    };

    ServerApp();
    virtual ~ServerApp();

    void Setup(Ipv4Address address, uint16_t port, QueueStrategy strategy, uint32_t queueCapacity);
    void SendServiceStatus(void);

    TracedCallback<Ptr<const Packet>> m_serverReponseRequestPacketTrace;
    TracedCallback<Ptr<const Packet>> m_serverExecRequestPacketTrace;
    TracedCallback<Ptr<const Packet>> m_serverReceiveRequestPacketTrace; //True Positive
    TracedCallback<Ptr<const Packet>> m_serverDropRequestPacketTrace; // False Positive
    Ptr<ServerQueue>& GetRequestQueue();
    ServerResourceTracker GetServerResourceTracker(void);//获取当前可用的资源
private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    void HandleRead(Ptr<Socket> socket);
    void ProcessQueue();
    void freeSeats(uint32_t num);

    // 设置随机过程，使用 Ptr<RandomVariableStream>
    void initializeRandomNumberGenerator(void);
    void SetStochasticProcess(Ptr<RandomVariableStream> process);
    Time GetRandomStatusUpdateInterval(void);
    // uint64_t  GetRandomStatusUpdateIntervalInMicroS(void);
    uint32_t calcComputingDelay(Status request);
    void SendResultPacket (Ptr<Packet> packet, Address to, Status request, Status response);
    Status extractStatus(const Ptr<Packet> packet);
    Ptr<Socket> m_socket;
    Ipv4Address m_peerAddress;
    uint16_t m_peerPort;
    std::atomic<uint32_t> currentRunning = 0;
    Logger m_logger;
    // std::deque<std::pair<Address, Ptr<Packet>>> m_requestQueue; // 请求队列
    Ptr<ServerQueue> m_requestQueue;
    EventId m_processEvent; // 处理队列的事件
    QueueStrategy m_strategy; // 队列处理策略
    uint32_t m_queueCapacity; // 队列最大容量
    ServerResourceTracker m_resourceTracker;
    uint32_t processSpeed;    // 处理速度，CPU cycles
    Ptr<RandomVariableStream> m_statusUpdateRnd;
    uint64_t statusCounter = 0; // 状态更新计数器
    bool useAgent = false;

    // uint64_t lastUpdateTimestamp = 0;
};
} // namespace ns3

#endif // SERVER_APP_H