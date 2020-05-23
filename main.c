#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include <fcntl.h>
#include "file.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

/*CONSTANTES*/
#define NB_TAXIS 3
#define NB_STATIONS_BUS 5
#define NB_STATION_METRO 3
#define CAPACITE_BUS 5
#define CAPACITE_METRO 8
#define NB_FILES NB_STATIONS_BUS+NB_STATION_METRO*2
#define TAILLE 30
#define MAX_ATTENTE 10

/*FONCTIONS THREAD*/
void* metro_ligne(void* i);
void* autobus_ligne(void* i);
void* verificateur_maj(void* i);
void* taxi_ligne(void* i);

/*FONCTIONS*/
passager_t atop(char* s,uint32_t taille);
void transferer(maillon_t*,uint32_t station_src,passager_t);
void descendre(file_t* f,uint32_t station);
void monter_bus(file_t* f, uint32_t station);
void monter_metro(file_t* f, uint32_t station);
uint32_t files_vides();
uint32_t file_vide(file_t* f);

void handler(int sig);
void handler2(int sig);

/*FILES*/
file_t* files[NB_FILES];
file_t* passagers_bus;
file_t* passagers_metro;

/*SEMAPHORE*/
sem_t sVerifMetro, sVerifBus, sMetro,sBus;

/*NOM TUBE*/
char nomTube[20] = "tube.fifo";

/*VARIABLES*/
int profit = 0;

/*MUTEX*/
pthread_mutex_t mutex;

/*PID_T*/
pid_t taxis;

int compteur = 0;

int main(int argc,char** argv)
{
  
  creer_files(files,NB_FILES);
  passagers_bus = new_file();
  passagers_metro = new_file();
  int nb_passagers;
  nb_passagers = remplir_files(files,NB_FILES, argv[1]);
  printf("Nous avons %d passagers.\n", nb_passagers);
  compteur++;
  //afficher_files(files,NB_FILES);

  // On créé notre tube nommé après l'avoir supprimé pour eviter EEXIST
  remove(nomTube);
  if(mkfifo(nomTube, 0644) != 0) 
	{
		perror("Impossible de créer le tube nommé");
		exit(0);
	}

  signal(SIGINT,handler);
  
  //Initialisation des semaphores
  sem_init(&sVerifBus,PTHREAD_PROCESS_SHARED,0);
  sem_init(&sVerifMetro,PTHREAD_PROCESS_SHARED,0);
  sem_init(&sMetro,PTHREAD_PROCESS_SHARED,1);
  sem_init(&sBus,PTHREAD_PROCESS_SHARED,1);

  /*initialisation du mutex*/

  pthread_mutex_init(&mutex,NULL);

  if ((taxis=fork())>0)
  {//Le processus principal

    /* On declare les trois thread du processus principal*/
    pthread_t metro;
    pthread_t autobus;
    pthread_t verificateur;
    
    /* Ici on Initialise les trois thread*/ 
    if (pthread_create(&metro, NULL, metro_ligne, NULL)==-1)
    {
      perror("Erreur lors de la création du thread!");
      exit(0);
    }
    if (pthread_create(&autobus, NULL, autobus_ligne, NULL)==-1)
    {
      perror("Erreur lors de la création du thread!");
      exit(0);
    }
    if (pthread_create(&verificateur, NULL, verificateur_maj, NULL)==-1)
    {
      perror("Erreur lors de la création du thread!");
      exit(0);
    }

    pthread_join(autobus,NULL);
    pthread_join(metro,NULL);
    pthread_join(verificateur,NULL);
  }
  else
  {//processus fils (taxis)
      pthread_t taxi[NB_TAXIS];
      for (int i=0; i<NB_TAXIS; i++)
      {
        if (pthread_create(&taxi[i], NULL, taxi_ligne, (void*)i+1)==-1)
        {
          perror("Erreur lors de la création du thread!");
          exit(0);
        }
      }
      for (uint32_t i = 0;i<NB_TAXIS;i++)
      {
        pthread_join(taxi[i],NULL);
      }
      exit(0);
  }
  wait(NULL);
  printf("Le profit : %d\n", profit);
  return EXIT_SUCCESS;
}
/*Handler de SIGINT*/
void handler(int sig){

  kill(taxis,9);
  printf("taxis arreté\n");
}

/*FONCTIONS THREAD*/
//Dans cette fonction, on fait appel aux fonctions 'descendre' et 'monter_metro' pour chacune des stations du metro tant que toutes les files d'attentes ne sont pas vides ou qu'il y ait encore des passagers à bord du bus ou du metro.
void* metro_ligne(void* i)
{
  uint32_t station = NB_STATIONS_BUS;
  while(!files_vides() || !file_vide(passagers_metro) || !file_vide(passagers_bus))
  {
    for (int station=NB_STATIONS_BUS; station<NB_STATIONS_BUS+NB_STATION_METRO && (!files_vides() || !file_vide(passagers_metro) || !file_vide(passagers_bus)); station++)
    {
      
      sem_wait(&sMetro);
      //puts("METRO EN MARCHE");
      descendre(passagers_metro, station);
      monter_metro(passagers_metro, station);
      sem_post(&sVerifMetro);
    }
    for (int station=NB_FILES-1; station>=NB_STATIONS_BUS+NB_STATION_METRO && (!files_vides() || !file_vide(passagers_metro) || !file_vide(passagers_bus)); station--)
    { 
      sem_wait(&sMetro);
      //puts("METRO EN MARCHE");
      descendre(passagers_metro, station-3);
      monter_metro(passagers_metro, station);
     sem_post(&sVerifMetro);
    }
  }
  //puts("out METRO");
  sem_post(&sVerifMetro);

  pthread_exit(NULL);
}

//Dans cette fonction, on fait appel aux fonctions 'descendre' et 'monter_bus' pour chacune des stations de bus tant que toutes les files d'attentes ne sont pas vides ou qu'il y ait encore des passagers à bord du bus ou du metro.
void* autobus_ligne(void* i)
{
  uint32_t station = 0;
  while(!files_vides() || !file_vide(passagers_bus) || !file_vide(passagers_metro))
  {
    sem_wait(&sBus);
   // puts("BUS EN MARCHE");
    descendre(passagers_bus, station%(NB_STATIONS_BUS));
    monter_bus(passagers_bus, station%(NB_STATIONS_BUS));
   
    station++;
    sem_post(&sVerifBus);
  }
  //puts("out BUS");
  sem_post(&sVerifBus);
  pthread_exit(NULL);
}

// Lorsqu'un passager monte dans un taxi, on incrémente le profit de 3.
void* verificateur_maj(void* i)
{
  int entreeTube; 
  maillon_t *avant_temp, *temp , *tmp,*suiv,*prec;
  passager_t passagerTmp;
  char s[TAILLE];
  if((entreeTube = open(nomTube, O_WRONLY)) == -1) 
	{
		perror("Impossible d'ouvrir l'entrée du tube nommé.");
		exit(0);
	}
  while(!files_vides(files) || !file_vide(passagers_bus) || !file_vide(passagers_metro) )
  {
    sem_wait(&sVerifMetro);
    sem_wait(&sVerifBus);

    //puts("VERIFICATION EN MARCHE");
    for (int i=0; i<NB_FILES; i++)
    {
      for (temp=files[i]->tete; temp!=NULL; temp=temp->suivant)
        {
          temp->value.tps_attente++;
          if (temp->value.tps_attente == temp->value.tps_max)
          {
            passagerTmp = temp->value;

            //On le supprime
            //rem_maillon(files[i], temp);
            if (temp == files[i]->tete)
              avant_temp = files[i]->tete;
            else
              avant_temp = temp->precedent;
            
            maillon_rem(files[i],temp);
            temp = avant_temp;
            //On le transfere
            sprintf(s,"%d %d %d %ld %d %ld\n",passagerTmp.id,passagerTmp.depart,passagerTmp.arrivee,passagerTmp.tps_attente,passagerTmp.transfert,passagerTmp.tps_max );
            write(entreeTube,s,TAILLE);
            printf("verificateur : transfert du passager %d vers le taxi\n",passagerTmp.id);
            // On incremente le profit de 3
            pthread_mutex_lock(&mutex);
            profit += 3;
            pthread_mutex_unlock(&mutex);
          }     
        }
    }   
    sem_post(&sBus);
    sem_post(&sMetro);
  }
  sem_post(&sBus);
  sem_post(&sMetro);
  for (uint32_t i = 0 ; i < NB_TAXIS ; i++ ){
    write(entreeTube,"1\n",TAILLE);
  }
  pthread_exit(NULL);
}

void* taxi_ligne(void* i)
{
  int taille;
  int id_taxi = (int) i;
  int sortieTube	;
  char r[TAILLE];
  passager_t p;
  if((sortieTube = open (nomTube, O_RDONLY)) == -1)
	{
		perror("Impossible d'ouvrir la sortie du tube nommé.");
		exit(EXIT_FAILURE);
	}
  while(1)
  {
    taille = read(sortieTube,r,TAILLE);
    printf("r : %s de taille : %ld \t taille : %d\n",r,strlen(r),taille);
    if (strlen(r) == 2)
    {
      puts("taxi quitte");
      pthread_exit(NULL);
    }
    else{
    sscanf(r,"%d %d %d %ld %d %ld\n",&p.id,&p.depart,&p.arrivee,&p.tps_attente,&p.transfert,&p.tps_max); 
    usleep(MAX_ATTENTE);
    printf("taxi#%d : passager %d est rendu a la station %d\n",id_taxi,p.id,p.arrivee);
    }
  }
  pthread_exit(NULL);
}

/*FONCTIONS*/
//Dans cette fonction, on supprime un passager de la file d'attente correspondant à la station passée en argumant et on l'ajoute à la file 'f' si la capacité de cette dernière n'est pas atteinte. Lorsqu'un passager monte à bord, on incrémente le profit de 1.
void monter_metro(file_t* f, uint32_t station)
{
  passager_t passagerTmp;
  while(files[station]->taille > 0 && f->taille < CAPACITE_METRO)
  {
    passagerTmp = files[station]->tete->value;
    rem_tete(files[station]);
    add_tete(f,passagerTmp);
    printf("Metro : embarque le passager %u à la station %u.\n", passagerTmp.id, station);
  compteur++;
    // On incrémente le profit
    pthread_mutex_lock(&mutex);
    profit += 1;
    pthread_mutex_unlock(&mutex);
  }
}

//Dans cette fonction, on supprime un passager de la file d'attente correspondant à la station passée en argumant et on l'ajoute à la file 'f' si la capacité de cette dernière n'est pas atteinte. Lorsqu'un passager monte à bord, on incrémente le profit de 1.
void monter_bus(file_t* f, uint32_t station)
{
  passager_t passagerTmp;
  maillon_t* dernier = files[station]->tete;
  while(dernier != NULL && f->taille < CAPACITE_BUS)
  {
    passagerTmp = files[station]->tete->value;
    rem_tete(files[station]);
    add_tete(f,passagerTmp);
    dernier = files[station]->tete;
    printf("BUS : embarque le passager %u à la station %u.\n", passagerTmp.id, station);
    // On incrémente le profit
    pthread_mutex_lock(&mutex);
    profit += 1;
    pthread_mutex_unlock(&mutex);
  }
}

//Cette fonction test si un des passager de la file 'f' a pour station d'arrivée la station 'station', si oui le maillon correpondant au passager est libéré. On vérifie aussi si le passager doit être transféré vers une autre file.
void descendre(file_t* f, uint32_t station)
{
  if (f->taille>0)
  {
    passager_t passagerTmp;
    maillon_t* dernier = f->tete;
    maillon_t *avant_dernier;
    
    while(dernier != NULL)
    {
      passagerTmp = dernier->value;
      if (dernier->value.arrivee == station)
      {
        if (dernier == f->tete)
          avant_dernier = f->tete;
        else 
          avant_dernier = dernier->precedent;
        maillon_rem(f,dernier);
        dernier = avant_dernier;
        //rem_maillon(f, dernier);
        if (station<NB_STATIONS_BUS){
          printf("BUS : debarque le passager %u à la station %u.\n", passagerTmp.id, station);
        }
        else{
          printf("METRO : debarque le passager %u à la station %u.\n", passagerTmp.id, station);
        }
      }
      
      if (dernier != NULL && (station==0 || station == NB_STATIONS_BUS || station == NB_STATIONS_BUS + NB_STATION_METRO) && passagerTmp.transfert )
      { // eviter les erreurs d'entrees
        printf("transfert passager %u \t vers station: %u\n",passagerTmp.transfert,station); 
        transferer(dernier, station,passagerTmp);
      }
      dernier = dernier->suivant;
    }
  }
}

//Cette fonction permet de tranférer un passager entre les files 'passager_bus' et 'passager_metro' ou inversement en fonction de la station src d'où il arrive.
void transferer(maillon_t* mtmp, uint32_t station_src, passager_t p)
{
  p.transfert = 0;
  if (station_src == 0)
  {
    p.depart = NB_STATIONS_BUS;
    maillon_rem(passagers_bus,mtmp);
    //rem_maillon(passagers_bus, mtmp);
    add_queue(files[NB_STATIONS_BUS], p);
  }
  else if( station_src == NB_STATIONS_BUS || station_src == NB_STATIONS_BUS + NB_STATION_METRO )
  {
    p.depart = 0;
    maillon_rem(passagers_metro,mtmp);
    //rem_maillon(passagers_metro, mtmp); 
    add_queue(files[0], p);
  }
  else 
  {
    perror("erreur lors de la descente d'un passager");
    exit(0);
  }
}

//Teste si chacune des files du tableau de files déclaré en variable global sont vides ou non.
uint32_t files_vides()
{
  for(uint32_t i = 0; i < NB_FILES; i++)
    if (files[i]->tete != NULL )
      return 0;
  return 1;
}

//Teste si la file passée en paramètre est vide ou non.
uint32_t file_vide(file_t* f)
{
  return (f->tete == NULL);
}

//Cette fonction prend en paramètre une chaîne de charactères ainsi que sa taille contenant des informations sur un passager, et renvoie une les données de ce passager dans une structure passager_t.
//Cependant elle ne marche pas très bien
passager_t atop(char* s, uint32_t taille)
{
  uint32_t i = 0;
  uint32_t j = 0;
  uint32_t k = 0;
  passager_t p;
  int tab[6];
  char buffer[5];
  while(s[i] != '\0')
  {
    if(s[i] == ' ')
    {
      j = 0;
      tab[k] = atoi(buffer);
      sprintf(buffer," ");
      k++;
    }
    else
    { 
      buffer[j] = s[i];
      j++;
    }
    i++;
  }
  tab[k] = atoi(buffer);

  p.id = tab[0];
  p.depart = tab[1];
  p.arrivee = tab[2];
  p.tps_attente = tab[3];
  p.transfert = tab[4];
  p.tps_max = tab[5];

  return p;
}