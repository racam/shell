#ifndef JOBSLIST_H
#define JOBSLIST_H

#include <sys/time.h>

struct cellule_t {
    pid_t pid;
    char *name;
    time_t timer;
    struct cellule_t *suiv;
};

void addJob(struct cellule_t **, pid_t, char *, time_t);
void deleteJob(struct cellule_t **, pid_t);
void printJobsList(struct cellule_t **const);
struct cellule_t* getJob(struct cellule_t **const, pid_t);
void deleteJobsList(struct cellule_t **);

#endif