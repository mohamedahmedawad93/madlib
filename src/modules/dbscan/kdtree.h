#ifndef _KDTREEH_  /* Include guard */
#define _KDTREEH_
typedef struct kdtree {
	struct kdnode *root;
	int k;
}kdtree;

#ifndef _POINTH_
#define _POINTH_
typedef struct Point {
	int k;
	double *coordinates;
}Point;
#endif

#ifndef _KDNODEH_
#define _KDNODEH_
typedef struct kdnode {
	struct kdnode *right;
	struct kdnode *left;
	struct Point *point;
	int visited;
}kdnode;
#endif

#ifndef _BESTH_
#define _BESTH_
typedef struct best {
	kdnode *best;
	double distance;
}best;
#endif

#ifndef _RANGEH_
#define _RANGEH_
typedef struct range
{
	double *range;
}range;
#endif

Point *__initPoint__(int k);
kdnode *__initNode__ (Point *point);
kdtree *__initTree__(int k);
kdnode *insert(Point *point, kdnode *node, int k);
kdnode *minimum(kdnode *current, kdnode *left, kdnode *right, int i);
kdnode *FindMin(kdnode *parent, int cd,int i);
kdnode *maximum(kdnode *current, kdnode *left, kdnode *right, int i);
kdnode *FindMax(kdnode *parent, int cd,int i);
double dis(kdnode *a, Point *b);
best *NNeighbour(kdnode *parent,Point *p, int cd, best * close);
int range_contains_point(Point *p, range *r);
int cd_contains_point(Point *p, int cd, double *r);
int Neighbourhood_count(kdnode *parent,range *r, int cd, int count);
void print_coordinates(double coordinates[]);
void print_node(kdnode *node);
void print_range(range *r, int k, double *coords);
void destroy_range(range *r);
range *set_range(double *coords, double eps, int dimensions);
void destroy_point(Point *p);
void destroy_nodes(kdnode *node);
void destroy_tree(kdtree *tree);  /* An example function declaration */

#endif