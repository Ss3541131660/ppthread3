#include <pthread.h>
#include <iostream>
#include<time.h>
#include<stdio.h>
#include <semaphore.h>
#define NUM_THREADS 4
using namespace std;
int n = 2048;
float pA[2048][2048];
pthread_mutex_t mutex;
//float** pA = new float* [n];


//timespec_get(&sts,TIME_UTC);

typedef struct {
	int k;          // ��ȥ���ִ�
	int t_id;       // �߳�id
} threadParam_t;
//�ṹ�嶨��

// �ź�������
sem_t sem_main;
sem_t sem_workerstart[NUM_THREADS]; //ÿ���߳����Լ�ר�����ź���
sem_t sem_workerend[NUM_THREADS];
void* threadFunc(void* param) {
	threadParam_t* p = (threadParam_t*)param;
	int t_id = p->t_id;

	for (int k = 0; k < n; ++k) {
		sem_wait(&sem_workerstart[t_id]); //�������ȴ�������ɳ��������������Լ�ר�����ź�����

		// ѭ����������
		for (int i = k + 1 + t_id; i < n; i += NUM_THREADS) {
			// ��ȥ
			for (int j = k + 1; j < n; ++j) {
				pA[i][j] = pA[i][j] - pA[i][k] * pA[k][j];
			}
			pA[i][k] = 0.0;
		}

		sem_post(&sem_main); //�������߳�
		sem_wait(&sem_workerend[t_id]); //�������ȴ����̻߳��ѽ�����һ��
	}
	pthread_exit(NULL);
}

int main() {
	// ����A��n
	// ...
	//struct timespec sts, ets;
	//timespec_get(&sts, TIME_UTC);

	srand(time(NULL));
	for (int i = 0; i < n; i++) {
		pA[i][i] = 1.0;//�Խ���Ϊ1.0
		for (int j = 0; j < n; j++) {
			if (j > i)pA[i][j] = rand() % 10;
			else if (j < i)pA[i][j] = 0;
		}
	}
	//�����Ǿ���
	for (int k = 0; k < n; k++) {
		for (int i = k + 1; i < n; i++) {
			for (int j = 0; j < n; j++) {
				pA[i][j] += pA[k][j];
			}
		}
	}

	sem_init(&sem_main, 0, 0);
	for (int i = 0; i < NUM_THREADS; ++i) {
		sem_init(&sem_workerstart[i], 0, 0);
		sem_init(&sem_workerend[i], 0, 0);
	}

	// �����߳�
	pthread_t handles[NUM_THREADS];
	threadParam_t param[NUM_THREADS];
	for (int t_id = 0; t_id < NUM_THREADS; t_id++) {
		param[t_id].t_id = t_id;
		pthread_create(&handles[t_id], NULL, threadFunc, &param[t_id]);
	}

	for (int k = 0; k < n; ++k) {
		// ���߳�����������
		for (int j = k + 1; j < n; j++) {
			pA[k][j] = pA[k][j] / pA[k][k];
		}
		pA[k][k] = 1.0;

		// ��ʼ���ѹ����߳�
		for (int t_id = 0; t_id < NUM_THREADS; ++t_id) {
			sem_post(&sem_workerstart[t_id]);
		}

		// ���߳�˯�ߣ��ȴ����еĹ����߳���ɴ�����ȥ����
		for (int t_id = 0; t_id < NUM_THREADS; ++t_id) {
			sem_wait(&sem_main);
		}

		// ���߳��ٴλ��ѹ����߳̽�����һ�ִε���ȥ����
		for (int t_id = 0; t_id < NUM_THREADS; ++t_id) {
			sem_post(&sem_workerend[t_id]);
		}
	}

	for (int t_id = 0; t_id < NUM_THREADS; t_id++) {
		pthread_join(handles[t_id], NULL);
	}

	// ���������ź���
	sem_destroy(&sem_main);
	for (int i = 0; i < NUM_THREADS; ++i) {
		sem_destroy(&sem_workerstart[i]);
		sem_destroy(&sem_workerend[i]);
	}

	//timespec_get(&ets,TIME_UTC);
	//time_t dsec = ets.tv_sec - sts.tv_sec;
	//long dnsec=ets.tv_nsec-sts.tv_nsec;
	//if(dnsec<0){
	//dsec--;
	//dnsec+=1000000000ll;
	//}
	// printf ("%lld.%09llds\n",dsec,dnsec);
	return 0;
}
