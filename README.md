This project consists of a client and server to simulate FTP functionality. Note that this code is only compactability with Linux platforms as OS compactibility was not part of the initial project specifications. 

The code base consists of server. The code be run using:
```
$ ./bin/server
```
By default it will use the current directory as path. In case you want t*initialize diffrent directory simply try the following command.
```
$ ./bin/server <path>
```

Running the server will open a TCP connection at a hardcoded port defined in config.h. The client can be run using 
```
$ ./bin/client
```
It will connect to the server which spawns a thread that will coninously read and respond to client messages. Following commands have been targetted for the implementation. 

* USER  
* CWD 
* CDUP
* REIN
* QUIT
* PORT (define named pipe to send or store data)
* RETR
* STOR
* APPE
* REST
* RNFR
* RNTO
* ABOR
* DELE
* RMD
* MKD
* PWD
* LIST
* STAT
* NOOP