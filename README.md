# Read Write Locks based on condition variable
- This is my custom implementation of read-write locks using `pthread_t condvar_t`
- I have defined `struct rwlock` which defines the read-write lock
- The key characteristic of read write locks is that multiple people can read the data at once  
