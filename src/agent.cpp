#include <httplib.h>
#include <json/json.h>
#include <cstdio>
#include <string>

void WriteLog(std::string log){
    using sc = std::chrono::system_clock;
    std::time_t t = sc::to_time_t(sc::now());
    char timebuf[20];
    strftime(timebuf, 20, "%Y.%m.%d-%H:%M:%S", localtime(&t));
    std::cout<<timebuf<<": "<<log<<'\n';
    return;
}

int main(){
    
    return 0;
}