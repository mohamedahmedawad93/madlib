#ifndef _LINKH_

#define _LINKH_

typedef struct link {
	char *type;
	double *coordinates;
	int cluster_id;
	struct link *next;
}link;

link *__initlink__(int k);
void destroy_link(link *l);

#endif