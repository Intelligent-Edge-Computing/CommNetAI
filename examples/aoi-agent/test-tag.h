//
// Created by 齐建鹏 on 2024/10/12.
//

#ifndef TEST_TAG_H
#define TEST_TAG_H
#include "ns3/tag.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include <iostream>

using namespace ns3;

// 自定义 MyFlowIdTag
class MyFlowIdTag : public Tag
{
public:
    MyFlowIdTag() : m_flowId(0) {}

    MyFlowIdTag(uint32_t flowId) : m_flowId(flowId) {}

    // 必须实现的序列化函数
    void Serialize(TagBuffer i) const override
    {
        i.WriteU32(m_flowId);  // 将流 ID 写入 TagBuffer
    }

    // 必须实现的反序列化函数
    void Deserialize(TagBuffer i) override
    {
        m_flowId = i.ReadU32();  // 从 TagBuffer 中读取流 ID
    }

    // 返回序列化大小（字节数）
    uint32_t GetSerializedSize() const override
    {
        return 4;  // 4字节的 flowId
    }

    // 打印 Tag 信息
    void Print(std::ostream &os) const override
    {
        os << "FlowId=" << m_flowId;
    }

    // 获取实例类型 ID
    static TypeId GetTypeId()
    {
        static TypeId tid = TypeId("MyFlowIdTag")
                                .SetParent<Tag>()
                                .AddConstructor<MyFlowIdTag>();
        return tid;
    }

    // 获取对象的实例类型 ID
    TypeId GetInstanceTypeId() const override
    {
        return GetTypeId();
    }

    // 设置和获取 FlowId
    void SetFlowId(uint32_t flowId) { m_flowId = flowId; }
    uint32_t GetFlowId() const { return m_flowId; }

private:
    uint32_t m_flowId;
};
#endif //TEST_TAG_H
