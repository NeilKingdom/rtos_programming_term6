Title: Lab 4 - Exploring Threads in QNX

Status:
Lab 4 is complete. The thread_factory and thread_waker projects both work
as intended. Thread_factory creates the number of threads that the user
specifies from the command line, and then thread_waker will unblock a 
number of threads specified by the user using named semaphores. 
Thread_factory can be terminated from a shell using kill -s SIGUSR1 <pid>.
Thread_waker can be terminated by entering 0 or q when prompted.

Known Issues: N/A

Expected Grade: A+
