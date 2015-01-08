/*
cc -fpic -c postgresql_test.c dbscan.c kdtree.c linkedlist.c link.c -I/usr/include/postgresql/9.3/server
cc -shared -o postgresql_test.so postgresql_test.o dbscan.o kdtree.o linkedlist.o link.o
*/

#include <stdio.h>
#include <stdlib.h>
#include "time.h"
#include "postgres.h"
#include "fmgr.h"
#include <string.h>
#include "executor/executor.h"  /* for GetAttributeByName() */
#include "executor/spi.h"
#include "funcapi.h"
#include "miscadmin.h" 
#include "utils/builtins.h"
#include "dbscan.h"
#include "kdtree.h"
#include "linkedlist.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif
/* by value */

PG_FUNCTION_INFO_V1(retcomposite);

Datum
dbscan_main(PG_FUNCTION_ARGS)
{
    elog(INFO, "hi");
    text                *sql = PG_GETARG_TEXT_PP(0);
    int                  x = PG_GETARG_INT16(1);
    char xs[10];
    snprintf(xs,12,"%d",x);
    elog(INFO,xs);
    double               y = PG_GETARG_FLOAT4(2);
    elog(INFO, "hi");
    char                *command = text_to_cstring(sql);
    ReturnSetInfo       *rsinfo = (ReturnSetInfo *) fcinfo->resultinfo;
    FuncCallContext     *funcctx;
    int                  call_cntr;
    int                  max_calls;
    TupleDesc            tupdesc;
    AttInMetadata       *attinmeta;
    SPITupleTable       *spi_tuptable;
    MemoryContext        per_query_ctx;
    MemoryContext        oldcontext;
    TupleDesc            spi_tupdesc;
    Tuplestorestate     *tupstore;

    int ret = SPI_connect();
    per_query_ctx = rsinfo->econtext->ecxt_per_query_memory;
  /*ereport(ERROR,(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),errmsg("hi")));*/

    if (rsinfo == NULL || !IsA(rsinfo, ReturnSetInfo))
        ereport(ERROR,
               (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                 errmsg("set-valued function called in context that cannot accept a set")));


   if (!(rsinfo->allowedModes & SFRM_Materialize))
       ereport(ERROR,
               (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                errmsg("materialize mode required, but it is not " \
                       "allowed in this context")));
 	ret = SPI_execute(command, false, 0);
 	int proc = SPI_processed;
 	
    if (ret != SPI_OK_SELECT || proc <= 0)
     {
         SPI_finish();
         rsinfo->isDone = ExprEndResult;
         PG_RETURN_NULL();
    }


 	char sproc[5];
 	snprintf(sproc, 12,"%d",proc);
 	elog(INFO, sproc);

 	spi_tuptable = SPI_tuptable;
 	spi_tupdesc = spi_tuptable->tupdesc;
     /* get a tuple descriptor for our result type */
       switch (get_call_result_type(fcinfo, NULL, &tupdesc))
       {
           case TYPEFUNC_COMPOSITE:
               /* success */
           	   /*elog(INFO, "heeey");*/
               break;
           case TYPEFUNC_RECORD:
               /* failed to determine actual type of RECORD */
               ereport(ERROR,
                       (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
                        errmsg("function returning record called in context "
                               "that cannot accept type record")));
               break;
           default:
               /* result type isn't composite */
               elog(ERROR, "return type must be a row type");
               break;
       }

 	oldcontext = MemoryContextSwitchTo(per_query_ctx);

 	tupdesc = CreateTupleDescCopy(tupdesc);

 	tupstore = tuplestore_begin_heap(rsinfo->allowedModes & SFRM_Materialize_Random,false, work_mem);

 	MemoryContextSwitchTo(oldcontext);

 	attinmeta = TupleDescGetAttInMetadata(tupdesc);

 	max_calls = proc;

 	int num_categories = tupdesc->natts - 1;

 	kdtree *tree = __initTree__(num_categories);
 	char satt[15];

	clock_t begin, end;
	double time_spent;
	begin = clock();	
	elog(INFO, "before reading");
 	for (call_cntr = 0; call_cntr < max_calls; call_cntr++) {
 		HeapTuple   spi_tuple = spi_tuptable->vals[call_cntr];
 		char        *rowid = SPI_getvalue(spi_tuple, spi_tupdesc, 1);;
 		Point       *p;
 		int          att = 0;
    double      *values = (double *) palloc0((1 + num_categories) * sizeof(double));

 		snprintf(satt, 12,"%d",num_categories);

 		for (att= 0; att < num_categories; ++att)
 		{
 			rowid = SPI_getvalue(spi_tuple, spi_tupdesc, att+1);
 			values[att] = atof(rowid);
 		}

 		p = __initPoint__(num_categories);
 		p->coordinates = values;
 		tree->root = insert(p,tree->root,num_categories);    	
 	}

 	elog(INFO, "processed to kdtree");

 	linkedlist *ll;
	
  char cat[20];
	snprintf(cat,12,"%d",num_categories);
	elog(INFO, cat);

  ll = run_dbscan(tree, num_categories, x, y);

  char length[20];
  snprintf(length, 12,"%d",ll->length);
  elog(INFO,length);
  if(!ll->start) {
       SPI_finish();
       rsinfo->isDone = ExprEndResult;
       PG_RETURN_NULL();
  }

  link *current = ll->start;
  int c = 0;
	int cx = 0;
  
  while(current) {
  	HeapTuple   tuple;
  	char      **results = (char **) palloc( sizeof(char*)*num_categories);
  	int         att;
  	char        id[20];
  	int         cluster_id = current->cluster_id;
  	
    snprintf(id,12,"%d",cluster_id);
  	results[0] = id;
 		
    for (att= 0; att < num_categories; ++att)
 		{
    	char temp[20];
    	snprintf(temp,19,"%f",current->coordinates[att]);
    	int n = strlen(temp);
 			results[att+1] = (char *) palloc(sizeof(char)*n+1);
 			strcpy(results[att+1], temp);
		}

  	tuple = BuildTupleFromCStrings(attinmeta, results);
  	tuplestore_puttuple(tupstore, tuple);
  	heap_freetuple(tuple);

  	current = current->next;

  }

  rsinfo->returnMode = SFRM_Materialize;
  rsinfo->setResult = tupstore;
  rsinfo->setDesc = tupdesc;
  SPI_finish();
/*  destroy_tree(tree);
  destroy_linkedlist(ll);
*/ 	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
 	char stime[20];
 	snprintf(stime, 12,"%f",time_spent);
 	elog(INFO, stime);
 	return(Datum) 0;
}
