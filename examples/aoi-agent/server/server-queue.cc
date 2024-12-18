//
// Created by 齐建鹏 on 2024/11/1.
//
#include "server-queue.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (ServerQueue);

TypeId
ServerQueue::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::RequestQueue")
      .SetParent<Object> ()
      .SetGroupName ("Tutorial")
      .AddConstructor<ServerQueue> ();
    return tid;
}

ServerQueue::ServerQueue ()
{
}

ServerQueue::ServerQueue (uint32_t capacity)
{
    maxCapacity = capacity;
}

void
ServerQueue::Enqueue (std::pair<Address, Ptr<Packet>> request)
{
    m_queue.push_back (request);
}

std::pair<Address, Ptr<Packet>>
ServerQueue::Dequeue (void)
{
    if (m_queue.empty ())
    {
        return std::pair<Address, Ptr<Packet>> ();
    }
    auto request = m_queue.front ();
    m_queue.pop_front ();
    return request;
}

uint32_t
ServerQueue::GetSize (void) const
{
    return m_queue.size ();
}

bool
ServerQueue::isFull() const{
 return m_queue.size() >= maxCapacity;
}

bool ServerQueue::empty (void) const
{
    return m_queue.empty ();
}
std::pair<Address, Ptr<Packet>> ServerQueue::front(void)
{
    return m_queue.front ();
}
void ServerQueue::pop_front (void)
{
    m_queue.pop_front ();
}
std::pair<Address, Ptr<Packet>> ServerQueue::back(void)
{
    return m_queue.back ();
}
void ServerQueue::pop_back (void)
{
    m_queue.pop_back ();
}
void ServerQueue::push_back (std::pair<Address, Ptr<Packet>>   entry)
{
    m_queue.push_back (entry);
}

void
ServerQueue::set_max_capacity(uint32_t max_capacity)
{
  maxCapacity = max_capacity;
}


} // namespace ns3
