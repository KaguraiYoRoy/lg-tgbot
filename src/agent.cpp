#include <httplib.h>
#include <json/json.h>
#include <cstdio>
#include <string>
#include <regex>
#include <iostream>
#include <fstream>

void WriteLog(const std::string log){
    using sc = std::chrono::system_clock;
    std::time_t t = sc::to_time_t(sc::now());
    char timebuf[20];
    strftime(timebuf, 20, "%Y.%m.%d-%H:%M:%S", localtime(&t));
    std::cout<<timebuf<<": "<<log<<'\n';
    return;
}

std::string generate_uuid_v4() {
    // 用于生成随机字节的随机数引擎和分布
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint16_t> dis(0, 255);
    
    // 存储16个随机字节 (128位)
    std::array<uint8_t, 16> bytes;
    for (auto& byte : bytes) {
        byte = static_cast<uint8_t>(dis(gen));
    }
    
    // 设置UUID版本 (第7字节高4位为4)
    bytes[6] = (bytes[6] & 0x0F) | 0x40;  // 0100xxxx
    
    // 设置变体 (第9字节高2位为10)
    bytes[8] = (bytes[8] & 0x3F) | 0x80;  // 10xxxxxx
    
    // 将字节转换为十六进制字符串
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    
    // 按UUID格式输出
    for (size_t i = 0; i < 16; ++i) {
        // 在指定位置插入分隔符
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            oss << '-';
        }
        oss << std::setw(2) << static_cast<unsigned>(bytes[i]);
    }
    
    return oss.str();
}

std::string sanitize(const std::string& args){
    // 定义允许的字符集合（直接列出比正则更高效）
    static const std::string allowed_chars = 
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789"
        "._: \"\\/-";  // 包含空格、双引号、正斜杠、反斜杠、连字符
    
    std::string result;
    result.reserve(args.size()); // 预分配内存提高效率
    
    // 遍历输入字符串的每个字符
    for (char c : args) {
        // 检查字符是否在允许集合中
        if (allowed_chars.find(c) != std::string::npos) {
            result += c;  // 安全字符加入结果
        }
    }
    return result;
}

std::string exec_command(const char* cmd) {
    // 创建管道并执行命令
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    // 读取命令输出
    std::string result;
    char buffer[128];
    while(fgets(buffer, sizeof(buffer), pipe.get())) {
        result += buffer;
    }

    return result;
}

std::string res_to_json(std::string cmdres){
    Json::Value root;
    Json::FastWriter writer;
    root["status"]=200;
    root["res"]=cmdres;
    return writer.write(root);
}

int main(){
    Json::Value configRoot;
    Json::Reader configReader;
    Json::FastWriter configWriter;
    std::ifstream configReadFile("config.json");
    std::string uuid;
    std::string bind;
    unsigned short port=0;
    if(!configReadFile 
        || !configReader.parse(configReadFile,configRoot)
        || !configRoot.isMember("uuid")
    ){
        uuid=generate_uuid_v4();
        configRoot["uuid"]=uuid;
        WriteLog("No UUID found. Generated: "+uuid);
    }
    else
        uuid=configRoot["uuid"].asString();
    WriteLog("Using uuid: "+uuid);

    if(!configRoot.isMember("bind")){
        WriteLog("No bind address defined. Using default: 0.0.0.0");
        configRoot["bind"]="0.0.0.0";
    }
    bind=configRoot["bind"].asString();
    WriteLog("Using bind address: "+bind);
    
    if(!configRoot.isMember("port")){
        WriteLog("No port defined. Using default: 8080");
        configRoot["port"]=8080;
    }
    WriteLog("Using port: "+configRoot["port"].asString());
    port=configRoot["port"].asUInt();

    std::ofstream configWriteFile("config.json");
    configWriteFile<<configWriter.write(configRoot);
    configWriteFile.close();

    httplib::Server svr;

    svr.Get("/trace", [&](const httplib::Request &req, httplib::Response &res) {
        if (!req.has_param("target") || !req.has_param("uuid")) {
            res.status = 400; // Bad Request
            return;
        }
        if(req.get_param_value("uuid") != uuid.c_str()){
            res.status = 401;
            return;
        }
        std::string target = sanitize(req.get_param_value("target"));
        char cmd[1024];
        sprintf(cmd, "traceroute -w 1 -N 100 %s 2>&1",target.c_str());
        std::string cmdres = exec_command(cmd);
        std::string jsonres = res_to_json(cmdres);
        
        WriteLog("Traceroute "+target);

        res.set_content(jsonres.c_str(), "application/json");

    });

    svr.Get("/ping", [&](const httplib::Request &req, httplib::Response &res) {
        if (!req.has_param("target") || !req.has_param("uuid")) {
            res.status = 400; // Bad Request
            return;
        }
        if(req.get_param_value("uuid") != uuid.c_str()){
            res.status = 401;
            return;
        }
        std::string target = sanitize(req.get_param_value("target"));
        char cmd[1024];
        sprintf(cmd, "ping -i 0.01 -c 4 -W 1 %s 2>&1",target.c_str());
        std::string cmdres = exec_command(cmd);
        std::string jsonres = res_to_json(cmdres);
        
        WriteLog("Ping "+target);

        res.set_content(jsonres.c_str(), "application/json");

    });

    svr.Get("/tcping", [&](const httplib::Request &req, httplib::Response &res) {
        if (!req.has_param("host") || !req.has_param("port") || !req.has_param("uuid")) {
            res.status = 400; // Bad Request
            return;
        }
        if(req.get_param_value("uuid") != uuid.c_str()){
            res.status = 401;
            return;
        }
        std::string host = sanitize(req.get_param_value("host"));
        std::string port = sanitize(req.get_param_value("port"));
        char cmd[1024];
        sprintf(cmd, "tcping -x 4 %s %s 2>&1",host.c_str(),port.c_str());
        std::string cmdres = exec_command(cmd);
        std::string jsonres = res_to_json(cmdres);

        WriteLog("TCPing "+host+":"+port);

        res.set_content(jsonres.c_str(), "application/json");

    });

    svr.Get("/route", [&](const httplib::Request &req, httplib::Response &res) {
        if (!req.has_param("target") || !req.has_param("uuid")) {
            res.status = 400; // Bad Request
            return;
        }
        if(req.get_param_value("uuid") != uuid.c_str()){
            res.status = 401;
            return;
        }
        std::string target = sanitize(req.get_param_value("target"));
        char cmd[1024];
        sprintf(cmd, "birdc show route for %s 2>&1",target.c_str());

        std::string cmdres = exec_command(cmd);
        std::string jsonres = res_to_json(cmdres);

        WriteLog("Show route for "+target);

        res.set_content(jsonres.c_str(), "application/json");

    });

    svr.listen(bind.c_str(), port);

    return 0;
}