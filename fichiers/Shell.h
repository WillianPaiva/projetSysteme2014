#ifndef ANALYSE
#define ANALYSE


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <pwd.h>


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


#define NB_ARGS 50
#define TAILLE_ID 500

typedef enum expr_t {
  VIDE,	         		// Commande vide 
  SIMPLE,        		// Commande simple 
  SEQUENCE,      		// Séquence (;) 
  SEQUENCE_ET,   		// Séquence conditionnelle (&&) 
  SEQUENCE_OU,   		// Séquence conditionnelle (||) 
  BG,	         		// Tache en arriere plan 
  PIPE,	         		// Pipe 
  REDIRECTION_I, 		// Redirection entree 
  REDIRECTION_O, 		// Redirection sortie standard 
  REDIRECTION_A, 		// Redirection sortie standard, mode append 
  REDIRECTION_E, 		// Redirection sortie erreur 
  REDIRECTION_EO,		// Redirection sorties erreur et standard 
} expr_t;

typedef struct Expression {
  expr_t type;
  struct Expression *gauche;
  struct Expression *droite;
  char   **arguments;
} Expression;

typedef struct job {
	int id;
	char *name;
	pid_t pid;
	int status;
	int futurewait;
	struct job *next;
} t_job;


char cwd[1024];
char *user;


static t_job* jobsList = NULL;
static int numActiveJobs = 0;




static char *get_pwd(char *str)
{
  static char buffer[4096];
  char *p;
  char *orig = getenv("HOME");
  char *rep = "~";

  if(!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
    return str;

  strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  return buffer;
}

static char *getUserName()
{
  uid_t uid = geteuid();
  struct passwd *pw = getpwuid(uid);
  if (pw)
  {
    return pw->pw_name;
  }

  return "";
}




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

static void rl_input (buf, result, max)
	char *buf;
	int  *result;
	int   max;
{

  if (rl_len == 0)
    {
		getcwd(cwd, sizeof(cwd));
        char *cwd2 = get_pwd(cwd);
		if(getuid()==0){
			user = "#";
		}else{
			user = "$";
		}
        char hostname[1024];
        hostname[1023] = '\0';
        gethostname(hostname, 1023);


		char ps1[1024]=" " ;
		strcat(ps1,"\x1b[33m");
		strcat(ps1,"[");
		strcat(ps1,"\x1b[36m");
		strcat(ps1,getUserName());
		strcat(ps1,"\x1b[33m");
		strcat(ps1,"@");
		strcat(ps1,"\x1b[34m");
		strcat(ps1,hostname);
		strcat(ps1,"\x1b[33m");
		strcat(ps1,"]");
		strcat(ps1,"\x1b[32m");
		strcat(ps1,"  ");
		strcat(ps1,cwd2);
		strcat(ps1," ");
		strcat(ps1,"\x1b[34m");
		strcat(ps1,user);
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



int yyparse(void);
extern char *previous_command_line(void);
extern void reset_command_line(void);
Expression *ConstruireNoeud (expr_t, Expression *, Expression *, char **);
char **AjouterArg (char **, char *);
char **InitialiserListeArguments (void);
int LongueurListe(char **);
void EndOfFile(void);
int execute(Expression *e , int wait, int fdin,int fdout,int fderror, int lastflag, int futurewait);
void ch_lastfd(int max);
void addjob(int job);
void printjobs();











int builtincommands(Expression *e);
void printJobs();


void waitJob(t_job* job);

void killJob(int jobId);

t_job* insertJob(pid_t pid,char* name,int status, int futurewait);

t_job* getJob(int searchValue, int searchParameter);

void putJobForeground(t_job* job, int continueJob);


void putJobBackground(t_job* job, int continueJob);

t_job* delJob(t_job* job);
int changeJobStatus(int pid, int status);


void yyerror (char *s);
Expression *ExpressionAnalysee;

#endif /* ANALYSE */
