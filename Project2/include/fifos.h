#define PERM_FILE 0777

int counter;
//create unique named pipe for communication of the monitor with the Travel
void named_pipe_init(bool setup,char *read_p, char *write_p);
