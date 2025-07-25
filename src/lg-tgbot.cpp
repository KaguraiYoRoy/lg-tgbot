#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <tgbot/tgbot.h>
#include <json/json.h>
#include <httplib.h>
#include <Log.h>

#define DEFAULT_LOGLEVEL_P LEVEL_WARN
#define DEFAULT_LOGLEVEL_W LEVEL_ERROR
#define DEFAULT_TIMEOUT_CONN 300
#define DEFAULT_TIMEOUT_READ 3000
#define DEFAULT_TIMEOUT_ALL 5000
#define DEFAULT_SERVER_ID 0
#define DEFAULT_DELETE_TIMER 30

struct MsgTimerData {
    int64_t chatId;
    int32_t messageId;
    unsigned int countdown;
};

Log mLog;
Json::Value configRoot;
std::map<std::pair<int64_t, int32_t>, MsgTimerData> messageTimers;
std::mutex timersMutex;
std::atomic<bool> exit_requested(false);
std::thread threadAutoDelete;
unsigned int timeoutConn,timeoutRead,timeoutAll;
unsigned int autoDeleteTime;

std::string ping(int serverid,std::string target){
    std::string weburl=configRoot["nodes"][serverid]["url"].asString();
    httplib::Client cli(weburl.c_str());
    cli.set_connection_timeout(std::chrono::milliseconds(timeoutConn));
    cli.set_read_timeout(std::chrono::milliseconds(timeoutRead));
    cli.set_max_timeout(std::chrono::milliseconds(timeoutAll)); 

    auto res = cli.Get(((std::string)("/ping?target="+target+"&uuid="+configRoot["nodes"][serverid]["uuid"].asString())).c_str());
    if(!res){
        mLog.push(LEVEL_ERROR,"Failed to test from node: %s. HTTP error: %s",
            configRoot["nodes"][serverid]["name"].asCString(),
            httplib::to_string(res.error()).c_str()
        );
        return "HTTP Error";
    }

    if(res->status==httplib::StatusCode::Unauthorized_401){
        mLog.push(LEVEL_ERROR,"Failed to test from node: %s. HTTP 401: Unauthorized. Please check UUID",
            configRoot["nodes"][serverid]["name"].asCString()
        );
        return "Node Unauthorized";
    }

    Json::Reader resReader;
    Json::Value resRoot;
    if(!resReader.parse(res->body,resRoot)){
        mLog.push(LEVEL_ERROR,"Failed to parse result from node: %s.",
            configRoot["nodes"][serverid]["name"].asCString()
        );
        return "Return value parse error";
    }
    mLog.push(LEVEL_VERBOSE,"Ping from %s to %s success.",
        configRoot["nodes"][serverid]["name"].asCString(),
        target.c_str()
    );
    return "```shell\n"+resRoot["res"].asString()+"```";
}

std::string trace(int serverid,std::string target){
    std::string weburl=configRoot["nodes"][serverid]["url"].asString();
    httplib::Client cli(weburl.c_str());
    cli.set_connection_timeout(std::chrono::milliseconds(timeoutConn));
    cli.set_read_timeout(std::chrono::milliseconds(timeoutRead));
    cli.set_max_timeout(std::chrono::milliseconds(timeoutAll)); 

    auto res = cli.Get(((std::string)("/trace?target="+target+"&uuid="+configRoot["nodes"][serverid]["uuid"].asString())).c_str());
    if(!res){
        mLog.push(LEVEL_ERROR,"Failed to test from node: %s. HTTP error: %s",
            configRoot["nodes"][serverid]["name"].asCString(),
            httplib::to_string(res.error()).c_str()
        );
        return "HTTP Error";
    }
    
    if(res->status==httplib::StatusCode::Unauthorized_401){
        mLog.push(LEVEL_ERROR,"Failed to test from node: %s. HTTP 401: Unauthorized. Please check UUID",
            configRoot["nodes"][serverid]["name"].asCString()
        );
        return "Node Unauthorized";
    }

    Json::Reader resReader;
    Json::Value resRoot;
    if(!resReader.parse(res->body,resRoot)){
        mLog.push(LEVEL_ERROR,"Failed to parse result from node: %s.",
            configRoot["nodes"][serverid]["name"].asCString()
        );
        return "Return value parse error";
    }
    mLog.push(LEVEL_VERBOSE,"Trace from %s to %s success.",
        configRoot["nodes"][serverid]["name"].asCString(),
        target.c_str()
    );
    return "```shell\n"+resRoot["res"].asString()+"```";
}

std::string tcping(int serverid,std::string host,std::string port){
    std::string weburl=configRoot["nodes"][serverid]["url"].asString();
    httplib::Client cli(weburl.c_str());
    cli.set_connection_timeout(std::chrono::milliseconds(timeoutConn));
    cli.set_read_timeout(std::chrono::milliseconds(timeoutRead));
    cli.set_max_timeout(std::chrono::milliseconds(timeoutAll)); 

    auto res = cli.Get(((std::string)("/tcping?host="+host+"&port="+port+"&uuid="+configRoot["nodes"][serverid]["uuid"].asString())).c_str());
    if(!res){
        mLog.push(LEVEL_ERROR,"Failed to test from node: %s. HTTP error: %s",
            configRoot["nodes"][serverid]["name"].asCString(),
            httplib::to_string(res.error()).c_str()
        );
        return "HTTP Error";
    }
    
    if(res->status==httplib::StatusCode::Unauthorized_401){
        mLog.push(LEVEL_ERROR,"Failed to test from node: %s. HTTP 401: Unauthorized. Please check UUID",
            configRoot["nodes"][serverid]["name"].asCString()
        );
        return "Node Unauthorized";
    }

    Json::Reader resReader;
    Json::Value resRoot;
    if(!resReader.parse(res->body,resRoot)){
        mLog.push(LEVEL_ERROR,"Failed to parse result from node: %s.",
            configRoot["nodes"][serverid]["name"].asCString()
        );
        return "Return value parse error";
    }
    mLog.push(LEVEL_VERBOSE,"TCPing from %s to %s:%s success.",
        configRoot["nodes"][serverid]["name"].asCString(),
        host.c_str(),port.c_str()
    );
    return "```shell\n"+resRoot["res"].asString()+"```";
}

std::string route(int serverid, std::string target){
    std::string weburl=configRoot["nodes"][serverid]["url"].asString();
    httplib::Client cli(weburl.c_str());
    cli.set_connection_timeout(std::chrono::milliseconds(timeoutConn));
    cli.set_read_timeout(std::chrono::milliseconds(timeoutRead));
    cli.set_max_timeout(std::chrono::milliseconds(timeoutAll)); 

    auto res = cli.Get(((std::string)("/route?target="+target+"&uuid="+configRoot["nodes"][serverid]["uuid"].asString())).c_str());
    if(!res){
        mLog.push(LEVEL_ERROR,"Failed to test from node: %s. HTTP error: %s",
            configRoot["nodes"][serverid]["name"].asCString(),
            httplib::to_string(res.error()).c_str()
        );
        return "HTTP Error";
    }
    
    if(res->status==httplib::StatusCode::Unauthorized_401){
        mLog.push(LEVEL_ERROR,"Failed to test from node: %s. HTTP 401: Unauthorized. Please check UUID",
            configRoot["nodes"][serverid]["name"].asCString()
        );
        return "Node Unauthorized";
    }

    Json::Reader resReader;
    Json::Value resRoot;
    if(!resReader.parse(res->body,resRoot)){
        mLog.push(LEVEL_ERROR,"Failed to parse result from node: %s.",
            configRoot["nodes"][serverid]["name"].asCString()
        );
        return "Return value parse error";
    }
    mLog.push(LEVEL_VERBOSE,"Query route from %s to %s success.",
        configRoot["nodes"][serverid]["name"].asCString(),
        target.c_str()
    );
    return "```shell\n"+resRoot["res"].asString()+"```";
}

TgBot::InlineKeyboardMarkup::Ptr buildInlineKeyboard(unsigned int current_serverid,std::string cmd,std::string* params){
    TgBot::InlineKeyboardMarkup::Ptr keyboard(new TgBot::InlineKeyboardMarkup);
    std::vector<TgBot::InlineKeyboardButton::Ptr> row;
    for(unsigned int i=0;i<configRoot["nodes"].size();i++){
        TgBot::InlineKeyboardButton::Ptr button(new TgBot::InlineKeyboardButton);
        if(current_serverid==i)
            button->text = "✅ "+configRoot["nodes"][i]["name"].asString();
        else 
            button->text = configRoot["nodes"][i]["name"].asString();
        if(cmd=="tcping"){
            char buffer[256];
            sprintf(buffer,"cmd=tcping;targetserver=%d;host=%s;port=%s",i,params[1].c_str(),params[2].c_str());
            button->callbackData=buffer;
        }else{
            char buffer[256];
            sprintf(buffer,"cmd=%s;targetserver=%d;target=%s",cmd.c_str(),i,params[1].c_str());
            button->callbackData=buffer;
        }

        row.push_back(button);
    }
    keyboard->inlineKeyboard.push_back(row);
    return keyboard;
}

void autoDeleteThread(TgBot::Bot* bot){
    while(!exit_requested.load(std::memory_order_relaxed)){
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::lock_guard<std::mutex> lock(timersMutex);

        for (auto it = messageTimers.begin(); it != messageTimers.end(); ) {
            MsgTimerData& data = it->second;

            if (--data.countdown <= 0) {
                mLog.push(LEVEL_VERBOSE,"Deleting message %ld in chat %ld",data.messageId,data.chatId);
                bot->getApi().deleteMessage(data.chatId, data.messageId);
                it = messageTimers.erase(it);
                mLog.push(LEVEL_INFO,"Message %ld in chat %ld deleted",data.messageId,data.chatId);
            } else {
                ++it;
            }
        }
    }
}

std::map<std::string, std::string> parseCallbackData(const std::string& data) {
    std::map<std::string, std::string> result;
    std::istringstream ss(data);
    std::string item;

    while (getline(ss, item, ';')) {
        auto eq = item.find('=');
        if (eq != std::string::npos) {
            result[item.substr(0, eq)] = item.substr(eq + 1);
        }
    }
    return result;
}

void setMsgTimer(int64_t chatID,int32_t msgID,unsigned int timer){
    if(!autoDeleteTime)return;
    std::lock_guard<std::mutex> lock(timersMutex);
    messageTimers[{chatID, msgID}] = {chatID, msgID, timer};
    mLog.push(LEVEL_VERBOSE,"Message %d in chat %ld will be deleted in %d second(s)",msgID,chatID,timer);
}

void sigHandler(int s){
    switch(s){
        case SIGINT:
            mLog.push(LEVEL_INFO,"Got SIGINT");
            exit_requested.store(true,std::memory_order_relaxed);
            if(autoDeleteTime)threadAutoDelete.join();
            exit(0);
            break;
        case SIGTERM:
            mLog.push(LEVEL_INFO,"Got SIGTERM");
            exit_requested.store(true,std::memory_order_relaxed);
            if(autoDeleteTime)threadAutoDelete.join();
            exit(0);
            break;
    }
}

int main(){
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

    unsigned int defaultSrvID=configRoot.isMember("default-server")&&configRoot["default-server"].isInt()?configRoot["default-server"].asUInt():DEFAULT_SERVER_ID;
    autoDeleteTime=configRoot.isMember("autodelete")&&configRoot["autodelete"].isInt()?configRoot["autodelete"].asUInt():DEFAULT_DELETE_TIMER;
    timeoutConn=configRoot.isMember("timeout-connection")&&configRoot["timeout-connection"].isInt()?configRoot["timeout-connection"].asUInt():DEFAULT_TIMEOUT_CONN;
    timeoutRead=configRoot.isMember("timeout-read")&&configRoot["timeout-read"].isInt()?configRoot["timeout-read"].asUInt():DEFAULT_TIMEOUT_READ;
    timeoutAll=configRoot.isMember("timeout-all")&&configRoot["timeout-all"].isInt()?configRoot["timeout-all"].asUInt():DEFAULT_TIMEOUT_ALL;

    TgBot::Bot bot(token);
    
    if(autoDeleteTime){
        threadAutoDelete=std::thread(autoDeleteThread,&bot);
        mLog.push(LEVEL_INFO,"Auto delete enabled. Timeout: %d",autoDeleteTime);
    }
    else
        mLog.push(LEVEL_INFO,"Auto delete disabled");

    std::vector<TgBot::BotCommand::Ptr> commands;
    TgBot::BotCommand::Ptr cmdArray(new TgBot::BotCommand);
    cmdArray->command = "start";
    cmdArray->description = "Show manual";

    commands.push_back(cmdArray);

    cmdArray = TgBot::BotCommand::Ptr(new TgBot::BotCommand);
    cmdArray->command = "ping";
    cmdArray->description = "Ping target";
    commands.push_back(cmdArray);

    cmdArray = TgBot::BotCommand::Ptr(new TgBot::BotCommand);
    cmdArray->command = "trace";
    cmdArray->description = "Traceroute target";
    commands.push_back(cmdArray);

    cmdArray = TgBot::BotCommand::Ptr(new TgBot::BotCommand);
    cmdArray->command = "tcping";
    cmdArray->description = "TCPing target host:ip";
    commands.push_back(cmdArray);
    
    cmdArray = TgBot::BotCommand::Ptr(new TgBot::BotCommand);
    cmdArray->command = "route";
    cmdArray->description = "Query route for target (via BIRD2)";
    commands.push_back(cmdArray);
    
    cmdArray = TgBot::BotCommand::Ptr(new TgBot::BotCommand);
    cmdArray->command = "whois";
    cmdArray->description = "Whois lookup";
    commands.push_back(cmdArray);

    bot.getApi().setMyCommands(commands);

    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Manual:\n/ping <host>: Ping target host\n/tcping <host> <port>: TCPing host:port\n/trace <host>: Traceroute target host\n/route <host>: Get route for target host\n/whois <target>: Whois lookup");
    });

    bot.getEvents().onCommand("ping", [&](TgBot::Message::Ptr message){
        std::istringstream messageParser(message->text);
        std::string msgparams[16];
        short msgparamsCount=0;
        while(messageParser>>msgparams[msgparamsCount])msgparamsCount++;

        mLog.push(LEVEL_INFO,"User %li[%s %s] required: Ping target: %s",
            message->from->id,
            message->from->firstName.c_str(),message->from->lastName.c_str(),
            msgparams[1].c_str()
        );

        auto message_sent=bot.getApi().sendMessage(
            message->chat->id,
            ping(defaultSrvID,msgparams[1]),
            nullptr,
            nullptr,
            buildInlineKeyboard(defaultSrvID,"ping",msgparams),
            "Markdown"
        );

        setMsgTimer(message_sent->chat->id,message_sent->messageId,autoDeleteTime);

        mLog.push(LEVEL_INFO,"Ping finish.");
    });
    
    bot.getEvents().onCommand("trace", [&](TgBot::Message::Ptr message){
        std::istringstream messageParser(message->text);
        std::string msgparams[16];
        short msgparamsCount=0;
        while(messageParser>>msgparams[msgparamsCount])msgparamsCount++;

        mLog.push(LEVEL_INFO,"User %li[%s %s] required: Traceroute target: %s",
            message->from->id,
            message->from->firstName.c_str(),message->from->lastName.c_str(),
            msgparams[1].c_str()
        );
        
        auto message_sent=bot.getApi().sendMessage(
            message->chat->id,
            trace(defaultSrvID,msgparams[1]),
            nullptr,
            nullptr,
            buildInlineKeyboard(defaultSrvID,"trace",msgparams),
            "Markdown"
        );
        
        setMsgTimer(message_sent->chat->id,message_sent->messageId,autoDeleteTime);

        mLog.push(LEVEL_INFO,"Traceroute finish.");
    });

    
    bot.getEvents().onCommand("tcping", [&](TgBot::Message::Ptr message){
        std::istringstream messageParser(message->text);
        std::string msgparams[16];
        short msgparamsCount=0;
        while(messageParser>>msgparams[msgparamsCount])msgparamsCount++;

        mLog.push(LEVEL_INFO,"User %li[%s %s] required: TCPing target: %s:%s",
            message->from->id,
            message->from->firstName.c_str(),message->from->lastName.c_str(),
            msgparams[1].c_str(),msgparams[2].c_str()
        );
        
        auto message_sent=bot.getApi().sendMessage(
            message->chat->id,
            tcping(defaultSrvID,msgparams[1],msgparams[2]),
            nullptr,
            nullptr,
            buildInlineKeyboard(defaultSrvID,"tcping",msgparams),
            "Markdown"
        );
        
        setMsgTimer(message_sent->chat->id,message_sent->messageId,autoDeleteTime);
        
        mLog.push(LEVEL_INFO,"TCPing finish.");
    });
    
    bot.getEvents().onCommand("route", [&](TgBot::Message::Ptr message){
        std::istringstream messageParser(message->text);
        std::string msgparams[16];
        short msgparamsCount=0;
        while(messageParser>>msgparams[msgparamsCount])msgparamsCount++;

        mLog.push(LEVEL_INFO,"User %li[%s %s] required: Query route for target: %s",
            message->from->id,
            message->from->firstName.c_str(),message->from->lastName.c_str(),
            msgparams[1].c_str()
        );
        
        auto message_sent=bot.getApi().sendMessage(
            message->chat->id,
            route(defaultSrvID,msgparams[1]),
            nullptr,
            nullptr,
            buildInlineKeyboard(defaultSrvID,"route",msgparams),
            "Markdown"
        );
        
        setMsgTimer(message_sent->chat->id,message_sent->messageId,autoDeleteTime);
        
        mLog.push(LEVEL_INFO,"Query route finish.");
    });

    bot.getEvents().onCommand("whois", [&](TgBot::Message::Ptr message){
        std::string resstr = "";

        std::istringstream messageParser(message->text);
        std::string msgparams[16];
        short msgparamsCount=0;
        while(messageParser>>msgparams[msgparamsCount])msgparamsCount++;

        std::string params="/whois?target="+msgparams[1];
        mLog.push(LEVEL_INFO,"User %li[%s %s] required: Query route for target: %s",
            message->from->id,
            message->from->firstName.c_str(),message->from->lastName.c_str(),
            msgparams[1].c_str()
        );

        std::string weburl=configRoot["nodes"][0]["url"].asString();
        httplib::Client cli(weburl.c_str());
        cli.set_connection_timeout(std::chrono::milliseconds(timeoutConn));
        cli.set_read_timeout(std::chrono::milliseconds(timeoutRead));
        cli.set_max_timeout(std::chrono::milliseconds(timeoutAll));
        
        auto res = cli.Get(((std::string)(params+"&uuid="+configRoot["nodes"][0]["uuid"].asString())).c_str());
        if(!res||res->status!=httplib::StatusCode::OK_200){
            resstr+="Test failed.\nServer returned an error.\n";
            mLog.push(LEVEL_ERROR,"Failed to test from node: %s. HTTP error: %s",
                configRoot["nodes"][0]["name"].asCString(),
                httplib::to_string(res.error()).c_str()
            );
        }
        else {
            Json::Reader resReader;
            Json::Value resRoot;
            if(!resReader.parse(res->body,resRoot)){
                resstr+="Test failed.\nCannot parse result.\n";
                mLog.push(LEVEL_ERROR,"Failed to parse result from node: %s.",
                    configRoot["nodes"][0]["name"].asCString()
                );
            }
            else {
                resstr+="```plain\n"+resRoot["res"].asString()+"```\n\n";
                mLog.push(LEVEL_VERBOSE,"Whois lookup from %s to %s success.",
                    configRoot["nodes"][0]["name"].asCString(),
                    msgparams[1].c_str()
                );
            }
        }
            
        mLog.push(LEVEL_INFO,"Whois lookup finish.");
        
        TgBot::Message::Ptr message_sent;

        if(resstr.size()>4096)
            message_sent=bot.getApi().sendMessage(message->chat->id,"Error: Message too long.",nullptr,nullptr,nullptr,"Markdown");
        else 
            message_sent=bot.getApi().sendMessage(message->chat->id,resstr,nullptr,nullptr,nullptr,"Markdown");
        
        setMsgTimer(message_sent->chat->id,message_sent->messageId,autoDeleteTime);
    });

    bot.getEvents().onCallbackQuery([&bot](const TgBot::CallbackQuery::Ptr& query) {
        try {
            bot.getApi().answerCallbackQuery(query->id);
            mLog.push(LEVEL_VERBOSE,"Callback answered.");
        } catch (const std::exception& e) {
            mLog.push(LEVEL_ERROR,"Callback error");
        }

        auto dataMap = parseCallbackData(query->data);
        std::string cmd = dataMap["cmd"];
        
        TgBot::LinkPreviewOptions::Ptr previewOptions(new TgBot::LinkPreviewOptions);
        previewOptions->isDisabled = true;

        if (cmd=="ping") {
            std::string params[2];
            params[1]=dataMap["target"];
            unsigned int serverid = atoi(dataMap["targetserver"].c_str());
            
            bot.getApi().editMessageText(
                "Timestamp: "+std::to_string(time(NULL))+"\n"+ping(serverid,params[1]),
                query->message->chat->id,
                query->message->messageId,
                "",
                "Markdown",
                previewOptions,
                buildInlineKeyboard(serverid,"ping",params)
            );
        }else if(cmd=="trace") {
            std::string params[2];
            params[1]=dataMap["target"];
            unsigned int serverid = atoi(dataMap["targetserver"].c_str());
            
            bot.getApi().editMessageText(
                "Timestamp: "+std::to_string(time(NULL))+"\n"+trace(serverid,params[1]),
                query->message->chat->id,
                query->message->messageId,
                "",
                "Markdown",
                previewOptions,
                buildInlineKeyboard(serverid,"trace",params)
            );
        }else if(cmd=="tcping") {
            std::string params[3];
            params[1]=dataMap["host"];
            params[2]=dataMap["port"];
            unsigned int serverid = atoi(dataMap["targetserver"].c_str());
            
            bot.getApi().editMessageText(
                "Timestamp: "+std::to_string(time(NULL))+"\n"+tcping(serverid,params[1],params[2]),
                query->message->chat->id,
                query->message->messageId,
                "",
                "Markdown",
                previewOptions,
                buildInlineKeyboard(serverid,"tcping",params)
            );
        }else if(cmd=="route") {
            std::string params[2];
            params[1]=dataMap["target"];
            unsigned int serverid = atoi(dataMap["targetserver"].c_str());
            
            bot.getApi().editMessageText(
                "Timestamp: "+std::to_string(time(NULL))+"\n"+route(serverid,params[1]),
                query->message->chat->id,
                query->message->messageId,
                "",
                "Markdown",
                previewOptions,
                buildInlineKeyboard(serverid,"route",params)
            );
        }

        setMsgTimer(query->message->chat->id,query->message->messageId,autoDeleteTime);
    });

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

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