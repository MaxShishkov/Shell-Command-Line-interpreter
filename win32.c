/*
Maksim Shishkov
CSC415
HW7
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <tchar.h>

#define BUFFERSIZE 1024

int main() 
{
	HANDLE rhandle;
	HANDLE whandle;
    char *temp;
    char buffer[BUFFERSIZE];
    char *token;
    char input_one[100];
	char input_two[100];
    int i;
    int j;
	int background, pip;
   

    STARTUPINFO si1;
    PROCESS_INFORMATION pi1;
	STARTUPINFO si2;
    PROCESS_INFORMATION pi2;

	
	SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

    ZeroMemory( &si1, sizeof(si1) );
    si1.cb = sizeof(si1);
    ZeroMemory( &pi1, sizeof(pi1) );

	ZeroMemory( &si2, sizeof(si2) );
    si2.cb = sizeof(si2);
    ZeroMemory( &pi2, sizeof(pi2) );

	GetStartupInfo(&si1);

	si1.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	si1.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	si1.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	si1.dwFlags = STARTF_USESTDHANDLES;



    // Execution loop
    while(1) 
	{
		background = 0;
		pip = 0;
		j = 0;
		i = 0;
		token = NULL;
        
        for (j; j < 5; j++)
        {
            input_one[j] = '\0';
            input_two[j] = '\0';
        }
        

        printf ("Myshell> ");
        
        //get user input
        fgets(buffer, BUFFERSIZE, stdin);
        
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
				
				if(strcmp(token, ">") == 0)
				{
					token = strtok(NULL, " \n");

					if((si1.hStdOutput = CreateFile(token, GENERIC_WRITE, 0, &sa, CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
					{
						fprintf(stderr, "Error in creating output file\n");
						exit(0);
					}
				}
				else
				{
					token = strtok(NULL, " \n");
					if((si1.hStdError = CreateFile(token, GENERIC_WRITE, 0, &sa, CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
					{
						fprintf(stderr, "Error in creating output file\n");
						exit(0);
					}
				}
				
				
				break;
			}//end > && 2>

			else if(strcmp(token, ">>") == 0 || strcmp(token, "2>>") == 0)
			{
				if(strcmp(token, ">>") == 0)
				{
					token = strtok(NULL, " \n");

					if((si1.hStdOutput = CreateFile(token, FILE_APPEND_DATA, FILE_SHARE_WRITE, &sa, OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
					{
						if((si1.hStdOutput = CreateFile(token, GENERIC_WRITE, 0, &sa, CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
						{
							fprintf(stderr, "Error in creating output file\n");
							exit(0);
						}
					}
				}
				else
				{
					token = strtok(NULL, " \n");
					if((si1.hStdError = CreateFile(token, FILE_APPEND_DATA|GENERIC_WRITE, FILE_SHARE_WRITE, &sa, OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
					{
						if((si1.hStdError = CreateFile(token, GENERIC_WRITE, 0, &sa, CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
						{
							fprintf(stderr, "Error in creating output file\n");
							exit(0);
						}
					}
				}

				
				break;
			}//end >> && 2>>

			else if(strcmp(token, "<") == 0)
			{
				token = strtok(NULL, " \n");
				if((si1.hStdInput = CreateFile(token, GENERIC_READ, 0, &sa, OPEN_ALWAYS,
						FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
				{
					fprintf(stderr, "Error in opening input file\n");
					exit(0);
				}
				break;
			}
			else if(strcmp(token, "|") == 0)
			{
				pip = 1;
				token = strtok(NULL," \n");
				i = 0;
				if(i<3 && token != NULL)
				{
					
					strcat(input_two, token);
					strcat(input_two, " ");
					token = strtok(NULL," \n");
					i++;
				}
				i = 0;
				break;
			}
			else if(strcmp(token, "exit") == 0)
			{
				exit(0);
			}
			else
			{
				if(i<3)
				{
					strcat(input_one, token);
					strcat(input_one, " ");
					i++;
					
					token = strtok(NULL," \n");
					
				}
			}
		}// end parse the input

		

		if(pip)
		{
			if (!CreatePipe(&rhandle, &whandle, &sa, 0))
			{
				fprintf(stderr, "Create Pipe Failed\n");
				return 1;
			}

			
			GetStartupInfo(&si2);
			si1.hStdInput = rhandle;

			if(!CreateProcess(NULL, input_two, NULL, NULL, TRUE, 0, NULL, NULL, &si1, &pi1))
			{
				fprintf(stderr, "CreateProcess one failed.\n");
				return -1;
			}


			//Start the 2nd child
			si2.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
			si2.hStdError = GetStdHandle(STD_ERROR_HANDLE);
			si2.hStdOutput = whandle;
			si2.dwFlags = STARTF_USESTDHANDLES;

			if(!CreateProcess(NULL, input_one, NULL, NULL, TRUE, 0, NULL, NULL, &si2, &pi2))
			{
				fprintf(stderr, "CreateProcess two failed.\n");
				return -1;
			}
		
		}//end pipe
		else
		{
			if(!CreateProcess(NULL, input_one, NULL, NULL, TRUE, 0, NULL, NULL, &si1, &pi1))
			{
				fprintf(stderr, "CreateProcess single failed.\n");
				return -1;
			}
			else
			{
				Sleep(1000);
			}
		}

		if(pip)
		{
			WaitForSingleObject(pi2.hProcess, INFINITE);
			WaitForSingleObject(pi1.hProcess, INFINITE);
		}
		else
		{
			if(!background)
				WaitForSingleObject(pi1.hProcess, INFINITE);
		}


		// Close process, thread handles and file
			CloseHandle(pi1.hProcess);
			CloseHandle(pi1.hThread);

			if(si1.hStdInput != GetStdHandle(STD_INPUT_HANDLE))
				CloseHandle(si1.hStdInput);

			if(si1.hStdError != GetStdHandle(STD_ERROR_HANDLE))
				CloseHandle(si1.hStdError);

			if(si1.hStdOutput != GetStdHandle(STD_OUTPUT_HANDLE))
				CloseHandle(si1.hStdOutput);

			if(pip)
			{
				CloseHandle(pi2.hProcess);
				CloseHandle(pi2.hThread);
				CloseHandle(rhandle);
				CloseHandle(whandle);
			}


    }
    return 0;
}