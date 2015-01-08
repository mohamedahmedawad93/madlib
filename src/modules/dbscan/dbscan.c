#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <postgres.h>
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

int Neighbourhood_count_dbscan(kdnode *parent,range *r, int cd, int count, link *cluster, int clusters_num, int k) {
	if(!parent) return count;
	int c = cd%(parent->point->k);
	double cd_range[2] = {r->range[c*2],r->range[c*2+1]};
	if(range_contains_point(parent->point,r)==0 && parent->visited==0) {
		count = count + 1;
		link *marker;
		marker = __initlink__(k);
		memcpy(marker->coordinates,parent->point->coordinates,sizeof(double)*k);
		marker->cluster_id = clusters_num;
		while(cluster->next) cluster = cluster->next;
		cluster->next = marker;
		cluster = cluster->next;
		parent->visited = 1;
		count = Neighbourhood_count_dbscan(parent->left,r,cd+1,count,cluster,clusters_num,k);
		count = Neighbourhood_count_dbscan(parent->right,r,cd+1,count,cluster,clusters_num,k);
	}
	else {
		int check;
		check = cd_contains_point(parent->point,c,cd_range);
		if (check==0) {
			count = Neighbourhood_count_dbscan(parent->left,r,cd+1,count,cluster,clusters_num,k);
			count = Neighbourhood_count_dbscan(parent->right,r,cd+1,count,cluster,clusters_num,k);
		}
		else {
			if(cd_range[0] > parent->point->coordinates[c]) {
				count = Neighbourhood_count_dbscan(parent->right,r,cd+1,count,cluster,clusters_num,k);
			}
			else {
				if(cd_range[1] < parent->point->coordinates[c]) count = Neighbourhood_count_dbscan(parent->left,r,cd+1,count,cluster,clusters_num,k);
			}
		}
	}
	return count;
}

void expand_cluster(int MinPts, double eps, linkedlist *cluster_ll, kdtree *tree, int k, int clusters_num) {
	link *current = cluster_ll->start;
	link *status = cluster_ll->start;
	int i_current = 0;
	while(current) {
		current->cluster_id = clusters;
		range *rng  = set_range(current->coordinates, eps, k);
		link *new_current = __initlink__(k);
		memcpy(new_current->coordinates, current->coordinates, sizeof(double)*k );
		int count = Neighbourhood_count_dbscan(tree->root,rng, 0, 0, new_current, clusters_num, k);
		if(count>=MinPts) {
			while(status->next) status = status->next;
			status->next = new_current;
		}
		else {
			destroy_link(new_current);
		}
		current = current->next;
		destroy_range(rng);
	}
	return;
}

void DBSCAN(int k, int MinPts, double eps, kdnode *root, kdtree *tree, link *all) {
	if(!root) return;
	if(root->visited==0) {
		root->visited = 1;
		linkedlist *cluster_ll = __initlinkedkist__();
		link *cluster_l = __initlink__(k);
		memcpy(cluster_l->coordinates,root->point->coordinates,sizeof(double)*k);
		cluster_ll->start = cluster_l;
		range *rng = set_range(root->point->coordinates, eps, k);
		int count = Neighbourhood_count_dbscan(tree->root,rng, 0, 0, cluster_l, clusters,k);
		if(count>MinPts) {
			clusters = clusters + 1;
			cluster_ll->start->cluster_id = clusters;
			expand_cluster(MinPts, eps, cluster_ll, tree, k, clusters);
			while(all->next) all = all->next;
			all->next = cluster_ll->start;
		}
		else {
			destroy_linkedlist(cluster_ll);
		}
		destroy_range(rng);
	}
	DBSCAN(k, MinPts, eps, root->left, tree, all);
	DBSCAN(k, MinPts, eps, root->right, tree, all);
}

linkedlist *run_dbscan (kdtree *tree, int k, int Minpts, double eps) {
	link *all_clusters = __initlink__(k);
	char cat[20];
	snprintf(cat,12,"%d",k);
	elog(INFO, cat);	
	linkedlist *ll = __initlinkedkist__();
	clusters = 0;
	DBSCAN(k, Minpts, eps, tree->root, tree, all_clusters);
	ll->start = all_clusters;
	return ll;
}
