#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <string.h>
#include <time.h>
#include <omp.h>

/*
	Les memebres de l'équipe 8: 
		Bennemane Khadidja    - SIL2
		Beddek Lilya          - SIL2
		Bennecer Ahmed Zaki   - SIL2
		Ben Amar Akram        - SIL2
		Hab el Hames Khadidja - SIL2
*/

/* ----------------------------------Debut- Variables Globales-------------------------------------- */

// dimensions du tableau
unsigned long largeur;
unsigned long hauteur;
unsigned long nbIterations;

// Le programme a deux tableaux afin de sauvegarder chaque états
long **tab1;  // état courante 
long **tab2;	// état suivante 
long **tab3;  
// Pointeurs pour gérer les tableaux (états actuel et suivant, et temp pour le swap).
long **ptrCourant, **ptrSuiv, **temp;

// variable pour calculer le temps d'éxécution 
float tempsExecSeq; 
clock_t t1, t2; 

/* ---------------------------------fin - Variables Globales----------------------------------------- */


/* -------------------------------------------Debut - Fonctions--------------------------------------- */
//initialisation du tableau par des zéros et des 1 
void init_Tableau (long **tableau_courrant);

/*
  dans cette implémentation du jeu les limites restent inchangées et ne sont pas prises en compte
  dans le processus principal, (pour faciliter le calcul des voisins), dons nous copions ces
  bordures dans le tableau d'état suivant.
*/
void copier_bordures (long **tableau_courrant, long **suiv);
// Fonction pour renvoyer le nombre de voisins vivants de la cellule i, j.
long nb_voisins_vivants (long **tableau_courrant, long i, long j);

/*
Fonction de jeu principale pour le jeu de la vie.
Prend comme entrée deux pointeurs pour les tableaux
et le segment dans lequel appliquer le jeu.
*/
void GameOfLife (long **tableau_courrant, long **suiv, int debut, int fin);

// Afficher l'état du tableau 
void afficher_tableau (long **tableau_courrant);

/* --------------------------------------------FIN - Fonctions---------------------------------------- */

/* --------------------------------------------Main Program------------------------------------------- */
long main (int argc, char *argv[])
{	
	long i;
	hauteur=2048;
	largeur=2048;
	nbIterations=200;
	
	//récupération des données entrer par l'utilisateur (la heuteur, la largeur et le nombre d'itérations) 
	//printf("Saisissez la hauteur de la matrice du jeu n*n: ");
    //scanf("%ld",&hauteur);
	//printf("Saisissez la largeur de la matrice du jeu n*n: ");
    //scanf("%ld",&largeur);
	//printf("Saisissez le nombre d'iterations: ");
	//scanf("%ld",&nbIterations);
	//printf("\n----------------------------------------------\n");
	

	// Alloue les deux tableaux en raison des dimensions données spécifié par l'utilisateur: 
	//tab1	
	tab1 = (long **)malloc(largeur*sizeof(long *));
	if(tab1==NULL)
	{
		printf("Problème d'allocation de mémoire de tab1!\n");
		printf("Programme terminé.\n");
		return -1;
	}
	//allocation des cases 
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

	//tab2
	tab2 = (long **)malloc(largeur*sizeof(long *));
        if(tab2==NULL)
        {
                printf("Problème d'allocation de mémoire de tab2\n");
                printf("Programme terminé.\n");
                return -1;
        }
	//allocation des cases 
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

    //tab3
	tab3 = (long **)malloc(largeur*sizeof(long *));
	if(tab3==NULL)
	{
		printf("Problème d'allocation de mémoire de tab1!\n");
		printf("Programme terminé.\n");
		return -1;
	}
	//allocation des cases 
	for(i = 0; i < largeur; i++)
	{
		tab3[i] = (long *)malloc(512*sizeof(long));
		if(tab3[i]==NULL)
		{
			printf("Problème d'allocation de mémoire de tab1\n");
			printf("Programme terminé.\n");
			return -1;
		}
	}
	// Initier des pointeurs vers les tableaux pertinents
	ptrCourant=tab1; // état courante 
	ptrSuiv=tab2;    // état suivante
	
	// Initie le tableau tableau_courrant avec des valeurs aléatoires entre zéro et un 
	init_Tableau(ptrCourant);
	
	// Copie les cellules des bordures (qui ne changes pas)
	copier_bordures(ptrCourant, ptrSuiv);
	// les variables de la partie MPI	
	
	// MPI - initialization
    int test=0;
    MPI_Init(&argc, &argv);
    int myRank, numProc;
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	MPI_Comm_size(MPI_COMM_WORLD, &numProc);

	printf("Hello world1! I'm rank %d of size %d\n", myRank, numProc);
     
	MPI_Bcast(*ptrCourant, largeur*largeur, MPI_LONG, 0, MPI_COMM_WORLD);

	
	// Calcul des limites du tableau que chaque thread traitera
	int taille = largeur / numProc;
	int debut = myRank * taille;
	int fin = debut + taille;
	
	// Iliminer les cellules des extrémités, par ce qu'ils vont pas changer
	if(myRank==0) {debut++;}
	if(myRank==numProc-1) {fin=largeur-1;}

	// local est la matrice de travail de cette partie, au lieu d'utiliser la matrice ptrSuiv, on utilise 
    // la matrice local et puis all gather va effectuer un gather + Bcast sur ces matrices locales 
    long **local;
    local=tab3;
    printf("taille=%d\n",taille);

    MPI_Barrier(MPI_COMM_WORLD);
	// Jouez au jeu pour nbIterations spécifier par l'utilisateur 
    if (myRank == 0){ t1=clock(); }  
	for (i=0; i<nbIterations; i++)
	{	
		GameOfLife(ptrCourant, local, debut, fin);
		MPI_Allgather(*local, largeur*taille, MPI_LONG, *ptrCourant, largeur*taille, MPI_LONG, MPI_COMM_WORLD);
		MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
		MPI_Comm_size(MPI_COMM_WORLD, &numProc);
	
		if(myRank == 0){
            printf("itération = %d\n",test);
            test++;
		}	
	}
    MPI_Barrier(MPI_COMM_WORLD);

	if (myRank == 0)
	{
		t2=clock(); 
		tempsExecSeq = (float)(t2-t1)/CLOCKS_PER_SEC; 
		printf("\n------------------------------------------------\n");
		printf("Temps d'éxecution MPI  du Game of life = %f s\n", tempsExecSeq);
		printf("pour %ld itérations et matrice de taille = %ld*%ld.\n",nbIterations,largeur,largeur);
		printf("------------------------------------------------\n");	
		
	}
	sleep(10);
	MPI_Finalize();  /* EXIT MPI */
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
void GameOfLife (long **ptrCourant, long **ptrSuiv, int debut, int fin) 
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