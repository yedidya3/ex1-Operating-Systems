// Yedidya Bachar 316294784
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include <sys/mman.h>




#define SIZE 100

void history(char commands[SIZE][SIZE] , int numberCommand , int j, int isRunning[SIZE]);
void jobs(char commands[SIZE][SIZE] , int numberCommand , int j , int isRunning[SIZE]);
void doCommand(char* args[SIZE] , int numberCommand/* , int j*/ , int isRunning[SIZE]);
void doCommand2(char* args[SIZE] , int numberCommand/* , int j*/ , int isRunning[SIZE]);
int doCd(char* detail[], int size , char* cwdToChange);

int cd(char* args , char* pathOrginal , char* pathHome , char listOfPath[SIZE][SIZE] ,int* numberInListOfPath);
void sleepy(unsigned long nsec) {
    struct timespec delay = { nsec / 1000000000, nsec % 1000000000 };
    pselect(0, NULL, NULL, NULL, &delay, NULL);
}
void kill_child(int sig);

//int isRunning[SIZE] = {0};


int main(){
    
   /* int* glob;
    glob = mmap(NULL, sizeof *glob, PROT_READ | PROT_WRITE, 
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);*/

    int* isRunning =  mmap(NULL, sizeof(int)*100, PROT_READ | PROT_WRITE, 
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);


    
    
    char cwd[SIZE];
    
    
   if (getcwd(cwd, sizeof(cwd)) == NULL) {
       printf("An error occurred\n");
       return -1;
   } 


    char pathHome[100] = "/";
   //char* pathHome2 = strtok(cwd, "/");
   char* pathHome2 =  getenv("HOME");
   
   strcat(pathHome , pathHome2);
    
  
    char listOfPath[SIZE][SIZE];
    int numberOfPath = 0;
    
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
       printf("An error occurred\n");
       return -1;
   } 
    
    strcpy(listOfPath[numberOfPath] , cwd);
    //printf("%s" , listOfPath[numberOfPath]);


    char str[SIZE][SIZE];
    char* args[SIZE];
    char temp2[SIZE];
    char temp;
    char* amprasent= "&";
    char* amprasent2;
    int i;
    for (i = 0; i < SIZE; i++)
    {   
    
         isRunning[i]=0;
        //scanf("%c",&temp);
        printf("$ ");
        fflush(stdout);
        
       /* scanf("%[^\n]" , str[i]);
        strcpy(temp2 , str[i]);*/

        fgets(str[i] , SIZE ,stdin);
       
        str[i][strlen(str[i])-1] = '\0';
        strcpy(temp2 , str[i]);
        //printf("%s\n" ,str[i] );
        
        
        int j = 0;
        args[j] = strtok(temp2, " ");
        //printf("%s\n", args[j]);
        while (args[j] != NULL) {
           //printf("%s", args[j]);
            j++;
            args[j] = strtok(NULL, " ");
        }
        //printf("%s", args[j-1]);

       if(j>0){
           if(strcmp( "echo" , args[0])==0)
           {            
               int indexArgs;            
                for ( indexArgs = 1; indexArgs < j; indexArgs++ ) 
                {
                    char dest[strlen(args[indexArgs])+1];
                    int k =0;
                    int indexChar;
                    for(indexChar = 0 ;indexChar<strlen(args[indexArgs]); indexChar++){
                        
                        int found = 0;

                        if ( args[indexArgs][indexChar] == '\"' || args[indexArgs][indexChar] == '\'' ){
                            found = 1;
                        }
                        if (found == 0)
                        {
                            dest[k++] = args[indexArgs][indexChar];	    
                        }
                    }
                    dest[k]='\0';
                    strcpy(args[indexArgs],dest);
                }
            }





            isRunning[i] = 0;
            if (strcmp( "&" , args[j-1])==0)
            {
            
                args[j-1]=0;
                str[i][strlen(str[i])-1] = '\0';
                str[i][strlen(str[i])-1] = '\0';

                int pidFather;
                    
                    if((pidFather = fork() )== 0){
                        
                    // *glob  = 1;
                        //isRunning[i]=1;
                    doCommand(args , i,isRunning);
                        exit(EXIT_SUCCESS);

                    }
                
                
            }
            else
            {
                if(strcmp( "jobs" , args[0])==0){
                    jobs(str , i , j , isRunning);
                }
                else if(strcmp( "history" , args[0])==0){
                    history(str , i , j , isRunning);
                    
                }
                else if(strcmp( "cd" , args[0])==0){
                    if(j>2){
                        printf("Too many arguments\n");
                    }
                    else if(j==1){
                        
                        chdir(pathHome);
                        numberOfPath++;
                        strcpy(listOfPath[numberOfPath] ,pathHome );                        
                    }
                    else{
                        int* numberOfPathP = &numberOfPath;
                    cd(args[1],listOfPath[numberOfPath] ,pathHome, listOfPath ,numberOfPathP );
                  
                    }
                      isRunning[i] = 1; 
                    
                }
                else if(strcmp( "exit" , args[0])==0){    
                    return 0;
                }
                else{
                    

                    doCommand2(args , i,isRunning);
                
                }
       
                    
            }
        }
        else{
           i--;
        }
    }
    
    return 0;
}


void history(char commands[SIZE][SIZE] , int numberCommand , int j , int isRunning[SIZE]){
    if(j>1){
       return;
    }
    int k;
    for ( k = 0; k < numberCommand; k++)
    {
        char* isRun;
        switch(isRunning[k]){
            case 0:
                isRun = "RUNNING";
                break;
            case 1:
                isRun = "DONE";
                break;
        }
        printf("%s %s\n" , commands[k] , isRun);
        
    }
    printf("history RUNNING\n");
    isRunning[numberCommand] = 1;
}

void jobs(char commands[SIZE][SIZE] , int numberCommand , int j , int isRunning[SIZE]){
    if(j>1){
       return;
    }
    int k;
    for ( k = 0; k < numberCommand; k++)
    {
        
        switch(isRunning[k]){
            case 0:
                
                printf("%s\n" , commands[k]);
                break;
        }     
    }
    isRunning[numberCommand] = 1;
}

void doCommand(char* args[SIZE] , int numberCommand, int isRunning[SIZE]){
    
    int stat;
    pid_t pid;
    if((pid = fork())==0){
        int ret_code =execvp(*(args), args);


         
		if (ret_code == -1) 
		{
			perror("exec failed ");
			exit(-1);
		}

        
        
    }
    else if(fork>0){
        pid = wait(&stat);
        //isRunning[numberCommand] = 1;
        kill_child(pid);
        isRunning[numberCommand] = 1;
        exit(EXIT_SUCCESS);
        //kill(getpid(), SIGTERM);
    }
    else{
        printf("fork failed");
    }
    
    
}

void doCommand2(char* args[SIZE] , int numberCommand, int isRunning[SIZE]){
    
    int stat;
    pid_t pid;
    if((pid = fork())==0){

        int ret_code =execvp(*(args), args);

		if (ret_code == -1) 
		{
			perror("exec failed ");
			exit(-1);
		}
        
        
    }
    else if (pid>0){
        pid = wait(&stat);
        //isRunning[numberCommand] = 1;
        //kill_child(pid);
        isRunning[numberCommand] = 1;        
    }
    else{
        printf("fork failed");
    }
    
}


int cd(char* args , char* pathOrginal , char* pathHome , char listOfPath[SIZE][SIZE] ,int* numberInListOfPath){
    
     int numberInListOfPathTemp = *numberInListOfPath;
    
    
    if(strcmp(args, "/") == 0){
         if(chdir("/") != 0){
                printf("chdir failed\n");
                chdir(pathOrginal);
                return -1;
            }
        (*numberInListOfPath)= numberInListOfPathTemp;
        (*numberInListOfPath)++;
        getcwd(listOfPath[*numberInListOfPath], sizeof(listOfPath[*numberInListOfPath]));
        if(strcmp(listOfPath[*numberInListOfPath] ,listOfPath[(*numberInListOfPath)-1])==0){
            (*numberInListOfPath)--;
        }
        
          return 0;
    }



   
    
    int j = 0;
    char* pathPlace[SIZE];
        pathPlace[j] = strtok(args, "/");
        //printf("%s\n", args[j]);
        while (pathPlace[j] != NULL) {
           //printf("%s", args[j]);
            j++;
            pathPlace[j] = strtok(NULL, "/");
        }
  
    int i;
    for ( i = 0; i < j; i++){
        // return to the folder that contains
        if (strcmp(pathPlace[i], "..") == 0) 
        {
            if(chdir("..") != 0){
                printf("chdir failed\n");
                chdir(pathOrginal);
                return -1;
            }
        } 
        //return to the previous folder
        else if (strcmp(pathPlace[i], "-") == 0)
        {   
            numberInListOfPathTemp--;
            
            if(numberInListOfPathTemp<0){
                numberInListOfPathTemp++;
                continue;
            }
            
            if(chdir(listOfPath[numberInListOfPathTemp]) != 0){
                numberInListOfPathTemp++;
                printf("chdir failed\n");
                chdir(pathOrginal);
                return -1;
            }
        }
        //Returns to the home library
        else if (strcmp(pathPlace[i], "~") == 0)
        {
            if(chdir(pathHome) != 0){
                printf("chdir failed\n");
                chdir(pathOrginal);
                return -1;
            }
        }
        //name of folder
        else 
        {
            if(chdir(pathPlace[i]) != 0){
               // printf("%s\n" , pathPlace[i]);
                printf("chdir failed\n");
                chdir(pathOrginal);
                return -1;
            }
        }
        
    }

    (*numberInListOfPath)= numberInListOfPathTemp;
    (*numberInListOfPath)++;
    getcwd(listOfPath[*numberInListOfPath], sizeof(listOfPath[*numberInListOfPath]));
    if(strcmp(listOfPath[*numberInListOfPath] ,listOfPath[(*numberInListOfPath)-1])==0){
        (*numberInListOfPath)--;
    }
    return 0;
}

void kill_child(int sig)
{
    kill(sig,SIGKILL);
}
