Title: A Metronome Resource Manager

Authors: Mohit Nargotra and Neil Kingdom

Mohit's Work:
   Responsible for doing the io_read handler and the logic for the seconds
   per interval function. Also helped with testing.

Neil's Work:
   Responsible for main() and the io_write handler, as well as other basic 
   structs. Wrote the shell script and performed testing.

Status:
   The project handles all expected cases as described in the assignment
   instructions. The seconds per interval are calculated properly and 
   stored in an mtm_attr_t struct. The API functions such as set, pause,
   start, stop, and quit, all perform their intended actions. The pause,
   start and stop commands send pulses to the metronome server, which 
   tells it to update the timer accordingly. The quit command closes all
   connections, joins the metonome thread, and cleans up any additional
   resources before exiting gracefully. The set command updates the 
   metronome in real time to match the new bpm and time signature.

Known Issues:
   There are no known issues. The only thing worth mentioning is that 
   the compiler issues two warnings for the io_read and io_write 
   overrides in main(). This is becaues we have overridden the default
   iofunc_attr_t struct. QNX supports doing this, and provides 
   documentation on how it can be accomplished. So long as the first
   member of the overridden struct (mtm_attr_t in our case) is of type
   iofunc_attr_t, there should not be any issues.

Expected Grade: A+
