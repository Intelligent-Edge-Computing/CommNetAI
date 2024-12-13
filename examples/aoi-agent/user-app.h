#ifndef USER_APP_H
#define USER_APP_H

#include "ns3/application.h"
#include "ns3/address.h"
#include "ns3/socket.h"
#include "logger.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

class UserApp : public Application
{
public:
    UserApp();
    virtual ~UserApp();

    void Setup(Address apAddress, uint16_t apPort);

private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    void SendRequest(void);
    void HandleRead(Ptr<Socket> socket);

    Ptr<Socket> m_socket;
    Address m_peerAddress;
    uint16_t m_peerPort;
    Logger m_logger; // 添加 Logger 成员
    // 使用指数分布生成请求间隔
    Ptr<ExponentialRandomVariable> accessBehavior = CreateObject<ExponentialRandomVariable>();
};

} // namespace ns3

#endif // USER_APP_H