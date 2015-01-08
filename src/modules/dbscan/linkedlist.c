#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "linkedlist.h"
#include <postgres.h>

linkedlist *__initlinkedkist__() {
	linkedlist *ll;
	ll = (linkedlist *) palloc( sizeof(linkedlist) );
	ll->length = 0;
	ll->start = NULL;
	return ll;
}

void insert_linkedlist(linkedlist *ll, link *l, int k) {
	if(!ll->start) {
		ll->start = (link *) palloc( sizeof(link) );
		ll->start = l;
		ll->length = ll->length+1;
		return;
	}
	else {
		link *parent = ll->start;
		link *child = ll->start->next;
		while(child) {
			parent = child;
			child = parent->next;
		}
		parent->next = l;
		ll->length = ll->length+1;
	}
}

void print_linkedlist(linkedlist *ll) {
	if(!ll->start) {puts("LINKED LIST EMPTY");return;}
	FILE *fs = fopen("dbscan.csv", "a");
	link *l;
	l = ll->start;
	ll->length = 0;
	int id = 1;
	while(l->next) {
		fprintf(fs, "%d,%3.13f,%3.13f,%d\n", id,l->coordinates[0],l->coordinates[1], l->cluster_id);
		++id;
		ll->length = ll->length+1;
		l = l->next;
	}
	return;
}

void destroy_linkedlist(linkedlist *ll) {
	if(!ll->start) {pfree(ll); return;}
	destroy_link(ll->start);
	pfree(ll);
	return;
}
