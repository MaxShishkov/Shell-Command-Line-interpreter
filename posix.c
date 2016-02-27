/*
Maksim Shishkov
CSC415
HW7
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h> 

#define BUFF_SIZE 1024

int main(int argc, char *argv[])
{
    
    char buffer[BUFF_SIZE];
    char* token;
    char* input_one[100];
	char* input_two[100];
    int i;
    int j;
    pid_t pid1, pid2;
	int pipeID[2];
	int status;

	int background, file, err, infile, pip;
	

    
    //execution loop that goes foreever untll user enters exit as a command.
    
    while(1)
    {
		background = 0;
		file = 0;
		err = 0;
		infile = 0;
		pip = 0;
		j = 0;
		i = 0;
		token = NULL;
        
        for (j; j < 5; j++)
        {
            input_one[j] = NULL;
            input_two[j] = NULL;
        }
        

        printf ("Myshell> ");
        
        //get user input
        fgets(buffer, sizeof(buffer), stdin);
        
        //parse the string and break it into tokens
        token = strtok(buffer, " \n");
	
		
		//parse all the input
        while(token != NULL)
		{
			
			if(strcmp(token, "&") == 0)
			{
				background = 1;
				break;
			}
			else if(strcmp(token, ">") == 0 || strcmp(token, "2>") == 0)
			{
				if(strcmp(token, "2>") == 0)
					err = 1;

				token = strtok(NULL, " \n");
				file = open(token, O_WRONLY|O_CREAT, S_IRWXU|S_IRWXG);
				if(file == -1)
				{
					fprintf(stderr, "Error opening output file.\n");
				}
				break;
			}
			else if(strcmp(token, ">>") == 0 || strcmp(token, "2>>") == 0)
			{
				if(strcmp(token, "2>>") == 0)
					err = 1;

				token = strtok(NULL, " \n");
				file = open(token, O_WRONLY|O_APPEND|O_CREAT, S_IRWXU|S_IRWXG);
				if(file == -1)
				{
					fprintf(stderr, "Error opening output file.\n");
				}
				break;
			}
			else if(strcmp(token, "<") == 0)
			{
				token = strtok(NULL, " \n");
				infile = open(token, O_RDONLY, NULL);
				if(file == -1)
				{
					fprintf(stderr, "Error in opening input file\n");
					exit(0);
				}
				break;
			}
			else if(strcmp(token, "|") == 0)
			{
				pip = 1;
			
				i = 0;
				do
				{
					input_two[i++] = strtok(NULL, " \n");
				}while(i < 3);
				i = 0;
				break;
			}
			else if(strcmp(token, "exit") == 0)
			{
				exit(0);
			}
			else if(!background && !file && !infile && !err && !pip)
			{
				if(i<3 && token != NULL)
				{
					input_one[i] = token;
					i++;
					token = strtok(NULL," \n");
				}
			}
		}// end parse the input


		if(pip)
		{
			if ((status = pipe(pipeID)) == -1) 
			{
				perror("Bad pipe");
				exit(-1);
			}

			if ((pid1 = fork()) == -1) 
			{
				perror("Bad fork");
				exit(-1);
			}


			if(pid1 == 0)
			{
				close (0);
				dup (pipeID[0]);
				close (pipeID[0]);
				close(pipeID[1]);
				execlp(input_two[0],input_two[0],input_two[1],input_two[2],0); 
				/* error exit - exec returned */
				perror("Execl returned");
				exit(-1);
			}

			if ((pid2 = fork()) == -1) 
			{
				perror("Bad fork");
				exit(-1);
			}

			if(pid2 == 0)
			{
				close (1);
				dup(pipeID[1]);
				close(pipeID[0]);close(pipeID[1]);
				execlp(input_one[0],input_one[0],input_one[1],input_one[2],0); 
				/* error exit - exec returned */
				perror("Execl returned");
				exit(-1);
			}

			close(pipeID[0]);
			close(pipeID[1]);
			wait(pid1,0,0);
			wait(pid2,0,0);

		}//end pipe
		else //no piping
		{
			if((pid1 = fork()) == -1)
			{
				perror("Bad fork");
				exit(-1);
			}
			if(pid1 == 0)
			{
				if(file)
				{
					close(1);
					dup(file);
				}
				if(infile)
				{
					close(0);
					dup(infile);
				}
				if(err)
				{
					dup2(file,2);
				}

				if( execlp(input_one[0],input_one[0],input_one[1],input_one[2],0) < 0)
				{
					perror("Execl returned");
					exit(-1);
				}

				if(file || err)
					close(file);

				if(infile)
					close(infile);

			}
			else
			{
				if(!background)
					while(wait(&status) != pid1);
			}
		}
        
    }        
    
    return 0;
}
