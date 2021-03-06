/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*-------------------------------------------------------------------------
 *
 * Created:     H5Ctest.c
 *              June 7 2016
 *              Quincey Koziol
 *
 * Purpose:     Functions in this file support the metadata cache regression
 *		tests>
 *
 *-------------------------------------------------------------------------
 */

/****************/
/* Module Setup */
/****************/

#include "H5Cmodule.h"          /* This source code file is part of the H5C module */
#define H5C_TESTING		/*suppress warning about H5C testing funcs*/
#define H5F_FRIEND		/*suppress error about including H5Fpkg	  */


/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions			*/
#include "H5Cpkg.h"		/* Cache				*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5Fpkg.h"		/* Files				*/
#include "H5Iprivate.h"		/* IDs			  		*/


/****************/
/* Local Macros */
/****************/


/******************/
/* Local Typedefs */
/******************/

/* Typedef for tagged entry iterator callback context - verify cork tag */
typedef struct {
    hbool_t status;             /* Corked status */
} H5C_tag_iter_vct_ctx_t;


/********************/
/* Local Prototypes */
/********************/


/*********************/
/* Package Variables */
/*********************/


/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/




/*-------------------------------------------------------------------------
 * Function:    H5C__verify_cork_tag_test_cb
 *
 * Purpose:     Verify the cork status for an entry
 *
 * Return:      SUCCEED on success, FAIL on error
 *
 * Programmer:  Vailin Choi
 *		Feb 2014
 *
 *-------------------------------------------------------------------------
 */
static int
H5C__verify_cork_tag_test_cb(H5C_cache_entry_t *entry, void *_ctx)
{
    H5C_tag_iter_vct_ctx_t *ctx = (H5C_tag_iter_vct_ctx_t *)_ctx; /* Get pointer to iterator context */
    int ret_value = H5_ITER_CONT;       /* Return value */

    /* Function enter macro */
    FUNC_ENTER_STATIC

    /* Santify checks */
    HDassert(entry);
    HDassert(ctx);

    /* Verify corked status for entry */
    if(entry->is_corked != ctx->status)
        HGOTO_ERROR(H5E_CACHE, H5E_BADVALUE, H5_ITER_ERROR, "bad cork status")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* H5C__verify_cork_tag_test_cb() */


/*-------------------------------------------------------------------------
 * Function:    H5C__verify_cork_tag_test
 *
 * Purpose:     This routine verifies that all cache entries associated with
 *      the object tag are marked with the desired "cork" status.
 *
 * Return:      SUCCEED on success, FAIL on error
 *
 * Programmer:  Vailin Choi
 *		Feb 2014
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5C__verify_cork_tag_test(hid_t fid, haddr_t tag, hbool_t status)
{
    H5F_t * f;                  /* File Pointer */
    H5C_t * cache;              /* Cache Pointer */
    H5C_tag_iter_vct_ctx_t ctx; /* Context for iterator callback */
    herr_t ret_value = SUCCEED; /* Return value */

    /* Function enter macro */
    FUNC_ENTER_PACKAGE

    /* Get file pointer */
    if(NULL == (f = (H5F_t *)H5I_object_verify(fid, H5I_FILE)))
	HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a file")

    /* Get cache pointer */
    cache = f->shared->cache;

    /* Construct context for iterator callbacks */
    ctx.status = status;

    /* Iterate through tagged entries in the cache */
    if(H5C__iter_tagged_entries(cache, tag, FALSE, H5C__verify_cork_tag_test_cb, &ctx) < 0)
        HGOTO_ERROR(H5E_CACHE, H5E_BADITER, FAIL, "iteration of tagged entries failed")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* H5C__verify_cork_tag_test() */

