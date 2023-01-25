# Title: Lab 2 - Signals and Processes
## Status:
Both programs meet all criterion outlined in the lab specification. 
The behaviour is as expected. For part A, sending a SIGUSR1 signal to
the process causes the signal handler to be invoked, thus altering
the condvar usr1Happened, and unblocking the while loop. For part B,
the program creates the appropriate amount of children processes based
upon the number entered by the user on the command line. Each child can
be terminated by sending a SIGUSR1 according to the PID entered from the
command line. Once all children have died, the parent process exits. No
zombie processes remain.
## Known Issues: N/A
## Expected Grade: A+ 