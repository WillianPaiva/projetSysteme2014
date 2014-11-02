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

static t_job* jobsList = NULL;
static int numActiveJobs = 0;




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









 
 
//void append(int num); 
//void add( int num );
//void addafter(int num, int loc) ; 
//void insert(int num) ; 
//int delete(int num);  
//void  display(); 
//int count();


void yyerror (char *s);
Expression *ExpressionAnalysee;

#endif /* ANALYSE */
