#include "JptUtils.h"
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
	sprintf(filename,"/proc/%s/stat",argv[1]);
	readProc();
	int pid = atoi(argv[1]);
	readTask(pid);
	
	j = 0;
	while(j < 10)
	{
		j++;
		//保存上次cpu信息
		for(i = 0; i < old_procs_num; i++)
		{
			free(old_procs[i]);
		}
		
		old_procs = procs;
		old_procs_num = procs_num;
		procs_num = 0;
		procs_size = 2;
		memcpy(&old_cpu, &new_cpu, sizeof(struct cpu_info));
		sleep(3);
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
	return 0;
}
