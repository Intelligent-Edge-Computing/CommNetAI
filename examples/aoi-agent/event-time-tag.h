//
// Created by 齐建鹏 on 2024/10/12.
//

#ifndef EVENT_TIME_TAG_H
#define EVENT_TIME_TAG_H
#include "ns3/tag.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include <iostream>

class EventTimeTag : public ns3::Tag
{
public:
    // 注册类的 TypeId
    static ns3::TypeId GetTypeId (void)
    {
        static ns3::TypeId tid = ns3::TypeId ("QueueProcessTimeTag")
            .SetParent<ns3::Tag> ()
            .AddConstructor<EventTimeTag> ();
        return tid;
    }

    virtual ns3::TypeId GetInstanceTypeId (void) const
    {
        return GetTypeId ();
    }

    // 序列化方法：将时间戳转为字节流，写入 TagBuffer
    virtual void Serialize (ns3::TagBuffer i) const
    {
        int64_t fwdServerStatusEnqueueTime = m_fwdServerStatusEnqueueTime.GetNanoSeconds ();
        int64_t fwdServerStatusProcessTime = m_fwdServerStatusProcessTime.GetNanoSeconds ();
        int64_t fwdServerStatusFinishTime = m_fwdServerStatusFinishTime.GetNanoSeconds ();
        int64_t serverStatusGeneratedTime = m_serverStatusGeneratedTime.GetNanoSeconds ();
        int64_t serverStatusReceiveTime = m_serverStatusReceiveTime.GetNanoSeconds ();
        //Stage 2
        int64_t requestGeneratedTimeNanoSeconds = m_requestGeneratedTime.GetNanoSeconds(); // 用户请求生成（发送）时间

        //Stage 3
        int64_t requestAPReceivedTimeNanoSeconds = m_requestAPReceivedTime.GetNanoSeconds();//AP 收到用户请求的时间
        int64_t requestAPForwardTTimeNanoSeconds = m_requestAPForwardTime.GetNanoSeconds(); //AP 转发用户请求的时间
        int64_t requestServerReceivedTimeNanoSeconds = m_requestServerReceivedTime.GetNanoSeconds();//Server 收到用户请求的时间

        //Stage 4
        int64_t requestServerProcessTimeNanoSeconds = m_requestServerProcessTime.GetNanoSeconds();//Server开始处理用户请求的时间
        int64_t requestServerFinishTimeNanoSeconds = m_requestServerFinishTime.GetNanoSeconds(); //Server处理用户请求完毕的时间
        int64_t requestServerResponseTimeNanoSeconds = m_requestServerResponseTime.GetNanoSeconds(); //Server响应用户请求的时间

        i.Write (reinterpret_cast<const uint8_t *> (&fwdServerStatusEnqueueTime), sizeof (fwdServerStatusEnqueueTime));
        i.Write (reinterpret_cast<const uint8_t *> (&fwdServerStatusProcessTime), sizeof (fwdServerStatusProcessTime));
        i.Write (reinterpret_cast<const uint8_t *> (&fwdServerStatusFinishTime), sizeof (fwdServerStatusFinishTime));
        i.Write (reinterpret_cast<const uint8_t *> (&serverStatusGeneratedTime), sizeof (serverStatusGeneratedTime));
        i.Write (reinterpret_cast<const uint8_t *> (&serverStatusReceiveTime), sizeof (serverStatusReceiveTime));
        i.Write (reinterpret_cast<const uint8_t *> (&requestGeneratedTimeNanoSeconds), sizeof (requestGeneratedTimeNanoSeconds));
        i.Write (reinterpret_cast<const uint8_t *> (&requestAPReceivedTimeNanoSeconds), sizeof (requestAPReceivedTimeNanoSeconds));
        i.Write (reinterpret_cast<const uint8_t *> (&requestAPForwardTTimeNanoSeconds), sizeof (requestAPForwardTTimeNanoSeconds));
        i.Write (reinterpret_cast<const uint8_t *> (&requestServerReceivedTimeNanoSeconds), sizeof (requestServerReceivedTimeNanoSeconds));
        i.Write (reinterpret_cast<const uint8_t *> (&requestServerProcessTimeNanoSeconds), sizeof (requestServerProcessTimeNanoSeconds));
        i.Write (reinterpret_cast<const uint8_t *> (&requestServerFinishTimeNanoSeconds), sizeof (requestServerFinishTimeNanoSeconds));
        i.Write (reinterpret_cast<const uint8_t *> (&requestServerResponseTimeNanoSeconds), sizeof (requestServerResponseTimeNanoSeconds));
    }

    // 反序列化方法：从 TagBuffer 中读取字节流并转换回时间戳
    virtual void Deserialize (ns3::TagBuffer i)
    {
        int64_t fwdServerStatusEnqueueTimeNanoSeconds;
        int64_t fwdServerStatusProcessTimeNanoSeconds;
        int64_t fwdServerStatusFinishTimeNanoSeconds;
        int64_t serverStatusGeneratedTimeNanoSeconds;
        int64_t serverStatusReceiveTimeNanoSeconds;
        int64_t requestGeneratedTimeNanoSeconds;
        int64_t requestAPReceivedTimeNanoSeconds;
        int64_t requestAPForwardTTimeNanoSeconds;
        int64_t requestServerReceivedTimeNanoSeconds;
        int64_t requestServerProcessTimeNanoSeconds;
        int64_t requestServerFinishTimeNanoSeconds;
        int64_t requestServerResponseTimeNanoSeconds;

        i.Read (reinterpret_cast<uint8_t *> (&fwdServerStatusEnqueueTimeNanoSeconds), sizeof (fwdServerStatusEnqueueTimeNanoSeconds));
        i.Read (reinterpret_cast<uint8_t *> (&fwdServerStatusProcessTimeNanoSeconds), sizeof (fwdServerStatusProcessTimeNanoSeconds));
        i.Read (reinterpret_cast<uint8_t *> (&fwdServerStatusFinishTimeNanoSeconds), sizeof (fwdServerStatusFinishTimeNanoSeconds));
        i.Read (reinterpret_cast<uint8_t *> (&serverStatusGeneratedTimeNanoSeconds), sizeof (serverStatusGeneratedTimeNanoSeconds));
        i.Read (reinterpret_cast<uint8_t *> (&serverStatusReceiveTimeNanoSeconds), sizeof (serverStatusReceiveTimeNanoSeconds));
        i.Read (reinterpret_cast<uint8_t *> (&requestGeneratedTimeNanoSeconds), sizeof(requestGeneratedTimeNanoSeconds));
        i.Read (reinterpret_cast<uint8_t *> (&requestAPReceivedTimeNanoSeconds), sizeof(requestAPReceivedTimeNanoSeconds));
        i.Read (reinterpret_cast<uint8_t *> (&requestAPForwardTTimeNanoSeconds), sizeof(requestAPForwardTTimeNanoSeconds));
        i.Read (reinterpret_cast<uint8_t *> (&requestServerReceivedTimeNanoSeconds), sizeof(requestServerReceivedTimeNanoSeconds));
        i.Read (reinterpret_cast<uint8_t *> (&requestServerProcessTimeNanoSeconds), sizeof(requestServerProcessTimeNanoSeconds));
        i.Read (reinterpret_cast<uint8_t *> (&requestServerFinishTimeNanoSeconds), sizeof(requestServerFinishTimeNanoSeconds));
        i.Read (reinterpret_cast<uint8_t *> (&requestServerResponseTimeNanoSeconds), sizeof(requestServerResponseTimeNanoSeconds));

        m_fwdServerStatusEnqueueTime = ns3::NanoSeconds (fwdServerStatusEnqueueTimeNanoSeconds);
        m_fwdServerStatusProcessTime = ns3::NanoSeconds (fwdServerStatusProcessTimeNanoSeconds);
        m_fwdServerStatusFinishTime = ns3::NanoSeconds (fwdServerStatusFinishTimeNanoSeconds);
        m_serverStatusGeneratedTime = ns3::NanoSeconds (serverStatusGeneratedTimeNanoSeconds);
        m_serverStatusReceiveTime = ns3::NanoSeconds (serverStatusReceiveTimeNanoSeconds);
        m_requestGeneratedTime = ns3::NanoSeconds(requestGeneratedTimeNanoSeconds);
        m_requestAPReceivedTime = ns3::NanoSeconds(requestAPReceivedTimeNanoSeconds);
        m_requestAPForwardTime = ns3::NanoSeconds(requestAPForwardTTimeNanoSeconds);
        m_requestServerReceivedTime = ns3::NanoSeconds(requestServerReceivedTimeNanoSeconds);
        m_requestServerProcessTime = ns3::NanoSeconds(requestServerProcessTimeNanoSeconds);
        m_requestServerFinishTime = ns3::NanoSeconds(requestServerFinishTimeNanoSeconds);
        m_requestServerResponseTime = ns3::NanoSeconds(requestServerResponseTimeNanoSeconds);
    }

    // 获取序列化大小
    virtual uint32_t GetSerializedSize (void) const
    {
        return 12 * sizeof (int64_t); // 存储两个时间戳，分别为64位整数
    }

    // 打印标签内容
    virtual void Print (std::ostream &os) const
    {
        os << prettyResults();
    }
   std::string prettyResults() const
    {
        std::ostringstream os;
        os << "--------------------------------------------------" << std::endl
        << "|                Server Status Report             |" << std::endl
        << "--------------------------------------------------" << std::endl
        << "| Metric                        | Value (seconds) |" << std::endl
        << "--------------------------------------------------" << std::endl
        << "| Server Status GenTime         | " << m_serverStatusGeneratedTime.As(ns3::Time::S) << "              |" << std::endl
        << "| FWD Status EnqueueTime        | " << m_fwdServerStatusEnqueueTime.As(ns3::Time::S) << "              |" << std::endl
        << "| FWD Status ProcessTime        | " << m_fwdServerStatusProcessTime.As(ns3::Time::S) << "              |" << std::endl
        << "| FWD Status FinishTime         | " << m_fwdServerStatusFinishTime.As(ns3::Time::S) << "              |" << std::endl
        << "| AP Status ReceiveTime         | " << m_serverStatusReceiveTime.As(ns3::Time::S) << "              |" << std::endl
        << "| User Request GenTime          | " << m_requestGeneratedTime.As(ns3::Time::S) << "              |" << std::endl
        << "| AP Request ReceiveTime        | " << m_requestAPReceivedTime.As(ns3::Time::S) << "              |" << std::endl
        << "| AP Request ForwardTime        | " << m_requestAPForwardTime.As(ns3::Time::S) << "              |" << std::endl
        << "| Server Request ReceivedTime   | " << m_requestServerReceivedTime.As(ns3::Time::S) << "              |" << std::endl
        << "| Server Request ProcessTime    | " << m_requestServerProcessTime.As(ns3::Time::S) << "              |" << std::endl
        << "| Server Request FinishTime     | " << m_requestServerFinishTime.As(ns3::Time::S) << "              |" << std::endl
        << "| Server Request ResponseTime   | " << m_requestServerResponseTime.As(ns3::Time::S) << "              |" << std::endl
        << "--------------------------------------------------";
        return os.str();
    }
    // 设置进入队列时间
    void SetFwdServerStatusEnqueueTime (ns3::Time time)
    {
        m_fwdServerStatusEnqueueTime = time;
    }

    // 获取进入队列时间
    ns3::Time GetServerStatusEnqueueTimeAtFwd (void) const
    {
        return m_fwdServerStatusEnqueueTime;
    }
    // 设置开始服务时间
    void SetFwdServerStatusProcessTime (ns3::Time time)
    {
        m_fwdServerStatusProcessTime = time;
    }

    // 获取开始服务时间
    ns3::Time GetServerStatusProcessTimeAtFwd(void) const
    {
        return m_fwdServerStatusProcessTime;
    }
    // 设置完成时间
    void SetFwdServerStatusFinishTime (ns3::Time time)
    {
        m_fwdServerStatusFinishTime = time;
    }

    // 获取完成时间
    ns3::Time GetServerStatusFinishTimeAtFwd(void) const
    {
        return m_fwdServerStatusFinishTime;
    }

    void SetServerStatusGeneratedTime (ns3::Time time)
    {
        m_serverStatusGeneratedTime = time;
    }

    ns3::Time GetStatusGeneratedTimeAtServer(void) const
    {
        return m_serverStatusGeneratedTime;
    }

    void SetServerStatusReceiveTime (ns3::Time time)
    {
        m_serverStatusReceiveTime = time;
    }

    ns3::Time GetServerStatusReceiveTimeAtAP(void) const
    {
        return m_serverStatusReceiveTime;
    }
    ns3::Time GetRequestGeneratedTime(void) const
    {
        return m_requestGeneratedTime;
    }
    void SetRequestGeneratedTime(ns3::Time time)
    {
        m_requestGeneratedTime = time;
    }
    ns3::Time GetRequestAPReceivedTime(void) const
    {
        return m_requestAPReceivedTime;
    }
    void SetRequestAPReceivedTime(ns3::Time time)
    {
        m_requestAPReceivedTime = time;
    }
    ns3::Time GetRequestAPForwardTime(void) const
    {
        return m_requestAPForwardTime;
    }
    void SetRequestAPForwardTime(ns3::Time time)
    {
        m_requestAPForwardTime = time;
    }
    ns3::Time GetRequestServerReceivedTime(void) const
    {
        return m_requestServerReceivedTime;
    }
    void SetRequestServerReceivedTime(ns3::Time time)
    {
        m_requestServerReceivedTime = time;
    }
    ns3::Time GetRequestServerProcessTime(void) const
    {
        return m_requestServerProcessTime;
    }
    void SetRequestServerProcessTime(ns3::Time time)
    {
        m_requestServerProcessTime = time;
    }

    ns3::Time GetRequestServerFinishTime(void) const
    {
        return m_requestServerFinishTime;
    }
    void SetRequestServerFinishTime(ns3::Time time)
    {
        m_requestServerFinishTime = time;
    }
    ns3::Time GetRequestServerResponseTime(void) const
    {
        return m_requestServerResponseTime;
    }
    void SetRequestServerResponseTime(ns3::Time time)
    {
        m_requestServerResponseTime = time;
    }
    void campareAndCopyTo(EventTimeTag& targetTag)
    {
        if (m_serverStatusGeneratedTime.IsStrictlyPositive())
        {
           targetTag.SetServerStatusGeneratedTime(m_serverStatusGeneratedTime);
        }
        if (m_fwdServerStatusEnqueueTime.IsStrictlyPositive())
        {
            targetTag.SetFwdServerStatusEnqueueTime(m_fwdServerStatusEnqueueTime);
        }
        if (m_fwdServerStatusProcessTime.IsStrictlyPositive())
        {
            targetTag.SetFwdServerStatusProcessTime(m_fwdServerStatusProcessTime);
        }
        if (m_fwdServerStatusFinishTime.IsStrictlyPositive())
        {
            targetTag.SetFwdServerStatusFinishTime(m_fwdServerStatusFinishTime);
        }
        if (m_serverStatusReceiveTime.IsStrictlyPositive())
        {
            targetTag.SetServerStatusReceiveTime(m_serverStatusReceiveTime);
        }
        if (m_requestGeneratedTime.IsStrictlyPositive())
        {
            targetTag.SetRequestGeneratedTime(m_requestGeneratedTime);
        }
        if (m_requestAPReceivedTime.IsStrictlyPositive())
        {
            targetTag.SetRequestAPReceivedTime(m_requestAPReceivedTime);
        }
        if (m_requestAPForwardTime.IsStrictlyPositive())
        {
            targetTag.SetRequestAPForwardTime(m_requestAPForwardTime);
        }
        if (m_requestServerReceivedTime.IsStrictlyPositive())
        {
            targetTag.SetRequestServerReceivedTime(m_requestServerReceivedTime);
        }
        if (m_requestServerProcessTime.IsStrictlyPositive())
        {
            targetTag.SetRequestServerProcessTime(m_requestServerProcessTime);
        }
        if (m_requestServerFinishTime.IsStrictlyPositive())
        {
            targetTag.SetRequestServerFinishTime(m_requestServerFinishTime);
        }
        if (m_requestServerResponseTime.IsStrictlyPositive())
        {
            targetTag.SetRequestServerResponseTime(m_requestServerResponseTime);
        }
    }
private:
    //Stage 1
    ns3::Time m_serverStatusGeneratedTime; //生成时间
    ns3::Time m_fwdServerStatusEnqueueTime;  // 在FWD的进队列时间
    ns3::Time m_fwdServerStatusProcessTime; // 在FWD开始服务时间
    ns3::Time m_fwdServerStatusFinishTime;   // 在FWD完成处理并转发时间

    ns3::Time m_serverStatusReceiveTime;// AP 接收到时间

    //Stage 2
    ns3::Time m_requestGeneratedTime; // 用户请求生成（发送）时间
    ns3::Time m_requestAPReceivedTime;//AP 收到用户请求的时间

    //Stage 3
    ns3::Time m_requestAPForwardTime; //AP 转发用户请求的时间
    ns3::Time m_requestServerReceivedTime;//Server 收到用户请求的时间

    //Stage 4
    ns3::Time m_requestServerProcessTime;//Server开始处理用户请求的时间
    ns3::Time m_requestServerFinishTime; //Server处理用户请求完毕的时间
    ns3::Time m_requestServerResponseTime; //Server响应用户请求的时间
};
#endif //PROCESSTIMETAG_H
