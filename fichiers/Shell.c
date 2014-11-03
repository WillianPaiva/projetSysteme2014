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

#define TRUE 1
#define FALSE !TRUE

#define FOREGROUND 'F'
#define BACKGROUND 'B'
#define SUSPENDED 'S'
#define WAITING_INPUT 'W'


#define BY_PROCESS_ID 1
#define BY_JOB_ID 2
#define BY_JOB_STATUS 3



int lastfd;

pid_t actualJob;

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

void sigchild_handler(int sig)
{
    pid_t pid;
    int terminationStatus;
    pid = waitpid(-1, &terminationStatus, WUNTRACED | WNOHANG);
    if (pid > 0) {
        t_job* job = getJob(pid, BY_PROCESS_ID);
        if (job == NULL){
            return;
        }
        if (WIFEXITED(terminationStatus)) {
            if (job->status == BACKGROUND) {
                printf("\n[%d]+  Done\t   %s\n", job->id, job->name);
                jobsList = delJob(job);
            }
        } else if (WIFSIGNALED(terminationStatus)) {
            printf("\n[%d]+  KILLED\t   %s\n", job->id, job->name);
            jobsList = delJob(job);
        } else if (WIFSTOPPED(terminationStatus)) {
            if (job->status == BACKGROUND) {
                changeJobStatus(pid, WAITING_INPUT);
                printf("\n[%d]+   suspended [wants input]\t   %s\n",
                        numActiveJobs, job->name);
            } else {
                changeJobStatus(pid, SUSPENDED);
                printf("\n[%d]+   stopped\t   %s\n", numActiveJobs, job->name);
            }
            return;
        } else {
            if (job->status == BACKGROUND) {
                jobsList = delJob(job);
            }
        }
    }

}

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

void job_stop(int sig)
{
    if (kill(actualJob, SIGTSTP) < 0)
    {
        perror("kill (SIGTSTP)");
    }
}

void job_kill(int sig)
{
    if (kill(actualJob, SIGINT) < 0){
        perror("kill (SIGINT)");
    }		
}

int main (int argc, char **argv) 
{  
    chdir(getenv("HOME"));
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigchild_handler;
    sigaction(SIGCHLD, &sa, NULL);

    struct sigaction sb;
    sigemptyset(&sb.sa_mask);
    sb.sa_flags = 0;
    sb.sa_handler = job_stop;
    sigaction(SIGTSTP, &sb, NULL);


    signal(SIGQUIT, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGINT,	&job_kill);





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
            execute(e,1,0,1,2,1,0);

            t_job* job2 = jobsList;
            if (job2 != NULL) {
                while (job2 != NULL) {
                    if(job2->futurewait == 1){
                        putJobForeground(job2,FALSE);
                    }
                    job2 = job2->next;
                }
            }


        }
        else {
            /* L'analyse de la ligne de commande a donné une erreur */
            fprintf (stderr,"Expression syntaxiquement incorrecte !\n");
            reset_command_line();
        }
    }
    return 0;
}

int execute(Expression *e , int wait, int fdin,int fdout,int fderror, int lastflag, int futurewait)
{
    pid_t childPID;
    int fd;
    int pp[2];
    int mode;
    t_job* job;



    switch (e->type) {
        case SIMPLE:
            if(builtincommands(e) == 1){
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

                        execvp(e->arguments[0], &e->arguments[0]);
                        perror(e->arguments[0]);
                        exit(1);
                    }
                    else//parent process
                    {
                        if(wait == 1){
                            mode = FOREGROUND;
                        }else{
                            mode = BACKGROUND;
                        }

                        jobsList = insertJob(childPID,e->arguments[0],(int) mode,futurewait);
                        job = getJob(childPID, BY_PROCESS_ID);

                        if(wait == 1){
                            for(int i = 3; i <= lastfd; i++){
                                close(i);
                            }
                            putJobForeground(job, FALSE);

                        }else{
                            putJobBackground(job, FALSE);
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
            execute(e->gauche,1,fdin,fdout,fderror,0,0);
            execute(e->droite,1,fdin,fdout,fderror,0,0);
            break;
        case SEQUENCE_ET:
            execute(e->gauche,0,fdin,fdout,fderror,0,1);
            execute(e->droite,1,fdin,fdout,fderror,0,0);
            break;
        case SEQUENCE_OU://needs better implementation 
            execute(e->gauche,1,fdin,fdout,fderror,0,0);
            execute(e->droite,1,fdin,fdout,fderror,0,0);
            break;
        case BG:
            execute(e->gauche,0,fdin,fdout,fderror,0,0);
            break;
        case PIPE:
            if(pipe(pp) < 0){
                perror("pipe");
                exit(1);
            }
            ch_lastfd(pp[1]);
            execute(e->gauche,0,fdin,pp[1],fderror,0,1);
            if(lastflag == 1){
                execute(e->droite,1,pp[0],fdout,fderror,0,0);
            }else{
                execute(e->droite,0,pp[0],fdout,fderror,0,1);
            }
            break;
        case REDIRECTION_I:
            fd = open(e->arguments[0],O_RDONLY, 0666);
            ch_lastfd(fd);
            execute(e->gauche,1,fd,fdout,fderror,0,0);
            break;
        case REDIRECTION_O:
            fd = open(e->arguments[0],O_CREAT | O_RDWR, 0666);
            ch_lastfd(fd);
            execute(e->gauche,1,fdin,fd,fderror,0,0);
            break;
        case REDIRECTION_A:
            fd = open(e->arguments[0], O_TRUNC | O_CREAT | O_RDWR, 0666);
            ch_lastfd(fd);
            execute(e->gauche,1,fdin,fd,fderror,0,0);
            break;
        case REDIRECTION_E:
            fd = open(e->arguments[0], O_CREAT | O_RDWR, 0666);
            ch_lastfd(fd);
            execute(e->gauche,1,fdin,fdout,fd,0,0);
            break;
        case REDIRECTION_EO:
            fd = open(e->arguments[0], O_CREAT | O_RDWR, 0666);
            ch_lastfd(fd);
            execute(e->gauche,1,fdin,fd,fd,0,0);
            break;
        case VIDE:
            putchar('\n');
            break;
        default:
            break;

    }


    return 0;


}

void ch_lastfd(int max)
{
    if(lastfd < max){
        lastfd = max;
    }
}

void waitJob(t_job* job)
{ 
    int terminationStatus;
    actualJob = job->pid;

    while (waitpid(job->pid, &terminationStatus, WNOHANG) == 0) {
        if (job->status == SUSPENDED)
            return;
    }
    jobsList = delJob(job);
    actualJob = 0;

}

void killJob(int jobId)
{
    t_job *job = getJob(jobId, BY_JOB_ID);
    kill(job->pid, SIGKILL);
}

t_job* insertJob(pid_t pid,char* name,int status,int futurewait)
{
    t_job *newJob = malloc(sizeof(t_job));

    newJob->name = (char*) malloc(sizeof(name));
    newJob->name = strcpy(newJob->name, name);
    newJob->pid = pid;
    newJob->status = status;
    newJob->futurewait = futurewait;
    newJob->next = NULL;

    if (jobsList == NULL) {
        numActiveJobs++;
        newJob->id = numActiveJobs;
        return newJob;
    } else {
        t_job *auxNode = jobsList;
        while (auxNode->next != NULL) {
            auxNode = auxNode->next;
        }
        newJob->id = auxNode->id + 1;
        auxNode->next = newJob;
        numActiveJobs++;
        return jobsList;
    }
}

t_job* getJob(int searchValue, int searchParameter)
{
    t_job* job = jobsList;
    switch (searchParameter) {
        case BY_PROCESS_ID:
            while (job != NULL) {
                if (job->pid == searchValue)
                    return job;
                else
                    job = job->next;
            }
            break;
        case BY_JOB_ID:
            while (job != NULL) {
                if (job->id == searchValue)
                    return job;
                else
                    job = job->next;
            }
            break;
        case BY_JOB_STATUS:
            while (job != NULL) {
                if (job->status == searchValue)
                    return job;
                else
                    job = job->next;
            }
            break;
        default:
            return NULL;
            break;
    }
    return NULL;
}

void putJobForeground(t_job* job, int continueJob)
{
    job->status = FOREGROUND;
    if (continueJob) {
        if (kill(job->pid, SIGCONT) < 0)
            perror("kill (SIGCONT)");
    }
    waitJob(job);
}

void putJobBackground(t_job* job, int continueJob)
{
    if(job->status == FOREGROUND){
        job->status = BACKGROUND;
    }
    if (job == NULL){
        return;
    }
    if (continueJob && job->status != WAITING_INPUT){
        job->status = WAITING_INPUT;
    }
    if (continueJob){
        if (kill(job->pid, SIGCONT) < 0){
            perror("kill (SIGCONT)");
        }
    }
}

t_job* delJob(t_job* job)
{
    if (jobsList == NULL)
        return NULL;
    t_job* currentJob;
    t_job* beforeCurrentJob;

    currentJob = jobsList->next;
    beforeCurrentJob = jobsList;

    if (beforeCurrentJob->pid == job->pid) {

        beforeCurrentJob = beforeCurrentJob->next;
        numActiveJobs--;
        return currentJob;
    }

    while (currentJob != NULL) {
        if (currentJob->pid == job->pid) {
            numActiveJobs--;
            beforeCurrentJob->next = currentJob->next;
        }
        beforeCurrentJob = currentJob;
        currentJob = currentJob->next;
    }
    return jobsList;
}

void printJobs()
{
    printf("\nActive jobs:\n");
    printf(
            "--------------------------------------------------------------\n");
    printf("| %7s  | %30s | %5s | %6s |\n", "job no.", "name", "pid", "status");
    printf(
            "--------------------------------------------------------------\n");
    t_job* job = jobsList;
    if (job == NULL) {
        printf("| %s %49s |\n", "No Jobs.", "");
    } else {
        while (job != NULL) {
            printf("|  %7d | %30s | %5d | %6c |\n", job->id, job->name,
                    job->pid, job->status);
            job = job->next;
        }
    }
    printf(
            "--------------------------------------------------------------\n");
}

int builtincommands(Expression *e)
{
    if(strcmp(e->arguments[0],"cd") ==0){
        if(e->arguments[1] != NULL){
            chdir(e->arguments[1]);
        }else{
            chdir(getenv("HOME"));
        }
        return 1;
    }
    if (strcmp(e->arguments[0],"jobs") ==0){
        printJobs();
        return 1;
    }
    if(strcmp(e->arguments[0],"exit") ==0){
        exit(EXIT_SUCCESS);
    }
    if(strcmp(e->arguments[0],"fg") ==0){
        if(e->arguments[1]== NULL){
            return 1;
        }
        int jobid = (int) atoi(e->arguments[1]);
        t_job* job = getJob(jobid, BY_JOB_ID);
        if(job == NULL){
            return 1;
        }
        if(job->status == SUSPENDED || job->status == WAITING_INPUT){
            putJobForeground(job,TRUE);
        }else{
            putJobForeground(job,FALSE);
        }
        return 1;
    }
    if(strcmp(e->arguments[0],"kill") ==0){
        if(e->arguments[1] == NULL){
            return 1;
        }else{
            killJob(atoi(e->arguments[1]));
            return 1;
        }
    }
    if(strcmp(e->arguments[0],"dirs") ==0){
        printf("%s\n","TODO");
        return 1;
    }
    if(strcmp(e->arguments[0],"history") ==0){
        HIST_ENTRY **the_history_list =  history_list ();
        for(int i = 0;;i++){
            if(the_history_list[i] != NULL){
                printf("[%d]-------->   %s\n",i,the_history_list[i]->line);
            }else{
                break;
            }
        }
        return 1;
    }


    return 0;




}

int changeJobStatus(int pid, int status)
{
    t_job *job = jobsList;
    if (job == NULL) {
        return 0;
    } else {
        int counter = 0;
        while (job != NULL) {
            if (job->pid == pid) {
                job->status = status;
                return TRUE;
            }
            counter++;
            job = job->next;
        }
        return FALSE;
    }
}


















