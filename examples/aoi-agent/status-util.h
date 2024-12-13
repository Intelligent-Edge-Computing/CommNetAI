#ifndef STATUS_UTIL_GENERATOR_H
#define STATUS_UTIL_GENERATOR_H

#include "json.hpp"

#include "ns3/core-module.h"
#include "ns3/simulator.h"

#include <atomic> // 添加头文件
#include <cstdlib>
#include <sstream>
#include <string>

namespace ns3
{
class Packet;}using json = nlohmann::json;
using namespace ns3;
class IdGenerator {
public:
    IdGenerator();

    uint32_t getNextId();

private:
    std::atomic<uint32_t> currentId;
};
enum StatusType
{
    REQUEST, // 0, User request
    SERVICE,  // 1, Service status
    RESPONSE // 2, Response status
};
class Status {
public:
    std::string seqid;
    std::string message;
    int64_t generatedTime = -1 ;
    int64_t experiencedTimeSinceLastGenerate = 0;
    int64_t serviceTimeInFwdStart = -1;
    int64_t serviceTimeInFwdEnd = -1;
    std::string jsonString;
    
    //service
    StatusType type = SERVICE;
    uint32_t capacity = 0; //当前能力
    
    //user
    uint32_t requirements = 0; //需求量
    
    Status(uint32_t seqid, std::string message, int64_t generatedTime);
    Status(std::string seqid, std::string message, int64_t generatedTime);
    Status();
};
static IdGenerator idGenServiceStatus;
static IdGenerator idGenRequest;
static IdGenerator idGenResponse;
Status generateMessage(StatusType, std::string);
Status generateServiceStatus();
Status deserialize(std::string jsonString);
std::string serialize(Status status);

class ConfigManager 
{
public:
    static json& GetConfig() {
        static json config;
        static bool isLoaded = false;
        // std::string file = "/Users/qijianpeng/Documents/git/ns3/ns-allinone-3.42/ns-3.42/scratch/aoi/settings.json";
        std::string file = "/workspace/ns-3/scratch/aoi/settings.json";

        if (!isLoaded) {
            std::ifstream inputFile(file);
            if (!inputFile.is_open()) {
                throw std::runtime_error("Unable to open settings.json");
            }
            config = json::parse(std::string((std::istreambuf_iterator<char>(inputFile)),
                                             std::istreambuf_iterator<char>()));
            isLoaded = true;
        }
        return config;
    }
private:
    ConfigManager() = default;  // 私有构造函数，防止实例化
};

#endif // STATUS_UTIL_GENERATOR_H