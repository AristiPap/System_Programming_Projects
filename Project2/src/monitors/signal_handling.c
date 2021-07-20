
#include "../../include/header.h"
#include "../../include/dir_queries.h"
#include "../../include/VaccinMonitor_Utilities.h"
#include "../../include/signal_handling.h"

//if signals 0 then signal pending
//if signals 1 the opposite.
static volatile sig_atomic_t intquit;
static volatile sig_atomic_t usr1;

static sigset_t ccode_set;  // Mask with blocked signals during a command process


//handler functions for the sa_handler 
void handler_int_quit(int signo) {
  intquit = 1;
}

void handler_usr1(int signo) {
  usr1 = 1;
}

// Unblock signals INT, QUIT, USR1 .
void signals_unblock() {
  sigprocmask(SIG_UNBLOCK, &ccode_set, NULL);
}

// Block signals INT, QUIT, USR1 .
void signals_block() {
  sigprocmask(SIG_BLOCK, &ccode_set, NULL);
}

void signal_handler_init()
{
  struct sigaction sa;
  sigset_t ss;
  
  memset(&sa, 0, sizeof(struct sigaction));
  //Block signals till the configuration is done
  sigfillset(&ss);
  sigprocmask(SIG_SETMASK, &ss, NULL);

  intquit = 0;
  usr1 = 0;  

  sigfillset(&sa.sa_mask); 
  //Restart functions if interrupted by handler
  sa.sa_flags = SA_RESTART;
  //init handler that takes care of SIGUSR1 
  sa.sa_handler = handler_usr1;
  sigaction(SIGUSR1, &sa, NULL);
  
  //Block signals while processing
  sigemptyset(&ccode_set); 
  sigaddset(&ccode_set, SIGUSR1);
  sigemptyset(&ss);
  sigprocmask(SIG_SETMASK, &ss, NULL); 
}

// Check if signals are pending & handle them.
void checkusr1(int write_fd,char *input_dir,int bufferSize,List files_read,char *country_name)
{
  //if monitor received SIGUSR1 signal then find new file to read
  if (usr1) 
    checkForNewFiles_inCountry(write_fd,input_dir,bufferSize,files_read,country_name);

  intquit = 0;
  usr1 = 0;
}
