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
 * Created:     H5Cepoch.c
 *              June 5 2004
 *              Quincey Koziol
 *
 * Purpose:     Metadata cache epoch callbacks.
 *
 *-------------------------------------------------------------------------
 */


/****************/
/* Module Setup */
/****************/

#include "H5Cmodule.h"          /* This source code file is part of the H5C module */


/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions			*/
#include "H5Cpkg.h"		/* Cache				*/


/****************/
/* Local Macros */
/****************/


/******************/
/* Local Typedefs */
/******************/


/********************/
/* Local Prototypes */
/********************/

/****************************************************************************
 *
 * declarations for epoch marker cache entries.
 *
 * As a strategy for automatic cache size reduction, the cache may insert
 * marker entries in the LRU list at the end of each epoch.  These markers
 * are then used to identify entries that have not been accessed for n
 * epochs so that they can be evicted from the cache.
 *
 ****************************************************************************/
static herr_t H5C__epoch_marker_get_load_size(const void *udata_ptr,
    size_t *image_len_ptr);
static void * H5C__epoch_marker_deserialize(const void * image_ptr,
    size_t len, void * udata, hbool_t * dirty_ptr);
static herr_t H5C__epoch_marker_image_len(const void * thing,
    size_t *image_len_ptr, hbool_t *compressed_ptr, size_t *compressed_len_ptr);
static herr_t H5C__epoch_marker_pre_serialize(const H5F_t *f,
    hid_t dxpl_id, void * thing, haddr_t addr, size_t len,
    size_t compressed_len, haddr_t * new_addr_ptr, size_t * new_len_ptr,
    size_t * new_compressed_len_ptr, unsigned * flags_ptr);
static herr_t H5C__epoch_marker_serialize(const H5F_t *f,
    void * image_ptr, size_t len, void * thing);
static herr_t H5C__epoch_marker_notify(H5C_notify_action_t action, void *thing);
static herr_t H5C__epoch_marker_free_icr(void * thing);
static herr_t H5C__epoch_marker_clear(const H5F_t *f, void * thing, 
    hbool_t about_to_destroy);
static herr_t H5C__epoch_marker_fsf_size(const void H5_ATTR_UNUSED * thing, 
    size_t H5_ATTR_UNUSED * fsf_size_ptr);


/*********************/
/* Package Variables */
/*********************/


/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/


const H5C_class_t H5C__epoch_marker_class =
{
    /* id               = */ H5C__EPOCH_MARKER_TYPE,
    /* name             = */ "epoch marker",
    /* mem_type         = */ H5FD_MEM_DEFAULT, /* value doesn't matter */
    /* flags		= */ H5C__CLASS_NO_FLAGS_SET,
    /* get_load_size    = */ H5C__epoch_marker_get_load_size,
    /* deserialize      = */ H5C__epoch_marker_deserialize,
    /* image_len        = */ H5C__epoch_marker_image_len,
    /* pre_serialize    = */ H5C__epoch_marker_pre_serialize,
    /* serialize        = */ H5C__epoch_marker_serialize,
    /* notify           = */ H5C__epoch_marker_notify,
    /* free_icr         = */ H5C__epoch_marker_free_icr,
    /* clear            = */ H5C__epoch_marker_clear,
    /* fsf_size         = */ H5C__epoch_marker_fsf_size,
};


/***************************************************************************
 * Class functions for H5C__EPOCH_MAKER_TYPE:
 *
 * None of these functions should ever be called, so there is no point in
 * documenting them separately.
 *                                                     JRM - 11/16/04
 *
 ***************************************************************************/


static herr_t
H5C__epoch_marker_get_load_size(const void H5_ATTR_UNUSED *udata_ptr,
    size_t H5_ATTR_UNUSED *image_len_ptr)
{
    FUNC_ENTER_STATIC_NOERR /* Yes, even though this pushes an error on the stack */

    HERROR(H5E_CACHE, H5E_SYSTEM, "called unreachable fcn.");

    FUNC_LEAVE_NOAPI(FAIL)
} /* end H5C__epoch_marker_get_load_size() */


static void *
H5C__epoch_marker_deserialize(const void H5_ATTR_UNUSED * image_ptr, size_t H5_ATTR_UNUSED len,
    void H5_ATTR_UNUSED * udata, hbool_t H5_ATTR_UNUSED * dirty_ptr)
{
    FUNC_ENTER_STATIC_NOERR /* Yes, even though this pushes an error on the stack */

    HERROR(H5E_CACHE, H5E_SYSTEM, "called unreachable fcn.");

    FUNC_LEAVE_NOAPI(NULL)
} /* end H5C__epoch_marker_deserialize() */


static herr_t
H5C__epoch_marker_image_len(const void H5_ATTR_UNUSED *thing,
    size_t H5_ATTR_UNUSED *image_len_ptr, hbool_t H5_ATTR_UNUSED *compressed_ptr,
    size_t H5_ATTR_UNUSED *compressed_len_ptr)
{
    FUNC_ENTER_STATIC_NOERR /* Yes, even though this pushes an error on the stack */

    HERROR(H5E_CACHE, H5E_SYSTEM, "called unreachable fcn.");

    FUNC_LEAVE_NOAPI(FAIL)
} /* end H5C__epoch_marker_image_len() */


static herr_t
H5C__epoch_marker_pre_serialize(const H5F_t H5_ATTR_UNUSED *f, hid_t H5_ATTR_UNUSED dxpl_id,
    void H5_ATTR_UNUSED *thing, haddr_t H5_ATTR_UNUSED addr, size_t H5_ATTR_UNUSED len,
    size_t H5_ATTR_UNUSED compressed_len, haddr_t H5_ATTR_UNUSED *new_addr_ptr, 
    size_t H5_ATTR_UNUSED *new_len_ptr, size_t H5_ATTR_UNUSED *new_compressed_len_ptr,
    unsigned H5_ATTR_UNUSED *flags_ptr)
{
    FUNC_ENTER_STATIC_NOERR /* Yes, even though this pushes an error on the stack */

    HERROR(H5E_CACHE, H5E_SYSTEM, "called unreachable fcn.");

    FUNC_LEAVE_NOAPI(FAIL)
} /* end H5C__epoch_marker_pre_serialize() */


static herr_t
H5C__epoch_marker_serialize(const H5F_t H5_ATTR_UNUSED *f, void H5_ATTR_UNUSED *image_ptr,
    size_t H5_ATTR_UNUSED len, void H5_ATTR_UNUSED *thing)
{
    FUNC_ENTER_STATIC_NOERR /* Yes, even though this pushes an error on the stack */

    HERROR(H5E_CACHE, H5E_SYSTEM, "called unreachable fcn.");

    FUNC_LEAVE_NOAPI(FAIL)
} /* end H5C__epoch_marker_serialize() */


static herr_t
H5C__epoch_marker_notify(H5C_notify_action_t H5_ATTR_UNUSED action,
                       void H5_ATTR_UNUSED * thing)
{
    FUNC_ENTER_STATIC_NOERR /* Yes, even though this pushes an error on the stack */

    HERROR(H5E_CACHE, H5E_SYSTEM, "called unreachable fcn.");

    FUNC_LEAVE_NOAPI(FAIL)
} /* end H5C__epoch_marker_notify() */


static herr_t
H5C__epoch_marker_free_icr(void H5_ATTR_UNUSED * thing)
{
    FUNC_ENTER_STATIC_NOERR /* Yes, even though this pushes an error on the stack */

    HERROR(H5E_CACHE, H5E_SYSTEM, "called unreachable fcn.");

    FUNC_LEAVE_NOAPI(FAIL)
} /* end H5C__epoch_marker_free_icr() */


static herr_t 
H5C__epoch_marker_clear(const H5F_t H5_ATTR_UNUSED *f, void H5_ATTR_UNUSED * thing, hbool_t H5_ATTR_UNUSED about_to_destroy)
{
    FUNC_ENTER_STATIC_NOERR /* Yes, even though this pushes an error on the stack */

    HERROR(H5E_CACHE, H5E_SYSTEM, "called unreachable fcn.");

    FUNC_LEAVE_NOAPI(FAIL)
} /* end H5C__epoch_marker_clear() */


static herr_t 
H5C__epoch_marker_fsf_size(const void H5_ATTR_UNUSED * thing, size_t H5_ATTR_UNUSED *fsf_size_ptr)
{
    FUNC_ENTER_STATIC_NOERR /* Yes, even though this pushes an error on the stack */

    HERROR(H5E_CACHE, H5E_SYSTEM, "called unreachable fcn.");

    FUNC_LEAVE_NOAPI(FAIL)
} /* end H5C__epoch_marker_fsf_size() */

