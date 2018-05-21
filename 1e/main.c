#include <stdio.h>
#include <linux/limits.h>
#include <unistd.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include "line_parser.h"
#include "job_control.h"

#define PATH_MAX 4096
#define FREE(X) if(X) free((void*)X)


//#include "line_parser.c"

int exit_error_free(char* msg, job* job_list){
    perror(msg);
    free_job_list(&job_list);
    exit(EXIT_FAILURE);
}
int exit_error(char* msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

void sig_handler(int sig){
    //0a- interrupted -> ignored message
    if (sig == SIGQUIT  || sig==SIGCHLD)
        // printf("The interrupted signal was ignored by- %s\n",strsignal(sig));
        //0c- stop signals ->ignored message for now
        /*if(sig==SIGTTIN ||sig==SIGTTOU || sig==SIGTSTP)
            printf("The stop signal was ignored by- %s\n",strsignal(sig));*/
        fflush(stdout);
    //exit(EXIT_SUCCESS);
}

void set_default_signals(){
    signal(SIGQUIT,SIG_DFL);
    signal(SIGCHLD,SIG_DFL);
    signal(SIGTTIN,SIG_DFL);
    signal(SIGTTOU,SIG_DFL);
    signal(SIGTSTP,SIG_DFL);
}
//single execute- task5 ->NOT USED in task 6
int single_execute(cmd_line *line){
    int inputStream,outputStream;
    //check the input and output fields before execute
    if(line->input_redirect!=NULL) {
        inputStream = open(line->input_redirect, O_RDONLY, 0);
        if(inputStream<0) {
            exit_error("inputStream error");
        }
        dup2(inputStream,0);
    }
    if(line->output_redirect!=NULL) {
        outputStream = open(line->output_redirect, O_TRUNC | O_CREAT | O_WRONLY, 0644);
        if(outputStream<0) {
            exit_error("outputStream error");
        }
        dup2(outputStream,1);
    }
    int output=execvp(line->arguments[0],line->arguments);
    //close streaming ->for each son
    if(line->output_redirect!=NULL)
        close( outputStream);
    if(line->input_redirect!=NULL)
        close(inputStream);

    // -1 ->error
    return output;
}
//double execute - task5 ->NOT USED in task 6
int pipeExecute(cmd_line *line){
    int fd[2],child1,child2;
    //fd[0]-> for using read end    fd[1]-> for using write end

    //#1
    if(pipe(fd)==-1){
        perror("pipe(fd)1 error");
        exit(EXIT_FAILURE);
    }
    //#2
    child1 = fork();
    //#3
    if (child1 == 0) {
        close(1);
        dup(fd[1]);
        close(fd[1]);
        if(single_execute(line)==-1){
            perror("execvp1 error");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    //#4
    close(fd[1]);
    //#5-6
    child2 = fork();
    if (child2 == 0) {
        close(0);
        dup(fd[0]);
        close(fd[0]);
        if(single_execute(line->next)==-1){
            perror("execvp2 error");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    //#7
    close(fd[0]);
    //#8
    waitpid(child1, 0, 0);
    waitpid(child2, 0, 0);
    exit(EXIT_SUCCESS);
}

int execute (cmd_line *line, int* left_pipe, int* right_pipe , int* first_child_pid, struct termios *main_termios,job *job_list,job *tmp_job){
    int fd[2],child1,result=0, status;
    pid_t child_pid = 0;

    child1=fork();
    //child part
    if(child1==0){
        set_default_signals();
        if(right_pipe && line->next){
            close(1);
            dup(right_pipe[1]);
            close(right_pipe[1]);
        }
        if(left_pipe){
            close(0);
            dup(left_pipe[0]);
            close(left_pipe[0]);
        }
        result=single_execute(line);
        waitpid(child_pid,&status,0);
        exit(EXIT_SUCCESS);
    }
    //parent part
    if(right_pipe)
        close(right_pipe[1]);
    if(left_pipe){
        //TODO- need to free the pipe
        close(left_pipe[0]);
    }
    //if there is a new command- create new pipe are Recursively call the same func
    if(line->next){
        if(pipe(fd)==-1)
            exit_error("pipe(fd)1 error");
        result=execute(line->next,right_pipe,fd,NULL,main_termios,job_list,tmp_job);
    }
    if (line->blocking == 1)
        run_job_in_foreground(&job_list,tmp_job,1,main_termios,getpgid(0));
    return result;
}
//purpose:   wrapper function will handle the initial call and waiting for the last child
int initial_execute(cmd_line *line, pid_t *child_pid, struct termios *main_termios,job *job_list,job *tmp_job){
    int *right_pipe=NULL;
    int fd[2];
    int output=0;
    if(line->next){
        if(pipe(fd)==-1)
            exit_error("pipe(fd)1 error");
        else
            right_pipe=fd;
    }
    output= execute(line,NULL,right_pipe,child_pid,main_termios,job_list,tmp_job);
    if (line->blocking == 1)
        run_job_in_foreground(&job_list,tmp_job,1,main_termios,getpgid(0));
    return output;
}

/*
int MultiPipeExecute(cmd_line *line, job* cur_job , job* job_list, int last_child_pid){
    int fd[2],child1,child2;
    //fd[0]-> for using read end    fd[1]-> for using write end

    //#1
    if(pipe(fd)==-1){
        exit_error("pipe(fd)1 error");
    }
    //#2
    child1 = fork();
    //if true -> the pid of all processes and the job.pgid equals to the pid of this process
    if(last_child_pid<0){
        last_child_pid=getpid();
        cur_job->pgid=last_child_pid;
    }
    //#3
    if (child1 == 0) {
        //child1 process
        set_default_signals();
        pid_t child_pid=getpid();
        //setpgid(child_pid,child_pid);
        setpgid(getpid(),last_child_pid);

        close(1);
        dup(fd[1]);
        close(fd[1]);
        if(single_execute(line)==-1){
            exit_error("execvp #1 error");
        }
        exit(EXIT_SUCCESS);
    }
    //#4
    close(fd[1]);
    //#5-6
    child2 = fork();
    if (child2 == 0) {
        set_default_signals();
        //TODO- not sure that this is what they want
        setpgid(getpid(),last_child_pid);
        //child2 process
        close(0);
        dup(fd[0]);
        close(fd[0]);
        //check if there is a recursive call to execute
        if(line->next->next!=0)
            MultiPipeExecute(line->next,cur_job,job_list,last_child_pid);
        if(single_execute(line->next)==-1)
            exit_error("execvp #2 error");
        exit(EXIT_SUCCESS);
    }
    //#7
    close(fd[0]);

    //#8
    waitpid(child1, &cur_job->status, WUNTRACED);
    waitpid(child2, &cur_job->status, WUNTRACED);
    exit(EXIT_SUCCESS);
}*/

int main(int argc, char *argv[]) {
    job *job_list = NULL;
    //buf=getcwd input (getting the address)
    char buf[PATH_MAX];
    //userStream = used in fgets input
    char userStream[2048];
    int done = 0;
    char *s;
    job *tmp_job;
    //int cpid;
    pid_t child_pid_t;
    //lab 6 additions
    //0a signals
    signal(SIGQUIT, sig_handler);
    signal(SIGCHLD, sig_handler);
    //0c signals
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    //set group id
    setpgid(getpid(), getpid());
    struct termios *main_termios = (struct termios *) (malloc(sizeof(struct termios)));
    tcgetattr(STDIN_FILENO, main_termios);
    //job *tmpJob;
    while (1) {
        //get the current path
        getcwd(buf, PATH_MAX);
        printf("%s", buf);

        //get the current command line
        s = fgets(userStream, 2048, stdin);

        //check \n case
        if (strcmp(s, "\n") == 0 || strcmp(s, " \n") == 0 || strcmp(s, "    \n") == 0) {
            strncpy(buf, buf + 1, strlen(buf) - 1);
            getcwd(userStream, 2048);
            printf("%s", userStream);
            fgets(buf, 2048, stdin);
            continue;
        }

        //case jobs->  print the job list
        if (strcmp(s, "jobs\n") == 0 || strcmp(s, "jobs") == 0) {
            print_jobs(&job_list);
            continue;
        }

        //case quit ->exit 0 after free
        if (strcmp(s, "quit") == 0 || strcmp(s, "quit\n") == 0) {
            if (job_list != NULL)
                free_job_list(&job_list);
            //exit(EXIT_SUCCESS);
            break;
        }
        else {
            cmd_line *line = parse_cmd_lines(s);
            //case of fg command - task 1d
            if (strcmp(line->arguments[0], "fg\n") == 0 || strcmp(line->arguments[0], "fg") == 0) {
                if(!(line->arguments[1] ||isdigit(line->arguments[1])))
                    exit_error("error in fg - need 2 arguments");
                tmp_job = find_job_by_index(job_list,atoi(line->arguments[1]));
                run_job_in_foreground(&job_list,tmp_job,1,main_termios,getpgid(0));
                /*if(tmp_job)
                    FREE(tmp_job);*/
            }else {
                if (strcmp(line->arguments[0], "bg\n") == 0 || strcmp(line->arguments[0], "bg") == 0) {
                    if (!(line->arguments[1] || isdigit(line->arguments[1])))
                        exit_error("error in bg - need 2 arguments");
                    run_job_in_background(job_list, atoi(line->arguments[1]));
                } else {
                    //else - go to parse the line and execute it
                    tmp_job = add_job(&job_list, s);
                    done = initial_execute(line, &child_pid_t,main_termios,job_list,tmp_job);

                }
            }

            free_cmd_lines(line);

        }
        switch (done){
            case EXIT_SUCCESS:
                break;
            case EXIT_FAILURE:
                exit_error("runtime error");
            default:
                break;
        }

    }
    if(job_list!=NULL)
        free_job_list(&job_list);
    return 0;
}

            /*
            //else - go to parse the line and execute it
            cmd_line *line = parse_cmd_lines(s);
            tmpJob=add_job(&job_list, s);
            //add the line to the job_line list
            cpid = fork();
            if (cpid == -1) {
                exit_error("cpid -fork() error");
            }
            if (cpid == 0) {
                //TODO - need to transfer the single execute into the multiExecute
                //sent to execute in execute func, if -1 ->error
                if (line->next == 0){

                    done = single_execute(line);
                }
                    //else ->there is a next -> need a pipe module
                else {
                    line=parse_cmd_lines(tmpJob->cmd);
                    //wrapper function will handle the initial call and waiting for the last child
                    done = initial_execute(line, child_pid_t);
                }
                if (done == -1)
                    exit_error("illegal execute line");
                exit(0);
            }
            //check if there is a need to wait for son, if 0->don't wait
            if (line->blocking == 1){
                while(waitpid(tmpJob->pgid, &tmpJob->status, WUNTRACED)!= -1){};
            }
            //release the curr cmd_line from memory
            free_cmd_lines(line);
        }

    }
    if(job_list!=NULL)
        free_job_list(&job_list);
    return 0;


}
             */
