#ifndef _LINKH_
#include "link.h"
#endif

#ifndef _LINKEDLISTH_
#define _LINKEDLISTH_   /* Include guard */

typedef struct linkedlist {
	link *start;
	int length;
}linkedlist;

linkedlist *__initlinkedkist__();
void insert_linkedlist(linkedlist *ll, link *l, int k);
void print_linkedlist(linkedlist *ll);
void destroy_linkedlist(linkedlist *ll);

#endif