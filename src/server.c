#include <stdio.h>      // printf
#include <sys/socket.h> // socket
#include <string.h>     // strcmp
#include <netinet/in.h> // sockaddr_in
#include <unistd.h>     // close
#include <stdlib.h>     // exit, EXIT_FAILURE
#include <sys/stat.h>   // mkdir, S_ISDIR, mkfifo
#include <dirent.h>     // DIR, opendir
#include <pthread.h>    // pthread_t, pthread_create,
#include <signal.h>     // SIGNINT, signal
#include <stdlib.h>     // read, write, bzero
#include <time.h>       // time
#include <fcntl.h>      // O_RDONLY, O_WRONLY, O_CREAT, O_TRUNC
#include <ctype.h>      // tolower

#include "status.h"
#include "config.h" // PORT
#include "log.h"    // log

static Log *log;
static int is_running = 1;
static int sockfd; // socket file descriptor



char* lowercase(char* s) {
  for(char *p=s; *p; p++) *p=tolower(*p);
  return s;
}

void send_response(int connfd, const char *buffer)
{
    write(connfd, buffer, strlen(buffer));
}

// print working directory
void cmd_pwd(int connfd, const char *buffer)
{
    log->debugf("cmd: PWD\n");
    char address[BUFFER_SIZE];
    getcwd(address, BUFFER_SIZE);
    write(connfd, address, sizeof(address));
    write(connfd, "\n", 1);
}

void cmd_unknown(int connfd, const char *buffer)
{
    log->debugf("unknown: %s\n", buffer);
    send_response(connfd, STATUS_COMMAND_UNKNOWN);
}

// make directory
void cmd_mkd(int connfd, const char *folder)
{
    log->debugf("cmd: MKD\n");
    if (folder == NULL)
        return send_response(connfd, STATUS_SYNTAX_ERROR);

    const char *response =
        (mkdir(folder, 0700) == 0) ? STATUS_COMMAND_OK : STATUS_NO_SUCH_FILE_IN_DIRECTORY;
    send_response(connfd, response);
}

void cmd_noop(int connfd, const char *buffer)
{
    // TODO
    log->debugf("cmd: NOOP\n");
    send_response(connfd, STATUS_COMMAND_OK);
}

// remove directory
void cmd_rmd(int connfd, const char *buffer)
{
    log->debugf("cmd: RMD\n");
    if (buffer == NULL)
        return send_response(connfd, STATUS_SYNTAX_ERROR);

    const char *response =
        (rmdir(buffer) == 0) ? STATUS_FILE_ACTION_OK : STATUS_NO_SUCH_FILE_IN_DIRECTORY;
    send_response(connfd, response);
}

void cmd_rest(int connfd, const char *buffer)
{
    // TODO
    log->debugf("cmd: REST\n");
    send_response(connfd, STATUS_COMMAND_OK);
}

// rename to
void cmd_rnto(int connfd, const char *from, const char *to)
{
    log->debugf("cmd: RNTO\n");
    if (from == NULL || to == NULL)
        return send_response(connfd, STATUS_INVALID_SEQUENCE);
    const char *response =
        (rename(from, to) != 0) ? STATUS_NO_SUCH_FILE_IN_DIRECTORY : STATUS_FILE_ACTION_OK;
    send_response(connfd, response);
}

// rename from
void cmd_rnfr(int connfd, const char *from)
{
    log->debugf("cmd: RNFR\n");
    if (from == NULL)
        return send_response(connfd, STATUS_SYNTAX_ERROR);
    send_response(connfd, STATUS_COMMAND_OK);
    //  after rnfr, rntr should be called immediately
    char buffer[BUFFER_SIZE];
    bzero(buffer, sizeof(buffer));
    read(connfd, buffer, BUFFER_SIZE);
    const char *token = strtok(buffer, DELIMITER);
    const char *to = strtok(NULL, DELIMITER);
    if (strncmp("rnto", token, 4) == 0)
        cmd_rnto(connfd, from, to);
    else
        send_response(connfd, STATUS_INVALID_SEQUENCE);
}

void cmd_rein(int connfd, const char *buffer)
{
    // TODO
    log->debugf("cmd: REIN\n");
    send_response(connfd, STATUS_COMMAND_NOT_IMPLEMENTED);
}

// change to upper directory
void cmd_cdup(int connfd, const char *buffer)
{
    log->debugf("cmd: CDUP\n");
    const char *response =
        (chdir("..") != 0) ? STATUS_NO_SUCH_FILE_IN_DIRECTORY : STATUS_FILE_ACTION_OK;
    send_response(connfd, response);
}

void cmd_abor(int connfd, const char *buffer)
{
    // TODO
    log->debugf("cmd: ABOR\n");
    send_response(connfd, STATUS_COMMAND_NOT_IMPLEMENTED);
}

void cmd_stat(int connfd, const char *buffer)
{
    // TODO
    log->debugf("cmd: STAT\n");
    send_response(connfd, STATUS_COMMAND_NOT_IMPLEMENTED);
}

void cmd_user(int connfd, const char *buffer)
{
    log->debugf("cmd: USER\n");
    send_response(connfd, STATUS_USER_LOGGED_IN);
}

void cmd_list(int connfd, const char *buffer)
{
    DIR *obj;
    struct dirent *dir;
    log->debugf("cmd: LIST\n");
    obj = opendir(".");

    if (obj)
    {
        while ((dir = readdir(obj)) != NULL)
        {
            write(connfd, dir->d_name, strlen(dir->d_name));
            write(connfd, "\n", 1);
        }
    }
    else
        send_response(connfd, STATUS_BAD_FILE);
}

void cmd_port(int connfd, const char *buffer, char *named_pipe)
{
    log->debugf("cmd: PORT\n");
    if (buffer == NULL)
        return send_response(connfd, STATUS_SYNTAX_ERROR);
    unlink(buffer);
    if (mkfifo(buffer, 0666) != 0)
        return send_response(connfd, STATUS_CANT_OPEN_CONNECTION);
    chmod(buffer, 0666);
    bzero(named_pipe, BUFFER_SIZE);
    strcpy(named_pipe, buffer);
    log->infof("named_pipe:%s\n", named_pipe);
    send_response(connfd, STATUS_COMMAND_OK);
}

// chnage directory
void cmd_cwd(int connfd, const char *directory)
{
    log->debugf("cmd: CWD\n");
    if (directory == NULL)
        return send_response(connfd, STATUS_SYNTAX_ERROR);

    const char *response =
        (chdir(directory) != 0) ? STATUS_NO_SUCH_FILE_IN_DIRECTORY : STATUS_FILE_ACTION_OK;
    send_response(connfd, response);
}

void cmd_stor(int connfd, const char *to, const char *from)
{
    log->debugf("cmd: STOR\n");
    int readfd, writefd;
    if (from == NULL)
        return send_response(connfd, STATUS_INVALID_SEQUENCE);

    if ((readfd = open(from, O_RDONLY)) < 0)
        return send_response(connfd, STATUS_CANT_OPEN_CONNECTION);

    if ((writefd = open(to, O_CREAT | O_WRONLY | O_TRUNC, 0666)) < 0)
    {
        close(readfd);
        return send_response(connfd, STATUS_NO_SUCH_FILE_IN_DIRECTORY);
    }
    send_response(connfd, STATUS_CONNECTION_ALREADY_OPEN);
    if (copy_data(readfd, writefd) == -1)
        send_response(connfd, STATUS_CANT_WRITE_TO_FILE);
    close(readfd);
    close(writefd);
    send_response(connfd, STATUS_COMMAND_OK);
}

void cmd_appe(int connfd, const char *to, const char *from)
{
    log->debugf("cmd: APPE\n");
    int readfd, writefd;
    if (from == NULL)
        return send_response(connfd, STATUS_INVALID_SEQUENCE);

    if ((readfd = open(from, O_RDONLY)) < 0)
        return send_response(connfd, STATUS_CANT_OPEN_CONNECTION);

    if ((writefd = open(to, O_CREAT | O_WRONLY | O_APPEND, 0666)) < 0)
    {
        close(readfd);
        return send_response(connfd, STATUS_NO_SUCH_FILE_IN_DIRECTORY);
    }
    send_response(connfd, STATUS_CONNECTION_ALREADY_OPEN);
    if (copy_data(readfd, writefd) == -1)
        send_response(connfd, STATUS_CANT_WRITE_TO_FILE);
    close(readfd);
    close(writefd);
    send_response(connfd, STATUS_COMMAND_OK);
}

// delete file
void cmd_dele(int connfd, const char *filename)
{
    log->debugf("cmd: DELE\n");
    if (filename == NULL)
        return send_response(connfd, STATUS_SYNTAX_ERROR);
    const char *response =
        (remove(filename) == 0) ? STATUS_FILE_ACTION_OK : STATUS_NO_SUCH_FILE_IN_DIRECTORY;
    send_response(connfd, response);
}

void cmd_retr(int connfd, const char *from, const char *to)
{
    log->debugf("cmd: RETR\n");
    cmd_appe(connfd, to, from);
}

void *server_process(void *fd)
{
    int quit = 0;
    char *token, *argument;
    int connfd = *((int *)fd);
    char buffer[BUFFER_SIZE];
    char named_pipe[BUFFER_SIZE];
    while (is_running && !quit)
    {
        bzero(buffer, sizeof(buffer));
        read(connfd, buffer, BUFFER_SIZE);
        log->infof("payload:%d command:%s", strlen(buffer), buffer);
        token = lowercase(strtok(buffer, DELIMITER));
        argument = strtok(NULL, DELIMITER);
        if (token == NULL)
            continue;
        if (strncmp("quit", token, 4) == 0)
            quit = 1;
        else if (strncmp("port", token, 4) == 0)
            cmd_port(connfd, argument, named_pipe);
        else if (strncmp("pwd", token, 3) == 0)
            cmd_pwd(connfd, argument);
        else if (strncmp("user", token, 4) == 0)
            cmd_user(connfd, argument);
        else if (strncmp("mkd", token, 3) == 0)
            cmd_mkd(connfd, argument);
        else if (strncmp("rmd", token, 3) == 0)
            cmd_rmd(connfd, argument);
        else if (strncmp("stor", token, 4) == 0)
            cmd_stor(connfd, argument, named_pipe);
        else if (strncmp("appe", token, 4) == 0)
            cmd_appe(connfd, argument, named_pipe);
        else if (strncmp("list", token, 4) == 0)
            cmd_list(connfd, argument);
        else if (strncmp("stat", token, 4) == 0)
            cmd_stat(connfd, argument);
        else if (strncmp("noop", token, 4) == 0)
            cmd_noop(connfd, argument);
        else if (strncmp("rnto", token, 4) == 0)
            cmd_rnto(connfd, NULL, argument);
        else if (strncmp("rnfr", token, 4) == 0)
            cmd_rnfr(connfd, argument);
        else if (strncmp("rest", token, 4) == 0)
            cmd_rest(connfd, argument);
        else if (strncmp("rein", token, 4) == 0)
            cmd_rein(connfd, argument);
        else if (strncmp("cdup", token, 4) == 0)
            cmd_cdup(connfd, argument);
        else if (strncmp("retr", token, 4) == 0)
            cmd_retr(connfd, argument, named_pipe);
        else if (strncmp("abor", token, 4) == 0)
            cmd_abor(connfd, argument);
        else if (strncmp("cwd", token, 3) == 0)
            cmd_cwd(connfd, argument);
        else if (strncmp("dele", token, 4) == 0)
            cmd_dele(connfd, argument);
        else
            cmd_unknown(connfd, buffer);
    }
    log->debugf("cmd: QUIT\n");
    send_response(connfd, STATUS_SERVER_CLOSED);
    close(connfd);
    pthread_exit(NULL);
}

void handle_signal()
{
    is_running = 0;
    close(sockfd);
    free(log);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    pthread_t thread;
    int connfd;
    int opt = 1;
    socklen_t clientLen;
    struct sockaddr_in server_addr, client_addr;

    signal(SIGINT, &handle_signal);  // cntr + c
    signal(SIGTSTP, &handle_signal); // cntr + z

    log = new_logger(Debug);
    if (chdir((argc == 1) ? "." : argv[1]) != 0) // changing directory
    {
        log->warnf("could not change directory\n");
        free(log);
        exit(EXIT_FAILURE);
    }
    log->infof("#############################\n");
    log->infof("### STARTED SERVER V0.0.1 ###\n");
    log->infof("#############################\n");

    log->debugf("creating socket\n");
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        log->warnf("socket creation\n");
        free(log);
        exit(EXIT_FAILURE);
    }
    log->debugf("initialize socket\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = SERVER_PORT;

    // preventing binding issue
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        log->warnf("setsock opt error\n");
        free(log);
        exit(EXIT_FAILURE);
    }
    log->debugf("binding socket\n");
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        log->warnf("binding error\n");
        free(log);
        exit(EXIT_FAILURE);
    }
    log->debugf("listining connection\n");
    listen(sockfd, MAX_ALLOWED_CONNECTION);
    while (is_running)
    {
        clientLen = sizeof(client_addr);
        if ((connfd = accept(sockfd, (struct sockaddr *)&client_addr, &clientLen)) >= 0)
            pthread_create(&thread, 0, server_process, (void *)&connfd); // the process will call quit on its own
    }
    free(log);
    pthread_join(thread, NULL); // wait for all pthread to end
    log->infof("Bye...\n");
}