#include "logger.h"

#include "event-time-tag.h"
#include "server/server-app.h"
#include "server/server-queue.h"
#include "status-util.h"

#include "ns3/application-container.h"
#include "ns3/core-module.h"
#include "ns3/node-list.h"

#include <iomanip>
#include <iostream>
#include <sstream>
NS_LOG_COMPONENT_DEFINE("Logger");

Logger::Logger(const std::string &dbFile)
{

    if (!config["log"]["enable"]) return;
    if (!m_sqliteOut)
    {
        m_sqliteOut = new SQLiteOutput(dbFile);
    }
    // ExecuteSQL("CREATE TABLE IF NOT EXISTS Logs ("
    //            "Timestamp TEXT NOT NULL, "
    //            "Component TEXT NOT NULL, "
    //            "Message TEXT NOT NULL);");
    // ExecuteSQL("CREATE TABLE IF NOT EXISTS StateObservation ("
    //             "Timestamp INTEGER NOT NULL, "
    //             "Phase TEXT NOT NULL, "
    //             "StateSeqID TEXT NOT NULL);");
    // ExecuteSQL("CREATE TABLE IF NOT EXISTS LogAge ("
    //             "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
    //             "Timestamp INTEGER NOT NULL, "
    //             "Component TEXT NOT NULL, "
    //             "Age INTEGER NOT NULL);");
    // ExecuteSQL("CREATE TABLE IF NOT EXISTS LogUtilization("
    //             "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
    //             "Timestamp INTEGER NOT NULL, "
    //             "Component TEXT NOT NULL, "
    //             "Age INTEGER NOT NULL,"
    //             "Utilization REAL NOT NULL,"
    //             "ServerStatusUpdateInterval REAL NOT NULL,"
    //             "RunID INTEGER default 1);");
    bool res;
    res = m_sqliteOut->SpinExec("CREATE TABLE IF NOT EXISTS StageOneLogs ("
            "Timestamp INTEGER, "
            "generatedTime INTEGER, "
            "fwdServerStatusEnqueueTime INTEGER, "
            "fwdServerStatusProcessTime INTEGER, "
            "fwdServerStatusFinishTime INTEGER, "
            "apReceiveTime INTEGER, "
            "lastServerStatusReceiveTimeAtAP INTEGER, "
            "lastServerStatusGeneratedTime INTEGER, "
            "runID INTEGER)");
    NS_ASSERT(res);

    res = m_sqliteOut->SpinExec("CREATE TABLE IF NOT EXISTS Accuracy ("
            "TP INTEGER, "
            "FP INTEGER, "
            "TN INTEGER, "
            "FN INTEGER, "
            "runID INTEGER)");
    if(ConfigManager::GetConfig()["log"]["clear"])
    {
        // ExecuteSQL("delete from StateObservation;");
        // ExecuteSQL("delete from LogAge;");
        // ExecuteSQL("delete from StateObservation;");
        // ExecuteSQL("delete from LogUtilization;");
        m_sqliteOut->SpinExec("delete from StageOneLogs;");
        m_sqliteOut->SpinExec("delete from Metadata;");
        m_sqliteOut->SpinExec("delete from Experiments;");
        m_sqliteOut->SpinExec("delete from Singletons;");
        m_sqliteOut->SpinExec("delete from Accuracy;");
        ConfigManager::GetConfig()["log"]["clear"] = false; // cleared
    }
    m_sqliteOut->Unref();
}

Logger::~Logger()
{
    if (m_sqliteOut)
    {
        m_sqliteOut = nullptr;
    }
}
//
// void Logger::Log(const std::string &component, const std::string &message)
// {
//     if (!m_sqliteOut)
//         return;
//
//     ns3::Time now = ns3::Simulator::Now();
//     std::stringstream ss;
//     ss << std::fixed << std::setprecision(6) << now.GetSeconds();
//     std::string timestamp = ss.str();
//
//     std::string sql = "INSERT INTO Logs (Timestamp, Component, Message) VALUES ('" +
//                       timestamp + "', '" + component + "', '" + message + "');";
//     ExecuteSQL(sql);
// }
//
// void Logger::logStateObservation(const std::string& phase, std::string seqid)
// {
//     int64_t now = ns3::Simulator::Now().GetMilliSeconds();
//     std::string sql = "INSERT INTO StateObservation (Timestamp, Phase, StateSeqID) VALUES (" +
//                       std::to_string(now) + ", '" + phase + "', " + seqid + ");";
//     ExecuteSQL(sql);
// }
// void Logger::logAge(const std::string &component, uint64_t age, uint64_t currentTime)
// {
//     long statusUpdateIntervalInMs = ConfigManager::GetConfig()["server"]["statusUpdateInterval"];
//     double updateRate = 1000.0/statusUpdateIntervalInMs;///note: in ms.
//     float serviceTime = ConfigManager::GetConfig()["forwarder"]["serviceTime"];
//     double serviceRate = 1.0/serviceTime;
//     double utilization = updateRate/serviceRate;
//     std::string sql = "INSERT INTO LogAge (Timestamp, Component, Age) VALUES (" +
//                       std::to_string(currentTime) + ", '" + component + "', " + std::to_string(age) + ");";
//
//     std::string sql2 = "INSERT INTO LogUtilization (Timestamp, Component, Age, Utilization, ServerStatusUpdateInterval, RunID) VALUES (" +
//                       std::to_string(currentTime) + ", '"+
//                       component + "', " +
//                       std::to_string(age) + ", " +
//                       std::to_string(utilization) + ", " +
//                       std::to_string(statusUpdateIntervalInMs) + ", " +
//                       std::to_string(SeedManager::GetRun()) + ");";
//     ExecuteSQL(sql);
//     ExecuteSQL(sql2);
//
// }
// void Logger::logAge(const std::string &component, uint64_t age, Status& status)
// {
//     int64_t currentTime = ns3::Simulator::Now().GetMilliSeconds();
//     long statusUpdateIntervalInMs = status.experiencedTimeSinceLastGenerate;
//
//     double updateRate = config["server"]["ExpDist"]["mean"];
//     updateRate = 1.0/ updateRate;
//
//     // float serviceTime = config["forwarder"]["serviceTime"];
//     float serviceTime = config["forwarder"]["serviceTime"];
//     // float waittingTime = age - serviceTime;
//
//     double serviceRate = 1.0/serviceTime;
//     double utilization = updateRate/serviceRate;
//     std::string sql = "INSERT INTO LogAge (Timestamp, Component, Age) VALUES (" +
//                       std::to_string(currentTime) + ", '" +
//                       component + "', " +
//                       std::to_string(age) + ");";
//
//     std::string sql2 = "INSERT INTO LogUtilization (Timestamp, Component, Age, Utilization, ServerStatusUpdateInterval, RunID) VALUES (" +
//                       std::to_string(currentTime) + ", '" +
//                       component + "', " +
//                       std::to_string(age) + ", " +
//                       std::to_string(utilization) + ", " +
//                       std::to_string(statusUpdateIntervalInMs) + ", "  +
//                       std::to_string(SeedManager::GetRun()) + ");";
//     ExecuteSQL(sql);
//     ExecuteSQL(sql2);
// }
// void Logger::logAge(const std::string &component, uint64_t age)
// {
//     // uint64_t currentTime = ns3::Simulator::Now().GetNanoSeconds();
//     uint64_t currentTime = ns3::Simulator::Now().GetMilliSeconds();
//     logAge(component, age, currentTime);
// }
// void Logger::ExecuteSQL(const std::string &sql)
// {
//     m_sqliteOut->SpinExec(sql);
// }
// void   Logger::logFwdDelay(std::string path, Ptr<const Packet> packet)
// {
//     EventTimeTag timeTag;
//     if (packet->PeekPacketTag(timeTag))
//     {
//         Time enqueueTime = timeTag.GetServerStatusEnqueueTimeAtFwd();
//         Time processTime = timeTag.GetServerStatusProcessTimeAtFwd();
//         Time finishTime = timeTag.GetServerStatusFinishTimeAtFwd();
//     }
// }

//Stage 1
 void Logger::APReceiveServerStatusPacketTrace(Ptr<const Packet> oldPacket, Ptr<const Packet> newPacket) {
    if (!config["log"]["enable"]) return;
    NS_LOG_DEBUG ("Stage 1. APReceivedServiceStatusPacketTrace triggered");
    EventTimeTag historyTimeTag, recentTimeTag;
    oldPacket->PeekPacketTag(historyTimeTag);
    newPacket->PeekPacketTag(recentTimeTag);

    Time recentTime = recentTimeTag.GetServerStatusReceiveTimeAtAP();
    Time generatedTime = recentTimeTag.GetStatusGeneratedTimeAtServer();
    Time fwdServerStatusEnqueueTime = recentTimeTag.GetServerStatusEnqueueTimeAtFwd();
    Time fwdServerStatusProcessTime = recentTimeTag.GetServerStatusProcessTimeAtFwd();
    Time fwdServerStatusFinishTime = recentTimeTag.GetServerStatusFinishTimeAtFwd();
    Time apReceiveTime = recentTimeTag.GetServerStatusReceiveTimeAtAP();
    Time lastServerStatusReceiveTimeAtAP =historyTimeTag.GetServerStatusReceiveTimeAtAP();//Old status.
    Time lastServerStatusGeneratedTime = historyTimeTag.GetStatusGeneratedTimeAtServer();//old status
    uint32_t runID = SeedManager::GetRun();

    bool res;
    sqlite3_stmt* stmt;
    res = m_sqliteOut->WaitPrepare(&stmt,
            "INSERT INTO StageOneLogs(Timestamp, generatedTime, fwdServerStatusEnqueueTime, "
                "fwdServerStatusProcessTime, fwdServerStatusFinishTime, "
                "apReceiveTime, lastServerStatusReceiveTimeAtAP, lastServerStatusGeneratedTime, runID) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    NS_ASSERT(res);
    res = m_sqliteOut->Bind(stmt, 1, recentTime);
    NS_ASSERT(res);
    res = m_sqliteOut->Bind(stmt, 2, generatedTime);
    NS_ASSERT(res);
    res = m_sqliteOut->Bind(stmt, 3, fwdServerStatusEnqueueTime);
    NS_ASSERT(res);
    res = m_sqliteOut->Bind(stmt, 4, fwdServerStatusProcessTime);
    NS_ASSERT(res);
    res = m_sqliteOut->Bind(stmt, 5, fwdServerStatusFinishTime);
    NS_ASSERT(res);
    res = m_sqliteOut->Bind(stmt, 6, apReceiveTime);
    NS_ASSERT(res);
    res = m_sqliteOut->Bind(stmt, 7, lastServerStatusReceiveTimeAtAP);
    NS_ASSERT(res);
    res = m_sqliteOut->Bind(stmt, 8, lastServerStatusGeneratedTime);
    NS_ASSERT(res);
    res = m_sqliteOut->Bind(stmt, 9, runID);
    NS_ASSERT(res);

    res = m_sqliteOut->SpinStep(stmt);
    NS_ASSERT(res);
    res = m_sqliteOut->SpinFinalize(stmt);
    NS_ASSERT(res == 0);
    // m_sqliteOut->Unref();
}

//Stage 2
void Logger::APForwardRequestToServerTrace(Ptr<const Packet> packet) {
    if (!config["log"]["enable"]) return;

    NS_LOG_DEBUG ("Stage 2. APForwardRequestToServerTrace triggered");

}

//Stage 3 Received and start to execute
void Logger::ServerReceiveRequestPacketTrace(Ptr<const Packet> packet)
{
    if (!config["log"]["enable"]) return;

    NS_LOG_DEBUG("Stage 3: ServerReceivedRequestPacketTrace triggered");
}

//Stage 4 Finshed
void Logger::ServerReponseRequestPacketTrace(Ptr<const Packet> packet) {
    if (!config["log"]["enable"]) return;

    NS_LOG_DEBUG ("Stage 4. ReponseRequestPacketTrace triggered");
}

void Logger::APReceiveResponseFromServerTrace(Ptr<const Packet> packet) {
    if (!config["log"]["enable"]) return;

    NS_LOG_DEBUG ("Stage 4. ReponseRequestPacketTrace triggered");
}
void Logger::CheckResultServerReceiveRequestPacketTrace(Ptr<const Packet> packet)
{
    CheckResult(0);
}

void Logger::CheckResultServerDropRequestPacketTrace(Ptr<const Packet> packet)
{
    CheckResult(1);
}

void Logger::CheckResultAPDropRequestTrace(Ptr<const Packet> packet)
{
    CheckResult(2);
}

void
Logger::clear()
{
   TP = FP = TN = FN = 0;
}

void Logger::CheckResult(uint32_t type)
{
    if (type == 0 )
    {//TP
        NS_LOG_DEBUG ("TP");
        TP ++;
    }
    else if (type == 1)
    {//FP
        NS_LOG_DEBUG ("FP");
        FP++;
    }
    else if (type == 2)
    {//FN+TN
        Ptr<ServerQueue> server_queue;
        Ptr<Application> app = NodeList::GetNode(0)->GetApplication(0);
        Ptr<ServerApp> serverApp = DynamicCast<ServerApp>(app);
        if (serverApp != nullptr)
        {
            server_queue = serverApp->GetRequestQueue();
            if (server_queue->isFull())
            {//TN
                NS_LOG_DEBUG ("TN");
                TN++;
            }
            else
            {//FN
                NS_LOG_DEBUG ("FN");
                FN++;
            }
        }
        // Config::Get("/NodeList/0/ApplicationList/0/$ns3::ServerApp/ServerQueue", server_queue);
    }
}
void Logger::logAccuracy()
{
    if (!config["log"]["enable"]) return;
    uint32_t runID = SeedManager::GetRun();
    bool res;
    sqlite3_stmt* stmt;
    res = m_sqliteOut->WaitPrepare(&stmt,
            "INSERT INTO Accuracy(TP, FP, TN, FN, runID) VALUES (?, ?, ?, ?, ?)");
    NS_ASSERT(res);
    res = m_sqliteOut->Bind(stmt, 1, TP);
    NS_ASSERT(res);
    res = m_sqliteOut->Bind(stmt, 2, FP);
    NS_ASSERT(res);
    res = m_sqliteOut->Bind(stmt, 3, TN);
    NS_ASSERT(res);
    res = m_sqliteOut->Bind(stmt, 4, FN);
    NS_ASSERT(res);
    res = m_sqliteOut->Bind(stmt, 5, runID);
    NS_ASSERT(res);

    res = m_sqliteOut->SpinStep(stmt);
    NS_ASSERT(res);
    res = m_sqliteOut->SpinFinalize(stmt);
    NS_ASSERT(res == 0);
}