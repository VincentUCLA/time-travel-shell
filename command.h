// UCLA CS 111 Lab 1 command interface

#include <stdbool.h>
#include "stack.h"

typedef struct command *command_t;
typedef struct command_stream *command_stream_t;

typedef enum token_type
{
  WORD,

  // operators
  PIPELINES,
  AND_LOGIC,
  OR_LOGIC,
  SEMICOLON,

  // redirections
  LEFT_ARROW,
  RIGHT_ARROW,

  // subshell
  LEFT_PARENT,
  RIGHT_PARENT,

  // misc
  BACKSLASH,
  COMMENT,
  NEWLINE,
  BLANK,
  ENDOFFILE,

  UNDEFINED
} token_type;

typedef struct command_stream{
  stack* cmdStack;
  stack* depStack;
  int cursor;
} command_stream;

/* Create a command stream from GETBYTE and ARG.  A reader of
   the command stream will invoke GETBYTE (ARG) to get the next byte.
   GETBYTE will return the next input byte, or a negative number
   (setting errno) on failure.  */
command_stream_t make_command_stream (int (*getbyte) (void *), void *arg);

/* Read a command from STREAM; return it, or NULL on EOF.  If there is
   an error, report the error and exit instead of returning.  */
command_t read_command_stream (command_stream_t stream);

/* Print a command to stdout, for debugging.  */
void print_command (command_t);

/* Execute a command.  Use "time travel" if the flag is set.  */
void execute_command (command_t, bool);

/* Return the exit status of a command, which must have previously
   been executed.  Wait for the command, if it is not already finished.  */
int command_status (command_t);
int execute_time_travel(command_stream_t);
int depTest (stack*, stack*);
stack* depStkGen(command_t);