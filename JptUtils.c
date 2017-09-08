#include "JptUtils.h"

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
}

int readTask(int pid)
{
	int i;
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
		bzero(filename,128);
		sprintf(filename,"/proc/%d/task/%s/stat",pid,dir_sub_task->d_name);
		readStat(filename);
		if(procs_num >= procs_size)
		{
			procs = realloc(procs, (THREAD_INIT + procs_size) * sizeof(struct proc_info*));
			//初始化刚申请的空间
			for(i = procs_size; i < (THREAD_INIT + procs_size); i++)
			{
				procs[i] = NULL;
			}
			procs_size = procs_size + THREAD_INIT;
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

void displayFileToConsole(const char* pfileName)
{
	/////文件输出到控制台///
	struct stat consFStat;
	int consFd;
	void* consStart;
	///////////////////////
	//打开文件
	consFd = open(pfileName, O_RDONLY);
	if(consFd == -1)
	{
		printf("open error :%m\n");
		exit(EXIT_FAILURE);
	} 
	//获得文件长度
	fstat(consFd, &consFStat);
	
	//映射文件
	consStart = mmap(NULL, consFStat.st_size, PROT_READ, MAP_PRIVATE,
              consFd, 0);
	
	printf("<<<<<<<<<<<<<<%s>>>>>>>>>>>>>>>>\n",pfileName);
	printf("%s\n",consStart);
	
	munmap(consStart, consFStat.st_size);
	close(consFd);
}

void h_sa_sigaction(int sigNo, siginfo_t *info, void *parm)
{
	printf("signal:%d\t pid:%d\t no entry! please wait...\n",sigNo,info->si_pid);
}
