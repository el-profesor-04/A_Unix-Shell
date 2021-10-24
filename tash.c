#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#define SH_TOK_BUFSIZE 64
#define SH_TOK_DELIM " \t\r\n\a"

//
//Execute function takes path and args for execv function.
//Execute does Redirect checking and file overwriting.
//The function returns 1 on successful termination (if command executes)...
//
int execute(char** myargs, char* s1){

    int redirect = 0;
    int redir_index = 0;
    int first_rd = 0;
    char* rd = ">";
    int rd_count = 0;
    char* y;
    char* break_rd;

    if (strcmp(myargs[0],"exit\n\r")==0 || strcmp(myargs[0],"exit\n")==0 || strcmp(myargs[0],"exit")==0){
        exit(0);
        return 1;
    }

    //
    //
    //if(myargs[1]==NULL || strlen(myargs[1])==0){
    //    myargs[1]=NULL;
    //}

    //
    //checks if redirect symbol ">" is present in args.
    //
    while(myargs[redir_index]!=NULL){     //
        
        if((y=strstr(myargs[redir_index], rd)) != NULL){
	    
	    first_rd = redir_index-1;
           //
            break_rd = strtok(myargs[redir_index],">");
            if(break_rd==NULL && redir_index==0){
	    	//char error_messag[300] = "bad redirect\n";
		//write(STDERR_FILENO, error_messag, strlen(error_messag));
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message,strlen(error_message));
		return 1;
	    }

	    //
            rd_count++;
            if (break_rd!=NULL){
                first_rd = redir_index;
		myargs[redir_index] = break_rd;
            }
            //
            break_rd = strtok(NULL,">");

	    if(break_rd==NULL && myargs[redir_index+1]==NULL){
	        //char error_messag[30] = "bad redir\n";
		//write(STDERR_FILENO, error_messag, strlen(error_messag));
		char error_message[30] = "An error has occured\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
		return 1;
	    }

            //
            if (break_rd!=NULL){
                myargs[redir_index+1] = break_rd;
            }
	    else{
	        myargs[redir_index] = myargs[redir_index+1];
	    }

            //
            if (myargs[redir_index+1]==NULL || myargs[redir_index+2]){
                //char error_messag[300] = "bad redirect\n";
                //write(STDERR_FILENO, error_messag, strlen(error_messag));
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
                return 1;
            }
            redirect = 1;
            //
        }
        if(rd_count>1){
            //redirect sybmol more than once
            //char error_messag[300] = "redirect sybmol more than once\n";
            //write(STDERR_FILENO, error_messag, strlen(error_messag));
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
        
        redir_index++;
    
    }
    int success = 0;
    //
    //Checks if the path has the command executable file..
    //
    if (access(s1, X_OK)==0){
        success = 1;
        int rc = fork();
        //
        if (rc<0){
            printf("fork failed");
        }else if(rc==0){
            //
            if (redirect==1){
                char* rargs[first_rd+2];
                int i =0 ;
                //
		//STD OUT to the file (redirection)..
		//
                for (i=0; i<=first_rd; i++){
                    rargs[i] = myargs[i];
                    //
                }rargs[first_rd+1]=NULL;
                //
                close(STDOUT_FILENO);
                char* file = malloc(SH_TOK_BUFSIZE * sizeof(char *));
                strcpy(file, "./");
                strcat(file, myargs[first_rd+1]);
                open(file, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
                execv(s1, rargs);    
                //printf("after re?\n");
            }
            else{
                //
                execv(s1, myargs);
                //printf("execv ");
            }
        }
    }
    return success;
}

//
//The function tokenize takes line and path as arguments
//and checks for built-in commands or calls execute
//


void tokenize(char* line, char** path){
    char* myexit = "exit\n";
    char* myexit2 = "exit";
    
    
    if (strcmp(line, myexit)==0){
        exit(0);
    }

    char* broke = NULL;
    
    char** broke_cmd = malloc(SH_TOK_BUFSIZE * sizeof(char *));
    int br_index = 0;
    
    //
    //Breaks the line to form separate commands
    //
    broke = strtok(line, "&");
    //
    while(broke!=NULL){
        //
        broke_cmd[br_index] = broke;
        br_index++;
        //
        broke = strtok(NULL, "&");
    }
    br_index=0;
    while(broke_cmd[br_index]!=NULL){

        char* token;
        // char* s1;
        char** tokens  = malloc(20000 * sizeof(char *));

        //
	//tokenize each command and args..
	//
        token = strtok(broke_cmd[br_index], SH_TOK_DELIM);
        int index = 0;

        
        while (token!=NULL){
            //
            tokens[index] = token;
            
            index++;
            token = strtok(NULL, SH_TOK_DELIM);
            if (token==NULL || strlen(token)==0){
                //
                break;
            }
        }
        if(tokens[0]==NULL){
            br_index++;
	    continue;
        }

        if (strcmp(tokens[0], myexit2)==0){
            if(tokens[1]!=NULL){
                //badexit
                //char error_messag[30] = "badexit\n";
                //write(STDERR_FILENO, error_messag, strlen(error_messag));
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
        }
        
        char* cd = "cd";
        char* cmd_path = "path";
        if (strcmp(tokens[0], cd)==0){
            //printf("tokens[1]:%s, tokens[2]:%s\n",tokens[1],tokens[2]);
            if (tokens[1]==NULL || tokens[2]!=NULL){
                //more than 1 arg for cd
                //char error_messag[30] = "more than 1 arg for cd\n";
                //write(STDERR_FILENO, error_messag, strlen(error_messag));
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
            }

            chdir(*(tokens+1));
            br_index++;
            continue;
        }
        else if(strcmp(tokens[0], cmd_path)==0){
            
            
            int index_c_path = 0;
            do{
                index_c_path++;
                path[index_c_path-1] = *(tokens+index_c_path);
                //
               
            }while(tokens[index_c_path]!=NULL);
            
            //
            br_index++;
            //
            continue;
        }

        int path_index = 0;
        int path_exists = 0;
        while (path[path_index]!=NULL){
            char* s1 = malloc(SH_TOK_BUFSIZE * sizeof(char *));
            strcpy(s1, path[path_index]);
            strcat(s1,"/");
            strcat(s1,tokens[0]);
            
            s1 = strtok(s1,">");
            //
            //
            if(execute(tokens, s1)==1){
                path_exists = 1;
            }

            path_index++;
        }
        if(path_exists==0){
            //no path for cmd:
            //char error_messag[30] = "no path for cmd\n";
            //write(STDERR_FILENO, error_messag, strlen(error_messag));
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
        }

        br_index++;
    }
        
    while(wait(NULL)!= -1 || errno != ECHILD){
        //printf("waitin..\n");
    }
    //free(s1);
    return;
}
//
//
//Main function takes argument in batch mode..
//Calls tokenize on each line...
//
int
main(int argc, char *argv[]) {


    char** path = malloc(SH_TOK_BUFSIZE * sizeof(char *));
    
    path[0] = "/bin";
    
    
    size_t bufsize = 0;
    
    FILE *fp = NULL;
    fp = fopen(*(argv+1), "r");
    size_t len = 0;
    ssize_t read;



    
    if (fp != NULL){
        //printf("in file if\n");
        while (1) {
            char* line = NULL;
            read = getline(&line, &len, fp);
            if (read==-1){
                break;
            }
            //printf("Retrieved line of length %zu:\n", read);
            //printf("line%s\n", line);
            //printf("while start, path:%s?\n",*(path+0));
            if (read == 0){
                continue;
            }

            char* single = NULL;
            char* copy_line = malloc(SH_TOK_BUFSIZE * sizeof(char *));
            strcpy(copy_line, line);

            
            single = strtok(copy_line,"&>");
            //printf()
            if (single==NULL && copy_line!=NULL){
                //line only has > or & or nl or rc
                // char error_messag[300] = "line only has > or & or nl or rc\n";
                //write(STDERR_FILENO, error_messag, strlen(error_messag));
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
                continue;
            }
            //printf("before tokenize call path[0]:%s?\n", path[0]);
            tokenize(line, path);
            //printf("after tokenize call path[0]:%s?\n", path[0]);
        }
        if (read==-1){
            //read =-1
            //char error_messag[30] = "read = -1\n";
            //write(STDERR_FILENO, error_messag, strlen(error_messag));
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        return 0;
    }

    while(1){
        char* line = NULL;
        char* copy_line = malloc(SH_TOK_BUFSIZE * sizeof(char *));
    	printf("tash> ");
        //int iter = 0;
        
        size_t characters = getline(&line, &bufsize, stdin);

        strcpy(copy_line, line);
        char* single = NULL;

        single = strtok(copy_line,"&>");
        if (single==NULL && copy_line!=NULL){
            //line only has > or & or nl or rc
            //char error_messag[300] = "line only has > or & or nl or rc\n";
            //write(STDERR_FILENO, error_messag, strlen(error_messag));
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            continue;
        }

        //printf("Retrieved line of length %zu:\n", characters);
        if (characters==-1){
            //char = -1
            //char error_messag[30] = " char = -1\n";
            //write(STDERR_FILENO, error_messag, strlen(error_messag));
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            continue;        
        }
        
        tokenize(line, path);
        
    }


    return EXIT_SUCCESS;
}
