#ifndef __CONFIG_H
#define __CONFIG_H

#define LOG_FOLDER "/tmp/"
#define RECV_TIMEOUT_IN_SEC 1
#define DELIMITER " \n\0"
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 49190
#define MAX_ALLOWED_CONNECTION 10
#define BUFFER_SIZE 1024

// this will copy the data from readfd from writefd
int copy_data(int readfd, int writefd)
{
    int n;
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    while ((n = read(readfd, buffer, BUFFER_SIZE)) > 0)
    {
        if (write(writefd, buffer, n) != n)
            return -1;
        else if (n < BUFFER_SIZE)
            break;
        bzero(buffer, BUFFER_SIZE);
    }
    return 0;
}

#endif