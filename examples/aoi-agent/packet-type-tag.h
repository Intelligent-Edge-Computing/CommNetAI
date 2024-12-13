//
// Created by 齐建鹏 on 2024/10/15.
//

#ifndef PACKET_TYPE_TAG_H
#define PACKET_TYPE_TAG_H

#include "ns3/tag.h"

#include <unordered_map>
#include <iostream>


enum PacketType
{
    UNDEFINED = 0,
    USER_REQUEST, // 0, User request
    SERVICE_STATUS,  // 1, Service status
    SERVICE_RESPONSE // 2, Response status
};
class PacketTypeTag : public ns3::Tag
{
public:
    // 使用unordered_map映射枚举值到字符串描述
    std::string getPacketTypeNames(PacketType type) const
    {
        switch (type)
        {
        case USER_REQUEST: return "User request";
        case SERVICE_STATUS: return "Service status";
        case SERVICE_RESPONSE: return "Service response";
        default: return "Unknown";
        }
    };
    // 默认构造函数
    PacketTypeTag() : m_type(UNDEFINED) {}

    // 带参数的构造函数，用于直接初始化 m_type
    PacketTypeTag(PacketType type) : m_type(type) {}
    // 注册类的 TypeId
    static ns3::TypeId GetTypeId (void)
    {
        static ns3::TypeId tid = ns3::TypeId ("PacketTypeTag")
            .SetParent<ns3::Tag> ()
            .AddConstructor<PacketTypeTag> ();
        return tid;
    }

    virtual ns3::TypeId GetInstanceTypeId (void) const
    {
        return GetTypeId ();
    }

    // 序列化方法：转为字节流，写入 TagBuffer
    virtual void Serialize (ns3::TagBuffer i) const
    {
        i.Write (reinterpret_cast<const uint8_t *> (&m_type), sizeof (m_type));
    }

    // 反序列化方法：从 TagBuffer 中读取字节流并转换
    virtual void Deserialize (ns3::TagBuffer i)
    {
        PacketType packetType;

        i.Read (reinterpret_cast<uint8_t *> (&packetType), sizeof (packetType));

        m_type = packetType;
    }

    // 获取序列化大小
    virtual uint32_t GetSerializedSize (void) const
    {
        return 1 * sizeof (m_type);
    }

    // 打印标签内容
    virtual void Print (std::ostream &os) const
    {
        os << "PacketType=" <<  getPacketTypeNames(m_type);
    }
    // 设置进入队列时间
    void SetPacketType (PacketType type)
    {
        m_type = type;
    }

    // 获取进入队列时间
    PacketType GetPacketType (void) const
    {
        return m_type;
    }

    //
private:
    PacketType m_type;


};


#endif //PACKET_TYPE_TAG_H
