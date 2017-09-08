#include "JptUtils.h"
static void usage(char *cmd) {  
    fprintf(stderr, "Usage: %s [ -n iterations ] [ -d delay ]\n"  
                    "    -n num  Updates to show before exiting.\n"  
                    "    -d num  Seconds to wait between updates.\n"  ,  
        cmd);  
} 
int main(int argc, char *argv[])
{
	struct sigaction s_sigdeal={0};
	char* pfileName;
	const char* delim = ",";
	int pipestatus;
	pid_t pipepid;
	int pipefd[2];
	FILE *presultfile;
	FILE *plogfile;
	char result_filename[32];
	char result_filename_dot[32];
	char topFileNames[8];
	char* topFileNames_dot;
	char* topFileNames_dot_dup;
	int topFileNames_dot_size;
	char result_filebuf[64];
	int status;
	int ret;
	int delay = 3;
	int iterations = 2;
	int i;
	int j;
	char filename[32];
	struct proc_info *old_proc;
	long unsigned utime, total_delta_time;
	char cmd[128];
	int cmdPosition = 0;
	procs_num = 0;
	procs_size = THREAD_INIT;
	
	///////////处理信号//////////
	s_sigdeal.sa_sigaction = h_sa_sigaction;
	s_sigdeal.sa_flags = SA_SIGINFO;
	sigemptyset(&s_sigdeal.sa_mask);
	sigaddset(&s_sigdeal.sa_mask,SIGUSR1);
	sigaction(SIGINT, &s_sigdeal, NULL);
	/////////////////////////////
	for(i = 1; i < argc; i++)
	{
		if(!strcmp(argv[i],"-n"))
		{
			if (i + 1 >= argc) {  
          fprintf(stderr, "Option -n expects an argument.\n");  
          usage(argv[0]);  
          exit(EXIT_FAILURE);  
      }  
      iterations = atoi(argv[++i]); 
      cmdPosition = i; 
      //printf("cmdPosition=%d\n",cmdPosition);
      continue;  
		}  
    if (!strcmp(argv[i], "-d")) {  
        if (i + 1 >= argc) {  
            fprintf(stderr, "Option -d expects an argument.\n");  
            usage(argv[0]);  
            exit(EXIT_FAILURE);  
        }  
        delay = atoi(argv[++i]);  
        cmdPosition = i; 
      	//printf("cmdPosition=%d\n",cmdPosition);
        continue;  
    }  
	}
	//建立匿名管道存入占用最大的线程id
	ret = pipe(pipefd);
	if(ret == -1) { printf("pipe error:%m\n"),exit(EXIT_FAILURE); }
		
	printf("proc_info size:%d\n",sizeof(struct proc_info));
  //printf("cmdPosition=%d\n",cmdPosition+1);
  //printf("argv=%s\n",argv[cmdPosition+1]);
	sprintf(filename,"/proc/%s/stat",argv[cmdPosition+1]);
	readProc();
	int pid = atoi(argv[cmdPosition+1]);
	readTask(pid);
	
	j = 0;
	while(j < iterations)
	{
		j++;
		//保存上次cpu信息
		/*
		for(i = 0; i < old_procs_size; i++)
		{
			printf("free old procs:%d\n",old_procs_size);
			if(old_procs[i])
				free(old_procs[i]);
			printf("%0x\n",old_procs[i]);
		}
		printf("free old procs completed\n");
		*/
		//free(old_procs);
		old_procs = procs;
		old_procs_num = procs_num;
		old_procs_size = procs_size;
		procs_num = 0;
		procs_size = THREAD_INIT;
		memcpy(&old_cpu, &new_cpu, sizeof(struct cpu_info));
		sleep(delay);
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
		
		//释放上一次结果
		for(i = 0; i < old_procs_size; i++)
		{
			//printf("free old procs:%d\n",old_procs_size);
			if(old_procs[i])
				free(old_procs[i]);
			//printf("%0x\n",old_procs[i]);
		}
		//不释放数组引起了内存泄漏
		free(old_procs);
		//printf("free old procs completed\n");
		
    qsort(procs, procs_num, sizeof(struct proc_info *),
                  comparDeltatime);
		
		
		printf("pid\t tid\t tname\t delta_time\t state\t utime\t stime\t vss\t rss\t\n");
		for(i = 0; i < procs_num; i++)
		{
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
		}
		bzero(cmd,128);
		/**system 返回值较多，建议使用popen
		sprintf(cmd,"jstack %d | grep -A 20 0x%x > %d-%d.log"
			,procs[0]->pid
			,procs[0]->tid
			,procs[0]->tid
			,j);
			
	  printf("%s\n" , cmd);
		status = system(cmd);
		if(status < 0)
		{
			printf("cmd:%s\t error:%s\n",cmdString,strerror(errno));
			return 2;
		}
		if(WIFEXITED(status))
		{
			printf("normal termination, exit status=%d\n",WEXITSTATUS(status));
		}
		else if(WIFSIGNALED(status))
		{
			//如果命令被信号中断取得信号值
			printf("abnormal termination, signal number=%d\n",WTERMSIG(status));
		}
		else if(WIFSTOPPED(status))
		{
			//如果命令被信号暂停执行取得信号值
			printf("process stopped, signal number=%d\n",WSTOPSIG(status));
		}
		*/
		sprintf(cmd,"jstack %d | grep -A 20 0x%x"
			,procs[0]->pid
			,procs[0]->tid);
		printf("%s\n" , cmd);
		presultfile = popen(cmd, "r");
		if(presultfile == NULL)
		{
			printf("popen error :%m\n");
			exit(1);
		}
		bzero(result_filebuf,sizeof(result_filebuf));
		bzero(result_filename,sizeof(result_filename));
		bzero(result_filename_dot,sizeof(result_filename_dot));
		sprintf(result_filename,"%d-%d.log"
			,procs[0]->tid
			,j);
		
		strncpy(result_filename_dot,result_filename,strlen(result_filename));
		strcat(result_filename_dot,",");
		//建立匿名管道存入占用最大的线程id
		//printf("result_filename_dot:%s \n ",result_filename_dot);
		write(pipefd[1],result_filename_dot,strlen(result_filename_dot));
		//write(pipefd[1],result_filename,strlen(result_filename));
		
		//做成结果文件	
		plogfile = fopen(result_filename, "w");
		if(plogfile == NULL)
		{
			printf("fopen error :%m\n");
			exit(1);
		}
		while(fgets(result_filebuf, sizeof(result_filebuf), presultfile) != NULL)
		{
			//利用fwrite写行首会有大量空格出现
			//fwrite(result_filebuf, sizeof(char), sizeof(result_filebuf), plogfile);
			fputs(result_filebuf, plogfile);
			bzero(result_filebuf,sizeof(result_filebuf));
		}
		//关闭文件
		fclose(plogfile);
		
		//关闭管道
    pclose(presultfile);
	}
  for(i = 0; i < procs_size; i++)
	{
		//printf("%d free procs:%0x\n",procs_size,procs[i]);
		if(procs[i])
			free(procs[i]);
	}
	//不释放数组引起了内存泄漏
	free(procs);
		//printf("free procs complete:%d\n",procs_size);
	//free(procs);
	
	//在子进程中读出匿名管道中的数据
	
	pipepid = fork();
	if(pipepid < 0)
	{
		printf("fork error :%m\n");
		exit(1);
	} 
	else if(pipepid == 0)
	{
		//不关闭写管道，read会处于阻塞
		close(pipefd[1]);
		//子进程读出文件名信息
		bzero(topFileNames,sizeof(topFileNames));
		topFileNames_dot = sbrk(0);
		topFileNames_dot_size = 8;
		j = 0;
		while((ret = read(pipefd[0],topFileNames,sizeof(topFileNames)-1)) != 0)
		{
			/*
			printf("filename size:%d\n",ret);
			topFileNames[ret] = 0;
			printf("filename:%s\n",topFileNames);
			*/
			j++;
			topFileNames[ret] = 0;
			brk(topFileNames_dot+(ret*j));
			strcat(topFileNames_dot,topFileNames);
			//printf("%s %d %d\n ",topFileNames,j, ret);
			bzero(topFileNames,sizeof(topFileNames));
		}
		//printf("filename:%s\n",topFileNames_dot);
		
		//分割字符串
		pfileName = strtok(topFileNames_dot,delim);
		//printf("delim:%s\n",pfileName);
		
		//将log文件显示到控制台
		if(pfileName != NULL)
		{
			displayFileToConsole(pfileName);
		}
		
		while((pfileName=strtok(NULL,delim)))
		{
			//printf("filename:%s\n",pfileName);
			displayFileToConsole(pfileName);
		}
		brk(topFileNames_dot);
		//关闭读管道
		close(pipefd[0]);
		exit(EXIT_SUCCESS);
	}
	else
	{
		//主进程关闭写管道
		close(pipefd[1]);
		//等待子进程结束
		waitpid(pipepid, &pipestatus, 0);
		if(WIFEXITED(pipestatus))
		{
			printf("read pipe child proc return :%d\n",WEXITSTATUS(pipestatus));
		}
		
	}
	
	return 0;
}
