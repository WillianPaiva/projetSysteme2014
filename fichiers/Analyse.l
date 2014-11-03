%{
#include "Shell.h"
#include "y.tab.h"
#include <readline/history.h>
#include <readline/readline.h>


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


#define READLINE
#define MAX_CMD_LINE 1024

static char cmd_line[2][MAX_CMD_LINE];
static unsigned previous_index = 0, current_index = 1;

char cwd[1024];
char *user;

#ifdef READLINE
/* Support for the readline and history libraries.  This allows
   nicer input on the interactive part of input. */

/* Have input call the following function. */
#undef  YY_INPUT
#define YY_INPUT(buf,result,max_size) \
		rl_input((char *)buf, &result, max_size)

/* Variables to help interface readline with bc. */
static char *rl_line = (char *)NULL;
static char *rl_start = (char *)NULL;
static int   rl_len = 0;

/* Definitions for readline access. */
extern FILE *rl_instream;

/* rl_input puts upto MAX characters into BUF with the number put in
   BUF placed in *RESULT.  If the yy input file is the same as
   rl_instream (stdin), use readline.  Otherwise, just read it.
*/

static void
rl_input (buf, result, max)
	char *buf;
	int  *result;
	int   max;
{

  /*if (yyin != rl_instream)*/
    /*{*/
		/*printf("---------------->test<---------------");*/
      /*while ( (*result = read( fileno(yyin), buf, max )) < 0 )*/
        /*if (errno != EINTR)*/
	  /*{*/
		/*yyerror( "read() in flex scanner failed" );*/
		/*exit (1);*/
	  /*}*/
      /*return;*/
    /*}*/

  /* Do we need a new string? */
  if (rl_len == 0)
    {
		getcwd(cwd, sizeof(cwd));
		if(getuid()==0){
			user = "#";
		}else{
			user = "$";
		}
		char ps1[1024]=" " ;
		strcat(ps1,"\x1b[32m");
		strcat(ps1,cwd);
		strcat(ps1," ");
		strcat(ps1,"\x1b[34m");
		strcat(ps1,user);
		strcat(ps1,"\x1b[31m");
		strcat(ps1,">>");
		strcat(ps1,"\x1b[0m");

      if (rl_start)
	free(rl_start);
      rl_start = readline(ps1);
      if (rl_start == NULL) {
	/* end of file */
	*result = 0;
	rl_len = 0;
	return;
      }
      rl_line = rl_start;
      rl_len = strlen(rl_line)+1;
      if (rl_len != 1)
	add_history(rl_line); 
      rl_line[rl_len-1] = '\n';
      fflush(stdout);
    }

  if (rl_len <= max)
    {
      strncpy(buf, rl_line, rl_len);
      *result = rl_len;
      rl_len = 0;
    }
  else
    {
      strncpy(buf, rl_line, max);
      *result = max;
      rl_line += max;
      rl_len -= max;
    }
}
#endif




#define _CAT strcat(cmd_line[current_index], yytext)
#define _EOC \
   do { \
      previous_index = 1 - previous_index; \
      current_index = 1 - current_index; \
      reset_command_line(); \
   } while(0)






char *previous_command_line(void)
{
   return cmd_line[previous_index];
}

void reset_command_line(void)
{
   cmd_line[current_index][0] = '\0';
}

%}

ID	([-.$/\\*?A-Za-z0-9]+)

%%

[ \t]+			_CAT;
^[ \t]*\n		_CAT;
{ID}|\"{ID}\"|\'{ID}\' {
  _CAT;
  strcpy (yylval.identificateur, yytext);
  return IDENTIFICATEUR;
  }
\<			_CAT; return IN;
\>			_CAT; return OUT;
"2>"			_CAT; return ERR;
"&>"			_CAT; return ERR_OUT;
">>"			_CAT; return OUT_APPEND;
"||"			_CAT; return OU;
"&&"			_CAT; return ET;
<<EOF>>			EndOfFile();
\n			_EOC; return yytext[0];
.                       _CAT; return yytext[0];
