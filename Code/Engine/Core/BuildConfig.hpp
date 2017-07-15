#pragma once

/* 
* 0 - Simple Tracking
* 1 - Verbose
* 2 - Insanely Verbose (Every new, every free)
* Not defined = no memory tracking
*/
//#define TRACK_MEMORY 0

//Enable Profiling
//#define PROFILING_ENABLED

//Enable checking for OpenGL Errors
#define CHECK_GL_ERRORS

/*
* 0 - All messages are logged
* 1 - Default messages and above
* 2 - Recoverable messages and above
* 3 - Severe messages only
* 4 - No messages are logged
*/
#define LOG_LEVEL_THRESHOLD 4

//Maximum number of log files to keep. Adding another log over this amount will delete the oldest log.
#define MAX_LOG_HISTORY 5

//Forwards log messages to the output window after logging them.
#define FORWARD_LOG_TO_OUTPUT_WINDOW

//Forwards log messages to the console after logging them.
//#define FORWARD_LOG_TO_CONSOLE