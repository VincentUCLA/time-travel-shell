// UCLA CS 111 Lab 1 command reading
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "command.h"
#include "command-internals.h"
#define END_OF_FILE "#EOF#"
#define NEW_CMDSTREAM "#NEW_CMDSTREAM#"

bool
boolWord(char c){
  if (   isalpha(c)
    || isdigit(c)
    || c == '!'
    || c == '%'
    || c == '+'
    || c == ','
    || c == '-'
    || c == '.'
    || c == '/'
    || c == ':'
    || c == '@'
    || c == '^'
    || c == '_')
    return true;
  else
    return false;
}

bool
boolBlank(char c){
  switch(c){
  case ' ':
  case '\t':
    return true;
    break;
  default:
    return false;
  }
}

bool
singleToken(token_type c){
  switch(c){
  case PIPELINES:
  case AND_LOGIC:
  case OR_LOGIC:
  case SEMICOLON:
  case LEFT_PARENT:
  case RIGHT_PARENT:
  case LEFT_ARROW:
  case RIGHT_ARROW:
  case BACKSLASH:
  case COMMENT:
  case NEWLINE:
  case ENDOFFILE:
    return true;
    break;
  default:
    return false;
  }
}

token_type
tokenType(char c){
  if (boolWord(c))
    return WORD;
  else if (boolBlank(c))
    return BLANK;
  else
    switch(c)
    {
    case '|':
      return OR_LOGIC;
    case '&':
      return AND_LOGIC;
    case ';':
      return SEMICOLON;
    case '<':
      return LEFT_ARROW;
    case '>':
      return RIGHT_ARROW;
    case '(':
      return LEFT_PARENT;
    case ')':
      return RIGHT_PARENT;
    case '\\':
      return BACKSLASH;
    case '#':
      return COMMENT;
    case '\n':
      return NEWLINE;
    default:
      return UNDEFINED;
    }
}

token_type
wordType(char* c){
  if (*c == '|' && *(c+1) == '|' && *(c+2) == '\0')
    return OR_LOGIC;
  else if (*c == '|' && *(c+1) == '\0')
    return PIPELINES;
  else if (strcmp(c, END_OF_FILE) == 0)
    return ENDOFFILE;
  else
    return tokenType(*c);
}

bool
judgeOptr(token_type source){
  if (   source == PIPELINES
    || source == LEFT_PARENT
    || source == RIGHT_PARENT
    || source == AND_LOGIC
    || source == OR_LOGIC)
    return true;
  else
    return false;
}

//  TOKENIZER
stack*
tokenize( int (*get_next_byte) (void *),
      void *get_next_byte_argument)
{
  //char* curStr;
  stack* ret = (stack*) malloc (sizeof(stack));
  stack_init(ret);
  char curChar;
  bool prevWord = false;
  int tokenStart = 0, tokenEnd = 0, index = 0;
  char* insert;
  char temp[1024];
  memset(temp, 0, sizeof(temp));
  while ((curChar = (char) (*get_next_byte) (get_next_byte_argument)) != EOF){
    token_type curType = tokenType(curChar);
    temp[index - tokenStart] = curChar;
    switch (curType){
    case BLANK:
    case AND_LOGIC:
    case OR_LOGIC:
    case SEMICOLON:
    case LEFT_ARROW:
    case RIGHT_ARROW:
    case LEFT_PARENT:
    case RIGHT_PARENT:
    case BACKSLASH:
    case COMMENT:
    case NEWLINE:
      tokenEnd = index;
      if(prevWord){
        insert = (char*) malloc(sizeof(char)*(tokenEnd-tokenStart+1));
        temp[tokenEnd-tokenStart] = '\0';
        strcpy(insert, temp);
        memset(temp, 0, sizeof(temp));
        stack_push(ret, insert);
      }
      prevWord = false;
      if (curType == BLANK){
        tokenStart = index+1;
        break;}
      insert = (char*) malloc (2*sizeof(char));
      insert[0] = curChar;
      insert[1] = '\0';
      tokenStart = index+1;
      stack_push(ret, insert);
      break;
    case WORD:
      prevWord = true;
      break;
    default:
      break;
    }
    index++;
  }
  insert = (char*) malloc(sizeof(char)*(index-tokenStart+1));
  temp[index-tokenStart] = '\0';
  strcpy(insert, temp);
  memset(temp, 0, sizeof(temp));
  stack_push(ret, insert);
  stack_push(ret, (char *)END_OF_FILE);
  return ret;
}

//  PREPROCESSOR
//  1a. combine adjacent 2 '|'s or '&'s into "||"s or "&&"s;
//  1b. delete comments from "#" to "\n" (not included);
void
rmComments(stack* source){
  if (source->next == NULL)
    return;
  stack* cur = source->next;
  char* str0 = (char*) cur->data;
  char* str1 = (char*) cur->next->data;
  char* str2 = (char*) cur->next->next->data;
  token_type type0 = tokenType(*str0);
  token_type type1 = tokenType(*str1);
  //token_type type2 = tokenType(str2);
  int i = 0, pound_index = -2, newline_index = 0;
  while (strcmp(str2, END_OF_FILE) != 0){
    // deal with "||", "&&", "|"
    if (  (type1 == OR_LOGIC    && type0 == OR_LOGIC  && strcmp(str0, "||") != 0)
         || (type1 == AND_LOGIC   && type0 == AND_LOGIC   && strcmp(str0, "&&") != 0)){
      stack_delete(source, i);
      i--;
      if (type1 == OR_LOGIC){
        char* newStr = "||";
        stack_change(source, i+1, newStr);
      }
      else{
        char* newStr = "&&";
        stack_change(source, i+1, newStr);
      }
    }

    // removal of COMMENTs
    if (type0 == COMMENT)
      pound_index = i-1;
    if (pound_index != -2
      && (type1 == NEWLINE || strcmp(str2, END_OF_FILE) == 0)){
      newline_index = i;
      if (strcmp(str2, END_OF_FILE) == 0)
        newline_index++;
      int j;
      for (j = 0; j<(newline_index - pound_index); j++){
        stack_delete(source, pound_index+1);
        i--;
      }
      pound_index = -2;
      newline_index = 0;
    }
    i++;
    cur = cur->next;
    str0 = (char*) cur->data;
    str1 = (char*) cur->next->data;
    str2 = (char*) cur->next->next->data;
    type0 = tokenType(*str0);
    type1 = tokenType(*str1);
    //type2 = tokenType(str2);
  }
  return;
}

/*  2a. "\" + "\n"            -> ' '
  2b. other than    "\n(" || "\n)"
           || "\n"+WORD   ->  !!!EXCEPTION!!!
  2c. ">\n" || "<\n"          ->  !!!EXCEPTION!!!
  2d. \n + (0+)BLANK + (1+)\n     ->  newStream   */
void
newLine(stack* source){
  int line_counter = 1;
  if (source->next == NULL)
    return;
  int i=0;
  for (i=0; i<source->size; i++){
    char* str0 = stack_data(source, i);
    char* str1 = stack_data(source, i+1);
    char* str2 = stack_data(source, i+2);
    token_type type0 = wordType(str0);
    token_type type1 = wordType(str1);
    token_type type2 = wordType(str2);
    if (type1 == NEWLINE){
      // 2a. "\" + "\n" -> ' '
      line_counter++;
      if (type0 == BACKSLASH){
        stack_delete(source, i);
        stack_delete(source, i);
        i--;
        str0 = stack_data(source, i);
        str1 = stack_data(source, i+1);
        str2 = stack_data(source, i+2);
        type0 = wordType(str0);
        type1 = wordType(str1);
        type2 = wordType(str2);
        continue;
      }

      // 2b. other than "\n(" || "\n)" || "\n"+WORD -> !!!EXCEPTION!!!
      else if (!(type2 == LEFT_PARENT || type2 == RIGHT_PARENT
          || type2 == WORD    || type2 == COMMENT
          || type2 == BLANK   || type2 == BACKSLASH
          || type2 == ENDOFFILE || type2 == NEWLINE || str2[0] == 0)){
        exception(line_counter);}
      // 2c. ">\n" || "<\n" -> !!!EXCEPTION!!!
      else if (type0 == LEFT_ARROW  || type0 == RIGHT_ARROW)
        exception(line_counter);
    }
    if (type1 == NEWLINE && type0 == NEWLINE){
      if (i>1){
        char* str_1 = stack_data(source, i-1);
        token_type type_1 = wordType(str_1);
        if (!(type_1 == AND_LOGIC || type_1 == OR_LOGIC || type_1 == PIPELINES)){
          char* insert = NEW_CMDSTREAM;
          stack_change(source, i, insert);
        }
      }
      else{
        char* insert = NEW_CMDSTREAM;
        stack_change(source, i, insert);
      }

      while(type1 == NEWLINE){
        stack_delete(source, i+1);
        type1 = wordType(stack_data(source, i+1));
      }
    }
    if (strcmp(str2, END_OF_FILE) == 0)
      break;
  }
  return;
}

//  3.  split the whole text in according to NEW_CMDSTREAM;
stack*
preprocessing(stack* source){
  rmComments(source);
  newLine(source);  
  // another step of preprocessing
  stack* using = (stack*) malloc (sizeof(stack));
  stack_init(using);
  int i, si = source->size;
  for (i = 0; i<si; i++){
    char* tempstr = stack_data(source, i);
    token_type type0 = wordType(tempstr);
    if (type0!=NEWLINE && tempstr[0]!=0)
      stack_push(using, tempstr);
    if (type0==ENDOFFILE)
      break;
  }
  // only 1 case
  stack* ret = (stack*) malloc (sizeof(stack));
  stack_init(ret);
  stack* insert = (stack*) malloc (sizeof(stack));
  stack_init(insert);
  int cmdStart = 0, cmdEnd = 0;
  for (i = 0; i<using->size; i++){
    char* str1 = stack_data(using, i);
    if ( strcmp(str1, END_OF_FILE) == 0 || strcmp(str1, NEW_CMDSTREAM) == 0 ){
      cmdEnd = i;
      int j;
      for (j = cmdStart; j<cmdEnd; j++){
        char* str0 = stack_data(using, j);
        //printf("%s ", str0);
        stack_push(insert, str0);
      }
      //printf("\n");
      cmdStart = i+1;
      stack_push(ret, insert);
      insert = (stack*) malloc (sizeof(stack));
      stack_init(insert);
    }
  }
  return ret;
}

// command_stream as a linked list (what we call as stack) of COMMANDs
void
command_init(struct command* object){
  object->type = SIMPLE_COMMAND;
  object->status = -1;
  object->input = NULL;
  object->output = NULL;
  object->u.command[0] = NULL;
  object->u.command[1] = NULL;
  object->u.word = NULL;
  object->u.subshell_command = NULL;
}

void
command_stream_init(command_stream_t object){
  object->cmdStack = (stack*) malloc (sizeof(stack));
  stack_init(object->cmdStack);
  object->depStack = (stack*) malloc (sizeof(stack));
  stack_init(object->depStack);
  object->cursor = 0;
}

int
precedenceOptr(token_type source){
  switch (source){
  case SEMICOLON:
  case NEWLINE:
    return 0;
    break;
  case AND_LOGIC:
  case OR_LOGIC:
    return 1;
    break;
  case PIPELINES:
    return 2;
    break;
  default:
    return -1;
  }
}

bool
isLetterOrCloseParen(token_type source){
  if ((precedenceOptr(source) == -1 && source != LEFT_PARENT) || source == RIGHT_PARENT)
    return true;
  else
    return false;
}

stack*
genPrototype(stack* source){
  stack_push(source, END_OF_FILE);
  int size = stack_size(source);
  //printf("%d\n", size);
  int i, cmdStart = 0, cmdEnd = 0;
  stack* ret = (stack*) malloc (sizeof(stack));
  stack_init(ret);
  stack* insert = (stack*) malloc (sizeof(stack));
  stack_init(insert);
  for (i=0; i<size; i++){
    char* str = (char*) stack_data(source, i);
    //printf("%s\n", str);
    token_type type = wordType(str);
    if (precedenceOptr(type) != -1 || strcmp(str, END_OF_FILE) == 0
        || type == LEFT_PARENT || type == RIGHT_PARENT){
      cmdEnd = i;
      int j;
      for(j=cmdStart; j<cmdEnd; j++){
        //printf("%s %d\n", stack_data(source, j), j);
        stack_push(insert, stack_data(source, j));
      }
      if (cmdEnd>cmdStart)
        stack_push(ret, insert);
      insert = (stack*) malloc (sizeof(stack));
      stack_init(insert);
      if (strcmp(stack_data(source, j), END_OF_FILE) != 0)
        stack_push(insert, stack_data(source, j));

      stack_push(ret, insert);
      insert = (stack*) malloc (sizeof(stack));
      stack_init(insert);
      cmdStart = i+1;
    }
    if (strcmp(stack_data(source, i), END_OF_FILE) == 0)
      break;
  }
  return ret;
}

command*
genSimpleCmd(stack* source, int lineCount){
  command* ret = (command*) malloc (sizeof(command));
  command_init(ret);
  int size = stack_size(source);
  int realSize = size;
  int i;
  for (i=0; i<size; i++){
    char* str = (char*) stack_data(source, i);
    token_type type = wordType(str);
    if (type == LEFT_ARROW || type == RIGHT_ARROW){
      if (!(i == size-2 || i == size-4))
        exception(lineCount);
      else{
        realSize = realSize - 2;
        char* redir = (char*) stack_data(source, i+1);
        if (type == LEFT_ARROW)
          ret->input = redir;
        else if (type == RIGHT_ARROW)
          ret->output = redir;
      }
    }
  }
  ret->u.word = (char**) malloc ((1+realSize)*sizeof(char*));
  for (i=0; i<realSize; i++){
    char* str = (char*) stack_data(source, i);
    ret->u.word[i] = str;
  }
  ret->u.word[i] = NULL;
  return ret;
}

command*
genOptrCmd(token_type operator, command* operand1, command* operand2){
  struct command* ret = (struct command*) malloc(sizeof(struct command));
  command_init(ret);
  switch(operator){
  case SEMICOLON:
    ret->type = SEQUENCE_COMMAND;
    break;
  case NEWLINE:
    ret->type = SEQUENCE_COMMAND;
    break;
  case AND_LOGIC:
    ret->type = AND_COMMAND;
    break;
  case OR_LOGIC:
    ret->type = OR_COMMAND;
    break;
  case PIPELINES:
    ret->type = PIPE_COMMAND;
    break;
  case LEFT_PARENT:
    ret->type = SUBSHELL_COMMAND;
    break;
  default:
    break;
  }
  if (operator == LEFT_PARENT)
    ret->u.subshell_command = operand1;
  else{
    ret->u.command[0] = operand1;
    ret->u.command[1] = operand2;
  }
  return ret;
}

void
dumpCmd(struct command* cmd, int blank){
  if (cmd->type == SIMPLE_COMMAND){
    int j;
    for (j = 0;;j++){
      if(cmd->u.word[j] == NULL)
        break;
      printf(" %s", cmd->u.word[j]);
    }
    printf("\n");
    return;
  }
  else if (cmd->type == SUBSHELL_COMMAND){
    dumpCmd(cmd->u.subshell_command, blank);
    return;
  }
  else{
    int k;
    switch(cmd->type){
    case AND_COMMAND:
      printf("(&&)\t|->");
      break;
    case SEQUENCE_COMMAND:
      printf("(;)\t|->");
      break;
    case OR_COMMAND:
      printf("(||)\t|->");
      break;
    case PIPE_COMMAND:
      printf("(|)\t|->");
      break;
    default:
      break;
    }
    dumpCmd(cmd->u.command[0], blank+1);
    for(k=0; k<blank; k++)
      printf("|\t");
    printf("|\t|->");
    dumpCmd(cmd->u.command[1], blank+1);
    return;
  }
}

command*
genCmd(stack* source){
  stack* command = (stack*) malloc (sizeof(stack));
  stack_init(command);
  stack* operator = (stack*) malloc (sizeof(stack));
  stack_init(operator);
  stack* temp = genPrototype(source);
  int i;
  int size = stack_size(temp)-1;
  int lineCount = 1;
  token_type type;
  token_type prevType;
  for (i=0; i<size; i++){
    stack* prototype = (stack*) stack_data(temp, i);
    char* str = (char*) stack_data(prototype, 0);
    type = wordType(str);
    switch (type){
    //  1. if a simple command; push it on to command stack
    case WORD:
      if (i>0 && isLetterOrCloseParen(prevType))
        exception(lineCount);
      struct command* simpleCmd = (struct command*) malloc(sizeof(command));
      simpleCmd = genSimpleCmd(prototype, lineCount);
      stack_push(command, simpleCmd);
      break;
    //  2. if "(", push into operator stack
    case LEFT_PARENT:
      if (i>0 && isLetterOrCloseParen(prevType))
        exception(lineCount);
      stack_push(operator, str);
      break;
    //  3. if an operator and operator stack is empty
    //     a. push operator onto operator stack
    //  4. if an operator and stack not empty
    //     a. pop all operators with >= precedence off operator stack
    //        + for each operator, pop 2 commands off command stacks
    //        + combine into new command, push it onto command stack
    //     b. stop when reach an operator with lower precedence or a "("
    //     c. push new operator onto operator stack
    case SEMICOLON:
    case NEWLINE:
    case AND_LOGIC:
    case OR_LOGIC:
    case PIPELINES:
      if (type == NEWLINE)
        lineCount++;
      if (i>0 && !isLetterOrCloseParen(prevType))
        exception(lineCount);
      if (stack_size(operator) == 0)
        stack_push(operator, str);
      else{
        char* top = (char*) stack_top(operator);
        token_type topType = wordType(top);
        while ((precedenceOptr(topType) >= precedenceOptr(type))
          && (topType != LEFT_PARENT)
          && (stack_size(operator)>0)){
          if (topType == NEWLINE)
            lineCount--;
          stack_pop(operator, lineCount);
          struct command* operand2 = (struct command*) stack_top(command);
          stack_pop(command, lineCount);
          struct command* operand1 = (struct command*) stack_top(command);
          stack_pop(command, lineCount);
          struct command* newCmd = genOptrCmd(topType, operand1, operand2);
          stack_push(command, newCmd);
          if(stack_size(operator) >= 0){
            top = (char*) stack_top(operator);
            topType = wordType(top);
          }
          else
            break;
        }
        stack_push(operator, str);
      }
      break;
    //  5. if encounter ")", pop operators off stack like 4a until see a matching "("
    //     + create a sub shell command by popping top command from command stack
    //     + push new command to command stack
    case RIGHT_PARENT:
      if (i>0 && !isLetterOrCloseParen(prevType))
        exception(lineCount);
      char* top = (char*) stack_top(operator);
      token_type topType = wordType(top);
      while (topType != LEFT_PARENT){
        if (topType == NEWLINE)
          lineCount--;
        stack_pop(operator, lineCount);
        struct command* operand2 = (struct command*) stack_top(command);
        stack_pop(command, lineCount);
        struct command* operand1 = (struct command*) stack_top(command);
        stack_pop(command, lineCount);
        struct command* newCmd = genOptrCmd(topType, operand1, operand2);
        stack_push(command, newCmd);
        if(stack_size(operator) >= 0){
          top = (char*) stack_top(operator);
          topType = wordType(top);
        }
        else
          break;
      }
      struct command* operand = (struct command*) stack_top(command);
      stack_pop(command, lineCount);
      struct command* newCmd = genOptrCmd(LEFT_PARENT, operand, NULL);
      stack_push(command, newCmd);
      stack_pop(operator, lineCount);
      break;
    default:
      break;
    }
    prevType = type;
    //printf("%d\t%d\n", stack_size(command), stack_size(operator));
  }
  //  7. nothing left, pop remaining operators like 4a
  if (stack_size(operator)>0){
    char* top = (char*) stack_top(operator);
    token_type topType = wordType(top);
    while (stack_size(operator)>0){
      if (topType == NEWLINE)
        lineCount--;
      stack_pop(operator, lineCount);
      struct command* operand2 = (struct command*) stack_top(command);
      stack_pop(command, lineCount);
      struct command* operand1 = (struct command*) stack_top(command);
      stack_pop(command, lineCount);
      struct command* newCmd = genOptrCmd(topType, operand1, operand2);
      stack_push(command, newCmd);
      if(stack_size(operator) >= 0){
        top = (char*) stack_top(operator);
        topType = wordType(top);
      }
      else
        break;
    }
  }
  return stack_top(command);
}

stack*
depStkGen(command_t c){
  stack* ret = (stack*) malloc (sizeof(stack));
  stack_init(ret);
  stack* sstemp = (stack*) malloc (sizeof(stack));
  stack* temp0 = (stack*) malloc (sizeof(stack));
  stack* temp1 = (stack*) malloc (sizeof(stack));
  stack_init(sstemp);
  stack_init(temp0);
  stack_init(temp1);
  int i;

  switch (c->type){
  case SUBSHELL_COMMAND:
    sstemp = depStkGen(c->u.subshell_command);
  case SIMPLE_COMMAND:
    if (c->input)
      stack_push(ret, c->input);
    if (c->output)
      stack_push(ret, c->output);
    if (stack_size(sstemp) != 0)
      for (i = 0; i<stack_size(sstemp); i++)
        stack_push(ret, stack_data(sstemp, i));
    break;
  case AND_COMMAND:
  case OR_COMMAND:
  case PIPE_COMMAND:
  case SEQUENCE_COMMAND:
    temp0 = depStkGen(c->u.command[0]);
    temp1 = depStkGen(c->u.command[1]);
    if (stack_size(temp0) != 0)
      for (i = 0; i<stack_size(temp0); i++)
        stack_push(ret, stack_data(temp0, i));
    if (stack_size(temp1) != 0)
      for (i = 0; i<stack_size(temp1); i++)
        stack_push(ret, stack_data(temp1, i));
    break;
  default:
    exception(0);
    break;
  }
  return ret;
}

int
depTest (stack* stk1, stack* stk2){
  int i, j;
  for (i = 0; i<stack_size(stk1); i++)
    for (j = 0; j<stack_size(stk2); j++)
      if (strcmp(stack_data(stk1, i), stack_data(stk2, j)) == 0)
        return 1;
  return 0;
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
          void *get_next_byte_argument)
{
  int i;
  command_stream_t ret = (command_stream_t) malloc(sizeof(command_stream));
  command_stream_init(ret);
  stack* tokenList = (stack*) malloc (sizeof(stack));
  stack_init(tokenList);
  tokenList = tokenize(get_next_byte, get_next_byte_argument);
  
  stack* cmdList = (stack*) malloc (sizeof(stack));
  stack_init(cmdList);
  cmdList = preprocessing(tokenList);

  for (i=0; i<cmdList->size; i++){
    stack* stk = (stack*) stack_data(cmdList, i);
    //stack* temp = genPrototype(stk);
    command* newCmd = genCmd(stk);
    //dumpCmd(newCmd, 0);
    stack_push(ret->cmdStack, newCmd);
    stack* stkDep;
    stkDep = depStkGen(newCmd);
    //printf("%d\n", stack_size(stkDep));
    stack_push(ret->depStack, stkDep);
  }
  return ret;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  if (stack_size(s->cmdStack)==0)
    return NULL;
  if (s->cursor < stack_size(s->cmdStack)){
    s->cursor++;
    return stack_data(s->cmdStack, (s->cursor-1));
  }
  else
    return NULL;
}
