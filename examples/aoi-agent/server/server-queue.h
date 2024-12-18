//
// Created by 齐建鹏 on 2024/11/1.
//

#ifndef SERVER_QUEUE_H
#define SERVER_QUEUE_H
#include "ns3/address.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"

#include <deque>

namespace ns3 {

class ServerQueue : public Object
{
public:
    static TypeId GetTypeId (void);
    ServerQueue ();
    ServerQueue (uint32_t capacity);
    void Enqueue (std::pair<Address, Ptr<Packet>> request);
    std::pair<Address, Ptr<Packet>> Dequeue (void);
    uint32_t GetSize (void) const;
    bool isFull(void) const;
    bool empty (void) const;
    std::pair<Address, Ptr<Packet>> front(void);
    void pop_front (void);
    std::pair<Address, Ptr<Packet>> back(void);
    void pop_back (void);
    void push_back(std::pair<Address, Ptr<Packet>>  entry);
    void set_max_capacity (uint32_t max_capacity);

private:
    std::deque<std::pair<Address, Ptr<Packet>>> m_queue;
    uint32_t maxCapacity;
};


} // namespace ns3
#endif //SERVER_QUEUE_H
