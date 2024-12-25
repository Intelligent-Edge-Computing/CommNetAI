#include "ap-running-info.h"

ApActivityTracker::ApActivityTracker()
    : m_storedServiceStatusPacket(nullptr),
      m_storedUserRequestPacket(nullptr),
      m_storedServiceResponsePacket(nullptr) {}

ApActivityTracker::~ApActivityTracker() {}

TypeId
ApActivityTracker::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::ApActivityTracker")
      .SetParent<Object> ()
      .SetGroupName ("Tutorial")
      .AddConstructor<ApActivityTracker> ();
    return tid;
}
uint32_t ApActivityTracker::GetAge() const {
  //计算AP节点上的AoI
  if (m_storedServiceStatusPacket != nullptr) {
    // Assuming the packet has a method to get the timestamp
    // Replace with appropriate logic based on ns3::Packet API
  }
  return 0;
}

Status ApActivityTracker::GetServiceStatus() const {
  // Extract and return the Status from the s tored packet
  if (m_storedServiceStatusPacket != nullptr) {
    // Replace with appropriate deserialization logic
//    return StatusUtil::DeserializeStatus(m_storedServiceStatusPacket);
  }
  return Status(); // Default Status if no packet exists
}


uint32_t ApActivityTracker::GetUserLastVisitDuration() const {
  // Extract and return user last visit duration from response packet
  if (m_storedUserRequestPacket != nullptr) {
    // Replace with appropriate deserialization logic
//    return StatusUtil::DeserializeVisitDuration(m_storedServiceResponsePacket);
  }
  return 0; // Default duration
}


void ApActivityTracker::UpdateStoredServiceStatusPacket(const Ptr<Packet> packet)
{
  m_storedServiceStatusPacket = packet;
}
void ApActivityTracker::UpdateStoredUserRequestPacket(const Ptr<Packet> packet)
{
  m_storedUserRequestPacket = packet;
}
void ApActivityTracker::UpdateStoredServiceResponsePacket(const Ptr<Packet> packet)
{
  m_storedServiceResponsePacket = packet;
}