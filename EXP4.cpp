#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<Windows.h>
#include<math.h>

#define generator_num 20 
#define Queue_size 5
#define MinTimeSlice 10
HANDLE sema1;//���ڻ���ķ��ʵ�һ�����ж���
HANDLE sema2;//���ڼ��༶�������Ƿ��н��̵ȴ�����

typedef struct PCB PCB;
typedef struct Queue Queue;

struct PCB{
	char pid;//���̱�ʶ�����õ����ַ���ʶ
	char state;//����״̬ r������״̬��w������״̬��b:����״̬
	int priority;//�������ȼ�
	int arrivalTime;//����ʱ��
	int neededTime;//����ʱ��
	int usedTime;//�ۼ�����ʱ�� ��Ϊʱ��Ƭ������µ�
	int totalWaitTime;//�ѵȴ�ʱ��
	struct PCB* next;//ָ����һ��PCB������ָ��
};

struct Queue{
	int priority;    //�ö��е����ȼ�
	int timeSlice;   //�ö��е�ʱ��Ƭ����
	struct PCB* list;   //ָ��ö����н���PCB�����ͷ�� 
};//����

Queue readyQueue[Queue_size];//��������
PCB* finial[Queue_size];//����������е�βָ�� 


DWORD WINAPI generator(Queue q) {
	int i, current = 0;
	srand((unsigned)time(NULL));
	for (i = 0; i < generator_num; i++)
	{
		int arrivetime = rand() % 100 + 1;//������ɵ���ʱ��
		PCB* p = (PCB*)malloc(sizeof(PCB));
		p->neededTime = rand() % 199 + 2;//�����������ʱ��
		p->priority = 1;
		p->pid = 'A' + i;
		p->state = 'w';
		p->arrivalTime = current;
		p->usedTime = 0;
		p->totalWaitTime = 0;
		p->next = NULL;
		current = current + arrivetime;//��ʼ��PCB
		if (i == 0) {
			q.list->next = p;//��һ�����̣�˵���˿̸ոտ�ʼ�������ﻹû�н���
		}
		finial[0]->next = p;//�ƶ�βָ��
		finial[0] = p;
		finial[0]->next = NULL;
		printf_s("Generator��Process %c is generated��neededtime = %d��arrivaltime = %d \n", p->pid,p->neededTime, p->arrivalTime);
		printf_s("Generator��Sleep for %d ms before generating next process...\n", arrivetime);
		ReleaseSemaphore(sema1, 1, NULL);//�ͷ��ź���
		ReleaseSemaphore(sema2, 1, NULL);
		Sleep(arrivetime);
	}
	return 0;
}
DWORD WINAPI scheduler(Queue *q) {
	void executor(int sch_index,  Queue * q);
	WaitForSingleObject(sema2, INFINITE);//�����༶�����л��н��̵ȴ�����ʱ��scheduler���ܿ�ʼִ�е���
	for (int i = 0; i < Queue_size; i++) {
			if (q[i].list->next != NULL) {//�������Ķ����ﲻΪ��
				executor(i, q);	
				q[i].list->next->usedTime += q[i].timeSlice;
				if (q[i].list->next->neededTime > q[i].list->next->usedTime) {
					printf("Scheduler:Process %c is moved to queue %d.\n", q[i].list->next->pid, i + 2);
					finial[i + 1]->next = q[i].list->next;
					finial[i + 1] = finial[i + 1]->next;
					q[i].list->next = q[i].list->next->next;//ͨ��ָ��仯������һ��������δ��ɵĽ��̲�����һ�����еĶ�β
					ReleaseSemaphore(sema2, 1, NULL);//�ͷ��ź�
					i = 0;//��ͷ��ʼ�����¼���Ķ���Ҳ�ܱ���⵽����Ϊ�¼���Ľ������ȼ���������ȼ�����

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
					return 0;//���н���ִ����ϣ��˳�����
				}
			}
		}
		return 0;
	}



void executor(int sch_index, Queue* q) {//sch_index�Ǵ�scheduler�д�������
	if (sch_index==0) {//����ǵ�һ������
		WaitForSingleObject(sema1, INFINITE);
		printf("Executor:Process %c in queue %d consumes %d ms.\n", q[sch_index].list->next->pid, sch_index + 1, q[sch_index].timeSlice);
		ReleaseSemaphore(sema1, 1, NULL);//�������
		Sleep(q[sch_index].timeSlice);
	}//�ֿ�������Ϊ��һ��������Ҫ���ź�������ʶ
	else {
		printf("Executor:Process %c in queue %d consumes %d ms.\n", q[sch_index].list->next->pid, sch_index + 1, q[sch_index].timeSlice);
		Sleep(q[sch_index].timeSlice);
	}
	//���¶����е�δ��ɽ���
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
		readyQueue[i].timeSlice = MinTimeSlice * pow(2,i);//ʱ��Ƭ��ϵ
		finial[i] = (PCB*)malloc(sizeof(PCB));
		readyQueue[i].list = finial[i];//��ʼ����������βָ���غ�
		finial[i]->next = NULL;
	}
	sema1 = CreateSemaphore(NULL, 1, 1, NULL);//����generator��executor������ʵ�һ�����ж���
	sema2 = CreateSemaphore(NULL, 0, 1, NULL);//����generator��scheduler��ͬ��
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

