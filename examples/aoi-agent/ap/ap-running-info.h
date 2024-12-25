#ifndef AP_ACTIVITY_TRACKER_H
#define AP_ACTIVITY_TRACKER_H

#include "../status-util.h"
#include "ns3/packet.h"
#include "ns3/core-module.h"

class ApActivityTracker  : public Object
{
public:
  static TypeId GetTypeId (void);
  ApActivityTracker(); // Now takes pointer to m_storedStatus
  ~ApActivityTracker();

  uint32_t GetAge() const;
  Status GetServiceStatus() const;
  uint32_t GetUserLastVisitDuration() const;

  void UpdateStoredServiceStatusPacket(const Ptr<Packet> packet);
  void UpdateStoredUserRequestPacket(const Ptr<Packet> packet);
  void UpdateStoredServiceResponsePacket(const Ptr<Packet> packet);

private:
  Ptr<Packet> m_storedServiceStatusPacket;
  Ptr<Packet> m_storedUserRequestPacket;
  Ptr<Packet> m_storedServiceResponsePacket;
};
#endif //AP_ACTIVITY_TRACKER_H
