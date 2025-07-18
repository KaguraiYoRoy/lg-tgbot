#include "Log.h"

std::string Log::getTime() {
	std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	char buf[20];
	strftime(buf, 20, "%Y.%m.%d-%H:%M:%S", localtime(&t));
	return buf;
}

std::string Log::level2str(short level, bool character_only)
{
	switch (level) {
	case LEVEL_VERBOSE: 
		return character_only ? "V" : "Verbose";
	case LEVEL_DEBUG:
		return character_only ? "D" : "Debug";
	case LEVEL_WARN:
		return character_only ? "W" : "Warning";
	case LEVEL_ERROR:
		return character_only ? "E" : "Error";
	case LEVEL_FATAL:
		return character_only ? "F" : "Fatal";
	}
	return character_only ? "I" : "Info";
}

void Log::logThread() {
	while (true) {
		LogMsg front = m_msgQueue.take(); 
		
		if (front.m_LogLevel >= m_levelLog) {
			std::lock_guard<std::mutex> lock(m_lockFile);
			if (m_ofLogFile) {
				m_ofLogFile << front.m_strTimestamp << ' '
					<< level2str(front.m_LogLevel, true) << ": "
					<< front.m_strLogMsg << std::endl;
			}
		}
		
		if (front.m_LogLevel >= m_levelPrint) {
			printf("%s %s: %s\n", front.m_strTimestamp.c_str(),
				level2str(front.m_LogLevel, true).c_str(),
				front.m_strLogMsg.c_str());
		}
		
		if (m_exit_requested.load() && m_msgQueue.empty()) break;
	}
	return;
}

Log::Log(short default_loglevel, short default_printlevel) {
	set_level(default_loglevel, default_printlevel);
	m_threadMain = std::thread(&Log::logThread, this);
}

Log::~Log() {
	m_exit_requested.store(true);
	m_msgQueue.push({ LEVEL_INFO, getTime(), "Exit." });
	if (m_threadMain.joinable()) m_threadMain.join();
	close();
}

void Log::push(short level, const char* msg, ...) {
	va_list args;
	va_start(args, msg);
	const int len = vsnprintf(nullptr, 0, msg, args);
	va_end(args);

	if (len < 0) return;

	std::vector<char> buf(len + 1);

	va_start(args, msg);
	vsnprintf(buf.data(), buf.size(), msg, args);
	va_end(args);

	m_msgQueue.push({level,getTime(),buf.data()});
}

void Log::set_level(short loglevel, short printlevel)
{
	m_levelLog = loglevel;
	m_levelPrint = printlevel;
}

bool Log::open(std::string filename) {
	m_lockFile.lock();
	m_ofLogFile.open(filename.c_str(), std::ios::out);
	m_lockFile.unlock();
	return (bool)m_ofLogFile;
}

bool Log::close() {
	m_lockFile.lock();
	m_ofLogFile.close();
	m_lockFile.unlock();
	return false;
}
