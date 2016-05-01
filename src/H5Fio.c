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
 * Created:             H5Fio.c
 *                      Jan 10 2008
 *                      Quincey Koziol <koziol@hdfgroup.org>
 *
 * Purpose:             File I/O routines.
 *
 *-------------------------------------------------------------------------
 */

/****************/
/* Module Setup */
/****************/

#include "H5Fmodule.h"          /* This source code file is part of the H5F module */


/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions			*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5Fpkg.h"             /* File access				*/
#include "H5FDprivate.h"	/* File drivers				*/
#include "H5Iprivate.h"		/* IDs			  		*/


/****************/
/* Local Macros */
/****************/


/******************/
/* Local Typedefs */
/******************/


/********************/
/* Package Typedefs */
/********************/


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
 * Function:	H5F_block_read
 *
 * Purpose:	Reads some data from a file/server/etc into a buffer.
 *		The data is contiguous.	 The address is relative to the base
 *		address for the file.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Jul 10 1997
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5F_block_read(const H5F_t *f, H5FD_mem_t type, haddr_t addr, size_t size,
    hid_t dxpl_id, void *buf/*out*/)
{
    H5F_io_info_t fio_info;             /* I/O info for operation */
    H5FD_mem_t  map_type;               /* Mapped memory type */
    hid_t       my_dxpl_id = dxpl_id;   /* transfer property to use for I/O */
    herr_t      ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_NOAPI(FAIL)
#ifdef QAK
HDfprintf(stderr, "%s: read from addr = %a, size = %Zu\n", FUNC, addr, size);
#endif /* QAK */

    HDassert(f);
    HDassert(f->shared);
    HDassert(buf);
    HDassert(H5F_addr_defined(addr));

    /* Check for attempting I/O on 'temporary' file address */
    if(H5F_addr_le(f->shared->tmp_addr, (addr + size)))
        HGOTO_ERROR(H5E_IO, H5E_BADRANGE, FAIL, "attempting I/O in temporary file space")

    /* Treat global heap as raw data */
    map_type = (type == H5FD_MEM_GHEAP) ? H5FD_MEM_DRAW : type;

#ifdef H5_DEBUG_BUILD
    /* GHEAP type is treated as RAW, so update the dxpl type property too */
    if(H5FD_MEM_GHEAP == type)
        my_dxpl_id = H5AC_rawdata_dxpl_id;
#endif /* H5_DEBUG_BUILD */

    /* Set up I/O info for operation */
    fio_info.f = f;
    if(NULL == (fio_info.dxpl = (H5P_genplist_t *)H5I_object(my_dxpl_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "can't get property list")

    /* Pass through metadata accumulator layer */
    if(H5F__accum_read(&fio_info, map_type, addr, size, buf) < 0)
        HGOTO_ERROR(H5E_IO, H5E_READERROR, FAIL, "read through metadata accumulator failed")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5F_block_read() */


/*-------------------------------------------------------------------------
 * Function:	H5F_block_write
 *
 * Purpose:	Writes some data from memory to a file/server/etc.  The
 *		data is contiguous.  The address is relative to the base
 *		address.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Jul 10 1997
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5F_block_write(const H5F_t *f, H5FD_mem_t type, haddr_t addr, size_t size,
    hid_t dxpl_id, const void *buf)
{
    H5F_io_info_t fio_info;             /* I/O info for operation */
    H5FD_mem_t  map_type;               /* Mapped memory type */
    hid_t       my_dxpl_id = dxpl_id;   /* transfer property to use for I/O */
    herr_t      ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_NOAPI(FAIL)
#ifdef QAK
HDfprintf(stderr, "%s: write to addr = %a, size = %Zu\n", FUNC, addr, size);
#endif /* QAK */

    HDassert(f);
    HDassert(f->shared);
    HDassert(H5F_INTENT(f) & H5F_ACC_RDWR);
    HDassert(buf);
    HDassert(H5F_addr_defined(addr));

    /* Check for attempting I/O on 'temporary' file address */
    if(H5F_addr_le(f->shared->tmp_addr, (addr + size)))
        HGOTO_ERROR(H5E_IO, H5E_BADRANGE, FAIL, "attempting I/O in temporary file space")

    /* Treat global heap as raw data */
    map_type = (type == H5FD_MEM_GHEAP) ? H5FD_MEM_DRAW : type;

#ifdef H5_DEBUG_BUILD
    /* GHEAP type is treated as RAW, so update the dxpl type property too */
    if(H5FD_MEM_GHEAP == type)
        my_dxpl_id = H5AC_rawdata_dxpl_id;
#endif /* H5_DEBUG_BUILD */

    /* Set up I/O info for operation */
    fio_info.f = f;
    if(NULL == (fio_info.dxpl = (H5P_genplist_t *)H5I_object(my_dxpl_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "can't get property list")

    /* Pass through metadata accumulator layer */
    if(H5F__accum_write(&fio_info, map_type, addr, size, buf) < 0)
        HGOTO_ERROR(H5E_IO, H5E_WRITEERROR, FAIL, "write through metadata accumulator failed")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5F_block_write() */


/*-------------------------------------------------------------------------
 * Function:    H5F_flush_tagged_metadata
 *
 * Purpose:     Flushes metadata with specified tag in the metadata cache 
 *              to disk.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:	Mike McGreevy
 *              September 9, 2010
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5F_flush_tagged_metadata(H5F_t * f, haddr_t tag, hid_t dxpl_id)
{
    H5F_io_info_t fio_info;             /* I/O info for operation */
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_NOAPI(FAIL)

    /* Use tag to search for and flush associated metadata */
    if(H5AC_flush_tagged_metadata(f, tag, dxpl_id)<0)
        HGOTO_ERROR(H5E_CACHE, H5E_CANTFLUSH, FAIL, "unable to flush tagged metadata")

    /* Set up I/O info for operation */
    fio_info.f = f;

    if(NULL == (fio_info.dxpl = (H5P_genplist_t *)H5I_object(dxpl_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "can't get property list")
    

    /* Flush and reset the accumulator */
    if(H5F__accum_reset(&fio_info, TRUE) < 0)
        HGOTO_ERROR(H5E_IO, H5E_CANTRESET, FAIL, "can't reset accumulator")

    /* Flush file buffers to disk. */
    if(H5FD_flush(f->shared->lf, dxpl_id, FALSE) < 0)
        HGOTO_ERROR(H5E_IO, H5E_WRITEERROR, FAIL, "low level flush failed")

done:
    FUNC_LEAVE_NOAPI(ret_value);
} /* end H5F_flush_tagged_metadata */

