#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

typedef struct {
  uint32_t id;
  uint32_t depart;
  uint32_t arrivee;
  uint64_t tps_attente;
  uint32_t transfert;
  uint64_t tps_max;
}passager_t;

typedef struct maillon_s{
  passager_t value;
  struct maillon_s * suivant;
  struct maillon_s * precedent;
}maillon_t;

typedef struct {
  maillon_t* tete;
  maillon_t* queue;
  uint64_t taille;
}file_t;

file_t* new_file();
int remplir_files(file_t**,uint32_t,const char*);
void creer_files(file_t**,uint32_t);
void afficher_files(file_t**,uint32_t);
maillon_t * new_maillon(passager_t d);
void add_tete(file_t* l,passager_t d);
void add_queue(file_t* l,passager_t d);
void add_i(file_t *l , passager_t d , uint32_t i);
void rem_i(file_t* l,uint32_t i);
void rem_tete(file_t *l );
void rem_queue(file_t * l);
void free_file(file_t *l);//ne marche pas
void parcours(file_t * l);
void parcours_inverse(file_t* l);
void afficher_data(passager_t d);
void rem_maillon(file_t *l, maillon_t*);
maillon_t* trouver_maillon(file_t* f, maillon_t*);
void maillon_rem(file_t* f,maillon_t* m);
