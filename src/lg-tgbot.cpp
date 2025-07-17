#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>
#include <tgbot/tgbot.h>
#include <json/json.h>
#include <httplib.h>

int main(){
    Json::Value configRoot;
    Json::Reader configReader;
    std::ifstream fileReader("config.json");

    if(!fileReader){
        std::cout<<"Error: No config file found"<<std::endl;
        return 1;
    }

    if (!configReader.parse(fileReader, configRoot)) {
        std::cout<<"Error: Config invalid"<<std::endl;
        return 1;
    }

    std::string token(configRoot["token"].asCString());
    printf("Token: %s\n", token.c_str());
    const bool debugmode=(configRoot.isMember("debug")&&configRoot["debug"].isBool())?configRoot["debug"].asBool():false;

    TgBot::Bot bot(token);
    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Manual:\n/ping <host>: Ping target host\n/tcping <host> <port>: TCPing host:port\n/trace <host>: Traceroute target host\n/route <host>: Get route for target host\n");
    });

    bot.getEvents().onCommand("ping", [&](TgBot::Message::Ptr message){
        std::string resstr = "";
        for(int i=0;i<configRoot["nodes"].size();i++){
            resstr+="Node: "+configRoot["nodes"][i]["name"].asString()+"\n";
            std::string weburl=configRoot["nodes"][i]["url"].asString();
            httplib::Client cli(weburl.c_str());

            std::istringstream messageParser(message->text);
            std::string msgparams[16];
            short msgparamsCount=0;
            while(messageParser>>msgparams[msgparamsCount])msgparamsCount++;

            std::string params="/ping?target="+msgparams[1];

            if(debugmode)resstr+="Debug info:\n"+weburl+params+"\n";

            auto res = cli.Get(params.c_str());
            if(res->status!=httplib::StatusCode::OK_200){
                resstr+="Test failed.\nServer returned an unknown status.\n";
            }
            else {
                Json::Reader resReader;
                Json::Value resRoot;
                if(!resReader.parse(res->body,resRoot))
                    resstr+="Test failed.\nCannot parse result.\n";
                else resstr+="```plain\n"+resRoot["res"].asString()+"```\n\n";
            }

            std::cout<<"Ping target: "+msgparams[1]<<std::endl;
        }
        bot.getApi().sendMessage(message->chat->id,resstr,nullptr,nullptr,nullptr,"Markdown");
    });
    
    bot.getEvents().onCommand("trace", [&](TgBot::Message::Ptr message){
        std::string resstr = "";
        for(int i=0;i<configRoot["nodes"].size();i++){
            resstr+="Node: "+configRoot["nodes"][i]["name"].asString()+"\n";
            std::string weburl=configRoot["nodes"][i]["url"].asString();
            httplib::Client cli(weburl.c_str());

            std::istringstream messageParser(message->text);
            std::string msgparams[16];
            short msgparamsCount=0;
            while(messageParser>>msgparams[msgparamsCount])msgparamsCount++;

            std::string params="/trace?target="+msgparams[1];

            if(debugmode)resstr+="Debug info:\n"+weburl+params+"\n";

            auto res = cli.Get(params.c_str());
            if(res->status!=httplib::StatusCode::OK_200){
                resstr+="Test failed.\nServer returned an unknown status.\n";
            }
            else {
                Json::Reader resReader;
                Json::Value resRoot;
                if(!resReader.parse(res->body,resRoot))
                    resstr+="Test failed.\nCannot parse result.\n";
                else resstr+="```plain\n"+resRoot["res"].asString()+"```\n\n";
            }
            
            std::cout<<"Trace target: "+msgparams[1]<<std::endl;
        }
        bot.getApi().sendMessage(message->chat->id,resstr,nullptr,nullptr,nullptr,"Markdown");
    });

    
    bot.getEvents().onCommand("tcping", [&](TgBot::Message::Ptr message){
        std::string resstr = "";
        for(int i=0;i<configRoot["nodes"].size();i++){
            resstr+="Node: "+configRoot["nodes"][i]["name"].asString()+"\n";
            std::string weburl=configRoot["nodes"][i]["url"].asString();
            httplib::Client cli(weburl.c_str());

            std::istringstream messageParser(message->text);
            std::string msgparams[16];
            short msgparamsCount=0;
            while(messageParser>>msgparams[msgparamsCount])msgparamsCount++;

            std::string params="/tcping?host="+msgparams[1]+"&port="+msgparams[2];

            if(debugmode)resstr+="Debug info:\n"+weburl+params+"\n";

            auto res = cli.Get(params.c_str());
            if(res->status!=httplib::StatusCode::OK_200){
                resstr+="Test failed.\nServer returned an unknown status.\n";
            }
            else {
                Json::Reader resReader;
                Json::Value resRoot;
                if(!resReader.parse(res->body,resRoot))
                    resstr+="Test failed.\nCannot parse result.\n";
                else resstr+="```plain\n"+resRoot["res"].asString()+"```\n\n";
            }
            
            std::cout<<"Tcping target: "+msgparams[1]+":"+msgparams[2]<<std::endl;
        }
        bot.getApi().sendMessage(message->chat->id,resstr,nullptr,nullptr,nullptr,"Markdown");
    });
    
    bot.getEvents().onCommand("route", [&](TgBot::Message::Ptr message){
        std::string resstr = "";
        for(int i=0;i<configRoot["nodes"].size();i++){
            resstr+="Node: "+configRoot["nodes"][i]["name"].asString()+"\n";
            std::string weburl=configRoot["nodes"][i]["url"].asString();
            httplib::Client cli(weburl.c_str());

            std::istringstream messageParser(message->text);
            std::string msgparams[16];
            short msgparamsCount=0;
            while(messageParser>>msgparams[msgparamsCount])msgparamsCount++;

            std::string params="/route?target="+msgparams[1];

            if(debugmode)resstr+="Debug info:\n"+weburl+params+"\n";

            auto res = cli.Get(params.c_str());
            if(res->status!=httplib::StatusCode::OK_200){
                resstr+="Test failed.\nServer returned an unknown status.\n";
            }
            else {
                Json::Reader resReader;
                Json::Value resRoot;
                if(!resReader.parse(res->body,resRoot))
                    resstr+="Test failed.\nCannot parse result.\n";
                else resstr+="```plain\n"+resRoot["res"].asString()+"```\n\n";
            }
            
            std::cout<<"Get route for target: "+msgparams[1]<<std::endl;
        }
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
        printf("SIGINT got\n");
        exit(0);
    });

    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        bot.getApi().deleteWebhook();

        TgBot::TgLongPoll longPoll(bot);
        printf("Long poll started\n");
        while (true) {
            longPoll.start();
        }
    } catch (std::exception& e) {
        printf("error: %s\n", e.what());
    }

    return 0;
}