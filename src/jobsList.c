#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jobsList.h"


void addJob(struct cellule_t** liste, pid_t pid, char *name, time_t timer){
    struct cellule_t *newCell = calloc(1, sizeof(struct cellule_t));assert(newCell);
    
    newCell->pid = pid;
    newCell->timer = timer;

    //Copy of the name
    newCell->name = calloc(strlen(name)+1, sizeof(char)); assert(newCell->name);
    strcpy(newCell->name, name);

    newCell->suiv = *liste;
    *liste = newCell;
}


void deleteJob(struct cellule_t** liste, pid_t pid){
    struct cellule_t* cell = *liste;
    struct cellule_t* prev = NULL;

    while(cell != NULL){
        
        //We found it !
        if(cell->pid == pid){
            if(prev != NULL){
                //Normal delete
                prev->suiv = cell->suiv;
            }else{
                //Delete on head
                *liste = (*liste)->suiv;
            }
            
            free(cell->name);
            free(cell);
            break;
        }

        prev = cell;
        cell = cell->suiv;
    }
}

struct cellule_t* getJob(struct cellule_t **const liste, pid_t pid){
    struct cellule_t* cell = *liste;
    while(cell != NULL){
        if(cell->pid == pid) return cell;
        cell = cell->suiv;
    }
    return NULL;
}

void printJobsList(struct cellule_t **const liste){
    struct cellule_t *cell = *liste;
    while(cell != NULL){
        printf("[%d]%s \n", cell->pid, cell->name);
        cell = cell->suiv;
    }
}

void deleteJobsList(struct cellule_t **liste){
    while (*liste != NULL) {
        struct cellule_t *suiv = (*liste)->suiv;
        free((*liste)->name);
        free(*liste);
        *liste = suiv;
    }
}