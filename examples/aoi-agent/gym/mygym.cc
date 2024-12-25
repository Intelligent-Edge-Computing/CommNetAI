/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Technische Universität Berlin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Piotr Gawlowicz <gawlowicz@tkn.tu-berlin.de>
 */

#include "mygym.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/node-list.h"
#include "ns3/log.h"
#include <sstream>
#include <iostream>
#include "ns3/network-module.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include <algorithm>
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/channel.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/simulator.h"
#include <limits>
#include "../server/server-app.h"
#include "../ap/ap-app.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MyGymEnv");

NS_OBJECT_ENSURE_REGISTERED (MyGymEnv);

MyGymEnv::MyGymEnv ()
{
  NS_LOG_FUNCTION (this);
  m_interval = MilliSeconds(1); //每m_interval与环境交互一次。
  Simulator::Schedule (Seconds(0.0), &MyGymEnv::ScheduleNextStateRead, this);
}

MyGymEnv::MyGymEnv (Time stepTime)
{
  NS_LOG_FUNCTION (this);
  m_interval = stepTime;

  Simulator::Schedule (Seconds(0.0), &MyGymEnv::ScheduleNextStateRead, this);
}
void MyGymEnv::DoInitialize()
{
    Ptr<OpenGymDiscreteContainer> defaultAction = CreateObject<OpenGymDiscreteContainer> ();
    defaultAction->SetValue(0);
    m_actionTracker.SaveAction(defaultAction);
}


void
MyGymEnv::ScheduleNextStateRead ()
{
  NS_LOG_FUNCTION (this);
  Simulator::Schedule (m_interval, &MyGymEnv::ScheduleNextStateRead, this);
  Notify();
}

MyGymEnv::~MyGymEnv ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
MyGymEnv::GetTypeId (void)
{
  static TypeId tid = TypeId ("MyGymEnv")
    .SetParent<OpenGymEnv> ()
    .SetGroupName ("OpenGym")
    .AddConstructor<MyGymEnv> ()
  ;
  return tid;
}

void
MyGymEnv::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

/*定义观测状态空间大小 9 维空间：
1. 计算节点状态 2D
  当前服务能力并发数S_c, 取值{0,1,2,3,...,M_{S_c}}
  当前服务能力状态（剩余并发数量）已经持续的时长\delta_t^c \in \{0, 1, 2, ..., M_c\}，上限M_c
2. AP节点状态 4D
  状态年龄∆t∈{0,1,2,…,∆}
  ∆t前的状态内容（t-∆t时刻的服务节点剩余并发数）
  \delta_t^u 用户上次访问距今的时长
  资源需求大小s_t^{cpu}
3. 网络状态 3D
  a_t^{req} \in \{0,1\}，请求发送状态, 1代表在时刻t有请求，0则相反
  s_t^d发送的数据量data大小，s_t^d\in\{0,1,2,\ldots,M_d\}分级、上取整，减少状态空间
  s_t^{bw} backward信道状态：带宽分n级\{B_1,B_2,\ldots,B_n\}
*/
Ptr<OpenGymSpace>
MyGymEnv::GetObservationSpace()
{
    uint32_t parameterNum = 9;
   //Server
    float service_capicity_low = 0.0;
    float service_capicity_high = 2.0;
    float service_capicity_holding_life_low = 0.0;
    float service_capicity_holding_life_high = 1000.0;

  //AP
    float ap_aoi_low = 0.0; //0是不实际的，传输过程的时延就已经大于0了。
    float ap_aoi_high = 1000.0;//上限应当结合实际调整
    float ap_service_capicity_low = service_capicity_low;
    float ap_service_capicity_high = service_capicity_high;
    float ap_delta_t_u_low = 0.0;
    float ap_delta_t_u_high = 1000.0;
    float ap_user_task_size_low = 1000.0;
    float ap_user_task_size_high = 1500.0;

  //Networking
    float net_a_t_req_low = 0.0;
    float net_a_t_req_high = 1.0;
    float net_s_t_d_low = 1.0; //data size, rounded in Bytes, KiB, MiB, ...
    float net_s_t_d_high = 1000.0;

    float net_s_t_bw_low = 0.0;
    DataRateValue dataRateValue = DataRate("1Mbps");
    uint64_t bitRate = dataRateValue.Get().GetBitRate();
    float net_s_t_bw_high = bitRate; //最大的链路带宽。NOTE：应该改为首先获取两节点路由，然后计算这条路由的最小（瓶颈）带宽

    std::vector<float> low = {service_capicity_low, service_capicity_holding_life_low, ap_aoi_low, ap_service_capicity_low, ap_delta_t_u_low, ap_user_task_size_low, net_a_t_req_low, net_s_t_d_low, net_s_t_bw_low};
    std::vector<float> high = {service_capicity_high, service_capicity_holding_life_high, ap_aoi_high, ap_service_capicity_high, ap_delta_t_u_high, ap_user_task_size_high, net_a_t_req_high, net_s_t_d_high, net_s_t_bw_high};

    std::vector<uint32_t> shape = {parameterNum,};
    std::string dtype = TypeNameGet<uint64_t> ();

    Ptr<OpenGymBoxSpace> space = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
    NS_LOG_UNCOND ("MyGetObservationSpace: " << space);
    return space;
}

/*
    a \in {0,1}
*/
Ptr<OpenGymSpace>
MyGymEnv::GetActionSpace()
{
    uint32_t decisions = 2; // 0或1，发送或不发松
    Ptr<OpenGymDiscreteSpace> space = CreateObject<OpenGymDiscreteSpace> (decisions);
    NS_LOG_UNCOND ("MyGetActionSpace: " << space);
    return space;
}

/*
Define game over condition
*/
bool
MyGymEnv::GetGameOver()
{
  bool isGameOver = false;
  bool test = false;
  static float stepCounter = 0.0;
  stepCounter += 1;
  if (stepCounter == 10 && test) {
      isGameOver = true;
  }
  NS_LOG_UNCOND ("MyGetGameOver: " << isGameOver);
  return isGameOver;
}

/*
Collect observations
*/
Ptr<OpenGymDataContainer>
MyGymEnv::GetObservation()
{
  uint32_t parameterNum = 9;
  std::vector<uint32_t> shape = {parameterNum,};
  Ptr<OpenGymBoxContainer<uint64_t> > box = CreateObject<OpenGymBoxContainer<uint64_t> >(shape);
  Ptr<Node> serverNode = NodeList::GetNode(0);
  Ptr<ServerApp> serverApp = serverNode->GetApplication(0)->GetObject<ServerApp>();

  Ptr<Node> apNode = NodeList::GetNode(2);
  Ptr<ApApp> apApp = apNode->GetApplication(0)->GetObject<ApApp>();

  ServerResourceTracker serverTracker = serverApp->GetServerResourceTracker();
  float service_capicity = serverTracker.GetAvailableSeats();
  float service_capicity_holding_life = serverTracker.GetDuration().GetMilliSeconds();

  //Server observiations
  box->AddValue(service_capicity);
  box->AddValue(service_capicity_holding_life);

  //AP observiations
  Ptr<ApActivityTracker> apActivityTracker = apApp->GetObject<ApActivityTracker>();
  if (apActivityTracker != nullptr)
  {
     box->AddValue(apActivityTracker->GetAge());
     box->AddValue(apActivityTracker->GetServiceStatus().capacity); //capacity at Age ago.
     box->AddValue(apActivityTracker->GetUserLastVisitDuration());

  //Networking observations
    box->AddValue(apActivityTracker->GetUserLastVisitDuration() == 0); //当前时刻有请求发送
    box->AddValue(apActivityTracker->GetUserRequestTaskSize()); //发送任务大小
    box->AddValue(apActivityTracker->GetUserRequestDataSize());//发送的数据量
  }
  else
  {
    NS_LOG_ERROR("RunningInfo is not set or not found!");
  }

  //  box->AddValue(net_s_t_bw); //backward 信道状态

  // Print data
  NS_LOG_INFO ("MyGetObservation: " << box);
  return box;
}

  // Sigmoid 函数实现
double Sigmoid(double x, double k = 1.0) {
  return 1.0 / (1.0 + std::exp(-k * x));
}
/*
Define reward function
*/
float
MyGymEnv::GetReward()
{
  Ptr<Node> serverNode = NodeList::GetNode(0);
  Ptr<ServerApp> serverApp = serverNode->GetApplication(0)->GetObject<ServerApp>();

  Ptr<Node> apNode = NodeList::GetNode(2);
  Ptr<ApApp> apApp = apNode->GetApplication(0)->GetObject<ApApp>();
  Ptr<ApActivityTracker> apActivityTracker = apApp->GetObject<ApActivityTracker>();
  float reward = 0.0;
  //1. TP、FP在执行后的时刻获得奖励；FN、TN在访问AP时可获得奖励
  //TODO reward += 1;
  //2. 其他时刻，获得更新代价
  Ptr<OpenGymDiscreteContainer> action = DynamicCast<OpenGymDiscreteContainer>(m_actionTracker.GetAction());
  uint32_t update = action->GetValue();
  if (update == 1)
  {
    reward -= 1.0;
  }
  if (apActivityTracker != nullptr)
  {
    int a_t_req = (apActivityTracker->GetUserLastVisitDuration() == 0? 1 : 0); // 恰好有用户请求
    reward = reward - a_t_req * Sigmoid(apActivityTracker->GetAge());
  }
  //3. 探索奖励
  // reward = reward + r_intrinsic
  return reward;
}

/*
Define extra info. Optional
*/
std::string
MyGymEnv::GetExtraInfo()
{
  std::string myInfo = "testInfo";
  NS_LOG_UNCOND("MyGetExtraInfo: " << myInfo);
  return myInfo;
}

/*
Execute received actions
*/
bool
MyGymEnv::ExecuteActions(Ptr<OpenGymDataContainer> action)
{
  //get server
  Ptr<Node> serverNode = NodeList::GetNode(0);
  Ptr<ServerApp> serverApp = serverNode->GetApplication(0)->GetObject<ServerApp>();
  //parse action and execute
  m_actionTracker.SaveAction(action);
  Ptr<OpenGymDiscreteContainer> action = DynamicCast<OpenGymDiscreteContainer>(action);
  uint32_t update = action->GetValue();
  if (update == 1)
  {
    serverApp->SendServiceStatus();
  }
  NS_LOG_UNCOND("[Agent] Decide to " << (update == 1 ? "update. " : "not update. ") << "action=" << action);
  return true;
}

} // ns3 namespace
