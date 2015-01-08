#include "link.h"
#include <stdio.h>
#include <stdlib.h>
#include <postgres.h>

link *__initlink__(int k) {
	link *l;
	l = (link *) palloc( sizeof(link) );
	double *coordinates = (double *) palloc( sizeof(double)*k );
	l->cluster_id = 0;
	l->coordinates = coordinates;
	l->next = NULL;
	return l;
}

void destroy_link(link *l) {
	if(l->next) destroy_link(l->next);
	pfree(l->coordinates);
	pfree(l);
	return;
}