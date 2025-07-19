#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>
#include <tgbot/tgbot.h>
#include <json/json.h>
#include <httplib.h>
#include <Log.h>

#define DEFAULT_LOGLEVEL_P LEVEL_WARN
#define DEFAULT_LOGLEVEL_W LEVEL_ERROR
#define DEFAULT_TIMEOUT_CONN 300
#define DEFAULT_TIMEOUT_READ 3000
#define DEFAULT_TIMEOUT_ALL 5000

Log mLog;

int main(){
    Json::Value configRoot;
    Json::Reader configReader;
    std::ifstream fileReader("config.json");

    if(!fileReader){
        mLog.push(LEVEL_FATAL,"No config file found!");
        return 1;
    }

    if (!configReader.parse(fileReader, configRoot)) {
        mLog.push(LEVEL_FATAL,"Config invalid!");
        return 1;
    }

    mLog.set_level(
        configRoot.isMember("loglevel-print")&&configRoot["loglevel-print"].isInt()?configRoot["loglevel-print"].asInt():DEFAULT_LOGLEVEL_P,
        configRoot.isMember("loglevel-write")&&configRoot["loglevel-write"].isInt()?configRoot["loglevel-write"].asInt():DEFAULT_LOGLEVEL_W
    );
    mLog.open("master.log");

    for(unsigned int i=0;i<configRoot["nodes"].size();i++){
        if(!configRoot["nodes"][i].isMember("uuid") || !configRoot["nodes"][i]["uuid"].isString()){
            if(!configRoot["nodes"][i].isMember("name") || !configRoot["nodes"][i]["name"].isString())
                mLog.push(LEVEL_FATAL,"Config parse error: Node %s does not have UUID!",configRoot["nodes"][i]["name"].asCString());
            else mLog.push(LEVEL_FATAL,"Config parse error: Node %d does not have UUID!",i);
            return 1;
        }
    }
    
    std::string token(configRoot["token"].asCString());
    mLog.push(LEVEL_INFO,"Token: %s",token.c_str());

    unsigned int timeoutConn=configRoot.isMember("timeout-connection")&&configRoot["timeout-connection"].isInt()?configRoot["timeout-connection"].asUInt():DEFAULT_TIMEOUT_CONN,
        timeoutRead=configRoot.isMember("timeout-read")&&configRoot["timeout-read"].isInt()?configRoot["timeout-read"].asUInt():DEFAULT_TIMEOUT_READ,
        timeoutAll=configRoot.isMember("timeout-all")&&configRoot["timeout-all"].isInt()?configRoot["timeout-all"].asUInt():DEFAULT_TIMEOUT_ALL;

    TgBot::Bot bot(token);
    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Manual:\n/ping <host>: Ping target host\n/tcping <host> <port>: TCPing host:port\n/trace <host>: Traceroute target host\n/route <host>: Get route for target host\n");
    });

    bot.getEvents().onCommand("ping", [&](TgBot::Message::Ptr message){
        std::string resstr = "";
        
        std::istringstream messageParser(message->text);
        std::string msgparams[16];
        short msgparamsCount=0;
        while(messageParser>>msgparams[msgparamsCount])msgparamsCount++;

        std::string params="/ping?target="+msgparams[1];

        mLog.push(LEVEL_INFO,"User %li[%s %s] required: Ping target: %s",
            message->from->id,
            message->from->firstName.c_str(),message->from->lastName.c_str(),
            msgparams[1].c_str()
        );

        for(unsigned int i=0;i<configRoot["nodes"].size();i++){
            resstr+="Node: "+configRoot["nodes"][i]["name"].asString()+"\n";
            std::string weburl=configRoot["nodes"][i]["url"].asString();
            httplib::Client cli(weburl.c_str());
            cli.set_connection_timeout(std::chrono::milliseconds(timeoutConn));
            cli.set_read_timeout(std::chrono::milliseconds(timeoutRead));
            cli.set_max_timeout(std::chrono::milliseconds(timeoutAll)); 

            auto res = cli.Get(((std::string)(params+"&uuid="+configRoot["nodes"][i]["uuid"].asString())).c_str());
            if(!res||res->status!=httplib::StatusCode::OK_200){
                resstr+="Test failed.\nServer returned an error.\n";
                mLog.push(LEVEL_ERROR,"Failed to test from node: %s. HTTP error: %s",
                    configRoot["nodes"][i]["name"].asCString(),
                    httplib::to_string(res.error()).c_str()
                );
            }
            else {
                Json::Reader resReader;
                Json::Value resRoot;
                if(!resReader.parse(res->body,resRoot)){
                    resstr+="Test failed.\nCannot parse result.\n";
                    mLog.push(LEVEL_ERROR,"Failed to parse result from node: %s.",
                        configRoot["nodes"][i]["name"].asCString()
                    );
                }
                else {
                    resstr+="```plain\n"+resRoot["res"].asString()+"```\n\n";
                    mLog.push(LEVEL_VERBOSE,"Ping from %s to %s success.",
                        configRoot["nodes"][i]["name"].asCString(),
                        msgparams[1].c_str()
                    );
                }
            }
        }

        mLog.push(LEVEL_INFO,"Ping finish.");

        bot.getApi().sendMessage(message->chat->id,resstr,nullptr,nullptr,nullptr,"Markdown");
    });
    
    bot.getEvents().onCommand("trace", [&](TgBot::Message::Ptr message){
        std::string resstr = "";
        
        std::istringstream messageParser(message->text);
        std::string msgparams[16];
        short msgparamsCount=0;
        while(messageParser>>msgparams[msgparamsCount])msgparamsCount++;

        std::string params="/trace?target="+msgparams[1];

        mLog.push(LEVEL_INFO,"User %li[%s %s] required: Traceroute target: %s",
            message->from->id,
            message->from->firstName.c_str(),message->from->lastName.c_str(),
            msgparams[1].c_str()
        );

        for(unsigned int i=0;i<configRoot["nodes"].size();i++){
            resstr+="Node: "+configRoot["nodes"][i]["name"].asString()+"\n";
            std::string weburl=configRoot["nodes"][i]["url"].asString();
            httplib::Client cli(weburl.c_str());
            cli.set_connection_timeout(std::chrono::milliseconds(timeoutConn));
            cli.set_read_timeout(std::chrono::milliseconds(timeoutRead));
            cli.set_max_timeout(std::chrono::milliseconds(timeoutAll));

            auto res = cli.Get(((std::string)(params+"&uuid="+configRoot["nodes"][i]["uuid"].asString())).c_str());
            if(!res||res->status!=httplib::StatusCode::OK_200){
                resstr+="Test failed.\nServer returned an error.\n";
                mLog.push(LEVEL_ERROR,"Failed to test from node: %s. HTTP error: %s",
                    configRoot["nodes"][i]["name"].asCString(),
                    httplib::to_string(res.error()).c_str()
                );
            }
            else {
                Json::Reader resReader;
                Json::Value resRoot;
                if(!resReader.parse(res->body,resRoot)){
                    resstr+="Test failed.\nCannot parse result.\n";
                    mLog.push(LEVEL_ERROR,"Failed to parse result from node: %s.",
                        configRoot["nodes"][i]["name"].asCString()
                    );
                }
                else {
                    resstr+="```plain\n"+resRoot["res"].asString()+"```\n\n";
                    mLog.push(LEVEL_VERBOSE,"Trace from %s to %s success.",
                        configRoot["nodes"][i]["name"].asCString(),
                        msgparams[1].c_str()
                    );
                }
            }
        }

        mLog.push(LEVEL_INFO,"Traceroute finish.");
        
        bot.getApi().sendMessage(message->chat->id,resstr,nullptr,nullptr,nullptr,"Markdown");
    });

    
    bot.getEvents().onCommand("tcping", [&](TgBot::Message::Ptr message){
        std::string resstr = "";

        std::istringstream messageParser(message->text);
        std::string msgparams[16];
        short msgparamsCount=0;
        while(messageParser>>msgparams[msgparamsCount])msgparamsCount++;

        std::string params="/tcping?host="+msgparams[1]+"&port="+msgparams[2];

        mLog.push(LEVEL_INFO,"User %li[%s %s] required: TCPing target: %s:%s",
            message->from->id,
            message->from->firstName.c_str(),message->from->lastName.c_str(),
            msgparams[1].c_str(),msgparams[2].c_str()
        );

        for(unsigned int i=0;i<configRoot["nodes"].size();i++){
            resstr+="Node: "+configRoot["nodes"][i]["name"].asString()+"\n";
            std::string weburl=configRoot["nodes"][i]["url"].asString();
            httplib::Client cli(weburl.c_str());
            cli.set_connection_timeout(std::chrono::milliseconds(timeoutConn));
            cli.set_read_timeout(std::chrono::milliseconds(timeoutRead));
            cli.set_max_timeout(std::chrono::milliseconds(timeoutAll));

            auto res = cli.Get(((std::string)(params+"&uuid="+configRoot["nodes"][i]["uuid"].asString())).c_str());
            if(!res||res->status!=httplib::StatusCode::OK_200){
                resstr+="Test failed.\nServer returned an error.\n";
                mLog.push(LEVEL_ERROR,"Failed to test from node: %s. HTTP error: %s",
                    configRoot["nodes"][i]["name"].asCString(),
                    httplib::to_string(res.error()).c_str()
                );
            }
            else {
                Json::Reader resReader;
                Json::Value resRoot;
                if(!resReader.parse(res->body,resRoot)){
                    resstr+="Test failed.\nCannot parse result.\n";
                    mLog.push(LEVEL_ERROR,"Failed to parse result from node: %s.",
                        configRoot["nodes"][i]["name"].asCString()
                    );
                }
                else {
                    resstr+="```plain\n"+resRoot["res"].asString()+"```\n\n";
                    mLog.push(LEVEL_VERBOSE,"TCPing from %s to %s:%s success.",
                        configRoot["nodes"][i]["name"].asCString(),
                        msgparams[1].c_str(),msgparams[2].c_str()
                    );
                }
            }
        }

        mLog.push(LEVEL_INFO,"TCPing finish.");
        
        bot.getApi().sendMessage(message->chat->id,resstr,nullptr,nullptr,nullptr,"Markdown");
    });
    
    bot.getEvents().onCommand("route", [&](TgBot::Message::Ptr message){
        std::string resstr = "";

        std::istringstream messageParser(message->text);
        std::string msgparams[16];
        short msgparamsCount=0;
        while(messageParser>>msgparams[msgparamsCount])msgparamsCount++;

        std::string params="/route?target="+msgparams[1];
        mLog.push(LEVEL_INFO,"User %li[%s %s] required: Query route for target: %s",
            message->from->id,
            message->from->firstName.c_str(),message->from->lastName.c_str(),
            msgparams[1].c_str()
        );

        for(unsigned int i=0;i<configRoot["nodes"].size();i++){
            resstr+="Node: "+configRoot["nodes"][i]["name"].asString()+"\n";
            std::string weburl=configRoot["nodes"][i]["url"].asString();
            httplib::Client cli(weburl.c_str());
            cli.set_connection_timeout(std::chrono::milliseconds(timeoutConn));
            cli.set_read_timeout(std::chrono::milliseconds(timeoutRead));
            cli.set_max_timeout(std::chrono::milliseconds(timeoutAll));
            
            auto res = cli.Get(((std::string)(params+"&uuid="+configRoot["nodes"][i]["uuid"].asString())).c_str());
            if(!res||res->status!=httplib::StatusCode::OK_200){
                resstr+="Test failed.\nServer returned an error.\n";
                mLog.push(LEVEL_ERROR,"Failed to test from node: %s. HTTP error: %s",
                    configRoot["nodes"][i]["name"].asCString(),
                    httplib::to_string(res.error()).c_str()
                );
            }
            else {
                Json::Reader resReader;
                Json::Value resRoot;
                if(!resReader.parse(res->body,resRoot)){
                    resstr+="Test failed.\nCannot parse result.\n";
                    mLog.push(LEVEL_ERROR,"Failed to parse result from node: %s.",
                        configRoot["nodes"][i]["name"].asCString()
                    );
                }
                else {
                    resstr+="```plain\n"+resRoot["res"].asString()+"```\n\n";
                    mLog.push(LEVEL_VERBOSE,"Query route from %s to %s success.",
                        configRoot["nodes"][i]["name"].asCString(),
                        msgparams[1].c_str()
                    );
                }
            }
        }
            
        mLog.push(LEVEL_INFO,"Query route finish.");
        
        bot.getApi().sendMessage(message->chat->id,resstr,nullptr,nullptr,nullptr,"Markdown");
    });

    // bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
    //     printf("User wrote %s\n", message->text.c_str());
    //     if (StringTools::startsWith(message->text, "/start")) {
    //         return;
    //     }
    //     bot.getApi().sendMessage(message->chat->id, "Your message is: " + message->text);
    // });

    signal(SIGINT, [](int s) {
        mLog.push(LEVEL_INFO,"Got SIGINT.");
        exit(0);
    });

    signal(SIGTERM, [](int s) {
        mLog.push(LEVEL_INFO,"Got SIGTERM.");
        exit(0);
    });

    try {
        mLog.push(LEVEL_INFO,"Bot username: %s",bot.getApi().getMe()->username.c_str());
        bot.getApi().deleteWebhook();

        TgBot::TgLongPoll longPoll(bot);
        mLog.push(LEVEL_INFO,"Long poll started");
        while (true) {
            longPoll.start();
        }
    } catch (std::exception& e) {
        mLog.push(LEVEL_ERROR,"Catch exception: %s",e.what());
    }

    return 0;
}