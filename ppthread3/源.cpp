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
	int k;          // 消去的轮次
	int t_id;       // 线程id
} threadParam_t;
//结构体定义

// 信号量定义
sem_t sem_main;
sem_t sem_workerstart[NUM_THREADS]; //每个线程有自己专属的信号量
sem_t sem_workerend[NUM_THREADS];
void* threadFunc(void* param) {
	threadParam_t* p = (threadParam_t*)param;
	int t_id = p->t_id;

	for (int k = 0; k < n; ++k) {
		sem_wait(&sem_workerstart[t_id]); //阻塞，等待主线完成除法操作（操作自己专属的信号量）

		// 循环划分任务
		for (int i = k + 1 + t_id; i < n; i += NUM_THREADS) {
			// 消去
			for (int j = k + 1; j < n; ++j) {
				pA[i][j] = pA[i][j] - pA[i][k] * pA[k][j];
			}
			pA[i][k] = 0.0;
		}

		sem_post(&sem_main); //唤醒主线程
		sem_wait(&sem_workerend[t_id]); //阻塞，等待主线程唤醒进入下一轮
	}
	pthread_exit(NULL);
}

int main() {
	// 读入A和n
	// ...
	//struct timespec sts, ets;
	//timespec_get(&sts, TIME_UTC);

	srand(time(NULL));
	for (int i = 0; i < n; i++) {
		pA[i][i] = 1.0;//对角线为1.0
		for (int j = 0; j < n; j++) {
			if (j > i)pA[i][j] = rand() % 10;
			else if (j < i)pA[i][j] = 0;
		}
	}
	//上三角矩阵
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

	// 创建线程
	pthread_t handles[NUM_THREADS];
	threadParam_t param[NUM_THREADS];
	for (int t_id = 0; t_id < NUM_THREADS; t_id++) {
		param[t_id].t_id = t_id;
		pthread_create(&handles[t_id], NULL, threadFunc, &param[t_id]);
	}

	for (int k = 0; k < n; ++k) {
		// 主线程做除法操作
		for (int j = k + 1; j < n; j++) {
			pA[k][j] = pA[k][j] / pA[k][k];
		}
		pA[k][k] = 1.0;

		// 开始唤醒工作线程
		for (int t_id = 0; t_id < NUM_THREADS; ++t_id) {
			sem_post(&sem_workerstart[t_id]);
		}

		// 主线程睡眠（等待所有的工作线程完成此轮消去任务）
		for (int t_id = 0; t_id < NUM_THREADS; ++t_id) {
			sem_wait(&sem_main);
		}

		// 主线程再次唤醒工作线程进入下一轮次的消去任务
		for (int t_id = 0; t_id < NUM_THREADS; ++t_id) {
			sem_post(&sem_workerend[t_id]);
		}
	}

	for (int t_id = 0; t_id < NUM_THREADS; t_id++) {
		pthread_join(handles[t_id], NULL);
	}

	// 销毁所有信号量
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
