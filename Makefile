#default:
#	cc -o shell shell.c
# See example: https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles$
# the compiler: gcc for C program, define as g++ for C++
CC = gcc
CCX = CC
RM = rm
# compiler flags:-D_XOPEN_SOURCE 
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall -Werror -g

# the build target executable:
TARGET = shell
OBJ = shell.c

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)
clean:
	$(RM) -f $(TARGET)


