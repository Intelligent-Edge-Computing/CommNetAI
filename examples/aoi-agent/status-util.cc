#include "status-util.h"

#include <sys/stat.h>
NS_LOG_COMPONENT_DEFINE("Utils");

IdGenerator::IdGenerator() : currentId(0) {}

uint32_t IdGenerator::getNextId() {
    return currentId++;
}
Status::Status(std::string seqid, std::string message, int64_t generatedTime)
    : seqid(seqid), message(message), generatedTime(generatedTime) {}
Status::Status(uint32_t seqid, std::string message, int64_t generatedTime)
    : Status(std::to_string(seqid), message, generatedTime) {}
Status::Status(){}



Status generateServiceStatus() {
    const char charset[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789";
    const size_t maxIndex = (sizeof(charset) - 1);
    std::string randomString;
    // Ptr<UniformRandomVariable> rndSize = CreateObject<UniformRandomVariable>();
    // rndSize->SetAttribute("Min", DoubleValue(100)); 
    // rndSize->SetAttribute("Max", DoubleValue(500)); 
    // uint64_t size = UintegerValue(rndSize->GetValue()).Get();
    for (uint64_t i = 0; i < 8; ++i) {
        randomString += charset[rand() % maxIndex];
    }
//TODO: 改为指定大小的空包
    Status status = generateMessage(StatusType::SERVICE, randomString);
    return status;
}


Status generateMessage(StatusType type, std::string message)
{
    int64_t now = ns3::Simulator::Now().GetMilliSeconds();
    uint32_t seqid;
    switch (type)
    {
    case StatusType::REQUEST:
        seqid = idGenRequest.getNextId();
        break;
    case StatusType::RESPONSE:
        seqid = idGenResponse.getNextId();
        break;
    case StatusType::SERVICE:
        seqid = idGenServiceStatus.getNextId();
        break;
    default:
        seqid = 0;
        break;
    }
    // json jsonMessage;
    // jsonMessage["generatedTime"] = now;
    // jsonMessage["type"] = type;
    // jsonMessage["seqid"] = seqid;
    // std::string jsonString = jsonMessage.dump();
    Status status = Status(seqid, message, now);
    status.type = type;
    return status;
}


Status deserialize(std::string jsonString) {
    json jsonMessage = json::parse(jsonString);
    std::string seqid = jsonMessage["seqid"];
    std::string message = jsonMessage["message"];
    int64_t generatedTime = jsonMessage["generatedTime"];
    Status status = Status(seqid, message, generatedTime);
    status.type = jsonMessage["type"];
    if (status.type == SERVICE)
    {//服务能力
        status.capacity = jsonMessage["capacity"];
        status.experiencedTimeSinceLastGenerate = jsonMessage["experiencedTimeSinceLastGenerate"];
        status.serviceTimeInFwdStart = jsonMessage["serviceTimeInFwdStart"];
        status.serviceTimeInFwdEnd = jsonMessage["serviceTimeInFwdEnd"];
    }
    if (status.type == REQUEST)
    {
        status.requirements = jsonMessage["requirements"];
    }
    status.jsonString = jsonString;
    return status;
}


std::string serialize(Status status) {
    json jsonMessage;
    jsonMessage["seqid"] = status.seqid;
    jsonMessage["message"] = status.message;
    jsonMessage["generatedTime"] = status.generatedTime;
    jsonMessage["type"] = status.type;
    ////服务能力
    if (status.type == SERVICE)
    {
        jsonMessage["capacity"] = status.capacity;
        jsonMessage["experiencedTimeSinceLastGenerate"] = status.experiencedTimeSinceLastGenerate;

        jsonMessage["serviceTimeInFwdStart"] = status.serviceTimeInFwdStart;
        jsonMessage["serviceTimeInFwdEnd"] = status.serviceTimeInFwdEnd;

    }
    //用户需求
    if (status.type == REQUEST)
    {
        jsonMessage["requirements"] = status.requirements;
    }
    status.jsonString = jsonMessage.dump();
    return status.jsonString;
}


json LoadConfig() {
    // std::string file = "/Users/qijianpeng/Documents/git/ns3/ns-allinone-3.42/ns-3.42/scratch/aoi/settings.json";
    std::string file = "/workspace/ns-3/scratch/aoi/settings.json";

    std::ifstream inputFile(file);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Unable to open settings.json" << std::endl;
    }
    try{
        std::string content((std::istreambuf_iterator<char>(inputFile)),
                            std::istreambuf_iterator<char>());
        return json::parse(content);
    } catch (json::parse_error &e) {
        std::cerr << "Error: Failed to parse settings.json - " << e.what() << std::endl;
    }
    return NULL;
}


