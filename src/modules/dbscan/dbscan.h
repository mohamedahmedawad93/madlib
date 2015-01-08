#ifndef _DBSCAN_
#define _DBSCAN_

#ifndef _LINKEDLISTH_
#include "linkedlist.h"
#endif

#ifndef _KDTREEH_
#include "kdtree.h"
#endif

linkedlist *run_dbscan (kdtree *tree, int k, int Minpts, double eps);

#endif
