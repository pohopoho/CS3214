//  On my honor:
//  //  //  //  //
//  //  //  //  //  - I have not discussed the C language code in my program with
//  //  //  //  //    anyone other than my instructor or the teaching assistants
//  //  //  //  //    assigned to this course.
//  //  //  //  //
//  //  //  //  //  - I have not used C language code obtained from another student,
//  //  //  //  //    the Internet, or any other unauthorized source, either modified
//  //  //  //  //    or unmodified.
//  //  //  //  //
//  //  //  //  //  - If any C language code or documentation used in my program
//  //  //  //  //    was obtained from an authorized source, such as a text book or
//  //  //  //  //    course notes, that has been clearly noted with a proper citation
//  //  //  //  //    in the comments of my program.
//  //  //  //  //
//  //  //  //  //  - I have not designed this program in such a way as to defeat or
//  //  //  //  //    interfere with the normal operation of the grading code.
//  //  //  //  //
//  //  //  //  //    Name: Robert Andrews
//  //  //  //  //    PID: robbiiie
//  //  //  //  //
//  //  //  //  //    Name: Michael Cheung
//  //  //  //  //    PID: michaelc97

#include "threadpool.h"
#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

/*
 * A work-stealing, fork-join thread pool.
 */

/**
 * Struct for a future element. A future is made up of
 * a fork join task, arguments, result, elements to access job,
 * semaphore used to mark if a task was complete, a threadpool pointer,
 * and an int for the task status. This object can be referenced using
 * "future" in the code
 */
typedef struct future {
	fork_join_task_t task;          //Fork join task
        void * args;			//Stores the arguments
        void * result;			//Stores the result
	struct list_elem elem;		//List element used to process jobs
	sem_t taskComplete;		//Semaphore used for ordering
	struct thread_pool * poolPointer; //Pointer for pool
	int taskStatus;			//2 if complete, 1 if in progress, 0 if not started
} future;

/**
 * Struct for the worker element. A worker element is made up of
 * a local queue of tasks, a list element of the queue, and a mutex
 * used for accessing local lock. This object can be referenced by using
 * "worker" in the code
 */
typedef struct worker {			//Abstracted thread into a "worker"
	struct list q_local;		//Each worker thread's local queue
	struct list_elem elem;		//A list element of the queue
	pthread_mutex_t localLock;	//Mutex for accessing local lock
} worker;

/**
 * Struct for the threadpool element. A threadpool is made up of a global task list,
 * int for number of threads, a list for the worker threads, an array for thread ID #s,
 * a cond var for waking up the workers, mutex for accessing threadpool data, and a mutex
 * for accessing worker data. This object can be referenced by using "thread_pool" 
 * in the code
 */
typedef struct thread_pool {
	struct list q_global;		//Global task list
        int threads;			//Int for number of threads
	struct list workerList;		//List of threads/workers

	pthread_t * threadArr;		//Array for thread ID numbers
	pthread_cond_t workerAlarm;	//Cond variable for waking up workers	
	pthread_mutex_t poolLock;	//Mutex for accessing pool data
	pthread_mutex_t wLock;		//Mutex for accessing worker data
	int exitFlag;
} thread_pool;

//static __thread struct list q_Worker;	//thread local queue for each thread/worker

/*
 * Function that steals work from another worker threads list.
 * Iterates through the pools worker list and then if the workers
 * queue is not empty it steals the element at the end of the workers
 * list. If all workers have no work then nothing happens
 */
static struct list_elem * steal(thread_pool * pool);

/*
 * Boolean function that returns if the workers are empty or not. It goes
 * through a thread pools worker list and checks if each workers list is
 * empty or not. If they are empty this method returns true, else false
 */
static bool areWorkersEmpty(thread_pool * pool);

/*
 * Worker thread routine
 * Number of these threads is specified by the user, each thread will run and
 * attempt to do work/tasks unless the global queue, its own local queue, and
 * other worker queues are all empty. In that scenario, the thread will wait
 * for more work to show up.
 */
static void * worker_thread(void* p) {
	//list_init(&q_Worker);
	//printf("entered worker\n");
	thread_pool * pointerToSwim = (thread_pool *)p;		//unvoid parameter
	pthread_mutex_lock(&pointerToSwim->poolLock);	
	int a = 0;
	worker * pointerToWorker = NULL;				//local pointer so that each thread can access their queues
	struct list_elem * iter = list_begin(&pointerToSwim->workerList);
	bool found = false;
	
	while(iter != list_end(&pointerToSwim->workerList) && !found)			//searching for worker's correct queue
	{
		if(pthread_self() == pointerToSwim->threadArr[a])			
		{
			pointerToWorker = list_entry(iter, struct worker, elem);
			found = true;
		}
		a++;
		iter = list_next(iter);
	}
	pthread_mutex_unlock(&pointerToSwim->poolLock);
	//printf("entered worker\n");
	while(1)
	{ 
		pthread_mutex_lock(&pointerToSwim->poolLock);				//LOCK A
		
		while(list_empty(&pointerToSwim->q_global) && areWorkersEmpty(pointerToSwim) && !pointerToSwim->exitFlag)
                {
                        pthread_cond_wait(&pointerToSwim->workerAlarm, &pointerToSwim->poolLock);//waiting for condition variable
                }
		
		if(pointerToSwim->exitFlag)
		{
			pthread_mutex_unlock(&pointerToSwim->poolLock); //unlock before exiting
			pthread_exit(NULL);
		} 
		struct list_elem  * toRun = NULL;
		pthread_mutex_lock(&pointerToWorker->localLock);
		//pthread_mutex_lock(&pointerToSwim->wLock);								
		if(!list_empty(&pointerToWorker->q_local))				//Do I have tasks?
		{
			toRun = list_pop_front(&pointerToWorker->q_local);		//pop from front
			pthread_mutex_unlock(&pointerToWorker->localLock);
		}
		else
		{
			pthread_mutex_unlock(&pointerToWorker->localLock);
			if(!list_empty(&pointerToSwim->q_global))			//Are there global tasks?
			{
				toRun = list_pop_back(&pointerToSwim->q_global);	//pop from back
			}
			else
			{
				toRun = steal(pointerToSwim);				//Does anyone else have tasks? pop from back
			}
		}
		
		//pthread_mutex_unlock(&pointerToSwim->wLock);
		future * t = list_entry(toRun, struct future, elem);
		t->taskStatus = 1;
		pthread_mutex_unlock(&pointerToSwim->poolLock);         //UNLOCK A executing future task does not require pool data
		t->result = (t->task)(pointerToSwim, t->args);		//do the task
		pthread_mutex_lock(&pointerToSwim->poolLock);
		t->taskStatus = 2;
		pthread_mutex_unlock(&pointerToSwim->poolLock);
		sem_post(&t->taskComplete);				//post semaphore after task is complete
		//pthread_mutex_unlock(&pointerToSwim->poolLock);
	}
	return NULL;	
}

/*
 * Function that steals work from another worker threads list.
 * Iterates through the pools worker list and then if the workers
 * queue is not empty it steals the element at the end of the workers
 * list. If all workers have no work then nothing happens
 */
static struct list_elem * steal(thread_pool * pool) {
	pthread_mutex_lock(&pool->wLock);				//Lock worker lock
	struct list_elem * iter = list_begin(&pool->workerList);	//Iterator to go though the whole list
	while(iter != list_end(&pool->workerList))			//Go through array of threads
	{	
		worker * w = list_entry(iter, struct worker, elem);
		if(!list_empty(&w->q_local))				//Goes in if the workers queue is not empty
		{
			pthread_mutex_unlock(&pool->wLock);		//Unlock worker lock
			return list_pop_back(&w->q_local);		//Return the item at back of list. This is the stolen work
			
		}
		iter = list_next(iter);
	}
	pthread_mutex_unlock(&pool->wLock);				//Unlock worker lock
	return NULL;
	
}

/*
 * Boolean function that returns if the workers are empty or not. It goes
 * through a thread pools worker list and checks if each workers list is
 * empty or not. If they are empty this method returns true, else false
 */
static bool areWorkersEmpty(thread_pool * pool) {
	//int a = 0;
	pthread_mutex_lock(&pool->wLock);					//Lock worker lock
	struct list_elem * iter = list_begin(&pool->workerList);		//Iterator for the list of the pools workers
	while(iter != list_end(&pool->workerList))				//Go through array of threads
	{
		worker * w = list_entry(iter, struct worker, elem);
		if(!list_empty(&w->q_local))					//If that thread's queue is not empty, return false
		{
			pthread_mutex_unlock(&pool->wLock);			//Unlock worker lock
			return false;						//Worker is not empty
		}
		iter = list_next(iter);
	}
	pthread_mutex_unlock(&pool->wLock);					//Unlock worker lock
	return true;								//Worker is empty
}

/* Create a new thread pool with no more than n threads. */
struct thread_pool * thread_pool_new(int nthreads) {
	thread_pool * swim = (thread_pool*)calloc(1, sizeof(thread_pool)); 	//Create threadpool

        swim->threads = nthreads;		//Set number of threads
	
        list_init(&swim->q_global);		//Initialize global queue 

	list_init(&swim->workerList);		//Initialize list of threads/workers	
	
	swim->threadArr = (pthread_t*)calloc(nthreads, sizeof(pthread_t));	//Give memory to the pools array of threads
	
	pthread_mutex_init(&swim->poolLock, NULL);	//Initialize pool lock
	pthread_mutex_init(&swim->wLock, NULL);		//Initialize worker lock
	pthread_cond_init(&swim->workerAlarm, NULL);	//Initialize worker alarm lock
	pthread_mutex_lock(&swim->poolLock);		//Lock pool lock
	int a = 0;
	pthread_mutex_lock(&swim->wLock);		//Lock worker lock
	while(a < swim->threads)
	{
		//pthread_mutex_lock(&swim->wLock);
		struct worker * work = (worker*)calloc(1, sizeof(worker));		//Create a worker
		list_init(&work->q_local);						//Initialize the worker's local queue
		pthread_mutex_init(&work->localLock, NULL);
		list_push_back(&swim->workerList, &work->elem);				//Add it to the workerList
		
		//pthread_mutex_unlock(&swim->wLock);
		pthread_create(&swim->threadArr[a], NULL, &worker_thread, swim); 		//Spawn a thread
		
		a++;
	}
	pthread_mutex_unlock(&swim->wLock);		//Unlock worker lock
	pthread_mutex_unlock(&swim->poolLock);		//Unlock pool lock
	//pthread_cond_init(&swim->workerAlarm, NULL);
	//pthread_mutex_unlock(&swim->poolLock);
	return swim;			//Returns the pool
}


 /* * Shutdown this thread pool in an orderly fashion.
 * * Tasks that have been submitted but not executed may or
 * * may not be executed.
 * *
 * * Deallocate the thread pool object before returning.
 * */
void thread_pool_shutdown_and_destroy(struct thread_pool * pool) {
	pthread_mutex_lock(&pool->poolLock);			//Lock A
	
	pool->exitFlag = 1;					//Set exit flag
	
	pthread_cond_broadcast(&pool->workerAlarm);		//Wake every thread
	pthread_mutex_unlock(&pool->poolLock);			//Unlock A
	int a = 0;
	//pthread_mutex_lock(&pool->wLock);
	//struct list_elem * iter = list_begin(&pool->workerList);
	while(a < pool->threads)
	{
	//	list_pop_front(&pool->workerList);
		pthread_join(pool->threadArr[a], NULL);		//Join threads
		a++;
	}
	//*******FREE workers
	//pthread_mutex_unlock(&pool->wLock);
	/*
        while(a < pool->threads) {
                pthread_join(pool->threadArr[a], NULL);
                a++;
		pool->threads--;
        }
	
	struct list_elem * iter = list_begin(&pool->workerList);	
	while(iter != list_end(&pool->workerList))
	{
		free(list_entry(iter, struct worker, elem));	
		iter = list_next(iter);
	}
	*/
	free(pool->threadArr);
	free(pool);
}

/*
 * * Submit a fork join task to the thread pool and return a
 * * future. The returned future can be used in future_get()
 * * to obtain the result.
 * * pool - the pool to which to submit
 * * task - the task to be submitted.
 * * data - data to be passed to the tasks function
 * *
 * * Returns a future representing this computation.
 * */
struct future * thread_pool_submit(struct thread_pool * pool, fork_join_task_t task, void * data) {
	pthread_mutex_lock(&pool->poolLock);					//LOCK A
	
	future * fut = (future *) calloc(1, sizeof(future));			//Initialize future
	fut->poolPointer = pool;						//Set pool pointer
	fut->args = data;							//Set future arguments
	fut->taskStatus = 0;							//Set task status
	fut->task = task;							//Set task
	sem_init(&fut->taskComplete, 0, 0);					//Initialize semaphore

	list_push_front(&pool->q_global, &fut->elem);				//Add to global queue
	
	pthread_cond_signal(&pool->workerAlarm);				//Wake up a worker since stuff was just added
	pthread_mutex_unlock(&pool->poolLock);					//UNLOCK A
	
	return fut;
}

/* Make sure that the thread pool has completed the execution
 * * of the fork join task this future represents.
 * *
 * * Returns the value returned by this task.
 * */
void * future_get(struct future * fut) {
	pthread_mutex_lock(&fut->poolPointer->poolLock);			//Lock pool pointer pool lock
	if(fut->taskStatus == 0)	//Taskstatus = 1
	{
		//fut->taskStatus = 1;
		list_remove(&fut->elem);	//Removes element form list
		fut->taskStatus = 1;
		pthread_mutex_unlock(&fut->poolPointer->poolLock);		//Unlock pool pointer pool lock
		fut->result = (fut->task)(fut->poolPointer, fut->args);
		//pthread_mutex_lock(&fut->poolPointer->poolLock);
		fut->taskStatus = 2;
		sem_post(&fut->taskComplete);	//Post semaphore
	}
	else if(fut->taskStatus == 1){
		pthread_mutex_unlock(&fut->poolPointer->poolLock);		//Unlock pool pointer pool lock
		sem_wait(&fut->taskComplete);	//Semaphore wait
	}
	else
	{
		pthread_mutex_unlock(&fut->poolPointer->poolLock);		//Unlock pool pointer pool lock
	}

	//sem_wait(&fut->taskComplete);
	return fut->result;
}

/* Deallocate this future. Must be called after future_get() */
void future_free(struct future * fut) {
	free(fut);	
}
