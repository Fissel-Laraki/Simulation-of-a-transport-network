#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "file.h"

file_t* new_file()
{
  file_t *t = (file_t*)malloc(sizeof(file_t));
  if(t == NULL)
    assert(0);
  t->taille = 0;
  t->tete = NULL;
  t->queue = NULL;
  return t;
}

maillon_t * new_maillon(passager_t d)
{
  maillon_t* m = (maillon_t*) malloc(sizeof(maillon_t));
  if(m == NULL)
    assert(0);
  m->value = d;
  return m;
}

void add_tete(file_t* l,passager_t d)
{
  maillon_t* m = new_maillon(d);
  m->suivant = l->tete;
  m->precedent = NULL;
  


  if (l->taille == 0)
  {
    l->queue = m;
  }
  else
  {
    l->tete->precedent = m;  
  }
  
  l->tete = m;
  l->taille += 1;
}

void add_queue(file_t* l,passager_t d)
{
  maillon_t* m = new_maillon(d);
  m->suivant = NULL;
  m->precedent = l->queue;

  if(l->taille > 0)
    l->queue->suivant = m;
  else
    l->tete = m ;

  l->queue = m;
  l->taille += 1;

}

void rem_queue(file_t *l)
 {

   maillon_t * tmp = l->queue;
   l->queue = l->queue->precedent;
   free(tmp);
   l->taille -=1;

  if(l->taille == 0)
  {
    l -> tete = NULL;
  }
  else
  {
    l->queue->suivant = NULL;  
  }
  
  
 }

void rem_tete(file_t *l )
{
  maillon_t* tmp = l->tete;
  l->tete = l->tete->suivant;

  free(tmp);
  l->taille -=1;
  if (l->taille == 0  ){
    l->queue = NULL;
  }
  else
  {
    l->tete->precedent = NULL;
  }
    
}

//Cette fonction ne marche pas
void free_file(file_t *l)
{

  printf("au debut : %lu\n",l->taille );
  while (l->tete != NULL)
  {

    rem_tete(l);
    printf("maintenant : %lu\n",l->taille );

  }
  //free(l);

}



void afficher_data(passager_t d)
{
  printf("%d\t%d\t%d\t%ld\t%d\t%ld\n",d.id,d.depart,d.arrivee,d.tps_attente,d.transfert,d.tps_max);
}

void parcours(file_t * l)
{
  if (l->taille>0)
  {
    maillon_t* tmp = l->tete;
    while(tmp != NULL)
    {
      afficher_data(tmp->value);
      tmp = tmp->suivant;
    }
  }
}

void creer_files(file_t**f,uint32_t NB_FILES)
{
  for (int i=0; i<NB_FILES; i++)
      f[i]=new_file();
}

void afficher_files(file_t**f,uint32_t NB_FILES)
{
  for(uint32_t i = 0;i < NB_FILES; i++)
  {
    parcours(f[i]);
  }
}

int remplir_files(file_t** f,uint32_t NB_FILES,const char* nomFichier){
  FILE* fichier = fopen(nomFichier,"r");
  uint32_t nbr_passagers = 0;
  passager_t p;
  if (fichier != NULL) // Succes
  {
    fscanf(fichier,"%d\n",&nbr_passagers);
    for (uint32_t i = 0; i< nbr_passagers && !(feof(fichier)); i++)
    {
      fscanf(fichier, "# %d %d %d %ld %d %ld\n", &p.id, &p.depart, &p.arrivee, &p.tps_attente, &p.transfert, &p.tps_max);
      if(p.depart<=4){
        add_queue(f[p.depart],p);
      }
      else if (p.depart<p.arrivee)
        add_queue(f[p.depart],p);
      else 
        add_queue(f[p.depart + 3],p);
    }
    fclose(fichier);
  }
  else
  {
    perror("erreur lors de l'ouverture du fichier ");
    exit(0);
  }
  return nbr_passagers;
}

void rem_maillon(file_t *l, maillon_t* mtmp)
 {
  maillon_t *m, *r;
  if (mtmp == l->tete){
    rem_tete(l);
  }
  else
  {
    r=trouver_maillon(l, mtmp);
    m = r->suivant;
    r->suivant = m->suivant;

    free(m);
    l->taille -= 1;
  }
}

maillon_t* trouver_maillon(file_t* f, maillon_t* mtmp)
{
  maillon_t* avant_dernier = NULL;
  for(avant_dernier = f->tete; avant_dernier->suivant != mtmp; avant_dernier=avant_dernier->suivant);
  return avant_dernier;

}

void maillon_rem(file_t* f,maillon_t* m)
{
    maillon_t * tmp,*suiv,*prec;

    if (m == f->tete){
      rem_tete(f);
    }
    else if(m == f->queue){
      rem_queue(f);
    }
    else{
        tmp = m;
        prec = tmp->precedent;
        suiv = tmp->suivant;
        prec->suivant = suiv;
        suiv->precedent = prec;
        free(tmp);
        f->taille -=1;
    }
}