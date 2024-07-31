#include<pthread.h>
struct rwlock {
	int readers;
	int writers;
	int read_waiters;
	int write_waiters;
	pthread_mutex_t lock;
	pthread_cond_t read_condvar;
	pthread_cond_t write_condvar;
};

void rwlock_init(struct rwlock * rwlock){
	pthread_mutex_init(&rwlock->lock, NULL);
	pthread_cond_init(&rwlock->read_condvar, NULL);
	pthread_cond_init(&rwlock->write_condvar, NULL);
	rwlock->readers = 0;
	rwlock->writers = 0;
	rwlock->read_waiters = 0;
	rwlock->write_waiters = 0;
}

int rwlock_rdlock_lock(struct rwlock* rwlock){
	// if there are no writers, we can proceed normally
	pthread_mutex_lock(&rwlock->lock);
	if(rwlock->writers == 0){
		rwlock->readers++; // simply increment the number of readers.
	} else {
		// wait till the signal on read_condvar given by writers
		rwlock->read_waiters++;
		while(pthread_cond_wait(&rwlock->read_condvar, &rwlock->lock) != 0);
		rwlock->read_waiters--;
		// can increment readers now
		rwlock->readers++; 
	}
	return pthread_mutex_unlock(&rwlock->lock);
}
int rwlock_wrlock_lock(struct rwlock* rwlock){
	pthread_mutex_lock(&rwlock->lock);
	if(rwlock->readers == 0 && rwlock->writers == 0){
		rwlock->writers = 1;
	} else {
		// either some people are reading, then we must wait for the signal on write_cond var
		// or there is a writer, even then we must wait for the signal on write_cond var
		// in either case wait for the signal on write_cond var
		rwlock->write_waiters++;
		while(pthread_cond_wait(&rwlock->write_condvar, &rwlock->lock) != 0);
		rwlock->write_waiters--;
		rwlock->writers = 1;
		// we have obtained the lock now
	}
	pthread_mutex_unlock(&rwlock->lock);
};
int rwlock_rdlock_unlock(struct rwlock* rwlock){
	pthread_mutex_lock(&rwlock->lock);
	if(rwlock->readers > 1) rwlock->readers--;
	else if(rwlock->readers == 1) {
		rwlock->readers = 0;
		// people might be waiting for us. Let's give priority to readers
		if(rwlock->read_waiters > 0){
			pthread_cond_broadcast(&rwlock->read_condvar);
		} else if(rwlock->write_waiters > 0){
			pthread_cond_broadcast(&rwlock->write_condvar);
		}
	}
	return pthread_mutex_unlock(&rwlock->lock);
};

int rwlock_wrlock_unlock(struct rwlock* rwlock){
	pthread_mutex_lock(&rwlock->lock);
	rwlock->writers = 0;
	if(rwlock->read_waiters > 0){
		pthread_cond_broadcast(&rwlock->read_condvar);
	} else if(rwlock->write_waiters > 0){
		pthread_cond_broadcast(&rwlock->write_condvar);
	}
	pthread_mutex_unlock(&rwlock->lock);
}