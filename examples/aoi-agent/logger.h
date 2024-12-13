#ifndef LOGGER_H
#define LOGGER_H

#include <sqlite3.h>
#include <string>
#include "ns3/simulator.h"
#include "status-util.h"
#include "ns3/packet.h"
#include "ns3/sqlite-output.h"

class Logger
{
public:
    Logger(const std::string &dbFile);
    ~Logger();
    // void Log(const std::string &component, const std::string &message);
    // void logStateObservation(const std::string& phase, std::string seqid);
    // void logAge(const std::string &component, uint64_t age, uint64_t currentTime);
    // void logAge(const std::string &component, uint64_t age);
    // void logAge(const std::string &component, uint64_t age, Status& status);
    // void logFwdDelay(std::string path, Ptr<const Packet> packet);

    void APReceiveServerStatusPacketTrace( Ptr<const Packet> oldPacket,  Ptr<const Packet> newPacket);
    void APForwardRequestToServerTrace(Ptr<const Packet> packet);
    void ServerReceiveRequestPacketTrace(Ptr<const Packet> packet);
    void ServerReponseRequestPacketTrace(Ptr<const Packet> packet);
    void APReceiveResponseFromServerTrace(Ptr<const Packet> packet);
    void CheckResult(uint32_t type);
    void CheckResultServerReceiveRequestPacketTrace(Ptr<const Packet> packet);
    void CheckResultServerDropRequestPacketTrace(Ptr<const Packet> packet);
    void CheckResultAPDropRequestTrace(Ptr<const Packet> packet);
    void logAccuracy();
    void clear();
    uint32_t TP,FP,TN,FN = 0;

private:
    // sqlite3 *m_db;
    Ptr<SQLiteOutput> m_sqliteOut; //!< Database
    json& config = ConfigManager::GetConfig();
    void ExecuteSQL(const std::string &sql);

};

#endif // LOGGER_H