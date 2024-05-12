#pragma once

typedef enum
{
	NLOG_INFO,
	NLOG_DEBUG,
	NLOG_WARN,
	NLOG_ERROR,
	NLOG_FATAL,
	MAX_LOG_LEVEL,
} LOG_LEVEL_ENUM;

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_ERR
#endif

void _log(LOG_LEVEL_ENUM level, const char* msg, ...);

#ifndef NLOG_INFO
#define NLOG_INFO(msg, ...) _log(NLOG_INFO, msg, ##__VA_ARGS__)
#endif

#ifndef NLOG_DEBUG
#define NLOG_DEBUG(msg, ...) _log(NLOG_DEBUG, msg, ##__VA_ARGS__)
#endif

#ifndef NLOG_WARN
#define NLOG_WARN(msg, ...) _log(NLOG_WARN, msg, ##__VA_ARGS__)
#endif

#ifndef NLOG_ERR
#define NLOG_ERR(msg, ...) _log(NLOG_ERROR, msg, ##__VA_ARGS__)
#endif

#ifndef NLOG_FATAL
#define NLOG_FATAL(msg, ...) _log(NLOG_FATAL, msg, ##__VA_ARGS__)
#endif
