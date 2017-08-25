#ifndef JPTUTILS_H
#define JPTUTILS_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#define THREAD_NAME_LEN 32
#define MAX_LINE 256 
#define PROC_NAME_LEN 64  
#define THREAD_INIT 2
struct cpu_info {  
    long unsigned utime, ntime, stime, itime;  
    long unsigned iowtime, irqtime, sirqtime;  
}; 
struct proc_info {  
    struct proc_info *next;  
    pid_t pid;  
    pid_t tid;  
    uid_t uid;  
    gid_t gid;  
    char name[PROC_NAME_LEN];  
    char tname[THREAD_NAME_LEN];  
    char state;  
    long unsigned utime;  
    long unsigned stime;  
    long unsigned delta_utime;  
    long unsigned delta_stime;  
    long unsigned delta_time;  
    long vss;  
    long rss;  
    int num_threads;  
    char policy[32];  
};  

struct proc_info* new_proc_info,*old_proc_info;
struct cpu_info new_cpu,old_cpu;
struct proc_info** procs, **old_procs;
int procs_num = 0;
int old_procs_num = 0;
int procs_size= THREAD_INIT;

int readStat(char* filename);
void readProc();
int readTask(int pid);
struct proc_info* find_old_proc(pid_t pid, pid_t tid);
int numcmp(long unsigned a , long unsigned b);
int comparDeltatime(const void * a , const void * b);
#endif
