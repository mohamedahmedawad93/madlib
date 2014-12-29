#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifndef _KDTREEH_
#include "kdtree.h"
#endif

#ifndef _LINKEDLISTH_
#include "linkedlist.h"
#endif

#ifndef _LINKH_
#include "link.h"
#endif

#define NELEMS(x)  (sizeof(x) / sizeof(x[0]))
#define MAX_DIM 20

/*
	to compile:
		gcc -Wall dbscan.c kdtree.c linkedlist.c link.c -o my_app  
	to run:
		./my_app 
	to run under valgrind:
		valgrind --tool=memcheck --track-origins=yes --leak-check=full ./my_app
*/

int clusters;

int Neighbourhood_count_dbscan(kdnode *parent,range *r, int cd, int count, link *cluster, int clusters_num) {
	if(!parent) return count;
	int c = cd%(parent->point->k);
	double cd_range[2] = {r->range[c*2],r->range[c*2+1]};
	if(range_contains_point(parent->point,r)==0 && parent->visited==0) {
		count = count + 1;
		link *marker;
		marker = __initlink__(parent->point->k);
		int dimension;
		for (dimension = 0; dimension < parent->point->k; ++dimension)
		{
			marker->coordinates[dimension] = parent->point->coordinates[dimension];
		}
		marker->cluster_id = clusters_num;
		while(cluster->next) cluster = cluster->next;
		cluster->next = marker;
		cluster = cluster->next;
		parent->visited = 1;
		count = Neighbourhood_count_dbscan(parent->left,r,cd+1,count,cluster,clusters_num);
		count = Neighbourhood_count_dbscan(parent->right,r,cd+1,count,cluster,clusters_num);
	}
	else {
		int check;
		check = cd_contains_point(parent->point,c,cd_range);
		if (check==0) {
			count = Neighbourhood_count_dbscan(parent->left,r,cd+1,count,cluster,clusters_num);
			count = Neighbourhood_count_dbscan(parent->right,r,cd+1,count,cluster,clusters_num);
		}
		else {
			if(cd_range[0] > parent->point->coordinates[c]) {
				count = Neighbourhood_count_dbscan(parent->right,r,cd+1,count,cluster,clusters_num);
			}
			else {
				if(cd_range[1] < parent->point->coordinates[c]) count = Neighbourhood_count_dbscan(parent->left,r,cd+1,count,cluster,clusters_num);
			}
		}
	}
	return count;
}

void expand_cluster(int MinPts, double eps, linkedlist *cluster_ll, kdtree *tree, int k, int clusters_num) {
	link *current = cluster_ll->start;
	link *status = cluster_ll->start;
	int i_current = 0;
	int i_status = 0;
	while(current) {
		current->cluster_id = clusters;
		range *rng  = set_range(current->coordinates, eps, k);
		link *new_current = __initlink__(2);
		memcpy(new_current->coordinates, current->coordinates, sizeof(double)*k );
		int count = Neighbourhood_count_dbscan(tree->root,rng, 0, 0, new_current, clusters_num);
		if(count>=MinPts) {
			while(status->next) {status = status->next; i_status = i_status + 1;}
			status->next = new_current;
		}
		else {
			destroy_link(new_current);
		}
		current = current->next;
		i_current = i_current + 1;
		destroy_range(rng);
	}
	return;
}


void DBSCAN(int MinPts, double eps, kdnode *root, kdtree *tree, link *all) {
	if(!root) return;
	if(root->visited==0) {
		root->visited = 1;
		linkedlist *cluster_ll = __initlinkedkist__();
		link *cluster_l = __initlink__(2);
		int dimension;
		for (dimension = 0; dimension < root->point->k; ++dimension)
		{
			cluster_l->coordinates[dimension] = root->point->coordinates[dimension];
		}
		cluster_ll->start = cluster_l;
		range *rng = set_range(root->point->coordinates, eps, root->point->k);
		int count = Neighbourhood_count_dbscan(tree->root,rng, 0, 0, cluster_l, clusters);
		if(count>MinPts) {
			clusters = clusters + 1;
			cluster_ll->start->cluster_id = clusters;
			expand_cluster(MinPts, eps, cluster_ll, tree, root->point->k, clusters);
			while(all->next) all = all->next;
			all->next = cluster_ll->start;
		}
		else {
			destroy_linkedlist(cluster_ll);
		}
		destroy_range(rng);
	}
	DBSCAN(MinPts, eps, root->left, tree, all);
	DBSCAN(MinPts, eps, root->right, tree, all);
}

char **split(char *line, char sep, int fields) {
  char **r;
  if( !( r = (char **) malloc( sizeof(char *) * fields * 512) ) ) puts("CAN'T ALLOCATE");
  int lptr = 0, fptr = 0;
  r[fptr++] = line;
  while (line[lptr]) {
    if (line[lptr] == sep) {
      line[lptr] = '\0';
      r[fptr] = &(line[lptr+1]);
      fptr++;
    }

    lptr++;
  }
  return r;
}

int main (void) {
	puts("hello kd tree");
	kdtree *tree = __initTree__(2);
    FILE* stream = fopen("test_dbscan.csv", "r");
    char line[512];
    int csv_rows = 0;
    double lat_d, lng_d;
    Point *p;
    puts("before reading");
    while (fgets(line, 512, stream))
    {
    	/*if(csv_rows>10000) break;*/
		char **fields;
	    fields = split(line, ',', 15);
    	lat_d = atof(fields[1]);
    	lng_d = atof(fields[2]);
    	p = __initPoint__(2);
    	p->coordinates[0] = lat_d;
    	p->coordinates[1] = lng_d;
    	tree->root = insert(p,tree->root,2);
        csv_rows = csv_rows + 1;
        free(fields);
    }

	kdnode *min;
	printf("dimensions: %d\n", tree->root->point->k);
	min = FindMin(tree->root,0,1);
	puts("minimum:");
	printf("%lf  ", min->point->coordinates[0]);
	printf("%lf\n", min->point->coordinates[1]);
	kdnode *max;
	max = FindMax(tree->root,0,1);
	puts("maximum:");
	printf("%lf  ", max->point->coordinates[0]);
	printf("%lf\n", max->point->coordinates[1]);

	Point *q = __initPoint__(2);
	q->coordinates[0] = 30.043073;
	q->coordinates[1] = 31.232161;
	best *b;
	b = (best *) malloc( sizeof(best) );
	b->best = NULL;
	b->distance = 10000;
	b = NNeighbour(tree->root,q,0,b);
	printf("closest %lf  ", b->best->point->coordinates[0]);
	printf("%lf\n", b->best->point->coordinates[1]);
	destroy_point(q);
	free(b);
	int count;
	range *ran;
	ran = (range *) malloc( sizeof(range) );
	double *x = (double *) malloc (sizeof(double)*2*2);

	x[0] = 29.880389;
	x[1] = 30.181189;
	x[2] = 30.867899;
	x[3] = 31.473521;
	ran->range = x;
	count = Neighbourhood_count(tree->root,ran,0,0);
	printf("count: %d\n", count);
	/*	
	to test you can use the following query to test under PostGIS
	-----------------------------------------------------------------
	select count(*) 
	from MyTable 
	where 
	st_contains(st_geomfromtext('POLYGON((29.880389 30.867899, 29.880389 31.473521, 30.181189 31.473521, 30.181189 30.867899, 29.880389 30.867899))'), st_makepoint(lat,lng)); 
	------------------------------------------------------------------
	it will yield the same number...hopefully :/
	*/
	link *all_clusters = __initlink__(2);
	linkedlist *ll = __initlinkedkist__();	
	clusters = 0;
	clock_t begin, end;
	double time_spent;
	begin = clock();	
	puts("i should enter dbscan");
	DBSCAN(5, 0.001, tree->root, tree, all_clusters);
	puts("finished dbscan");
	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("total time: %f\n", time_spent);
	ll->start = all_clusters;
	print_linkedlist(ll);
	printf("total: %d\n", ll->length);
	
	destroy_linkedlist(ll);
	puts("destroy_linkedlist");
	puts("destroyed ll");
	destroy_tree(tree);
	puts("destroyed tree");
	destroy_range(ran);
	puts("finished printing");	
	return 0;
}
