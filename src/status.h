#ifndef __STATUS_H
#define __STATUS_H

// STATUS_CODE
#define STATUS_CONNECTION_ALREADY_OPEN "125 Data connection already open; transfer starting.\n"
#define STATUS_FILE_OK "150 File status okay; about to open data connection.\n"
#define STATUS_COMMAND_OK "200 Command okay.\n"
#define STATUS_SERVER_CLOSED "221 Service closing control connection. Logged out if appropriate.\n"
#define STATUS_FILE_COMPLETE "226 File transer complete.\n"
#define STATUS_USER_LOGGED_IN "230 User logged in, proceed.\n"
#define STATUS_FILE_ACTION_OK "250 Requested file action okay, completed.\n"
#define STATUS_CANT_OPEN_CONNECTION "425 Can't open data connection.\n"
#define STATUS_BAD_FILE "450 Requested file action not taken.\n"
#define STATUS_COMMAND_UNKNOWN "500 Command Unknown.\n"
#define STATUS_SYNTAX_ERROR "501 Arguments not found.\n"
#define STATUS_COMMAND_NOT_IMPLEMENTED "502 command not implemented.\n"
#define STATUS_INVALID_SEQUENCE "503 Bad sequence of cmds.\n"
#define STATUS_NO_SUCH_FILE_IN_DIRECTORY "550 no such file or directory.\n"
#define STATUS_CANT_WRITE_TO_FILE "552 Requested file action aborted. Can't write to file.\n"

#endif