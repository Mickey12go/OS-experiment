#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<Windows.h>
#define generator_num 20 
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
	struct PCB* next;
};

struct Queue{
	int priority;    //�ö��е����ȼ�
	int timeSlice;   //�ö��е�ʱ��Ƭ����
	struct PCB* list;   //ָ��ö����н���PCB�����ͷ�� 
};//����

Queue readyQueue[5];//��������
PCB* finial[5];//βָ��

HANDLE sema1;
HANDLE sema2;

DWORD WINAPI generator(Queue *q) {
	srand((unsigned)time(NULL));
	int currentTime = 0;
	PCB* p = NULL;
	for (int i = 0; i < generator_num; i++) {
		int neededTime = rand() % 199 + 2;//ÿ����������Ҫ���е�ʱ��neededTime��һ����Χ��(����Ϊ[2,200]ms)�������������
		int arrivalTime = rand() % 100 + 1;//ÿ��һ�������ʱ��Σ�����[1,100]ms֮���һ����������Ͳ���һ���µĽ���
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
			(q->list)->next=p;//��Ϊͷ��
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
			readyQueue[i].timeSlice = (readyQueue[i - 1].timeSlice )* 2;//ʱ��Ƭ��ϵ
		
	}
	sema1 = CreateSemaphore(NULL, 0, 1, NULL);//����generator��executor������ʵ�һ�����ж���
	sema2 = CreateSemaphore(NULL, 0, 1, NULL);//����generator��scheduler��ͬ��


}

