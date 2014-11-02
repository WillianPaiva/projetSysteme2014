/* Construction des arbres représentant des commandes */
#include <argz.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Shell.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"



int lastfd;
char cwd[1024];
char user;

struct node
{
	int data;
	struct node *next;
}*head;


struct node *jobs;


/*
 * Construit une expression à partir de sous-expressions
 */


Expression *ConstruireNoeud (expr_t type, Expression *g, Expression *d, char **args)
{
	Expression *e;

	if ((e = (Expression *)malloc(sizeof(Expression))) == NULL){
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	e->type   = type;
	e->gauche = g;
	e->droite = d;
	e->arguments = args;
	return e;
} /* ConstruireNoeud */







/*
 * Renvoie la longueur d'une liste d'arguments
 */
int LongueurListe(char **l)
{
	char **p;
	for (p=l; *p != NULL; p++)
		;
	return p-l;
} /* LongueurListe */



/*
 * Renvoie une liste d'arguments, la première case étant initialisée à NULL, la
 * liste pouvant contenir NB_ARGS arguments (plus le pointeur NULL de fin de
 * liste)
 */
char **InitialiserListeArguments (void)
{
	char **l;

	l = (char **) (calloc (NB_ARGS+1, sizeof (char *)));
	*l = NULL;
	return l;
} /* InitialiserListeArguments */



/*
 * Ajoute en fin de liste le nouvel argument et renvoie la liste résultante
 */
char **AjouterArg (char **Liste, char *Arg)
{
	char **l;

	l = Liste + LongueurListe (Liste);
	*l = (char *) (malloc (1+strlen (Arg)));
	strcpy (*l++, Arg);
	*l = NULL;
	return Liste;
} /* AjouterArg */



/*
 * Fonction appelée lorsque l'utilisateur tape "".
 */
void EndOfFile (void)
{
	exit (0);
} /* EndOfFile */

/*
 * Appelée par yyparse() sur erreur syntaxique
 */
void yyerror (char *s)
{
	fprintf(stderr, "%s\n", s);
}


	void
expression_free(Expression *e)
{
	if (e == NULL)
		return;

	expression_free(e->gauche);
	expression_free(e->droite);

	if (e->arguments != NULL)
	{
		for (int i = 0; e->arguments[i] != NULL; i++)
			free(e->arguments[i]);
		free(e->arguments);  
	}

	free(e);
}






	int
main (int argc, char **argv) 
{
	head=NULL;


	getcwd(cwd, sizeof(cwd));
	if(getuid()==0){
		user = '#';
	}else{
		user = '$';
	}	
	printf(ANSI_COLOR_GREEN "%s " ANSI_COLOR_BLUE " %c" ANSI_COLOR_RED ">>>"ANSI_COLOR_RESET " ",cwd,user);





	
	while (1){
		if (yyparse () == 0) {
			/*--------------------------------------------------------------------------------------.
			  | L'analyse de la ligne de commande est effectuée sans erreur.  La variable globale     |
			  | ExpressionAnalysee pointe sur un arbre représentant l'expression.  Le type            |
			  | "Expression" de l'arbre est décrit dans le fichier Shell.h. Il contient 4             |
			  | champs. Si e est du type Expression :                                                 |
			  |                                                                                       |
			  | - e.type est un type d'expression, contenant une valeur définie par énumération dans  |
			  | Shell.h. Cette valeur peut être :                                                     |
			  |                                                                                       |
			  | - VIDE, commande vide                                                                 |
			  | - SIMPLE, commande simple et ses arguments                                            |
			  | - SEQUENCE, séquence (;) d'instructions                                               |
			  | - SEQUENCE_ET, séquence conditionnelle (&&) d'instructions                            |
			  | - SEQUENCE_OU, séquence conditionnelle (|) d'instructions							  |
			  | - BG, tâche en arrière plan (&)                                                       |
			  | - PIPE, pipe (|).																	  |
			  | - REDIRECTION_I, redirection de l'entrée (<)                                          |
			  | - REDIRECTION_O, redirection de la sortie (>)                                         |
			  | - REDIRECTION_A, redirection de la sortie en mode APPEND (>>).                        |
			  | - REDIRECTION_E, redirection de la sortie erreur,                                     |
			  | - REDIRECTION_EO, redirection des sorties erreur et standard.                         |
			  |                                                                                       |
			  | - e.gauche et e.droite, de type Expression *, représentent une sous-expression gauche |
			  | et une sous-expression droite. Ces deux champs ne sont pas utilisés pour les          |
			  | types VIDE et SIMPLE. Pour les expressions réclamant deux sous-expressions            |
			  | (SEQUENCE, SEQUENCE_ET, SEQUENCE_OU, et PIPE) ces deux champs sont utilisés           |
			  | simultannément.  Pour les autres champs, seule l'expression gauche est                |
			  | utilisée.                                                                             |
			  |                                                                                       |
			  | - e.arguments, de type char **, a deux interpretations :                              |
			  |                                                                                       |
			  | - si le type de la commande est simple, e.arguments pointe sur un tableau à la        |
			  | argv. (e.arguments)[0] est le nom de la commande, (e.arguments)[1] est le             |
			  | premier argument, etc.                                                                |
			  |                                                                                       |
			  | - si le type de la commande est une redirection, (e.arguments)[0] est le nom du       |
			  | fichier vers lequel on redirige.                                                      |
			  `--------------------------------------------------------------------------------------*/





			Expression *e = ExpressionAnalysee;
			lastfd = 0;
			execute(e,1,0,1,2,1);
			printf("\n");
			printf(ANSI_COLOR_GREEN "%s " ANSI_COLOR_BLUE " %c" ANSI_COLOR_RED ">>>"ANSI_COLOR_RESET " ",cwd,user);



			/*fprintf(stderr,"Expression syntaxiquement correcte : ");
			  fprintf(stderr,"[%s]\n", previous_command_line());

			  if (e->type == SIMPLE)
			  {
			  if (fork() == 0){
			  execvp(e->arguments[0], &e->arguments[0]);
			  perror(e->arguments[0]);
			  exit(1);
			  }
			  putchar('\n');
			  }

			  else if (e->type == REDIRECTION_I){
			  int fd = open(e->arguments[0],O_RDONLY, 0440);
			  if (fork() == 0){
			  dup2(fd,0);
			  close(0);
			  execvp(e->gauche->arguments[0], &e->gauche->arguments[0]);
			  perror(e->gauche->arguments[0]);
			  exit(1);
			  }
			  close(fd);
			  }
			  else if (e->type == REDIRECTION_O){
			  int fd = open(e->arguments[0],O_WRONLY | O_CREAT | O_TRUNC, 0660);
			  }
			  expression_free(e);*/
		}
		else {
			/* L'analyse de la ligne de commande a donné une erreur */
			fprintf (stderr,"Expression syntaxiquement incorrecte !\n");
			reset_command_line();
		}
	}
	return 0;
}


int execute(Expression *e , int wait, int fdin,int fdout,int fderror, int lastflag){
	int status;
	pid_t childPID;
	int fd;
	int pp[2];


	switch (e->type) {
		case SIMPLE:
			if(strcmp(e->arguments[0],"cd") ==0){
				chdir(e->arguments[1]);
				getcwd(cwd, sizeof(cwd));
				break;
			}else if (strcmp(e->arguments[0],"jobs") ==0){
				display();
				break;
			}else{

				childPID = fork();
				if(childPID >= 0) //fork was successful
				{
					if(childPID == 0) //child process
					{
						if(fdin != 0){
							dup2(fdin,0);
							close(fdin);
						}
						if(fdout != 1){
							dup2(fdout,1);
							close(fdout);
						}
						if(fderror != 2){
							dup2(fderror,2);
							close(fderror);
						}
						for(int i = 3; i <= lastfd; i++){
							close(i);
						}

						status = execvp(e->arguments[0], &e->arguments[0]);
						perror(e->arguments[0]);
						exit(1);
					}
					else//parent process
					{

						if(wait == 1){
							for(int i = 3; i <= lastfd; i++){
								close(i);
							}
							waitpid(childPID, &status, 0);
						}else{
							insert(childPID);
						}
						putchar('\n');
						break;
					}
				}
				else// fork failed 
				{
					perror("fork");
				}
				break;
			}
		case SEQUENCE:
			execute(e->gauche,1,fdin,fdout,fderror,0);
			execute(e->droite,1,fdin,fdout,fderror,0);
			break;
		case SEQUENCE_ET:
			execute(e->gauche,0,fdin,fdout,fderror,0);
			execute(e->droite,1,fdin,fdout,fderror,0);
			break;
		case SEQUENCE_OU:
			execute(e->gauche,0,fdin,fdout,fderror,0);
			execute(e->droite,1,fdin,fdout,fderror,0);
			break;
		case BG:
			execute(e->gauche,0,fdin,fdout,fderror,0);
			break;
		case PIPE:
			if(pipe(pp) < 0){
				perror("pipe");
				exit(1);
			}
			ch_lastfd(pp[1]);
			execute(e->gauche,0,fdin,pp[1],fderror,0);
			if(lastflag == 1){
				execute(e->droite,1,pp[0],fdout,fderror,0);
			}else{
				execute(e->droite,0,pp[0],fdout,fderror,0);
			}
			break;
		case REDIRECTION_I:
			fd = open(e->arguments[0],O_RDONLY, 0666);
			ch_lastfd(fd);
			execute(e->gauche,1,fd,fdout,fderror,0);
			break;
		case REDIRECTION_O:
			fd = open(e->arguments[0],O_CREAT | O_RDWR, 0666);
			ch_lastfd(fd);
			execute(e->gauche,1,fdin,fd,fderror,0);
			break;
		case REDIRECTION_A:
			fd = open(e->arguments[0], O_TRUNC | O_CREAT | O_RDWR, 0666);
			ch_lastfd(fd);
			execute(e->gauche,1,fdin,fd,fderror,0);
			break;
		case REDIRECTION_E:
			fd = open(e->arguments[0], O_CREAT | O_RDWR, 0666);
			ch_lastfd(fd);
			execute(e->gauche,1,fdin,fdout,fd,0);
			break;
		case REDIRECTION_EO:
			fd = open(e->arguments[0], O_CREAT | O_RDWR, 0666);
			ch_lastfd(fd);
			execute(e->gauche,1,fdin,fd,fd,0);
			break;
		default:
			break;

	}
	return 0;


}



void ch_lastfd(int max){
	if(lastfd < max){
		lastfd = max;
	}
}




void append(int num)
{
	struct node *temp,*right;
	temp= (struct node *)malloc(sizeof(struct node));
	temp->data=num;
	right=(struct node *)head;
	while(right->next != NULL)
		right=right->next;
	right->next =temp;
	right=temp;
	right->next=NULL;
}



void add( int num )
{
	struct node *temp;
	temp=(struct node *)malloc(sizeof(struct node));
	temp->data=num;
	if (head== NULL)
	{
		head=temp;
		head->next=NULL;
	}
	else
	{
		temp->next=head;
		head=temp;
	}
}
void addafter(int num, int loc)
{
	int i;
	struct node *temp,*left,*right;
	right=head;
	for(i=1;i<loc;i++)
	{
		left=right;
		right=right->next;
	}
	temp=(struct node *)malloc(sizeof(struct node));
	temp->data=num;
	left->next=temp;
	left=temp;
	left->next=right;
	return;
}



void insert(int num)
{
	int c=0;
	struct node *temp;
	temp=head;
	if(temp==NULL)
	{
		add(num);
	}
	else
	{
		while(temp!=NULL)
		{
			if(temp->data<num)
				c++;
			temp=temp->next;
		}
		if(c==0)
			add(num);
		else if(c<count())
			addafter(num,++c);
		else
			append(num);
	}
}



int delete(int num)
{
	struct node *temp, *prev;
	temp=head;
	while(temp!=NULL)
	{
		if(temp->data==num)
		{
			if(temp==head)
			{
				head=temp->next;
				free(temp);
				return 1;
			}
			else
			{
				prev->next=temp->next;
				free(temp);
				return 1;
			}
		}
		else
		{
			prev=temp;
			temp= temp->next;
		}
	}
	return 0;
}


void  display()
{
	int i = 1;
	jobs=head;
	int status;
	char *stat;
	if(jobs==NULL)
	{
		return;
	}
	while(jobs!=NULL)
	{

		waitpid(jobs->data, &status, WNOHANG);
		if(WIFSTOPPED(status)) {
			stat = "stopped";

		}else{
			stat = "running";
		}

		printf("[%d]---(%s)---PID %d \n",i,stat,jobs->data);
		i++;
		jobs=jobs->next;
	}
	printf("\n");
}






int count()
{
	struct node *n;
	int c=0;
	n=head;
	while(n!=NULL)
	{
		n=n->next;
		c++;
	}
	return c;
}
