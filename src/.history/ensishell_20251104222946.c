/*****************************************************
 * Copyright Grégory Mounié 2008-2015                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "variante.h"
#include "readcmd.h"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>

#ifndef VARIANTE
#error "Variante non défini !!"
#endif

/* Guile (1.8 and 2.0) is auto-detected by cmake */
/* To disable Scheme interpreter (Guile support), comment the
 * following lines.  You may also have to comment related pkg-config
 * lines in CMakeLists.txt.
 */

#if USE_GUILE == 1
#include <libguile.h>

int question6_executer(char *line)
{
	/* Question 6: Insert your code to execute the command line
	 * identically to the standard execution scheme:
	 * parsecmd, then fork+execvp, for a single command.
	 * pipe and i/o redirection are not required.
	 */
	printf("Not implemented yet: can not execute %s\n", line);

	/* Remove this line when using parsecmd as it will free it */
	free(line);
	
	return 0;
}

SCM executer_wrapper(SCM x)
{
        return scm_from_int(question6_executer(scm_to_locale_stringn(x, 0)));
}
#endif


void terminate(char *line) {
#if USE_GNU_READLINE == 1
	/* rl_clear_history() does not exist yet in centOS 6 */
	clear_history();
#endif
	if (line)
	  free(line);
	printf("exit\n");
	exit(0);
}

void question4(int nbr_bg, int* pids_bg, char** l_bg){
	printf("Liste des processus lancés en tâche de fond :\n");
	printf("pid\tcommande\n");
	
	int res;
	
	for (int i = 0; i < nbr_bg; i++){
		res = waitpid(pids_bg[i], NULL, WNOHANG);
		if (res == 0){
			printf("%d\t%s\n", pids_bg[i], l_bg[i]);
		}
	}
}

// void question1(struct cmdline* l, int nbr_bg, char** l_bg, int* pids_bg){ // nbr_bg l_bg pids_bg sont des copies lors de l'appel de la fonction, fct copiée dans le main

// 	int cpt = 0;
// 	// On calcule le nombre de sequences 
// 	for (int i = 0 ; l ->seq[i] != 0; i++){
// 		cpt += 1;
// 	}
// 	if (cpt == 1){
// 		char** argumentlist = l->seq[0];
// 		char* command = argumentlist[0];

// 		if (strcmp(command, "jobs") == 0) question4(nbr_bg, pids_bg, l_bg);

// 		else{
// 			pid_t pid = fork();
// 			if (pid == 0){
// 				// printf("Je suis le processus fils! \n");
// 				int ret_value = execvp(command, argumentlist);
// 				if (ret_value == -1){
// 					printf("Commande %s non trouvée\n", command);
// 				}						

// 			}
// 			else{
// 				if (l->bg == 0){
// 					wait(NULL); // Pour avoir le ensishell> après l'execution du fils
// 				}
// 				else{
// 					printf("on entre ici\n");
// 					nbr_bg++;
// 					l_bg = realloc(l_bg, nbr_bg*sizeof(char*));
// 					pids_bg = realloc(pids_bg, nbr_bg*sizeof(int));
// 					l_bg[nbr_bg - 1] = strdup(command);
// 					pids_bg[nbr_bg - 1] = pid;

// 				}
				
// 			}
// 		}
// 	}
// 	free(l_bg);
// 	free(pids_bg);

// }


int main() {
	int nbr_bg = 0; // Nombre des processus en tache de fond
	char** l_bg = NULL ;
	int* pids_bg = NULL;
	bool error_
	int fd_save = dup(STDOUT_FILENO);
	if (fd_save == -1){
		perror("dup fd_out_save unsucessful\n");
		exit(EXIT_FAILURE);
	}
	int fd_save_in = dup(STDIN_FILENO);
	if (fd_save_in == -1){
		perror("dup fd_in_save unsucessful\n");
		exit(EXIT_FAILURE);
	}
	printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);

#if USE_GUILE == 1
        scm_init_guile();
        /* register "executer" function in scheme */
        scm_c_define_gsubr("executer", 1, 0, 0, executer_wrapper);
#endif

	while (1) {
		int ret_dup = dup2(fd_save, STDOUT_FILENO);
		if (ret_dup == -1){
			perror("dup2_out unsucessful\n");
			exit(EXIT_FAILURE);
		}
		int ret_dup_2 = dup2(fd_save_in, STDIN_FILENO);
		if (ret_dup_2 == -1){
			perror("dup2_in unsucessful\n");
			exit(EXIT_FAILURE);
		}
		struct cmdline *l;
		char *line=0;
		int i, j;
		char *prompt = "ensishell>";

		/* Readline use some internal memory structure that
		   can not be cleaned at the end of the program. Thus
		   one memory leak per command seems unavoidable yet */
		line = readline(prompt);
		if (line == 0 || ! strncmp(line,"exit", 4)) {
			terminate(line);
		}

#if USE_GNU_READLINE == 1
		add_history(line);
#endif


#if USE_GUILE == 1
		/* The line is a scheme command */
		if (line[0] == '(') {
			char catchligne[strlen(line) + 256];
			sprintf(catchligne, "(catch #t (lambda () %s) (lambda (key . parameters) (display \"mauvaise expression/bug en scheme\n\")))", line);
			scm_eval_string(scm_from_locale_string(catchligne));
			free(line);
                        continue;
                }
#endif

		/* parsecmd free line and set it up to 0 */
		l = parsecmd( & line);

		/* If input stream closed, normal termination */
		if (!l) {
		  
			terminate(0);
		}
		

		
		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		if (l->in) printf("in: %s\n", l->in);
		if (l->out) printf("out: %s\n", l->out);
		if (l->bg) printf("background (&)\n");

		/* Display each command of the pipe */
		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
			printf("seq[%d]: ", i);
				for (j=0; cmd[j]!=0; j++) {
						printf("'%s' ", cmd[j]);
				}
			printf("\n");
		}
		int cpt = 0;
		// On calcule le nombre de sequences 
		for (int i = 0 ; l ->seq[i] != 0; i++){
			cpt += 1;
		}
		// Cas de redirection vers un fichier (qst 6)
		if (l->in){
			// printf("on rentre ici\n");
			int fd_in = open(l->in, O_RDONLY);
			if (fd_in == -1){
				printf("Fichier d'entrée %s introuvable\n", l->in);
				goto error;
				break;
			}
			dup2(fd_in, STDIN_FILENO);
			close(fd_in);
			// printf("Redirection entrée depuis : %s\n", l->in);
		}
		if (l->out){
			int fd = open(l->out, O_WRONLY|O_CREAT|O_TRUNC, 0644);
			if (fd == -1){
				printf("Erreur d'ouverture du fichier de sortie, le nom est %s\n", l->out);
				break;
			}
			dup2(fd, STDOUT_FILENO);
			close(fd);
		}
		if (cpt == 1){
			/*
				AJOUT fichiers
			*/
			char** argumentlist = l->seq[0];
			char* command = argumentlist[0];

			if (strcmp(command, "jobs") == 0) question4(nbr_bg, pids_bg, l_bg);

			else{
				pid_t pid = fork();
				if (pid == 0){
					// printf("Je suis le processus fils! \n");

					int ret_value = execvp(command, argumentlist);
					if (ret_value == -1){
						printf("Commande %s non trouvée\n", command);
					}						

				}
				else{
					if (l->bg == 0){
						wait(NULL); // Pour avoir le ensishell> après l'execution du fils (qst 2)
					}
					else{
						nbr_bg++;
						l_bg = realloc(l_bg, nbr_bg*sizeof(char*));
						pids_bg = realloc(pids_bg, nbr_bg*sizeof(int));
						l_bg[nbr_bg - 1] = strdup(command); // commande est un pointeur est donc change par la fonction de readcmd.c  strdup fait donc une copie et l'alloue pour la commande qu'on a 
						pids_bg[nbr_bg - 1] = pid;
					}
				
					
				}
			}
		}
		if (cpt == 2){
			char** argumentlist1 = l->seq[0];
			char* command1 = argumentlist1[0];

			char** argumentlist2 = l->seq[1];
			char* command2 = argumentlist2[0];


			int tuyau[2];
			pipe(tuyau);
			int pid1 = fork(); // Pour faire un pipeline, les deux doivent être des frères 
			
			if (pid1 == 0){
				if (l->in){
					int fd_in = open(l->in, O_RDONLY);
					dup2(fd_in, STDIN_FILENO);
				}
				dup2(tuyau[1], STDOUT_FILENO);
				close(tuyau[0]);
				execvp(command1, argumentlist1);
			}
			else{
				int pid2 = fork();
				if (pid2 == 0){
					dup2(tuyau[0], STDIN_FILENO);
					close(tuyau[1]);
					execvp(command2, argumentlist2);
				}
				close(tuyau[0]); // A revoir
				close(tuyau[1]); // A revoir
				while (wait(NULL) > 0); // A revoir
			}
		

		}
		error:
			
	}
	free(pids_bg);
	free(l_bg);
	

}
