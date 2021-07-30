#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include <sys/time.h>

/*
	Les memebres de l'équipe 8: 
		Bennemane Khadidja    - SIL2
		Beddek Lilya          - SIL2
		Bennecer Ahmed Zaki   - SIL2
		Ben Amar Akram        - SIL2
		Hab el Hames Khadidja - SIL2
*/

/* ------------------Debut- Variables Globales------------------ */

// dimensions du tableau
unsigned long largeur;
unsigned long hauteur;
unsigned long nbIterations;

//clock pour le calcul de temps 
clock_t deb_clock, fin_clock,deb_clock2, fin_clock2;
double total,total2;

// Nombre de threads
int nthreads;

// Le programme a deux tableaux afin de sauvegarder chaque état
// La mémoire nécessaire est allouée dans le main en raison des valeurs de dimension
long **tab1;
long **tab2;

// Pointeurs pour gérer les tableaux (états actuel et suivant, et temp pour le swap).
long **ptrCourant, **ptrSuiv, **temp,**ptrCourant2;

// Variables=pour mesurer le temps d'exécution
struct timeval t_debut, t_end;

/* ------------------fin - Variables Globales------------------ */

/* ---------------------Debut - Functions--------------------- */

//initialisation avec des zéros si mode par défaut, ou avec 0,1 aléatoire.
void init_Tableau (long **tableau_courrant);

/*
Parce que les limites restent inchangées et ne sont pas prises en compte
dans le processus principal, nous copions la région dans le tableau d'état suivant.
*/

void copier_bordures (long **tableau_courrant, long **suiv);

// Fonction pour renvoyer le nombre de voisins vivants de la cellule i, j.
long nb_voisins (long **tableau_courrant, long i, long j);

/*
Fonction de jeu principale pour le jeu de la vie.
Prend comme entrée deux pointeurs pour les tableaux
et le segment dans lequel appliquer le jeu.
*/
void GameOfLife (long **tableau_courrant, long **suiv, long debut, long fin);

// Imprimer l'état du tableau 
void afficher_tableau (long **tableau_courrant);

/* ---------------------End - Functions--------------------- */

/* -----------------------Main Program---------------------- */
long main (long argc, char *argv[])
{	
	long i;
	int tid;
	hauteur=0;
	largeur=0;
	unsigned int debut, fin;
	nbIterations=0;
	nthreads=atoi(argv[1]);

    printf("Saisissez la hauteur de la matrice n*n:\n");
    	scanf("%ld",&hauteur);

    printf("Saisissez la largeur de la matrice n*n:\n");
    	scanf("%ld",&largeur);
   
    printf("Saisissez le nombre d'iterations n*n:\n");
    	scanf("%ld",&nbIterations);
	
	// Alloue les deux tableaux en raison des dimensions données (par défaut | spécifié par l'utilisateur)	
	tab1 = (long **)malloc(largeur*sizeof(long *));
	if(tab1==NULL)
	{
		printf("Problème d'allocation de mémoire de tab1!\n");
		printf("Programme terminé.\n");
		return -1;
	}
	for(i = 0; i < largeur; i++)
	{
		tab1[i] = (long *)malloc(hauteur*sizeof(long));
		if(tab1[i]==NULL)
		{
			printf("Problème d'allocation de mémoire de tab1\n");
			printf("Programme terminé.\n");
			return -1;
		}
	}

	tab2 = (long **)malloc(largeur*sizeof(long *));
        if(tab2==NULL)
        {
                printf("Problème d'allocation de mémoire de tab2\n");
                printf("Programme terminé.\n");
                return -1;
        }
	for(i = 0; i < largeur; i++)
        {       
                tab2[i] = (long *)malloc(hauteur*sizeof(long));
                if(tab2[i]==NULL)
                {
                        printf("Problème d'allocation de mémoire de tab2\n");
                        printf("Programme terminé.\n");
                        return -1;
                }
        }
	
 	// Initier des pointeurs à afficher vers les tableaux pertinents
	ptrCourant=tab1;
	ptrSuiv=tab2;
	ptrCourant2=tab1;

	// Initie le tableau tableau_courrant avec des zéros ou des valeurs aléatoires
	// Attribut spécifié par l'indicateur par défaut
	init_Tableau(ptrCourant);

	// Copie les cellules de la région inchangées
	copier_bordures(ptrCourant, ptrSuiv);
	ptrCourant2=ptrCourant;
		
	total=0;
	double start1;
	double end1;
	start1=omp_get_wtime();
	
	for (i=0; i<nbIterations; i++)
	{ 
		#pragma omp parallel shared(ptrCourant, ptrSuiv, temp, nthreads) private(tid, i, debut, fin) num_threads(nthreads)
		{
			tid = omp_get_thread_num();
			debut = tid * (hauteur / nthreads);
			fin = debut + (hauteur / nthreads);
			GameOfLife (ptrCourant, ptrSuiv, debut, fin);
			//attendre la fin du jeu pour toute les partie du tableau
			#pragma omp barrier
			if(tid==0)
			{
				temp=ptrCourant;
				ptrCourant=ptrSuiv;
				ptrSuiv=temp;
				//printf("------------------------------");
				//printf("Board's Initial State");
				//printf("-----------------------------\n");
				//print (ptrCourant);

			}
			// Une autre barrière est nécessaire pour assurer que les pointeurs ont été échangés avant de passer à l'itération suivante 
			#pragma omp barrier
		}
	}	
	end1=omp_get_wtime();

/*******************************LA SEQUENTIELLE ****************************************/

	copier_bordures(ptrCourant2, ptrSuiv);
	double start2;
	double end2;
	start2=omp_get_wtime();

	#pragma omp parallel shared(ptrCourant2, ptrSuiv, temp) private(tid, i, debut, fin) num_threads(1)
	{

		//Get thread number ID
		tid = omp_get_thread_num();
		long bound = hauteur;
		debut = 1;
		fin = debut + bound;
		if(tid==0) ;
		if(tid==nthreads-1) ;		
		for (i=0; i<nbIterations; i++)
		{ 
			GameOfLife (ptrCourant2, ptrSuiv, debut, fin);
			//attendre la fin du jeu pour toute les partie du tableau
			#pragma omp barrier
			if(tid==0)
			{
				temp=ptrCourant2;
				ptrCourant2=ptrSuiv;
				ptrSuiv=temp;
				//printf("------------------------------");
				//printf("tableau des  seq");
				//printf("-----------------------------\n");
				//print (ptrCourant2);
			}
			// Une autre barrière est nécessaire pour assurer que les pointeurs ont été échangés avant de passer à l'itération suivante 
			#pragma omp barrier
		}
	}
	end2=omp_get_wtime();

	//Calculer et Afficher le temps total
	printf("\n------------------------------------------------\n");
	printf("Temps d'éxecution séquentiel du Game of life = %f s\n", end2-start2);
	printf("pour %ld itérations et matrice de taille = %ld*%ld.\n",nbIterations,largeur,largeur);
	printf("------------------------------------------------\n");

	printf("\n------------------------------------------------\n");
	printf("Temps d'éxecution parallel avec OpenMP du Game of life = %f s\n", end1-start1);
	printf("pour %ld itérations et matrice de taille = %ld*%ld.\n",nbIterations,largeur,largeur);
	printf("------------------------------------------------\n");

	return 0;
}

/* ---------------------------------------------------- Main ------------------------------------------------------ */


/* ---------------------------------------------------Fonctions---------------------------------------------------- */
// Initialisation des valeurs du tableau par des valeurs entre zéro et un
void init_Tableau (long **ptrCourant)
{

	long i, j;
	for (i=0; i<largeur; i++) for (j=0; j<hauteur; j++) 
	{
		ptrCourant[i][j] = rand() % 2;
	}
}

// Garder les bordures qui ne changes pas d'une itération vers une autre 
void copier_bordures (long **ptrCourant, long **ptrSuiv)
{
	long i,j;
	for(j=0; j<hauteur; j++)
	{
		for(i=0; i<largeur; i++)
		{
			if((i==0)|(j==0)|(j==hauteur-1)|(i==largeur-1))
			{
				ptrSuiv[i][j]=ptrCourant[i][j];
			}
		}
	}
}


// le nombre des voisins d'une cellule 
long nb_voisins_vivants (long **ptrCourant, long i, long j) {
	long lig, col, cpt;
	cpt = 0;
	// Examiner tous les voisins
	for (lig=-1; lig<=1; lig++) 
		for (col=-1; col<=1; col++)
		{	
			// exclure la cellule du tableau_courrant du compte
			if (lig || col)
				if (ptrCourant[i+lig][j+col])
				{
 					cpt++;	
				}	
			// nous n'avons pas besoin de continuer à compter si le nombre est> 3 (pas de changement de comportement)		
			if(cpt>3)
			{
				break;
			}
		}		
	return cpt;
}

// la fonction qui fait le jeu 
void GameOfLife (long **ptrCourant, long **ptrSuiv, long debut, long fin) 
{
	long i, j, vivante;
	// Exclure les bordures du tab et appliquer pour chaque cellule les règles du jeu
	for (i=1; i<largeur-1; i++)
	{
		for (j=debut; j<fin; j++) 
		{
			vivante = nb_voisins_vivants (ptrCourant, i, j);
			if (vivante == 2) ptrSuiv[i][j] = ptrCourant[i][j];
			if (vivante == 3) ptrSuiv[i][j] = 1;
			if (vivante < 2) ptrSuiv[i][j] = 0;
			if (vivante > 3) ptrSuiv[i][j] = 0;
		}
	}
}
// affichage des tableau 
void afficher_tableau (long **ptrCourant) 
{
	long i, j;

	for (j=0; j<hauteur; j++) 
	{

		for (i=0; i<largeur; i++) 
		{
			printf ("%c", ptrCourant[i][j] ? 'x' : ' ');
		}
		printf ("\n");
	}
}

