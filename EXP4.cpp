#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<Windows.h>
#include<math.h>

#define generator_num 20 
#define Queue_size 5
#define MinTimeSlice 10
HANDLE sema1;//用于互斥的访问第一个运行队列
HANDLE sema2;//用于检查多级队列中是否还有进程等待运行

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
	struct PCB* next;//指向下一个PCB的链表指针
};

struct Queue{
	int priority;    //该队列的优先级
	int timeSlice;   //该队列的时间片长度
	struct PCB* list;   //指向该队列中进程PCB链表的头部 
};//队列

Queue readyQueue[Queue_size];//就绪队列
PCB* finial[Queue_size];//五个就绪队列的尾指针 


DWORD WINAPI generator(Queue q) {
	int i, current = 0;
	srand((unsigned)time(NULL));
	for (i = 0; i < generator_num; i++)
	{
		int arrivetime = rand() % 100 + 1;//随机生成到达时间
		PCB* p = (PCB*)malloc(sizeof(PCB));
		p->neededTime = rand() % 199 + 2;//随机生成运行时间
		p->priority = 1;
		p->pid = 'A' + i;
		p->state = 'w';
		p->arrivalTime = current;
		p->usedTime = 0;
		p->totalWaitTime = 0;
		p->next = NULL;
		current = current + arrivetime;//初始化PCB
		if (i == 0) {
			q.list->next = p;//第一个进程，说明此刻刚刚开始，队列里还没有进程
		}
		finial[0]->next = p;//移动尾指针
		finial[0] = p;
		finial[0]->next = NULL;
		printf_s("Generator：Process %c is generated，neededtime = %d，arrivaltime = %d \n", p->pid,p->neededTime, p->arrivalTime);
		printf_s("Generator：Sleep for %d ms before generating next process...\n", arrivetime);
		ReleaseSemaphore(sema1, 1, NULL);//释放信号量
		ReleaseSemaphore(sema2, 1, NULL);
		Sleep(arrivetime);
	}
	return 0;
}
DWORD WINAPI scheduler(Queue *q) {
	void executor(int sch_index,  Queue * q);
	WaitForSingleObject(sema2, INFINITE);//仅当多级队列中还有进程等待运行时，scheduler才能开始执行调度
	for (int i = 0; i < Queue_size; i++) {
			if (q[i].list->next != NULL) {//遍历到的队列里不为空
				executor(i, q);	
				q[i].list->next->usedTime += q[i].timeSlice;
				if (q[i].list->next->neededTime > q[i].list->next->usedTime) {
					printf("Scheduler:Process %c is moved to queue %d.\n", q[i].list->next->pid, i + 2);
					finial[i + 1]->next = q[i].list->next;
					finial[i + 1] = finial[i + 1]->next;
					q[i].list->next = q[i].list->next->next;//通过指针变化将在上一个队列里未完成的进程插入下一个队列的队尾
					ReleaseSemaphore(sema2, 1, NULL);//释放信号
					i = 0;//从头开始，让新加入的队列也能被检测到，因为新加入的进程首先加入最高优先级队列

				}
				else {
					printf("Executor:Process %c finished execution.Total wait time: %d ms.\n", q[i].list->next->pid, q[i].list->next->totalWaitTime);
					q[i].list->next = q[i].list->next->next;
					ReleaseSemaphore(sema2, 1, NULL);
					i = 0;
				}
			}
			else {
				if (i ==Queue_size-1) {
					printf("All Threads are finished!\n");
					return 0;//所有进程执行完毕，退出函数
				}
			}
		}
		return 0;
	}



void executor(int sch_index, Queue* q) {//sch_index是从scheduler中传过来的
	if (sch_index==0) {//如果是第一个队列
		WaitForSingleObject(sema1, INFINITE);
		printf("Executor:Process %c in queue %d consumes %d ms.\n", q[sch_index].list->next->pid, sch_index + 1, q[sch_index].timeSlice);
		ReleaseSemaphore(sema1, 1, NULL);//互斥访问
		Sleep(q[sch_index].timeSlice);
	}//分开就是因为第一个进程需要用信号量来标识
	else {
		printf("Executor:Process %c in queue %d consumes %d ms.\n", q[sch_index].list->next->pid, sch_index + 1, q[sch_index].timeSlice);
		Sleep(q[sch_index].timeSlice);
	}
	//更新队列中的未完成进程
	for (int i = 0; i < Queue_size; i++) {
		PCB* temp = (PCB*)malloc(sizeof(PCB));
		if (i == sch_index) {
			temp = q[i].list->next->next;
		}
		else {
			temp= q[i].list->next;
		}
		while (temp != NULL) {
			temp->totalWaitTime += q[i].timeSlice;
			temp = temp->next;
		}
	}
}

int main() {
	int i;
	for (int i = 0; i < Queue_size; i++) {
		readyQueue[i].list = NULL;
		readyQueue[i].priority = i+1;
		readyQueue[i].timeSlice = MinTimeSlice * pow(2,i);//时间片关系
		finial[i] = (PCB*)malloc(sizeof(PCB));
		readyQueue[i].list = finial[i];//初始化，让其首尾指针重合
		finial[i]->next = NULL;
	}
	sema1 = CreateSemaphore(NULL, 1, 1, NULL);//用于generator和executor互斥访问第一个运行队列
	sema2 = CreateSemaphore(NULL, 0, 1, NULL);//用于generator和scheduler的同步
	HANDLE T1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)generator, (void*)&readyQueue[0], 0, NULL);
	HANDLE T2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)scheduler, (void*)&readyQueue, 0, NULL);

	WaitForSingleObject(T1, INFINITE);
	WaitForSingleObject(T2, INFINITE);
	CloseHandle(T1);
	CloseHandle(T2);
	CloseHandle(sema1);
	CloseHandle(sema2);

	return 0;
}

