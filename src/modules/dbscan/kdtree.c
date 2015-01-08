#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "kdtree.h"
#include <postgres.h>

#define NELEMS(x)  (sizeof(x) / sizeof(x[0]))
#define MAX_DIM 20

/*to open gcc kdtree.c -std=c99 -g -o kdtree*/

Point *__initPoint__(int k) {
	Point *p;
	p = (Point *) palloc( sizeof(Point) );
	p->k = k;
	double *coordinates = (double *) palloc (sizeof(double)*k );
	p->coordinates = coordinates;
	return p;
}

kdnode *__initNode__ (Point *point) {
	kdnode *node;
	node = (kdnode *) palloc( sizeof(kdnode) );
	node->point = point;
	node->visited = 0;
	node->left=NULL;
	node->right=NULL;
	return node;
}

kdtree *__initTree__(int k){
	struct kdtree *tree;
	tree = (kdtree *) palloc( sizeof(kdtree) );
	tree->root = NULL;
	tree->k = k;
	return tree;
}

kdnode *insert(Point *point, kdnode *node, int k) {
	if (!node) {
		kdnode *n = __initNode__(point);
		return n;
	}
	else {
		int c = k;
		int dim = point->k;
		c = c%dim;
		if (point->coordinates[c] < node->point->coordinates[c]) {
			node->left = insert(point,node->left,c+1); 
		}
		else {
			node->right = insert(point,node->right,c+1);
		}
	}
	return node;
}

kdnode *minimum(kdnode *current, kdnode *left, kdnode *right, int i) {
	if (!current || !left || !right)
	{
		if (!current)
		{
			if (left->point->coordinates[i] > right->point->coordinates[i]) return right;
			else return left;
		}
		else{
			if (!left)
			{
				if (current->point->coordinates[i] > right->point->coordinates[i]) return right;
				else return current;
			}
			else {
				if (current->point->coordinates[i] > left->point->coordinates[i]) {puts("returning");return left;}
				else return current;
			}
		}
	}
	else {
		if (current->point->coordinates[i] < left->point->coordinates[i] && current->point->coordinates[i] < right->point->coordinates[i]) return current;
		if (left->point->coordinates[i] < current->point->coordinates[i] && left->point->coordinates[i] < right->point->coordinates[i]) return left;
		if (right->point->coordinates[i] < current->point->coordinates[i] && right->point->coordinates[i] < left->point->coordinates[i]) return right;
		else return NULL;
	}
}

kdnode *FindMin(kdnode *parent, int cd,int i) {
	if (!parent) return NULL;
	int dim = parent->point->k;
	int c = cd%dim;
	if (c == i) {
		if(!parent->left) return parent;
		else return FindMin(parent->left, cd+1, i);
	}
	else {
		kdnode *left;
		left = FindMin(parent->left, cd+1, i);
		kdnode *right;
		right = FindMin(parent->right, cd+1, i);
		if (right || left) return minimum(parent, left, right, i);
		else {
			if(parent) return parent;
			else return NULL;
		}
	}
}



kdnode *maximum(kdnode *current, kdnode *left, kdnode *right, int i) {
	if (!current || !left || !right)
	{
		if (!current)
		{
			if (left->point->coordinates[i] < right->point->coordinates[i]) return right;
			else return left;
		}
		else{
			if (!left)
			{
				if (current->point->coordinates[i] < right->point->coordinates[i]) return right;
				else return current;
			}
			else {
				if (current->point->coordinates[i] < left->point->coordinates[i]) return left;
				else return current;
			}
		}
	}
	else {
		if (current->point->coordinates[i] > left->point->coordinates[i] && current->point->coordinates[i] > right->point->coordinates[i]) return current;
		if (left->point->coordinates[i] > current->point->coordinates[i] && left->point->coordinates[i] > right->point->coordinates[i]) return left;
		if (right->point->coordinates[i] > current->point->coordinates[i] && right->point->coordinates[i] > left->point->coordinates[i]) return right;
		else return NULL;
	}
}

kdnode *FindMax(kdnode *parent, int cd,int i) {
	if (!parent) return NULL;
	int dim = parent->point->k;
	int c = cd%dim;
	if (c == i) {
		if(!parent->right) return parent;
		else return FindMax(parent->right, cd+1, i);
	}
	else {
		kdnode *left;
		left = FindMax(parent->left, cd+1, i);
		kdnode *right;
		right = FindMax(parent->right, cd+1, i);
		if (right || left) return maximum(parent, left, right, i);
		else {
			if(parent) return parent;
			else return NULL;
		}
	}
}

double dis(kdnode *a, Point *b) {
	int k = a->point->k;
	if (k != b->k) return 0;
	double distance = 0, d = 0, d2 = 0;
	int i;
	for (i = 0; i < k; ++i)
	{
		d = a->point->coordinates[i] - b->coordinates[i];
		d2 = d * d;
		distance = distance + d2;
	}
	return distance;
}

best *NNeighbour(kdnode *parent,Point *p, int cd, best * close) {
	if (!parent) return close;
	double distance = 0;
	distance = dis(parent, p);
	double d;
	int c = cd;
	int dim = p->k;
	c = c%dim;
	d = parent->point->coordinates[c] - p->coordinates[c];
	if (distance < close->distance) {close->distance=distance; close->best=parent;}
	struct best *sofar;
	if (d<0) {sofar = NNeighbour(parent->right, p, cd+1, close);}
	else {sofar = NNeighbour(parent->left, p, cd+1, close);}
	if(sofar->distance < close->distance) {return sofar;}
	else {return close;}
}

int range_contains_point(Point *p, range *r) {
	int dim = p->k;
	double min_k, max_k;
	int order = 0;
	int i;
	for (i = 0; i < dim; ++i)
	{
		min_k = r->range[order];
		order = order + 1;
		max_k = r->range[order];
		order = order + 1;
		if(!(p->coordinates[i]<=max_k) || !(p->coordinates[i]>=min_k)) return -1;
	}
	return 0;
}

int cd_contains_point(Point *p, int cd, double *r) {
	if(p->coordinates[cd]<=r[1] && p->coordinates[cd]>=r[0]) return 0;
	else return -1;
}

int Neighbourhood_count(kdnode *parent,range *r, int cd, int count) {
	if(!parent) return count;
	int c = cd%(parent->point->k);
	double cd_range[2] = {r->range[c*2],r->range[c*2+1]};
	if(range_contains_point(parent->point,r)==0) {
		count = count + 1;
		count = Neighbourhood_count(parent->left,r,cd+1,count);
		count = Neighbourhood_count(parent->right,r,cd+1,count);
	}
	else {
		int check;
		check = cd_contains_point(parent->point,c,cd_range);
		if (check==0) {
			count = Neighbourhood_count(parent->left,r,cd+1,count);
			count = Neighbourhood_count(parent->right,r,cd+1,count);
		}
		else {
			if(cd_range[0] > parent->point->coordinates[c]) {
				count = Neighbourhood_count(parent->right,r,cd+1,count);
			}
			else {
				if(cd_range[1] < parent->point->coordinates[c]) count = Neighbourhood_count(parent->left,r,cd+1,count);
			}
		}
	}
	return count;
}

void print_coordinates(double coordinates[]) {
	int i;
	for (i = 0; i <= NELEMS(coordinates); ++i)
	{
		printf("%f ", coordinates[i]);
	}
	printf("\n");
}

void print_node(kdnode *node) {
	int i;
	for (i = 0; i < node->point->k; ++i)
	{
		printf("%f,", node->point->coordinates[i]);
	}
	puts("");
}

void print_range(range *r, int k, double *coords) {
	int i;
	for (i = 0; i < k*2; ++i)
	{
		printf("(%f-->", r->range[i]);
		i = i + 1;
		printf("%f),", r->range[i]);
	}
	puts("");
}

void destroy_range(range *r) {
	pfree(r->range);
	pfree(r);
}

range *set_range(double *coords, double eps, int dimensions) {
	range *r;
	r = (range *) palloc( sizeof(range) );
	double *x = (double *) palloc ( sizeof(double)*dimensions*2 );
	double d = eps;
	int i;
	int dim_count = 0;
	for (i = 0; i < dimensions*2; ++i)
	{
		x[i] = coords[dim_count] - d;
		i = i + 1;
		x[i] = coords[dim_count] + d;
		++dim_count;
	}
	r->range = x;
	return r;
}

void destroy_point(Point *p) {
	pfree(p->coordinates);
	p->k = 0;
	pfree(p);
}

void destroy_nodes(kdnode *node) {
	if(!node) return;
	destroy_nodes(node->left);
	destroy_nodes(node->right);
	destroy_point(node->point);
	pfree(node);
}
void destroy_tree(kdtree *tree) {
	destroy_nodes(tree->root);
	pfree(tree);
}
