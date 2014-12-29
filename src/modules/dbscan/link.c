#include "link.h"
#include <stdio.h>
#include <stdlib.h>

link *__initlink__(int k) {
	link *l;
	l = (link *) malloc( sizeof(link) );
	double *coordinates = (double *) malloc( sizeof(double)*k );
	l->cluster_id = 0;
	l->coordinates = coordinates;
	l->next = NULL;
	return l;
}

void destroy_link(link *l) {
	if(l->next) destroy_link(l->next);
	free(l->coordinates);
	free(l);
	return;
}