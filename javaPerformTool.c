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
int readStat(char* filename)
{
	new_proc_info = malloc(sizeof(struct proc_info));
	bzero(new_proc_info,sizeof(struct proc_info));
	char buf[MAX_LINE], *open_paren, *close_paren;
	FILE *file;
	file = fopen(filename, "r");
	if(file == NULL)
		printf("fopen error:%m\n"),exit(1);
	fgets(buf,MAX_LINE,file);
  fclose(file);
  
  //获取线程名称
  open_paren = strchr(buf, '(');
  close_paren = strchr(buf, ')');
  if (!open_paren || !close_paren) return 1;  
  
  *open_paren =  *close_paren = '\0';
  strncpy(new_proc_info->tname, open_paren+1, THREAD_NAME_LEN);
  new_proc_info->tname[THREAD_NAME_LEN-1] = 0;
  
  sscanf(close_paren + 1, " %c %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d "  
                 "%lu %lu %*d %*d %*d %*d %*d %*d %*d %lu %ld",  
                 &new_proc_info->state, &new_proc_info->utime, &new_proc_info->stime, &new_proc_info->vss, &new_proc_info->rss);  
                 
  //printf("tname:%s\n",new_proc_info->tname);
  //printf("close_paren:%s\n",close_paren + 1);
  //printf("%c \n",new_proc_info->state);
  //printf("%lu \n",new_proc_info->utime);
  //printf("%lu \n",new_proc_info->stime);
  //printf("%lu \n",new_proc_info->vss);
  //printf("%ld \n",new_proc_info->rss);
}

void readProc()
{
	FILE *file;
	file = fopen("/proc/stat", "r");
	if(file == NULL)
		printf("fopen error:%m\n"),exit(1);
	fscanf(file, "cpu  %lu %lu %lu %lu %lu %lu %lu", &new_cpu.utime, &new_cpu.ntime, &new_cpu.stime,  
            &new_cpu.itime, &new_cpu.iowtime, &new_cpu.irqtime, &new_cpu.sirqtime);
  fclose(file);
  //printf(" %lu %lu %lu %lu %lu %lu %lu\n",new_cpu.utime, new_cpu.ntime, new_cpu.stime,  
  //          new_cpu.itime, new_cpu.iowtime, new_cpu.irqtime, new_cpu.sirqtime);
}

int readTask(int pid)
{
	char dirName[128];
	char filename[128];
	DIR * dir_task;
	struct dirent * dir_sub_task;
	sprintf(dirName,"/proc/%d/task",pid);
	dir_task = opendir(dirName);
	
	procs = calloc(THREAD_INIT, sizeof(struct proc_info*));
	while(dir_sub_task = readdir(dir_task))
	{
		if(!isdigit(dir_sub_task->d_name[0]))
			continue;
		//printf("%s\n",dir_sub_task->d_name);
		bzero(filename,128);
		sprintf(filename,"/proc/%d/task/%s/stat",pid,dir_sub_task->d_name);
		readStat(filename);
		if(procs_num >= procs_size)
		{
			procs = realloc(procs, (THREAD_INIT + procs_size) * sizeof(struct proc_info*));
			procs_size = procs_size + 2;
		} 
		new_proc_info->pid = pid;
		new_proc_info->tid = atoi(dir_sub_task->d_name);
		procs[procs_num] = new_proc_info;
		procs_num++;
		
	}
	closedir(dir_task);
	
}

struct proc_info* find_old_proc(pid_t pid, pid_t tid)
{
	int i;
	for(i = 0; i < old_procs_num; i++)
	{
		if(old_procs[i] && (old_procs[i]->pid == pid) && (old_procs[i]->tid == tid))
		{
			return old_procs[i];
		}
	}
	return NULL;
}

int numcmp(long unsigned a , long unsigned b)
{
	if(a<b) return -1;
	if(a>b) return 1;
	return 0;
}

int comparDeltatime(const void * a , const void * b)
{
	struct proc_info* pa, *pb;
	pa = *((struct proc_info**)a);
	pb = *((struct proc_info**)b);
	if(!pa && !pb) return 0;
	if(!pa) return 1;
	if(!pb) return -1;
	return -numcmp(pa->delta_time,pb->delta_time);
	
}
              
int main(int argc, char *argv[])
{
	int ret;
	int delay = 3;
	int i;
	int j;
	char filename[32];
	struct proc_info *old_proc;
	long unsigned utime, total_delta_time;
	char cmd[128];
	//printf("argv0:%s\n",argv[1]);
	sprintf(filename,"/proc/%s/stat",argv[1]);
	//procs = calloc(THREAD_INIT, sizeof(struct proc_info*));
	//readStat(filename);
	readProc();
	int pid = atoi(argv[1]);
	readTask(pid);
	
	//old_proc_info = new_proc_info;
	//procs[procs_num] = new_proc_info;
	//procs_num++;
	j = 0;
	while(j < 10)
	{
		j++;
		//保存上次cpu信息
		//printf("oldpid\t tid\t tname\t delta_time\t state\t utime\t stime\t vss\t rss\t\n");
		for(i = 0; i < old_procs_num; i++)
		{
			/*
			printf("%d\t %d\t %s\t %lu \t %c\t %lu \t %lu \t %lu \t %ld \t\n"
				,procs[i]->pid
				,procs[i]->tid
				,procs[i]->tname
				,procs[i]->delta_time
				,procs[i]->state
				,procs[i]->utime
				,procs[i]->stime
				,procs[i]->vss
				,procs[i]->rss);
				*/
			free(old_procs[i]);
		}
		
		old_procs = procs;
		old_procs_num = procs_num;
		procs_num = 0;
		procs_size = 2;
		memcpy(&old_cpu, &new_cpu, sizeof(struct cpu_info));
		sleep(3);
		//readProc();
		readTask(pid);
		
		for(i = 0; i < procs_num; i++)
		{
			if(procs[i])
			{
				old_proc = find_old_proc(procs[i]->pid,procs[i]->tid);
				if(old_proc)
				{
					procs[i]->delta_utime = procs[i]->utime - old_proc->utime;
					procs[i]->delta_stime = procs[i]->stime - old_proc->stime;
				}
				else
				{
					procs[i]->delta_utime = 0;
					procs[i]->delta_stime = 0;
				}
				procs[i]->delta_time = procs[i]->delta_utime + procs[i]->delta_stime;
			} 
		}
		
		/*total_delta_time = (new_cpu.utime + new_cpu.ntime + new_cpu.stime + new_cpu.itime  
                        + new_cpu.iowtime + new_cpu.irqtime + new_cpu.sirqtime)  
                     - (old_cpu.utime + old_cpu.ntime + old_cpu.stime + old_cpu.itime  
                        + old_cpu.iowtime + old_cpu.irqtime + old_cpu.sirqtime);*/
    qsort(procs, procs_num, sizeof(struct proc_info *),
                  comparDeltatime);
		
		printf("pid\t tid\t tname\t delta_time\t state\t utime\t stime\t vss\t rss\t\n");
		for(i = 0; i < procs_num; i++)
		{
			//printf("procs_num:%d\n",procs_num);
			printf("%d\t %d\t %s\t %lu \t %c\t %lu \t %lu \t %lu \t %ld \t\n"
				,procs[i]->pid
				,procs[i]->tid
				,procs[i]->tname
				,procs[i]->delta_time
				,procs[i]->state
				,procs[i]->utime
				,procs[i]->stime
				,procs[i]->vss
				,procs[i]->rss);
			/*
			printf("tid:%d\n",procs[i]->tid);
			printf("tname:%s\n",procs[i]->tname);
		  printf("%c \n",procs[i]->state);
		  printf("%lu \n",procs[i]->utime);
		  printf("%lu \n",procs[i]->stime);
		  printf("%lu \n",procs[i]->vss);
		  printf("%ld \n",procs[i]->rss);
		  */
		}
		bzero(cmd,128);
		sprintf(cmd,"jstack %d | grep -A 20 0x%x > %d-%d.log"
			,procs[0]->pid
			,procs[0]->tid
			,procs[0]->tid
			,j);
			
	  printf("%s\n" , cmd);
		system(cmd);
	}
  for(i = 0; i < procs_num; i++)
	{
		free(procs[i]);
	}
  //free(old_proc_info);
	return 0;
}
