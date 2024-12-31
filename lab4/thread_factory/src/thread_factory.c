#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <sched.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/procmgr.h>

#define TR_LEN (2+2) /* +1 for \n, +1 for NULL terminator */
#define BASE 10
#define SEM_VAL (unsigned)0
#define SEM_NAME "tf_sem"

/* Static variables */
static sem_t *psem = NULL;
static pthread_t *thread_pool = NULL;
static int num_threads;

/* Function prototypes */
void sig_hndlr(int);
void* tr_func(void*);

void* tr_func(void *arg)
{
   while (true)
   {
      /* Semaphore loop */
      while ((sem_wait(psem) == -1) && (errno == EINTR)) ;; /* Blocked waiting for semaphore */

      /* Another thread decremented the semaphore */
      printf("Thread %d has woken\n", gettid());
      sleep(5); /* Sleep for 5 seconds */
   }

   return NULL; /* Unreachable */
}

void sig_hndlr(int signo)
{
   printf("Process received SIGUSR1\n");
   if (num_threads > 0)
   {
      for (int i = 0; i < num_threads; i++)
      {
         pthread_cancel(thread_pool[i]);
      }
   }

   sem_unlink(SEM_NAME);
   if (thread_pool)
      free(thread_pool);
   exit(EXIT_SUCCESS);
}

int main(void)
{
   /* Declare variables */
   char usr_in[TR_LEN];
   struct stat mode;
   struct sched_param sched_param;
   pthread_attr_t global_tr_attr;
   pthread_t thread;

   printf("Starting process. PID: %d\n", getpid());

   /* Ensure we have the correct permissions */
   if (procmgr_ability(0, (PROCMGR_ADN_ROOT | PROCMGR_AOP_ALLOW | PROCMGR_AID_PRIORITY), PROCMGR_AID_EOL) != EOK)
   {
      fprintf(stderr, "This process does not have the proper permissions to be executed\n");
      exit(EXIT_FAILURE);
   }

   /* Initialize thread attribute struct */
   sched_param.sched_priority = 10;
   pthread_attr_init(&global_tr_attr);
   pthread_attr_setschedparam(&global_tr_attr, &sched_param);
   pthread_attr_setinheritsched(&global_tr_attr, PTHREAD_EXPLICIT_SCHED);
   pthread_attr_setschedpolicy(&global_tr_attr, SCHED_RR);

   /* Setup sigaction */
   const struct sigaction sig_act = { .sa_handler = sig_hndlr };
   if (sigaction(SIGUSR1, &sig_act, NULL) == -1)
   {
      perror("sigaction()");
      exit(EXIT_FAILURE);
   }

   /* Initialize semaphore */
   mode.st_mode = (S_IRWXO | S_IRWXU);
   assert(SEM_VAL < SEM_VALUE_MAX);
   psem = sem_open(SEM_NAME, O_CREAT, mode.st_mode, SEM_VAL);
   if (psem == SEM_FAILED)
   {
      perror("sem_open()");
      exit(EXIT_FAILURE);
   }

   /* Get user input */
   printf("Enter the number of threads: ");
   do
   {
      fflush(stdin);
      fgets(usr_in, TR_LEN, stdin);
      usr_in[TR_LEN - 1] = '\0';
   } while ((num_threads = (int) strtol(usr_in, NULL, BASE)) == 0);

   printf("Number of threads: %d\n", num_threads);

   /* Create threads */
   thread_pool = malloc(sizeof(pthread_t) * num_threads);
   if (thread_pool == NULL)
   {
      perror("malloc()");
      sem_unlink(SEM_NAME);
      exit(EXIT_FAILURE);
   }

   for (int i = 0; i < num_threads; i++)
   {
      pthread_create(&thread, &global_tr_attr, tr_func, NULL);
      thread_pool[i] = thread;
      printf("Thread %d created\n", thread_pool[i]);
   }

   while (true) ;; /* Don't let main thread exit */
}
