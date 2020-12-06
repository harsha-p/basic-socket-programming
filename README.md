# basic-socket-programming


The files server.c and client.c can be placed in any two different directories.
The client can request to download the files which are in the server's directory

### Instructions
- First,start server 
```
gcc server.c -o server
  ./server
```
- And then,start client and request files
```
gcc client.c -o client
./client <file1> <file2> ...
```

