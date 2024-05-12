#include "logger.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

void _log(LOG_LEVEL_ENUM level, const char* msg, ...) {
	const int LOG_CHAR_CAP = 32000;
	const char *level_map[MAX_LOG_LEVEL] = {
		"[INFO]",
		"[DEBUG]",
		"[WARNING]",
		"[ERROR]",
		"[FATAL]"
	};

	char message_buffer[LOG_CHAR_CAP];
	memset(message_buffer, 0, sizeof(message_buffer));


	// parse formating arguments
	__builtin_va_list arg_ptr;
	va_start(arg_ptr, msg);
	vsnprintf(message_buffer, LOG_CHAR_CAP, msg, arg_ptr);
	va_end(arg_ptr);

	char out_message[LOG_CHAR_CAP];
	// construct final message
	// [level]: <formated message>\n
	sprintf(out_message, "%s: %s\n", level_map[level], message_buffer);

	// print to stdout
	// could add colors to this maybe in the future
	printf("%s", out_message);
};
