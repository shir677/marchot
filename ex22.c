#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>


int io_error(){  
    int fder = open("errors.txt", O_RDONLY);
    if (fder == -1) {
        write(1,"Erro in: open\n",15);
        return -1;
    }

    // Redirect stderr to the file descriptor
    if (dup2(fder, STDERR_FILENO) == -1) {
        write(1,"Erro in: dup2\n",15);
        return -1;
    }
    close(fder);
}

int io_output(char * input){
    int inputFile = open(input, O_RDONLY);
    if (inputFile == -1) {
        write(1,"Erro in: open\n",15);
        exit(EXIT_FAILURE);
    }

    if (dup2(inputFile, STDIN_FILENO) == -1) {
        write(1,"Erro in: dup2\n",15);
        return -1;
    }
    close(inputFile);

    int outputFile = open("output_results.txt",  O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outputFile == -1) {
        write(1,"Erro in: open\n",15);
        return -1;
    }
    // Set the file descriptors for stdin and stdout
    if (dup2(outputFile, STDOUT_FILENO) == -1) {
        write(1,"Erro in: dup2\n",15);
        return -1;
    }
    close(outputFile);

}

int remove_file(){
    if (remove("output_results.txt") == 0) {
        return 1;
    } 
    return 0;
}

//EXIT AFTER 5 SEC
void alarm_handler(int signum) {
    exit(0);
}

//ECEC USER COMAMAND
int execute_command(char **args,char * inputfile) {  
    pid_t pid = fork();
    if (pid == -1) {
        write(1,"Erro in: fork\n",15);
        return -1;
    } else if (pid == 0) {
        io_error();
        // Open the input and output files
        if (inputfile!=NULL){
        int x = io_output(inputfile);
        }
        alarm(5);
        execvp(args[0], args);
        write(1,"Erro in: execvp\n",17); 
        exit(-1);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            return WTERMSIG(status);
        }
    }
}


//COMPILE THE .C FILE
int compile_and_run(const char* cfile, const char* inputfile, const char* outputfile) {
    char execfile[150];
    snprintf(execfile, 150, "%.*s", (int)(strlen(cfile) - 2), cfile); // Remove the ".c" extension
    // Compile the C file
    char command[150 + 20];

    const char* gcc_args[] = {"gcc", "-w", "-o", execfile, cfile, NULL};

    if (execute_command(gcc_args,NULL) != 0) {
        return -1;
    }

    const char *exec_args[] = {execfile , NULL};
    int n = execute_command(exec_args,inputfile);
    if (n == 14) {
        remove_file();
        return 14;
    }

    const char *ex21_args[] = {"./comp.out", outputfile, "output_results.txt",NULL};
    int res = execute_command(ex21_args,NULL);
    remove_file();
    return res;
}

//RETURN STR WITH THE STUDENT GRADE
char * add_result(char* str1,char* str2){
    char* result = malloc(strlen(str1) + strlen(str2) + 1);
    strcpy(result, str1);
    strcat(result, str2);
    return result;
}


//MAINLY SCRIPT SERCHING FOR ECHE STUDENT THE C FILE AND SEND TO RUN THEN WRITE RESULTS
void search_for_c_files(const char* folder, const char* inputfile, const char* outputfile) {
    DIR* dirp = opendir(folder);
    if (dirp == NULL) {
        write(1,"Erro in: opendir\n",18);
        return;
    }
    const char* extension = ".c";
    struct dirent* dp;
    while ((dp = readdir(dirp)) != NULL) {
        if (dp->d_type == DT_DIR && strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            int cfile =0;
            char name[150];
            char subfolder[150];
            snprintf(name, 150, "%s", dp->d_name);
            snprintf(subfolder, 150, "%s/%s", folder, dp->d_name);
            DIR* subdirp = opendir(subfolder);
            if (subdirp == NULL) {
                write(1,"Erro in: opendir\n",18);
                continue;
            }

            // Look for a C file in the subfolder
            const char* message;
            struct dirent* subdp;
            int res;
            while ((subdp = readdir(subdirp)) != NULL&&cfile==0) {
                int len = strlen(subdp->d_name);

                char cfilename[150];
                snprintf(cfilename, 150, "%s/%s", subfolder, subdp->d_name);
                struct stat file_info;

                // Use stat to retrieve file information
                int result = stat(cfilename, &file_info);

                int flag = S_ISDIR(file_info.st_mode);
                 if (len >= 2 && strcmp(subdp->d_name + len - 2, extension) == 0 && flag==0 ) {
                    cfile=1;
                    res = compile_and_run(cfilename, inputfile, outputfile);
                 }
            }
            int fd = open( "results.csv", O_WRONLY | O_APPEND);
            if (fd == -1) {
                write(1,"Erro in: open\n",15);
                return 1;
            }
            if (cfile==0){
                // Write data to the file
                message = add_result(name,",0,NO_C_FILE\n");
                write(fd, message, strlen(message));
            }
            else {
                if(res==-1){
                    // Write data to the file
                    message = add_result(name,",10,COMPILATION_ERROR\n");
                    write(fd, message, strlen(message));
                }
                else if(res==1){
                    // Write data to the file
                    message = add_result(name,",100,EXCELLENT\n");
                    write(fd, message, strlen(message));
                }
                else if (res==2){
                    // Write data to the file
                    message = add_result(name,",50,WRONG\n");
                    write(fd, message, strlen(message));
                }
                else if (res==3){
                    // Write data to the file
                    message =  add_result(name,",75,SIMILAR\n");
                    write(fd, message, strlen(message));
                }
                else if (res==14){
                    // Write data to the file
                    message =  add_result(name,",20,TIMEOUT\n");
                    write(fd, message, strlen(message));
                }
            }
            // Close the file descriptor
            close(fd);
            closedir(subdirp);
        }
    }
    closedir(dirp);
}

//CREATE RESULT FILE
int results() {
    const char* filename = "results.csv";
    int fd = open(filename, O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {
        write(1,"Erro in: open",15) ;
        return 1;
    }
    // Close the file descriptor
    close(fd);
    return 0;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        write(1,"Erro in: argc\n",15);
        return 1;
    }

    int fder = open("errors.txt", O_CREAT | O_RDONLY , 0644);
    if (fder == -1) {
        write(1,"Erro in: open\n",15);
        return -1;
    }
    // Close the original file descriptor
    close(fder);

    
    char folder[151];
    char inputfile[151];
    char outputfile[151];

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0 ) {
        return -1; // Error in opening file
    }

    // Read the file to array
    int i=0,j=0,w=0,c = 0;
    char buf;
    while (read(fd, &buf, 1) > 0) {
        //if (buf == '\r'){
          //  c+=1;
        //}
        if(buf == '\n'){
             c+=1;
        }
        else if (c==0){
            folder[i++]=buf;
        }
        else if(c==1){
            inputfile[j++]=buf;
        }
        else if (c==2){
            outputfile[w++]=buf;
        }
    }
    close(fd);
    folder[i]='\0';
    inputfile[j]='\0';
    outputfile[w]='\0';

    // Open the files for reading
    int fd1 = open(folder, O_RDONLY);
    int fd2 = open(inputfile, O_RDONLY);
    int fd3 = open(outputfile, O_RDONLY);

   struct stat statbuf;
   if (lstat(folder, &statbuf) != 0){
        write(1,"Erro in: lstat\n",16);
       return -1;
   }

   if (S_ISDIR(statbuf.st_mode) == 0){
        write(1,"Not a valid directory\n",23);
        return -1;
   }

    if (fd2 < 0 ){
        write(1,"Input file not exist\n",22);
         return -1; // Error in opening file
    } 
    if( fd3 < 0) {
        write(1,"Output file not exist\n",23);
        return -1; // Error in opening file
    }
    results();
    search_for_c_files(folder,inputfile,outputfile);

}