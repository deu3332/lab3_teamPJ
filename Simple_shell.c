#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


int getargs(char *cmd, char **argv) {
    int narg = 0;
    while (*cmd) {
        if (*cmd == ' ' || *cmd == '\t')
            *cmd++ = '\0';
        else {
            argv[narg++] = cmd++;
            while (*cmd != '\0' && *cmd != ' '&& *cmd != '\t')
                cmd++;
        }
    }
    argv[narg] = NULL;
    return narg;
}
int checkTaskOption(char *argv){  //실행 인자 확인 ( -1 = &, 0 = 없음,1 = pipe, 2 = <, 3 = > )
    int opt = 0;
    if(argv == NULL){
        return opt;
    }
    for(int i=0; argv[i] != NULL; i++){
        if (argv[i] == '&'){
            opt = -1;
            return opt;
        }
        if (argv[i] == '|'){
            opt = 1;
            return opt;
        }
        if (argv[i] == '<'){
            opt = 2;
            return opt;
        }
        if (argv[i] == '>'){
            opt = 3;
            return opt;
        } }
    return opt;
}

void mj_ctrlz(int sig, int flag){
    signal(sig, SIG_IGN);
    printf(" 쉘 일시정지..\n");
    printf(" fg 명령으로 재개 가능..\n");
    raise(SIGSTOP);
    printf(" 쉘 재개..\n");
    signal(sig, mj_ctrlz);
}

void mj_ctrlc(int sig){
    signal(sig, SIG_IGN);
    printf("  ctrl_C 입력 - 쉘 종료\n");
    exit(1);
}


void mj_ls(){
    DIR *pdir;
    struct dirent *pde;
    int i = 0;

    if ( (pdir = opendir(".")) < 0 ) {
        perror("opendir");
        exit(1);
    }
    while ((pde = readdir(pdir)) != NULL) {
        printf("%20s ", pde->d_name);
        if (++i % 3 == 0)
            printf("\n");
    }
    printf("\n");
    closedir(pdir);
}

void mj_pwd(){
    char buf[1024];
    getcwd(buf, 1024);
    printf("%s\n", buf);
}

void pwd_print(){
    char buf[1024];
    getcwd(buf, 1024);
    printf("%s", buf);
}

void mj_cd(char *path){
    if(chdir(path) < 0){
        perror("chdir");
        exit(1);
    }
    else{
        printf("Move to ");
        mj_pwd();
    }
}

void mj_ln(char *src, char *target){
    if(link(src, target) <0){
        perror("link");
    }
}

void mj_mkdir(char *path){
    if(mkdir(path, 0777) < 0){
        perror("rmdir");
    }
}

void mj_rmdir(char *dir){
    if(rmdir(dir) < 0){
        perror("rmdir");
    }
}


void mj_cp(char *src, char *target){
    int src_fd; /* source file descriptor */
    int dst_fd; /* destination file descriptor */
    char buf[10];
    ssize_t rcnt; 
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; 
    if ((src_fd = open(src, O_RDONLY)) == -1 ){
        perror("src open");
        exit(1);
    }
    if ( (dst_fd = creat(target, mode)) == -1 ){
        perror("dst open"); 
        exit(1);
    }
    while ( (rcnt = read(src_fd, buf, 10)) > 0){
        write(dst_fd, buf, rcnt);
    }
    if (rcnt < 0) {
        perror("read");
        exit(1);
    }
    close(src_fd);
    close(dst_fd);
}

void mj_mv(char *file, char *path){
    mj_cp(file, path);
    mj_rm(file);
}

void mj_rm(char *target){
    remove(target);
}



void mj_cat(char *target){
    char buffer[512];
    int filedes;
    /* target 을 읽기 전용으로 */
    if ( (filedes = open (target, O_RDONLY) ) == -1)
    {
        printf("error in opening anotherfile\n");
        exit (1);
    }
    /* EOF 까지 반복. EOF 는 복귀값 0 에 의해 표시 */
    while (read (filedes, buffer, 512) > 0){
        printf("%s", buffer);
    }
}

void your_cat(int target){
    char buffer[512];
    while (read (target, buffer, 512) > 0){
        printf("%s", buffer);
    }
}

void selectCmd(int i, char **argv){
    
    if(!strcmp(argv[i], "cat")){
        if(argv[i+1] == NULL){
            fprintf(stderr, "A few argument..!\n");
        }
        int arg = argv[i + 1][0] - 48;
        if( arg == 3 ) {
            your_cat(arg);
        }
        else{
            mj_cat(argv[i+1]);
        }
    }
    else if(!strcmp(argv[i], "ls")){
        mj_ls();
    }
    else if(!strcmp(argv[i], "pwd")){
        mj_pwd();
    }
    else if(!strcmp(argv[i], "mkdir")){
        if(argv[i+1] == NULL){
            fprintf(stderr, "A few argument..!\n");
        }
        else{
            mj_mkdir(argv[i+1]);
        }
    }
    else if(!strcmp(argv[i], "rmdir")){
        if(argv[i+1] == NULL){
            fprintf(stderr, "A few argument..!\n");
        }
        else{
            mj_rmdir(argv[i+1]);
        }
    }
    else if(!strcmp(argv[i], "ln")){
        if(argv[i+1] == NULL || argv[i+2] == NULL){
            fprintf(stderr, "A few argument..!\n");
        }
        else{
            mj_ln(argv[i+1], argv[i+2]);
        }
    }
    else if(!strcmp(argv[i], "cp")){
        if(argv[i+1] == NULL || argv[i+2] == NULL){
            fprintf(stderr, "A few argument..!\n");
        }
        else{
            mj_cp(argv[i+1], argv[i+2]);
        }
    }
    else if(!strcmp(argv[i], "rm")){
        if(argv[i+1] == NULL){
            fprintf(stderr, "A few argument..!\n");
        }
        else{
            mj_rm(argv[i+1]);
        }
    }
    else if(!strcmp(argv[i], "mv")){
        if(argv[i+1] == NULL || argv[i+2] == NULL){
            fprintf(stderr, "A few argument..!\n");
        }
        else{
            mj_mv(argv[i+1], argv[i+2]);
        }
    }
    else{}
}

void run(int i, int t_opt, char **argv){
    pid_t pid;
    int fd; /* file descriptor */
    char *buf[1024];
    int flags = O_RDWR | O_CREAT;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; 
    memset(buf, 0, 1024);
    pid = fork();
    if (pid == 0){  //child
        //-1 = &, 1 = pipe, 2 = <, 3 = >
        if(t_opt == -1){
            printf("%s가 백그라운드로 실행...\n", argv[i]);
            selectCmd(i, argv);
            exit(0);
        }
        else if(t_opt == 0){
            selectCmd(i, argv);
            exit(0);
        }
        else if(t_opt == 2){
            if ((fd = open(argv[i + 2], flags, mode)) == -1) {
                perror("open");
                exit(1);
            }
            if (dup2(fd, STDIN_FILENO) == -1) {
                perror("dup2");  
                exit(1);
            }
            if (close(fd) == -1) {
                perror("close");  
                exit(1);
            }
            mj_cat(argv[i+2]);
            selectCmd(i, argv);
            exit(0);
        }
        else if(t_opt == 3){
            if ((fd = open(argv[i+2], flags, mode)) == -1) {
                perror("open");  
                exit(1);
            }
            if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("dup2"); 
                exit(1);
            }
            if (close(fd) == -1) {
                perror("close");  
                exit(1);
            }
            selectCmd(i, argv);
            exit(0);
        }
    }
    else if (pid > 0){  //parent - 백그라운드 아닐 때만 기다림
        if(t_opt >= 0){ //백그라운드가 아닐 때
            wait(pid);
	    }
        if(!strcmp(argv[i], "cd")){
            if(argv[i+1] == NULL){
                fprintf(stderr, "A few argument..!\n");
            }
            else{
                mj_cd(argv[i+1]);
            }
        }
    }
    else{
        perror("fork failed");
    }
}

void run_pipe(int i, char **argv){
    char buf[1024];
    int p[2];
    int pid;

    /* open pipe */
    if (pipe(p) == -1) {
        perror ("pipe call failed");
        exit(1);
    }

    pid = fork();
    if (pid == 0) { /* child process */
        close(p[0]);
        if (dup2(p[1], STDOUT_FILENO) == -1) {
            perror("dup2"); 
            exit(1);
        }
        close(p[1]);
        selectCmd(i, argv);
        exit(0);
    }
    else if (pid > 0) {
        wait(pid);
        char *arg[1024];
        close(p[1]);
        sprintf(buf, "%d", p[0]);
        arg[0] = argv[i + 2];
        arg[1] = buf;
        selectCmd(0, arg);
    }
    else
        perror ("fork failed");
}

void main() {
    char buf[256];
    char *argv[50];
    int narg;

    struct sigaction ctrlz_act;
    struct sigaction ctrlc_act;
    
    ctrlz_act.sa_handler = mj_ctrlz;
    ctrlc_act.sa_handler = mj_ctrlc;
    
    sigaction(SIGTSTP, &ctrlz_act, NULL);
    sigaction(SIGINT, &ctrlc_act, NULL);
    

    printf("**************************\nWelcome to Simple_shell !!\n**************************\n");

    while (1) {
        pwd_print();
        printf(" : shell> ");
        gets(buf);
        narg = getargs(buf, argv);  //받은 인자 수
        int t_opt = 0;  //task 옵션

        for (int i = 0; i < narg; i++) {

            if(!strcmp(argv[i], "exit")){
                printf("**************************\nClose Shell Good bye (__)..\n*************************\n");
                exit(1);
            }
            int t_opt = checkTaskOption(argv[i + 1]);    //-1 = &, 1 = pipe, 2 = <, 3 = >
            if(t_opt == 1){
                run_pipe(i, argv);
                i += 2;
            }
            else{
                run(i, t_opt, argv);
            }
            if(t_opt > 1){  
                i += 2;
            }
        }   //for 루프 끝

    }   //while 루프 끝
}
