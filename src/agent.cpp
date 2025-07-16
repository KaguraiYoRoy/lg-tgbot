#include <httplib.h>
#include <json/json.h>
#include <cstdio>
#include <string>
#include <regex>

void WriteLog(const std::string log){
    using sc = std::chrono::system_clock;
    std::time_t t = sc::to_time_t(sc::now());
    char timebuf[20];
    strftime(timebuf, 20, "%Y.%m.%d-%H:%M:%S", localtime(&t));
    std::cout<<timebuf<<": "<<log<<'\n';
    return;
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
    
    httplib::Server svr;

    svr.Get("/trace", [](const httplib::Request &req, httplib::Response &res) {
        if (!req.has_param("target")) {
            res.status = 400; // Bad Request
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

    svr.Get("/ping", [](const httplib::Request &req, httplib::Response &res) {
        if (!req.has_param("target")) {
            res.status = 400; // Bad Request
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

    svr.Get("/tcping", [](const httplib::Request &req, httplib::Response &res) {
        if (!req.has_param("host") || !req.has_param("port")) {
            res.status = 400; // Bad Request
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

    svr.Get("/route", [](const httplib::Request &req, httplib::Response &res) {
        if (!req.has_param("target")) {
            res.status = 400; // Bad Request
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

    svr.listen("0.0.0.0", 8080);

    return 0;
}