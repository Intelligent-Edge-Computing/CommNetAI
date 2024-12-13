//
// Created by 齐建鹏 on 2024/10/15.
//

#ifndef RESULT_CLASSIFIER_H
#define RESULT_CLASSIFIER_H

#include "ns3/tag.h"

#include <unordered_map>
#include <iostream>


enum Classifier
{
    TP, // True Positive
    FP, // False Positive
    TN, // True Negtive
    FN // False Negtive
};
class ReulstClassifierTag : public ns3::Tag
{
public:
    // 使用unordered_map映射枚举值到字符串描述
    std::string getPacketTypeNames(Classifier type) const
    {
        switch (type)
        {
        case TP: return "True Positive";
        case FP: return "False Positive";
        case TN: return "True Negtive";
        case FN: return "False Negtive";
        default: return "Unknown";
        }
    };
    // 默认构造函数
    ReulstClassifierTag() : m_type(TP) {}

    // 带参数的构造函数，用于直接初始化 m_type
    ReulstClassifierTag(Classifier type) : m_type(type) {}
    // 注册类的 TypeId
    static ns3::TypeId GetTypeId (void)
    {
        static ns3::TypeId tid = ns3::TypeId ("ReulstClassifierTag")
            .SetParent<ns3::Tag> ()
            .AddConstructor<ReulstClassifierTag> ();
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
        Classifier classifier;

        i.Read (reinterpret_cast<uint8_t *> (&classifier), sizeof (classifier));

        m_type = classifier;
    }

    // 获取序列化大小
    virtual uint32_t GetSerializedSize (void) const
    {
        return 1 * sizeof (m_type);
    }

    // 打印标签内容
    virtual void Print (std::ostream &os) const
    {
        os << "Classifier=" <<  getPacketTypeNames(m_type);
    }
    // 设置进入队列时间
    void SetClassifier (Classifier type)
    {
        m_type = type;
    }

    // 获取进入队列时间
    Classifier GetClassifier (void) const
    {
        return m_type;
    }

    //
private:
    Classifier m_type;


};

#endif //RESULT_CLASSIFIER_H
