//shell.c

// Shell starter file
// You may make any changes to any part of this file.

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
#define HISTORY_DEPTH 10


//global variables
char hist[HISTORY_DEPTH][COMMAND_LENGTH];
int cmd_count = 0;
_Bool signaled = false;
//

/**
 * Command Input and Processing
 */

/*
 * Tokenize the string in 'buff' into 'tokens'.
 * buff: Character array containing string to tokenize.
 *       Will be modified: all whitespace replaced with '\0'
 * tokens: array of pointers of size at least COMMAND_LENGTH/2 + 1.
 *       Will be modified so tokens[i] points to the i'th token
 *       in the string buff. All returned tokens will be non-empty.
 *       NOTE: pointers in tokens[] will all point into buff!
 *       Ends with a null pointer.
 * returns: number of tokens.
 */
int tokenize_command(char *buff, char *tokens[])
{
	int token_count = 0;
	_Bool in_token = false;
	int num_chars = strnlen(buff, COMMAND_LENGTH);
	for (int i = 0; i < num_chars; i++) {
		switch (buff[i]) {
		// Handle token delimiters (ends):
		case ' ':
		case '\t':
		case '\n':
			buff[i] = '\0';
			in_token = false;
			break;

		// Handle other characters (may be start)
		default:
			if (!in_token) {
				tokens[token_count] = &buff[i];
				token_count++;
				in_token = true;
			}
		}
	}
	tokens[token_count] = NULL;
	return token_count;
}

/**
 * Read a command from the keyboard into the buffer 'buff' and tokenize it
 * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
 * buff: Buffer allocated by the calling code. Must be at least
 *       COMMAND_LENGTH bytes long.
 * tokens[]: Array of character pointers which point into 'buff'. Must be at
 *       least NUM_TOKENS long. Will strip out up to one final '&' token.
 *       tokens will be NULL terminated (a NULL pointer indicates end of tokens).
 * in_background: pointer to a boolean variable. Set to true if user entered
 *       an & as their last token; otherwise set to false.
 */
void read_command(char *buff, char *tokens[], _Bool *in_background)
{
	*in_background = false;

	// Read input
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);

	if ((length < 0) && (errno!=EINTR)) {
		perror("Unable to read command from keyboard. Terminating.\n");
		exit(-1);
	}

	// Null terminate and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}

	// Tokenize (saving original command string)
	int token_count = tokenize_command(buff, tokens);
	if (token_count == 0) {
		return;
	}

	// Extract if running in background:
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		*in_background = true;
		tokens[token_count - 1] = 0;
	}
}

/*newly defined functions*/

//display: writes a c string to the screen
//dependent on: none
void display(char * message){
	write(STDOUT_FILENO,message,strlen(message));
}

//is_number_string: reads a string and determines if it contains a non-number character
int is_number_string(char * string_in_question, int length){
	
	int is_number_string = 1;
	for(int i=0; i<length; i++){
		if(string_in_question[i]<48 || string_in_question[i]>57){
			is_number_string = 0;
			break;
		}	
	}

	return is_number_string;
}

//count_spaces: gives the number of spaces between tokens
//dependent on: none
int count_spaces(char * t[NUM_TOKENS]){
	
	int spaces = 0;
	while(t[spaces]!=NULL){
		spaces+=1;
	}

	//needs two tokens per space; the last space does not count
	if(spaces>0){
		spaces-=1;
	}

	return spaces;
}



//setstr: copies a c style string history with the provided index.
//dependent on: none
void setstr(char * buff, char h[HISTORY_DEPTH][COMMAND_LENGTH], int index, int spaces, _Bool in_bg){
	
	int s_remainder = spaces;

	for(int i=0; i<COMMAND_LENGTH; i++){
		if(buff[i]!='\0'){
			h[index][i] = buff[i];
		} else if(s_remainder>0){
			h[index][i] = ' ';
			s_remainder-=1;
		} else {
			if(in_bg){
				h[index][i] = ' ';
				h[index][i+1] = '&';
				i+=1;
				in_bg = false;
			} else {
				h[index][i] = '\0';
			}
		}
	}

}

//update_hist: stores the most recent commands entered into a circular array
//dependent on: setstr() 
void update_hist(char * buff, char h[HISTORY_DEPTH][COMMAND_LENGTH], int * c_cnt, int spaces, _Bool in_bg){
	
	//note: modular will ensure the circular array will overlap itself at head-tail
	setstr(buff, h, (*c_cnt)%HISTORY_DEPTH, spaces, in_bg);
	(*c_cnt)+=1;
}

//history: prints the history of the most recent commands
//dependent on: display()
void history(char h[HISTORY_DEPTH][COMMAND_LENGTH], int c_cnt){
	
	char buffer[20]; //apparently the maximum number of characters a long can have is 20
	int loop_cap = 0;
	int padding = 0;
	//note: the padding variable shifts index to match the head of the circular array

	if(c_cnt>HISTORY_DEPTH){
		loop_cap = HISTORY_DEPTH;
		padding = c_cnt-HISTORY_DEPTH;
		/*for every unit cmd count exceeds history depth, shift the head over by 1 unit
		//for example when cmd count is 11 and history depth is 10, shift unit by 11-10=1
		//the padding is the base index where the array head is located
		//visual example:
		//[1]-[2]-[3]-[4]-[5]-[6]-[7]-[8]-[9]-[10] head = [1]
		//[11]-[2]-[3]-[4]-[5]-[6]-[7]-[8]-[9]-[10] head = [2]*/ 
	} else {
		loop_cap = c_cnt;
	}

	for(int i=0; i<loop_cap; i++){
		sprintf(buffer,"%d",(i+padding+1)); //int to string: want range from c_cnt-HISTORY_DEPTH to HISTORY_DEPTH
		display(buffer);
		display("\t");
		display(h[(i+padding)%HISTORY_DEPTH]); //circular array index calculation wrapped around HISTORY_DEPTH
		display("\n");
	}
}

//shellpwd: displays the path of the current work directory.
//dependent on: display()
void shellpwd(){

	long psize = pathconf(".", _PC_PATH_MAX); 
	//note: gets the path length to current folder.
	char * path = malloc(psize);	
	path = getcwd(path,(size_t)psize);
	display(path);
	display("\n");
	free(path);
}

//shellcd: changes directory
void shellcd(char * destination){
	int status = 0;
	if(destination!=NULL){
		status = chdir(destination);
		if(status<0){
			display("Directory changing error.\n");
		}
	}
}

//exefork: fork a new process and execute a command
//dependent on: display()
void exefork(char * tokens[NUM_TOKENS], _Bool in_background){
	
	//forking variables
	pid_t pid;
	int status;
	int error_state;

	//forking point: two processes are created
	//pid of 0 is the child; pid > 0 is the parent. pid < 0 is a fail.
	pid = fork();
	if(pid<0){
		perror("Failed to create a process.");
	} else if(pid==0){
		error_state = execvp(tokens[0],tokens);
		if(error_state<0){
			display("Unable to run process. Aborting.\n");
		}
		_exit(1);
	} else {
		//deal with background processes 
		if(!in_background){
			waitpid(pid, &status, 0);
		}
	}

}

void handle_SIGINT(){
	display("\n");
	history(hist, cmd_count);	
	signaled = true;
}

/* end of newly defined functions*/

/**
 * Main and Execute Commands
 */
int main(int argc, char* argv[])
{

	//SIGINT handler
	//borrowed from the signals guide
	struct sigaction handler;
	handler.sa_handler = handle_SIGINT;
	handler.sa_flags = 0;
	sigemptyset(&handler.sa_mask);
	sigaction(SIGINT, &handler, NULL);
	//end of handler

	//core variables
	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS];
	int tok_cnt = 0;

	//main loop
	while(true){	
		// Get command
		// Use write because we need to use read() to work with
		// signals, and read() is incompatible with printf().
		write(STDOUT_FILENO, "> ", strlen("> "));
		_Bool in_background = false;
		read_command(input_buffer, tokens, &in_background);

		if (in_background) {
			display("Running in background...\n");
		}

		//note: strcmp returns 0 if strings are an exact match
		if(tokens[0]!=NULL && !signaled){
			
			//update the information on command history.
			if(tokens[0][0]!='!'){
				update_hist(input_buffer, hist, &cmd_count, count_spaces(tokens), in_background);
			}

			//command gate
			if(!strcmp(tokens[0],"exit")){
				//if exit is called, no need to fork! just close the program
				exit(0);
			} else if(!strcmp(tokens[0],"pwd")){
				//if pwd is called, show path in main process
				shellpwd();
			} else if(!strcmp(tokens[0],"cd")) {
				//if cd called, change directory in main process
				shellcd(tokens[1]);
			} else if(!strcmp(tokens[0],"history")){
				//if history is called, display the recent history list
				history(hist, cmd_count);
			} else if(tokens[0][0]=='!'){
				//history commands
				if(tokens[0][1]!='\0'){
					//check if second entry is '!', if it is, it wants the previous command
					if(tokens[0][1]=='!' && tokens[0][2]=='\0'){
						if(cmd_count>0){
							display(hist[(cmd_count-1)%HISTORY_DEPTH]);
							display("\n");
							strcpy(input_buffer, hist[(cmd_count-1)%HISTORY_DEPTH]);
							tok_cnt = tokenize_command(input_buffer,tokens);
							if (tok_cnt > 0 && strcmp(tokens[tok_cnt - 1], "&") == 0) {
								in_background = true;
								tokens[tok_cnt - 1] = 0;
							}
							update_hist(input_buffer, hist, &cmd_count, count_spaces(tokens), in_background);
							if(!strcmp(tokens[0],"history")){
								history(hist,cmd_count);
							} else if(!strcmp(tokens[0],"pwd")) {
								shellpwd();
							} else if(!strcmp(tokens[0],"cd")){
								shellcd(tokens[1]);
							} else {
								exefork(tokens, in_background);
							}
						} else {
							display("No previous history!\n");
						}
					} else {
						//case where either input is invalid or it is a number in range
						int valid_numbers;
						int sub_index = 0;
						int sub_len = strlen(tokens[0]);
						char sub_cmd[sub_len];
						int lb = 0; //lower bound

						//copy the subcommand from tokens but remove the '!'
						for(int i=0; i<sub_len-1; i++){
							sub_cmd[i] = tokens[0][i+1];
						}
						sub_cmd[sub_len-1] = '\0';
						//this should honestly be its own function, but it's one-time so it's okay
						sub_index = atoi(sub_cmd); //atoi returns 0 on fail
						valid_numbers = is_number_string(sub_cmd, strlen(sub_cmd));

						if(sub_index!=0 && valid_numbers){

							if(cmd_count>HISTORY_DEPTH){
								lb = cmd_count-HISTORY_DEPTH;
							}

							if(sub_index>lb && sub_index<=cmd_count){
								//correct index given. execute history command.
								display(hist[(sub_index-1)%HISTORY_DEPTH]);
								display("\n");
								strcpy(input_buffer, hist[(sub_index-1)%HISTORY_DEPTH]);
								tok_cnt = tokenize_command(input_buffer,tokens);
								if (tok_cnt > 0 && strcmp(tokens[tok_cnt - 1], "&") == 0) {
									in_background = true;
									tokens[tok_cnt - 1] = 0;
								}
								update_hist(input_buffer, hist, &cmd_count, count_spaces(tokens), in_background);
								if(!strcmp(tokens[0],"history")){
									history(hist,cmd_count);
								} else if(!strcmp(tokens[0],"pwd")) {
									shellpwd();
								} else if(!strcmp(tokens[0],"cd")){
									shellcd(tokens[1]);
								} else {
									exefork(tokens, in_background);
								}				

							} else {
								display("Invalid history range.\n");
							}
						} else {
							display("Invalid history command.\n");
						}
					}
				} else {
					display("Invalid command.\n");
				}	
			} else {
				//system command
				exefork(tokens, in_background);
			}
		} else {
			signaled = false;	
		}
		
		//clean up zombies
		while(waitpid(-1,NULL,WNOHANG)>0);	
	}

	/*/
	 *	 End of loop
	/*/

	return 0;
}
