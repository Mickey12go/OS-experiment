#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<Windows.h>
#define generator_num 20 
typedef struct PCB PCB;
typedef struct Queue Queue;

struct PCB{
	char pid;//进程标识符，用单个字符标识
	char state;//进程状态 r：运行状态；w：就绪状态；b:阻塞状态
	int priority;//进程优先级
	int arrivalTime;//到达时间
	int neededTime;//运行时间
	int usedTime;//累计运行时间 因为时间片不足会下调
	int totalWaitTime;//已等待时间
	struct PCB* next;
};

struct Queue{
	int priority;    //该队列的优先级
	int timeSlice;   //该队列的时间片长度
	struct PCB* list;   //指向该队列中进程PCB链表的头部 
};//队列

Queue readyQueue[5];//就绪队列
PCB* finial[5];//尾指针

HANDLE sema1;
HANDLE sema2;

DWORD WINAPI generator(Queue *q) {
	srand((unsigned)time(NULL));
	int currentTime = 0;
	PCB* p = NULL;
	for (int i = 0; i < generator_num; i++) {
		int neededTime = rand() % 199 + 2;//每个进程所需要运行的时间neededTime在一定范围内(假设为[2,200]ms)内由随机数产生
		int arrivalTime = rand() % 100 + 1;//每隔一个随机的时间段，例如[1,100]ms之间的一个随机数，就产生一个新的进程
		(PCB*)p = (PCB*)malloc(sizeof(PCB));
		if (p == NULL) {
			continue;
		}
		p->arrivalTime = arrivalTime;
		p->neededTime = neededTime;
		p->pid = 65 + i;
		p->priority = 1;
		p->state = 'w';
		p->usedTime = 0;
		p->totalWaitTime = 0;
		p->next = NULL;
		currentTime += arrivalTime;
		if (i == 0) {
			(q->list)->next=p;//成为头部
			finial[0] = p;
		}
		else {
			finial[0]->next = p;
			finial[0] = p;
		}
		p->next = NULL;
		printf("Generator:Process %d is generated,neededTime=%d,arrivalTime=%d \n", p->pid, p->neededTime, p->arrivalTime);
		int sleepTime = rand() % 100 + 1;
		printf("Generator:Sleep for %d ms before generating next process...\n",sleepTime);
		ReleaseSemaphore(sema1, 1, NULL);
		ReleaseSemaphore(sema2, 1, NULL);	
		Sleep(sleepTime);
		currentTime += sleepTime;	
	}
	return 0;
}

void executor(int sch_index,Queue q[]) {
	if(sch_index==0)
		WaitForSingleObject(sema1, INFINITE);
	PCB pcb = *(q[sch_index].list);
	Sleep(q[sch_index].timeSlice);
	int i = 0;
	while (i < 5) {
		q[i].list->totalWaitTime += q[sch_index].timeSlice;
		q[i].list->next
	}
	
}

int index = -1;
void scheduler(Queue *q) {
	for (int i = 0; i < 5; i++) {
		if (q[0].list->next!=NULL) {
			executor(0, readyQueue);
		}
		else
		{
			if (q[i].list->next = NULL) {
				break;
			}
			else {
				if(q[i].list->neededTime>usedTime)
			}
		}
	}
}


int main() {
	int timeInitial = 10;
	readyQueue[0].timeSlice = timeInitial;
	for (int i = 0; i < 5; i++) {
		readyQueue[i].priority = i + 1;
		if (i != 0)
			readyQueue[i].timeSlice = (readyQueue[i - 1].timeSlice )* 2;//时间片关系
		
	}
	sema1 = CreateSemaphore(NULL, 0, 1, NULL);//用于generator和executor互斥访问第一个运行队列
	sema2 = CreateSemaphore(NULL, 0, 1, NULL);//用于generator和scheduler的同步


}

