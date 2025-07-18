#pragma once
#include <iostream>
#include <fstream>
#include <cstring>
#include <thread>
#include <chrono>
#include <mutex>
#include <cstdio>
#include <cstdarg>
#include <atomic>

#include "BlockingQueue.h"

enum LogLevel {
	LEVEL_VERBOSE,LEVEL_INFO,LEVEL_DEBUG,LEVEL_WARN,LEVEL_ERROR,LEVEL_FATAL,LEVEL_OFF
};

struct LogMsg {
	short m_LogLevel;
	std::string m_strTimestamp;
	std::string m_strLogMsg;
};

class Log {
private:
	std::ofstream m_ofLogFile;
	std::mutex m_lockFile;
	std::thread m_threadMain;
	BlockingQueue<LogMsg> m_msgQueue;
	short m_levelLog, m_levelPrint;
	std::atomic<bool> m_exit_requested{ false };

	std::string getTime();
	std::string level2str(short level, bool character_only);
	void logThread();

public:
	Log(short default_loglevel = LEVEL_WARN, short default_printlevel = LEVEL_INFO);
	~Log();
	void push(short level, const char* msg, ...);
	void set_level(short loglevel, short printlevel);
	bool open(std::string filename);
	bool close();
};

