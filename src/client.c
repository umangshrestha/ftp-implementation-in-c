#include <stdio.h>      // printf
#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr_in
#include <unistd.h>     // close
#include <arpa/inet.h>  // inet_addr
#include <string.h>     // bzero
#include <stdlib.h>     // exit, EXIT_FAILURE
#include <signal.h>     // SIGNINT, signal
#include <sys/stat.h>   // mkfifo
#include <fcntl.h>      // O_RDONLY, O_WRONLY, O_CREAT, O_TRUNC

#include "status.h"
#include "config.h" // PORT, SERVER_PORT
#include "log.h"    // log

static int sockfd;
static Log *log;

int store_file(const char *from, const char *to)
{
    int readfd, writefd;
    if ((readfd = open(from, O_RDONLY)) < 0)
    {
        log->warnf("could not read file:%s\n", from);
        return -1;
    }
    if ((writefd = open(to, O_WRONLY, 0666)) < 0)
    {
        log->warnf("could not read file:%s\n", from);
        close(readfd);
        return -1;
    }
    copy_data(readfd, writefd);
    close(readfd);
    close(writefd);
    return 0;
}

void handle_signal(int fd)
{
    if (sockfd >= 0)
    {
        const char *quit = "quit\n";
        write(sockfd, quit, strlen(quit));
        close(sockfd);
    }
    free(log);
    exit(EXIT_SUCCESS);
    log->infof("Bye...\n");
}

int process()
{
    static char named_pipe[BUFFER_SIZE / 2];
    char buff[BUFFER_SIZE];
    char *process_string;
    ;
    int quit = 0;
    char *token, *argument;
    while (!quit)
    {
        bzero(buff, BUFFER_SIZE);
        printf(">>> ");
        if (fgets(buff, BUFFER_SIZE, stdin) == NULL ||
            strlen(buff) == 1)
            continue;
        process_string = strdup(buff);
        token = strtok(process_string, DELIMITER);
        argument = strtok(NULL, DELIMITER);

        if (strncmp("stor", token, 4) == 0 || strncmp("appe", token, 4) == 0)
        {
            char from[BUFFER_SIZE / 4];
            bzero(from, BUFFER_SIZE / 4);
            realpath(argument, from);
            write(sockfd, buff, strlen(buff));
            store_file(from, named_pipe);
            continue;
        }
        else if (strncmp("retr", token, 4) == 0)
        {
            char from[BUFFER_SIZE / 4];
            bzero(from, BUFFER_SIZE / 4);
            realpath(argument, from);
            write(sockfd, buff, strlen(buff));
            store_file(named_pipe, from);
            continue;
        }
        else if (strncmp("port", token, 4) == 0 && argument != NULL)
        {
            strcpy(named_pipe, LOG_FOLDER);
            strcat(named_pipe, argument);
            mkfifo(named_pipe, 0777);
            sprintf(buff, "port %s\n", named_pipe);
            log->infof("named_pipe:\"%s\"\n", named_pipe);
        }

        write(sockfd, buff, strlen(buff));
        // writing data
        while (1)
        {
            bzero(buff, BUFFER_SIZE);
            if (read(sockfd, buff, sizeof(buff)) == -1)
                break;
            printf("%s", buff);
            quit = strcmp(buff, STATUS_SERVER_CLOSED) == 0;
        }
    }
    if (process_string != NULL)
        free(process_string);
}

int main(int argc, char **argv)
{
    struct sockaddr_in client_addr;
    signal(SIGINT, &handle_signal);  // cntr + c
    signal(SIGTSTP, &handle_signal); // cntr + z

    log = new_logger(Debug);
    log->infof("#############################\n");
    log->infof("### STARTED CLIENT V0.0.1 ###\n");
    log->infof("#############################\n");
    if (argc != 1)
    {
        log->warnf("[Usuage] ./client\n");
        free(log);
        exit(EXIT_FAILURE);
    }
    log->debugf("creating socket\n");
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        log->warnf("socket creation\n");
        free(log);
        exit(EXIT_FAILURE);
    }
    log->debugf("initialize socket\n");
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    client_addr.sin_port = SERVER_PORT;
    // socket should only wait for response for RECV_TIMEOUT_IN_SEC duration
    struct timeval tv;
    tv.tv_sec = RECV_TIMEOUT_IN_SEC;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

    log->debugf("connecting to server\n");
    if (connect(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
    {
        log->warnf("error connecting to server\n");
        free(log);
        exit(EXIT_FAILURE);
    }
    process();
    free(log);
}