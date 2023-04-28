// Project 3 AUbatch- A Batch Scheduling System 
// Goal: To use pthread library and design a batch scheduling system. 
// Used reference: fork_execv.c and process.c uploaded by Dr. Qin (also referred geeksforgeeks) 

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// defining constants 

#define CMD 999999999
#define BUFFER 20
#define INVALID 1
#define LINE 2 
#define MAXLINE 64
#define MAXARGS 7

//pthread

pthread_mutex_t queue_lock;
pthread_cond_t buffer_ne;
pthread_cond_t buffer_nf;

//intialization of functions 

int helpmenu(int n, char **a);
void showmenu(const char *name, const char *x[]);
void *commandLine();
void *executor();
int passer(char *cmd);
int list(int nargs, char **args);
int cmdrun(int nargs, char **args);
int fcfs(int nargs, char **args);
int sjf(int nargs, char **args);
int cmdpriority(int nargs, char **args);
int test(int nargs, char **args);
int quit(int nargs, char **args);

//declaring global variables

unsigned int buffer_head;
unsigned int buffer_tail;
unsigned int count;
unsigned int total = 0;
unsigned int totalj = 0;
unsigned int policy = 0;
float turnaroundtime;
float turnaroundtimef;
float cputime;
float cputimef;
float waitingtime;
float waitingtimef;

//flags

unsigned int oneprocess = 0;
unsigned int twoprocess = 1;
unsigned int run = 1;
unsigned int sch = 1;

//time of queue

unsigned int expectedwaitingtime = 0;

//job properties

struct job_def {
	char name[50];
	int position, burst, priority;
	time_t arrival;
};

//job buffer

struct job_def jobBuffer[BUFFER - 1];

//cmd table

static struct {
	        const char *name;
		int (*func)(int nargs, char **args);
} cmdtable[] = {
		{ "help\n",     helpmenu },
	    { "run",        cmdrun },
		{ "quit\n",     quit },
		{ "list\n",	list },
		{ "sjf\n",	sjf },
		{ "fcfs\n",	fcfs },
		{ "priority\n",	cmdpriority },
		{ "test",	test },
		{NULL, NULL}
};

// help command to show us how to run the program

int helpmenu(int n, char **a)
{
        (void)n;
        (void)a;
	char *help;
	printf("//////////////////////////////////////////////////////////////\n");
	printf("How to use 'run': run <job> <time> <priority>: submit a job named <job>,\n");
	printf("\t \t execution time is <time>,\n");
	printf("\t \t priority of jobs is <priority>.\n");
	printf("What 'list' displays, list: display the job status.\n");
	printf("[First Come First Serve] fcfs: change the scheduling policy to FCFS.\n");
	printf("[Shortest Job First] sjf: change the scheduling policy to SJF.\n");
	printf("[Priority scheduling] priority: change the scheduling policy to priority.\n");
	printf("How to use 'test': test <benchmark> <policy> <num_of_jobs> <priority_levels> <min_CPU_time> <max_CPU_time>\n");
    printf("help: Prints the same prompt again\n");
	printf("quit: Exit AUbatch Batch Scheduling System\n");
	printf("//////////////////////////////////////////////////////////////\n");
    return 0;
}

// run command is to submit a job 

int cmdrun(int nargs, char **args) {
	int i = 0;
	time_t rtime = time(NULL);
	if (nargs != 4) {
		printf("Wrong input! Type run <job> <time> <priority>");
		return INVALID;
	}
	char *name[50];
	sscanf(args[1], "%s", &jobBuffer[buffer_head].name);
	sprintf(name, "%s", jobBuffer[buffer_head].name);
	sscanf(args[3], "%u", &jobBuffer[buffer_head].priority);
	sscanf(args[2], "%u", &jobBuffer[buffer_head].burst);
	int burst = jobBuffer[buffer_head].burst;
	expectedwaitingtime = expectedwaitingtime + burst;
	jobBuffer[buffer_head].arrival = rtime;
	count = count + 1;
	buffer_head = buffer_head + 1;
	jobBuffer[buffer_head].position = buffer_head;
	pthread_cond_signal(&buffer_ne);
	char *policySet;
	if (policy == 0) {
		fcfs(NULL, NULL);
		policySet = "FCFS.";
	}
	else if (policy == 1) {
		sjf(NULL, NULL);
		policySet = "SJF.";
	}
	else if (policy == 2) {
		cmdpriority(NULL, NULL);
		policySet = "Priority.";
	}
	if (run) {
		printf("Job %s", name);
		printf(" was submitted.\n");
		printf("Total number of jobs in the queue: %u\n", count);
		printf("Expected waiting time: %u seconds\n", expectedwaitingtime);
		printf("Scheduling Policy: %s\n", policySet);
	}
	total++;
	totalj = total;
	return 0; /* if job run is success */
}

// fcfs command changes scheduling policy to first come first serve

int fcfs(int nargs, char **args) {
	if (policy == 0) {
		return 0;
	}
	policy = 0;
	struct job_def tempJob;
	int j;
	int minJ;
	int i;
	int k = buffer_tail;
	int l;
	for (l = 0; l < count; l++) {
		if (k == BUFFER) {
			k = 0;
		}
		j = k;
		minJ = k;
		int limit = count - l;
		long int minArrival = jobBuffer[j].arrival;
		for (i = l; i < limit; i++) {
			long int temp = jobBuffer[j].arrival;
			if (temp < minArrival) {
				minArrival = temp;
				minJ = j;
			}
			j++;
		}
		if (minJ != k) {
			strcpy(tempJob.name, jobBuffer[k].name);
			tempJob.burst = jobBuffer[k].burst;
			tempJob.priority = jobBuffer[k].priority;
			tempJob.arrival = jobBuffer[k].arrival;
			strcpy(jobBuffer[k].name, jobBuffer[minJ].name);
			jobBuffer[k].burst = jobBuffer[minJ].burst;
			jobBuffer[k].priority = jobBuffer[minJ].priority;
			jobBuffer[k].arrival = jobBuffer[minJ].arrival;
			strcpy(jobBuffer[minJ].name, tempJob.name);
			jobBuffer[minJ].burst = tempJob.burst;
			jobBuffer[minJ].priority = tempJob.priority;
			jobBuffer[minJ].arrival = tempJob.arrival;
		}
		k++;
	}
	if (sch) {
		printf("Scheduling policy is switched to FCFS. All the %u waiting jobs\nhave been rescheduled.", count);
	}

	return 0;
}

// sjf command changes the scheduling policy to shortest job first 

int sjf(int nargs, char **args) {
	if (policy == 1) {
		return 0;
	}
	policy = 1;
	struct job_def tempJob;
	int j;
	int minJ;
	int i;
	int k = buffer_tail;
	int l;
	for (l = 0; l < count; l++) {
		if (k == BUFFER) {
			k = 0;
		}
		j = k;
		minJ = k;
		int limit = count - l;
		int minBurst = jobBuffer[j].burst;
		for (i = l; i < limit; i++) {
			int temp = jobBuffer[j].burst;
			if (temp < minBurst) {
				minBurst = temp;
				minJ = j;
			}
			j++;
		}
		if (minJ != k) {
			strcpy(tempJob.name, jobBuffer[k].name);
			tempJob.burst = jobBuffer[k].burst;
			tempJob.priority = jobBuffer[k].priority;
			tempJob.arrival = jobBuffer[k].arrival;
			strcpy(jobBuffer[k].name, jobBuffer[minJ].name);
			jobBuffer[k].burst = jobBuffer[minJ].burst;
			jobBuffer[k].priority = jobBuffer[minJ].priority;
			jobBuffer[k].arrival = jobBuffer[minJ].arrival;
			strcpy(jobBuffer[minJ].name, tempJob.name);
			jobBuffer[minJ].burst = tempJob.burst;
			jobBuffer[minJ].priority = tempJob.priority;
			jobBuffer[minJ].arrival = tempJob.arrival;
		}
		k++;
	}
	if (sch) {
		printf("Scheduling policy is switched to SJF. All the %u waiting jobs\nhave been rescheduled.", count);
	}

	return 0;
}

// priority command changes scheduling policy to priority 

int cmdpriority(int nargs, char **args) {
	if (policy == 2) {
		return 0;
	}
	policy = 2;
	struct job_def tempJob;
	int j;
	int minJ;
	int i;
	int k = buffer_tail;
	int l;
	for (l = 0; l < count; l++) {
		if (k == BUFFER) {
			k = 0;
		}
		j = k;
		minJ = k;
		int limit = count - l;
		int minPriority = jobBuffer[j].priority;
		for (i = l; i < limit; i++) {
			int temp = jobBuffer[j].priority;
			if (temp < minPriority) {
				minPriority = temp;
				minJ = j;
			}
			j++;
		}
		if (minJ != k) {
			strcpy(tempJob.name, jobBuffer[k].name);
			tempJob.burst = jobBuffer[k].burst;
			tempJob.priority = jobBuffer[k].priority;
			tempJob.arrival = jobBuffer[k].arrival;
			strcpy(jobBuffer[k].name, jobBuffer[minJ].name);
			jobBuffer[k].burst = jobBuffer[minJ].burst;
			jobBuffer[k].priority = jobBuffer[minJ].priority;
			jobBuffer[k].arrival = jobBuffer[minJ].arrival;
			strcpy(jobBuffer[minJ].name, tempJob.name);
			jobBuffer[minJ].burst = tempJob.burst;
			jobBuffer[minJ].priority = tempJob.priority;
			jobBuffer[minJ].arrival = tempJob.arrival;
		}
		k++;
	}
	if (sch) {
		printf("Scheduling policy is switched to Priority. All the %u waiting jobs\nhave been rescheduled.", count);
	}
	return 0;
}

// list command: - lists the current running jobs and total jobs

int list(int nargs, char **args) {
	printf("\nTotal Number of jobs in the queue: ");
	printf("%u\n", count);
	printf("Scheduling Policy: ");
	if (policy == 0) {
		printf("FCFS.\n");
	}
	else if(policy == 1) {
		printf("SJF.\n");
	}
	else if(policy == 2) {
		printf("Priority.\n");
	}
	printf("Name	CPU_Time	Priority	Progress\n");
	int i = buffer_tail - 1;
	int j;
	struct tm *ptm;
	int run = 1;
	int jobs = count + 1;
	for (j = 0; j < jobs; j++) {
		if (i == BUFFER) {
			i = 0;
		}
		printf("%s\t",jobBuffer[i].name);
		printf("%u\t\t",jobBuffer[i].burst);
		printf("%u\t\t",jobBuffer[i].priority);
		if (run) {
			printf("RUN");
			run = 0;
		}
		printf("\n");
		i++;
	}
	return 0;
}

// Test Command - Tests multiple jobs with specified policy and returns results

int test(int nargs, char **args) {
	if (nargs != 7)
	{
		printf("\nWrong input! Give correct number of Arguments! test: <benchmark> <policy> <num_of_jobs> <priority_levels> <min_CPU_time> <max_CPU_time> \n");
		return INVALID;
	}
	totalj = 0;
	waitingtimef = 0;
	turnaroundtimef = 0;
	cputimef = 0;
	int mintime;
	int maxtime;
	int jobs;
	int prioritymax;
	sscanf(args[3], "%u", &jobs);
	sscanf(args[4], "%u", &prioritymax);
	sscanf(args[5], "%u", &mintime);
	sscanf(args[6], "%u", &maxtime);
	int i;
	if (count != 0) {
		printf("\nWaiting for other jobs to finish...\n");
		while((count != 0) || twoprocess) {
			//do nothing
		}
	}
	for (i = 0; i < jobs; i++) {
		char arg1[50], arg2[50], arg3[50], arg4[50];
		char **newArgs[10];
		int nargs = 4;
		int priority = (rand() % prioritymax + 1);
		int cputime = (rand() % (maxtime - mintime + 1) + mintime);
		sprintf(arg1,"%s", "run");
		sprintf(arg2, "%s", "testProcess");
		sprintf(arg3, "%u", cputime);
		sprintf(arg4, "%u", priority);
		newArgs[0] = arg1;
		newArgs[1] = arg2;
		newArgs[2] = arg3;
		newArgs[3] = arg4;
		run = 0;
		cmdrun(nargs, newArgs);
		run = 1;
		//call something that prints this out
	}
	sch = 0;
	if (strcmp(args[2], "fcfs") == 0) {
		fcfs(NULL, NULL);
	}
	else if (strcmp(args[2], "sjf") == 0) {
		sjf(NULL, NULL);
	}
	else if(strcmp(args[2], "priority") == 0) {
		cmdpriority(NULL, NULL);
	}
	else {
		return INVALID;
	}
	sch = 1;
	while((count != 0) || twoprocess) {
		//do nothing
	}
	float totalRuns = totalj;
	printf("Total number of jobs submitted: %u\n", totalj);
	float turnAround = turnaroundtimef / totalRuns;
	float cpu = cputimef / totalRuns;
	float wait = waitingtimef / totalRuns;
	float throughput = 1 / cpu;
	printf("Average turnaround time: %.2f seconds\n", turnAround);
	printf("Average CPU time: %.2f seconds\n", cpu);
	printf("Average waiting time: %.2f seconds\n", wait);
	printf("Throughput:	%.2f No./second\n", throughput);
	return 0;
}

// quit command to quit the program and return results

int quit(int nargs, char **args) {
	float totalRuns = total - count;
	if(totalRuns < 1) {
		printf("No jobs were run, can't display results \n");
		exit(0);
	}
	printf("Total number of jobs submitted: %u\n", total);
	float turnAround = turnaroundtime / totalRuns;
	float cpu = cputime / totalRuns;
	float wait = waitingtime / totalRuns;
	float throughput = 1 / cpu;
	printf("Average turnaround time:%.2f seconds\n", turnAround);
	printf("Average CPU time:%.2f seconds\n", cpu);
	printf("Average waiting time:%.2f seconds\n", wait);
	printf("Throughput:%.2f No./second\n", throughput);
        exit(0);
}

// main method 

int main() {
	count = 0;
	buffer_head = 0;
	buffer_tail = 0;
		
	printf("\n Welcome to Sajith's batch job scheduler version 1.0\n Type 'help' to find more about AUbatch commands.");
	pthread_t commandLineThread, executorThread;

	//concurrent running threads

	int iret1, iret2;
	iret1 = pthread_create(&commandLineThread, NULL, commandLine, NULL);
	iret2 = pthread_create(&executorThread, NULL, executor, NULL);
	
	//to check if threads are created are not

	if ((iret1 != 0) || (iret2 != 0)) {
		printf("cannot create threads");
		exit(1);
	}

	//intialize the lock and condition variables

	pthread_mutex_init(&queue_lock, NULL);
	pthread_cond_init(&buffer_nf, NULL);
	pthread_cond_init(&buffer_ne, NULL);

	// joining threads

	pthread_join(commandLineThread, NULL);
	pthread_join(executorThread, NULL);
	exit(0);
}

// The passer - this takes input from the command line and send it to the proper command.

int passer(char *cmd) {
	time_t beforesecs, aftersecs, secs;
	u_int32_t bsec, asec, nsecs;
	char *args[MAXARGS];
	int nargs=0;
	char *word;
	char *context;
	int i, result;
	for (word = strtok_r(cmd, " ", &context);
		word != NULL;
		word = strtok_r(NULL, " ", &context)) {
		if (nargs >= MAXARGS) {
			printf("command line has too many words\n");
			return LINE;
		}
		args[nargs++] = word;
	}
		if (nargs==0) {
			return 0;
		}
	for(i = 0; cmdtable[i].name; i++) {
		if(*cmdtable[i].name && !strcmp(args[0], cmdtable[i].name)) {
			assert(cmdtable[i].func != NULL);
			result = cmdtable[i].func(nargs, args);
			return result;
		}
	}
	printf("%s: Command not found\n", args[0]);
	return INVALID;
}

// Command line - Takes input from user

void *commandLine() {
	unsigned int i;
	char num_str[8];
	size_t command_size[64];
	char *input;

	for (i = 0; i < CMD; i++) {
		pthread_mutex_lock(&queue_lock);
		while (count == BUFFER) {
			pthread_cond_wait(&buffer_nf, &queue_lock);
		}
		pthread_mutex_unlock(&queue_lock);
		printf("\n>");
		getline(&input, &command_size, stdin);
		passer(input);
	}

}

// executor - this runs the jobs in the queue

void *executor() {
	unsigned int i;
	char arg1[50], arg2[50];
	for (i = 0; i < CMD; i++) {
		pthread_mutex_lock(&queue_lock);
	while (count == 0) {
		pthread_cond_wait(&buffer_ne, &queue_lock);
	}
	count--;
	sprintf(arg1,"%s",jobBuffer[buffer_tail].name);
	sprintf(arg2,"%u",jobBuffer[buffer_tail].burst);
	time_t startWait = jobBuffer[buffer_tail].arrival;
	int cTime = jobBuffer[buffer_tail].burst;
	time_t startTime = time(NULL);
	oneprocess = 1;
	pid_t run = fork();
	if(run == 0) {
		execv("./process",(char*[]){"./process",arg1,arg2,NULL});
	}
	buffer_tail++;
	if (buffer_tail == BUFFER) {
		buffer_tail = 0;
	}
	pthread_cond_signal(&buffer_nf);
	pthread_mutex_unlock(&queue_lock);
	wait();
	time_t finishTime = time(NULL);
	time_t runTime = finishTime - startTime;
	time_t waitTime = startTime - startWait;
	cputime = cputime + runTime;
	cputimef = cputime;
	expectedwaitingtime = expectedwaitingtime - cTime;
	waitingtime = waitingtime + waitTime;
	waitingtimef = waitingtime;
	turnaroundtime = turnaroundtime + runTime + waitTime;
	turnaroundtimef = turnaroundtime;
	if (count == 0) {
		sleep(20);
		twoprocess = 0;
	}
	}
}