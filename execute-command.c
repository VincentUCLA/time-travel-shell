// UCLA CS 111 Lab 1 command execution
#include "alloc.h"
#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int
command_status (command_t c)
{
  return c->status;
}

void execute_simple_command(command_t c){
  pid_t child = fork();
  if (child == 0)
  {
    if (c->input)
    {
      int in;
      if ((in = open(c->input, O_RDONLY, 0666)) == -1)
        error(3, 0, "Cannot open input file.");
      if (dup2(in, 0) == -1)
        error(3, 0, "Cannot perform input redirect.");
      close(in);
    }
    if (c->output)
    {
      int out;
      if ((out = open(c->output, O_WRONLY|O_CREAT|O_TRUNC, 0666)) == -1)
        error(3, 0, "Cannot open output file.");
      if (dup2(out, 1) == -1)
        error(3, 0, "Cannot perform output redirect.");
      close(out);
    }
    execvp(c->u.word[0], c->u.word);
    error(3, 0, "Cannot execute command.");
  }
  else if (child > 0)
  {
    int status;
    waitpid(child, &status, 0);
    c->status = status;
  }
  else
    error(3, 0, "Cannot create child process.");
}

void
execute_command (command_t c, bool time_travel)
{
  pid_t child;
  int fd[2];
  int r = 0;
  int w = 1;

  switch (c->type)
  {
    case SIMPLE_COMMAND:
      execute_simple_command(c);
      break;
    case AND_COMMAND:
      execute_command(c->u.command[0], time_travel);
      execute_command(c->u.command[1], time_travel);
      c->status = !(!c->u.command[0]->status && !c->u.command[1]->status);
      break;
    case OR_COMMAND:
      execute_command(c->u.command[0], time_travel);
      execute_command(c->u.command[1], time_travel);
      c->status = !(!c->u.command[0]->status || !c->u.command[1]->status);
      break;
    case SUBSHELL_COMMAND:
      execute_command(c->u.subshell_command, time_travel);
      c->status = c->u.subshell_command->status;
      break;
    case SEQUENCE_COMMAND:
      if (time_travel && !(depTest(depStkGen(c->u.command[0]), depStkGen(c->u.command[1]))))
      {
        child = fork();
        if (child == 0)
        {
          execute_command(c->u.command[0], time_travel);
          exit(0);
        }
        if (child > 0)
          execute_command(c->u.command[1], time_travel);
        else
          error(3, 0, "Cannot create child process.");
        waitpid(child, NULL, 0);
      }
      else
      {
        execute_command(c->u.command[0], time_travel);
        execute_command(c->u.command[1], time_travel);
      }
      c->status = c->u.command[1]->status;
      break;
    case PIPE_COMMAND:
      if (pipe(fd) == -1)
        error(3, 0, "Cannot create pipe.");
      child = fork();
      if (child == 0)
      {
        close(fd[r]);
        if (dup2(fd[w], w) == -1)
          error(3, 0, "Cannot write to pipe.");
        execute_command(c->u.command[0], time_travel);
        c->status = c->u.command[0]->status;
        close(fd[w]);
        exit(0);
      }
      else if (child > 0)
      {
        int status;
        waitpid(child, &status, 0);
        close(fd[w]);
        if (dup2(fd[r], r) == -1)
          error(3, 0, "Cannot read from pipe.");
        execute_command(c->u.command[1], time_travel);
        c->status = c->u.command[1]->status;
        close(fd[r]);
      }
      else
        error(3, 0, "Cannot create child process.");
      break;
    default:
        error(3, 0, "Command type not recognized.");
      break;
  }
  return ;
}

int 
execute_time_travel (command_stream_t cmdStream)
{
  int i;
  int j;
  int runnable = 0;
  stack* list = (stack*) malloc (sizeof(stack));
  stack_init(list);
  stack* depList = (stack*) malloc (sizeof(stack));
  stack_init(depList);
  for (i = 0; i<stack_size(cmdStream->cmdStack); i++)
  {
  	if (stack_size(list) == 0){
  	  stack_push(list, stack_data(cmdStream->cmdStack, i));
  	  stack_push(depList, stack_data(cmdStream->depStack, i));
  	  runnable = 1;
  	  continue;
  	}
  	else{
  	  int has_depends = 0;
  	  for (j = 0; j<stack_size(list); j++){
  	  	if (depTest(stack_data(depList, j), stack_data(cmdStream->depStack, i))){
  	  	  has_depends = 1;
  	  	  break;
  	  	}
  	  }
  	  if (!has_depends){
	  	stack_push(list, stack_data(cmdStream->cmdStack, i));
	  	stack_push(depList, stack_data(cmdStream->depStack, i));
  	  	runnable++;
  	  }
  	}
  }

  // generate a list of non-time-traveled commands
  stack* complementlist = (stack*) malloc (sizeof(stack));
  stack_init(complementlist);
  for (i = 0; i<stack_size(cmdStream->cmdStack); i++){
  	bool contained = false;
  	for (j = 0; j<stack_size(depList); j++)
  	  if (depTest(stack_data(cmdStream->depStack, j), stack_data(depList, j)))
  	  	contained = true;
  	if (!contained)
  	  stack_push(complementlist, stack_data(cmdStream->cmdStack, i));
  }

  pid_t* children = checked_malloc(runnable * sizeof(pid_t));
  j = 0;
  if (runnable > 0){
	  for (i = 0; i<stack_size(list); i++){
	  	command_t curr = stack_data(list, i);
	  	pid_t child = fork();
	  	if (!child){
	  		execute_command(curr, 1);
	  		exit(0);
	  	}
	  	else if (child>0)
	  		children[j] = child;
	  	else
	  		error(3, 0, "Cannot create child process.");
	  	j++;
	  }

	  int waiting;
	  do {
	    waiting = 0;
	    int k;
	    for (k = 0; k < runnable; k++)
	    {
	      if (children[k] > 0)
	      {
	        if (waitpid(children[k], NULL, 0) != 0)
	          children[k] = 0;
	        else
	          waiting = 1;
	      }
	      sleep (0);
	    }
	  } while (waiting == 1);
  }

  for (i = 0; i<stack_size(complementlist); i++)
  	execute_command(stack_data(complementlist, i), 1);
  return 0;
}
