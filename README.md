# simple-unix-shell
Simple Unix shell with history feature written in C language 

The shell accepts user commands and then executes each command in a separate process. The shell provides the user a prompt at which the next command is entered. One technique for implementing a shell interface is to have the parent process first read what the user enters on the command line and then create a separate child process that performs the command. Unless otherwise specified, the parent process waits for the child to exit before continuing. However, UNIX shells typically also allow the child process to run in the background - or concurrently - as well by specifying the ampersand (&) at the end of the command.

