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

/*
 * Programmer:  Robb Matzke <matzke@llnl.gov>
 *              Tuesday, March  3, 1998
 *
 * Purpose:	Tests datasets stored in external raw files.
 */
#include "h5test.h"
#include "H5srcdir.h"

/* File for external link test.  Created with gen_udlinks.c */
#define LINKED_FILE  "be_extlink2.h5"

const char *FILENAME[] = {
    "extern_1",
    "extern_2",
    "extern_3",
    "extern_4",     /* Remove when links test is moved */
    NULL
};


/*-------------------------------------------------------------------------
 * Function:    files_have_same_contents
 *
 * Purpose:     Determines whether two files contain the same data.
 *
 * Return:      Success:    nonzero if same, zero if different.
 *		        Failure:    zero
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March  4, 1998
 *
 *-------------------------------------------------------------------------
 */
static hbool_t
files_have_same_contents(const char *name1, const char *name2)
{
    int		fd1 = 0, fd2 = 0;
    ssize_t	n1, n2;
    char	buf1[1024], buf2[1024];
    hbool_t ret = false;            /* not equal until proven otherwise */

    if((fd1 = HDopen(name1, O_RDONLY, 0666)) < 0)
        goto out;
    if((fd2 = HDopen(name2, O_RDONLY, 0666)) < 0)
        goto out;

    /* Loop until files are empty or we encounter a problem */
    while(1) {
        HDmemset(buf1, 0, sizeof(buf1));
        HDmemset(buf2, 0, sizeof(buf2));

        n1 = HDread(fd1, buf1, sizeof(buf1));
        if(n1 < 0 || (size_t)n1 > sizeof(buf1))
            break;
        n2 = HDread(fd2, buf2, sizeof(buf2));
        if(n2 < 0 || (size_t)n2 > sizeof(buf2))
            break;

        if(n1 != n2)
            break;

        if(n1 == 0 && n2 == 0) {
            ret = true;
            break;
        }

        if(HDmemcmp(buf1, buf2, (size_t)n1))
            break;

    } /* end while */

out:
    if(fd1)
        HDclose(fd1);
    if(fd2)
        HDclose(fd2);
    return ret;
} /* end files_have_same_contents() */


/*-------------------------------------------------------------------------
 * Function:    test_non_extendible
 *
 * Purpose:     Tests a non-extendible dataset with a single external file.
 *
 * Return:      Success:    0
 *              Failure:    1
 *
 * Programmer:  Robb Matzke
 *              Monday, November 23, 1998
 *
 *-------------------------------------------------------------------------
 */
static int
test_non_extendible(hid_t file)
{
    hid_t	dcpl = -1;          /* dataset creation properties          */
    hid_t	space = -1;         /* data space                           */
    hid_t	dset = -1;          /* dataset                              */
    hsize_t	cur_size[1];        /* data space current size              */
    hsize_t	max_size[1];        /* data space maximum size              */
    int		n;                  /* number of external files             */
    char	name[256];          /* external file name                   */
    HDoff_t	file_offset;        /* external file offset                 */
    hsize_t	file_size;          /* sizeof external file segment         */
    haddr_t dset_addr;          /* address of dataset                   */

    TESTING("fixed-size data space, exact storage");

    /* Create the dataset */
    if((dcpl = H5Pcreate(H5P_DATASET_CREATE)) < 0)
        FAIL_STACK_ERROR
    cur_size[0] = max_size[0] = 100;
    if(H5Pset_external(dcpl, "ext1.data", (HDoff_t)0, (hsize_t)(max_size[0] * sizeof(int))) < 0)
        FAIL_STACK_ERROR
    if((space = H5Screate_simple(1, cur_size, max_size)) < 0)
        FAIL_STACK_ERROR
    if((dset = H5Dcreate2(file, "dset1", H5T_NATIVE_INT, space, H5P_DEFAULT, dcpl, H5P_DEFAULT)) < 0)
        FAIL_STACK_ERROR
    if(H5Dclose(dset) < 0)
        FAIL_STACK_ERROR
    if(H5Sclose(space) < 0)
        FAIL_STACK_ERROR
    if(H5Pclose(dcpl) < 0)
        FAIL_STACK_ERROR

    /* Read dataset creation information */
    if((dset = H5Dopen2(file, "dset1", H5P_DEFAULT)) < 0)
        FAIL_STACK_ERROR

    /* Test dataset address.  Should be undefined. */
    H5E_BEGIN_TRY {
        dset_addr = H5Dget_offset(dset);
    } H5E_END_TRY;
    if(dset_addr != HADDR_UNDEF)
        FAIL_STACK_ERROR

    /* Check external count */
    if((dcpl = H5Dget_create_plist(dset)) < 0)
        FAIL_STACK_ERROR
    if((n = H5Pget_external_count(dcpl)) < 0)
        FAIL_STACK_ERROR
    if(1 != n) {
        H5_FAILED();
        HDputs("    Returned external count is wrong.");
        printf("   got: %d\n    ans: 1\n", n);
        goto error;
    } /* end if */

    HDstrcpy(name + sizeof(name) - 4, "...");
    if(H5Pget_external(dcpl, 0, sizeof(name) - 4, name, &file_offset, &file_size) < 0)
        FAIL_STACK_ERROR

    /* Check file offset */
    if(file_offset != 0) {
        H5_FAILED();
        HDputs("    Wrong file offset.");
        printf("    got: %lu\n    ans: 0\n", (unsigned long)file_offset);
        goto error;
    } /* end if */

    /* Check file size */
    if(file_size != (max_size[0] * sizeof(int))) {
        H5_FAILED();
        HDputs("    Wrong file size.");
        printf("    got: %lu\n    ans: %lu\n", (unsigned long)file_size, (unsigned long)max_size[0]*sizeof(int));
        goto error;
    } /* end if */

    /* Done (dataspace was previously closed) */
    if(H5Pclose(dcpl) < 0) FAIL_STACK_ERROR
    if(H5Dclose(dset) < 0) FAIL_STACK_ERROR
    PASSED();
    return 0;

 error:
    H5E_BEGIN_TRY {
        H5Pclose(dcpl);
        H5Sclose(space);
        H5Dclose(dset);
    } H5E_END_TRY;
    return 1;
} /* end test_non_extendible() */


/*-------------------------------------------------------------------------
 * Function:    test_too_small
 *
 * Purpose:     Test a single external file which is too small to represent
 *              all the data.
 *
 * Return:      Success:    0
 *              Failure:    1
 *
 * Programmer:	Robb Matzke
 *              Monday, November 23, 1998
 *
 *-------------------------------------------------------------------------
 */
static int
test_too_small(hid_t file)
{
    hid_t	dcpl = -1;          /* dataset creation properties          */
    hid_t	space = -1;         /* data space                           */
    hid_t	dset = -1;          /* dataset                              */
    hsize_t	cur_size[1];        /* current data space size              */
    hsize_t	max_size[1];        /* maximum data space size              */

    TESTING("external storage is too small");

    if((dcpl = H5Pcreate(H5P_DATASET_CREATE)) < 0)
        FAIL_STACK_ERROR
    cur_size[0] = max_size[0] = 100;
    if(H5Pset_external(dcpl, "ext1.data", (HDoff_t)0, (hsize_t)(max_size[0] * sizeof(int) - 1)) < 0)
        FAIL_STACK_ERROR
    if((space = H5Screate_simple(1, cur_size, max_size)) < 0)
        FAIL_STACK_ERROR

    H5E_BEGIN_TRY {
        dset = H5Dcreate2(file, "dset2", H5T_NATIVE_INT, space, H5P_DEFAULT, dcpl, H5P_DEFAULT);
    } H5E_END_TRY;
    if(dset >= 0)
        FAIL_PUTS_ERROR("    Small external file succeeded instead of failing.");
    if(H5Sclose(space) < 0)
        FAIL_STACK_ERROR
    if(H5Pclose(dcpl) < 0)
        FAIL_STACK_ERROR

    PASSED();
    return 0;

 error:
    H5E_BEGIN_TRY {
        H5Sclose(space);
        H5Pclose(dcpl);
        H5Dclose(dset);
    } H5E_END_TRY;
    return 1;
} /* end test_too_small() */


/*-------------------------------------------------------------------------
 * Function:    test_large_enough_current_eventual
 *
 * Purpose:     Test a single external file which is large enough to
 *              represent the current data and large enough to represent the
 *              eventual size of the data.
 *
 * Return:  Success:    0
 *          Failure:    1
 *
 * Programmer:	Robb Matzke
 *              Monday, November 23, 1998
 *
 *-------------------------------------------------------------------------
 */
static int
test_large_enough_current_eventual(hid_t file)
{
    hid_t	dcpl = -1;          /* dataset creation properties          */
    hid_t	space = -1;         /* data space                           */
    hid_t	dset = -1;          /* dataset                              */
    hsize_t	cur_size[1];        /* current data space size              */
    hsize_t	max_size[1];        /* maximum data space size              */

    TESTING("extendible dataspace, exact external size");

    if((dcpl = H5Pcreate (H5P_DATASET_CREATE)) < 0)
        FAIL_STACK_ERROR
    cur_size[0] = 100;
    max_size[0] = 200;
    if(H5Pset_external(dcpl, "ext1.data", (HDoff_t)0, (hsize_t)(max_size[0] * sizeof(int))) < 0)
        FAIL_STACK_ERROR
    if((space = H5Screate_simple(1, cur_size, max_size)) < 0)
        FAIL_STACK_ERROR
    if((dset = H5Dcreate2(file, "dset3", H5T_NATIVE_INT, space, H5P_DEFAULT, dcpl, H5P_DEFAULT)) < 0)
        FAIL_STACK_ERROR

    if(H5Dclose(dset) < 0) FAIL_STACK_ERROR
    if(H5Sclose(space) < 0) FAIL_STACK_ERROR
    if(H5Pclose(dcpl) < 0) FAIL_STACK_ERROR

    PASSED();
    return 0;

error:
    H5E_BEGIN_TRY {
        H5Dclose(dset);
        H5Pclose(dcpl);
        H5Sclose(space);
    } H5E_END_TRY;
    return 1;
} /* end test_large_enough_current_eventual() */


/*-------------------------------------------------------------------------
 * Function:    test_large_enough_current_not_eventual
 *
 * Purpose:     Test a single external file which is large enough for the
 *              current data size but not large enough for the eventual size.
 *
 * Return:      Success:    0
 *              Failure:    1
 *
 * Programmer:  Robb Matzke
 *              Monday, November 23, 1998
 *
 *-------------------------------------------------------------------------
 */
static int
test_large_enough_current_not_eventual(hid_t file)
{
    hid_t	dcpl = -1;          /* dataset creation properties          */
    hid_t	space = -1;         /* data space                           */
    hid_t	dset = -1;          /* dataset                              */
    hsize_t	cur_size[1];        /* current data space size              */
    hsize_t	max_size[1];        /* maximum data space size              */

    TESTING("extendible dataspace, external storage is too small");

    if((dcpl = H5Pcreate(H5P_DATASET_CREATE)) < 0)
        FAIL_STACK_ERROR
    cur_size[0] = 100;
    max_size[0] = 200;
    if(H5Pset_external(dcpl, "ext1.data", (HDoff_t)0, (hsize_t)(max_size[0] * sizeof(int) - 1)) < 0)
        FAIL_STACK_ERROR
    if((space = H5Screate_simple(1, cur_size, max_size)) < 0)
        FAIL_STACK_ERROR

    H5E_BEGIN_TRY {
        dset = H5Dcreate2(file, "dset4", H5T_NATIVE_INT, space, H5P_DEFAULT, dcpl, H5P_DEFAULT);
    } H5E_END_TRY;
    if(dset >= 0)
        FAIL_PUTS_ERROR("    Small external file succeeded instead of failing.");

    if(H5Sclose(space) < 0) FAIL_STACK_ERROR
    if(H5Pclose(dcpl) < 0) FAIL_STACK_ERROR

    PASSED();
    return 0;

 error:
    H5E_BEGIN_TRY {
        H5Dclose(dset);
        H5Pclose(dcpl);
        H5Sclose(space);
    } H5E_END_TRY;
    return 1;
} /* end test_large_enough_current_not_eventual() */


/*-------------------------------------------------------------------------
 * Function:    test_1e
 *
 * Purpose:     Test a single external file of unlimited size and an
 *              unlimited data space.
 *
 * Return:      Success:    0
 *              Failure:    1
 *
 * Programmer:  Robb Matzke
 *              Monday, November 23, 1998
 *
 *-------------------------------------------------------------------------
 */
static int
test_unlimited(hid_t file)
{
    hid_t	dcpl = -1;          /* dataset creation properties          */
    hid_t	space = -1;         /* data space                           */
    hid_t	dset = -1;          /* dataset                              */
    hsize_t	cur_size[1];        /* data space current size              */
    hsize_t	max_size[1];        /* data space maximum size              */
    int		n;                  /* number of external files             */
    char	name[256];          /* external file name                   */
    HDoff_t	file_offset;        /* external file offset                 */
    hsize_t	file_size;          /* sizeof external file segment         */

    TESTING("unlimited dataspace, unlimited external storage");

    /* Create dataset */
    if((dcpl = H5Pcreate(H5P_DATASET_CREATE)) < 0)
        FAIL_STACK_ERROR
    if(H5Pset_external(dcpl, "ext1.data", (HDoff_t)0, H5F_UNLIMITED) < 0)
        FAIL_STACK_ERROR
    cur_size[0] = 100;
    max_size[0] = H5S_UNLIMITED;
    if((space = H5Screate_simple(1, cur_size, max_size)) < 0)
        FAIL_STACK_ERROR
    if((dset = H5Dcreate2(file, "dset5", H5T_NATIVE_INT, space, H5P_DEFAULT, dcpl, H5P_DEFAULT)) < 0)
        FAIL_STACK_ERROR

    if(H5Dclose(dset) < 0) FAIL_STACK_ERROR
    if(H5Sclose(space) < 0) FAIL_STACK_ERROR
    if(H5Pclose(dcpl) < 0) FAIL_STACK_ERROR

    /* Read dataset creation information */
    if((dset = H5Dopen2(file, "dset5", H5P_DEFAULT)) < 0)
        FAIL_STACK_ERROR
    if((dcpl = H5Dget_create_plist(dset)) < 0)
        FAIL_STACK_ERROR
    if((n = H5Pget_external_count(dcpl)) < 0)
        FAIL_STACK_ERROR
    if(1 != n) {
        H5_FAILED();
        HDputs("    Returned external count is wrong.");
        printf("    got: %d\n    ans: 1\n", n);
        goto error;
    } /* end if */

    HDstrcpy(name + sizeof(name) - 4, "...");
    if(H5Pget_external(dcpl, 0, sizeof(name) - 4, name, &file_offset, &file_size) < 0)
        FAIL_STACK_ERROR
    if(file_offset != 0) {
        H5_FAILED();
        HDputs("    Wrong file offset.");
        printf("    got: %lu\n    ans: 0\n", (unsigned long)file_offset);
        goto error;
    } /* end if */

    if(H5F_UNLIMITED != file_size) {
        H5_FAILED();
        HDputs("    Wrong file size.");
        printf("    got: %lu\n    ans: INF\n", (unsigned long)file_size);
        goto error;
    } /* end if */

    if(H5Pclose(dcpl) < 0) FAIL_STACK_ERROR
    if(H5Dclose(dset) < 0) FAIL_STACK_ERROR

    PASSED();
    return 0;

 error:
    H5E_BEGIN_TRY {
        H5Dclose(dset);
        H5Pclose(dcpl);
        H5Sclose(space);
    } H5E_END_TRY;
    return 1;
} /* end test_unlimited() */


/*-------------------------------------------------------------------------
 * Function:    test_multiple_files
 *
 * Purpose:     Test multiple external files for a dataset.
 *
 * Return:      Success:    0
 *              Failure:    1
 *
 * Programmer:  Robb Matzke
 *              Monday, November 23, 1998
 *
 *-------------------------------------------------------------------------
 */
static int
test_multiple_files(hid_t file)
{
    hid_t	dcpl = -1;          /* dataset creation properties          */
    hid_t	space = -1;         /* dataspace                            */
    hid_t	dset = -1;          /* dataset                              */
    hsize_t	cur_size[1];        /* data space current size              */
    hsize_t	max_size[1];        /* data space maximum size              */

    TESTING("multiple external files");

    if((dcpl = H5Pcreate(H5P_DATASET_CREATE)) < 0)
        FAIL_STACK_ERROR

    cur_size[0] = max_size[0] = 100;

    if(H5Pset_external(dcpl, "ext1.data", (HDoff_t)0, (hsize_t)(max_size[0]*sizeof(int)/4)) < 0)
        FAIL_STACK_ERROR
    if(H5Pset_external(dcpl, "ext2.data", (HDoff_t)0, (hsize_t)(max_size[0]*sizeof(int)/4)) < 0)
        FAIL_STACK_ERROR
    if(H5Pset_external(dcpl, "ext3.data", (HDoff_t)0, (hsize_t)(max_size[0]*sizeof(int)/4)) < 0)
        FAIL_STACK_ERROR
    if(H5Pset_external(dcpl, "ext4.data", (HDoff_t)0, (hsize_t)(max_size[0]*sizeof(int)/4)) < 0)
        FAIL_STACK_ERROR
    if((space = H5Screate_simple(1, cur_size, max_size)) < 0)
        FAIL_STACK_ERROR
    if((dset = H5Dcreate2(file, "dset6", H5T_NATIVE_INT, space, H5P_DEFAULT, dcpl, H5P_DEFAULT)) < 0)
        FAIL_STACK_ERROR

    if(H5Dclose(dset) < 0) FAIL_STACK_ERROR
    if(H5Sclose(space) < 0) FAIL_STACK_ERROR
    if(H5Pclose(dcpl) < 0) FAIL_STACK_ERROR

    PASSED();
    return 0;

error:
    H5E_BEGIN_TRY {
        H5Dclose(dset);
        H5Pclose(dcpl);
        H5Sclose(space);
    } H5E_END_TRY;
    return 1;
} /* end test_multiple_files() */


/*-------------------------------------------------------------------------
 * Function:    test_add_to_unlimited
 *
 * Purpose:     It should be impossible to define an unlimited external file
 *              and then follow it with another external file.
 *
 * Return:      Success:    0
 *              Failure:    1
 *
 * Programmer:  Robb Matzke
 *              Monday, November 23, 1998
 *
 *-------------------------------------------------------------------------
 */
static int
test_add_to_unlimited(void)
{
    hid_t	dcpl = -1;          /* dataset creation properties          */
    herr_t	status;             /* function return status               */
    int		n;                  /* number of external files             */

    TESTING("external file following unlimited file");

    if((dcpl = H5Pcreate (H5P_DATASET_CREATE)) < 0)
        FAIL_STACK_ERROR
    if(H5Pset_external(dcpl, "ext1.data", (HDoff_t)0, H5F_UNLIMITED) < 0)
        FAIL_STACK_ERROR

    H5E_BEGIN_TRY {
        status = H5Pset_external(dcpl, "ext2.data", (HDoff_t)0, (hsize_t)100);
    } H5E_END_TRY;
    if(status >= 0)
        FAIL_PUTS_ERROR("    H5Pset_external() succeeded when it should have failed.");

    if((n = H5Pget_external_count(dcpl)) < 0)
        FAIL_STACK_ERROR
    if(1 != n)
        FAIL_PUTS_ERROR("    Wrong external file count returned.");

    if(H5Pclose(dcpl) < 0) FAIL_STACK_ERROR
    PASSED();
    return 0;

 error:
    H5E_BEGIN_TRY {
        H5Pclose(dcpl);
    } H5E_END_TRY;
    return 1;
} /* end test_add_to_unlimited() */


/*-------------------------------------------------------------------------
 * Function:    test_overflow
 *
 * Purpose:     It should be impossible to create a set of external files
 *              whose total size overflows a size_t integer.
 *
 * Return:      Success:    0
 *              Failure:    1
 *
 * Programmer:  Robb Matzke
 *              Monday, November 23, 1998
 *
 *-------------------------------------------------------------------------
 */
static int
test_overflow(void)
{
    hid_t	dcpl = -1;          /* dataset creation properties          */
    herr_t	status;             /* return status                        */

    TESTING("address overflow in external files");

    if((dcpl=H5Pcreate(H5P_DATASET_CREATE)) < 0)
        FAIL_STACK_ERROR
    if(H5Pset_external(dcpl, "ext1.data", (HDoff_t)0, H5F_UNLIMITED-1) < 0)
        FAIL_STACK_ERROR

    H5E_BEGIN_TRY {
        status = H5Pset_external(dcpl, "ext2.data", (HDoff_t)0, (hsize_t)100);
    } H5E_END_TRY;
    if(status >= 0)
        FAIL_PUTS_ERROR("    H5Pset_external() succeeded when it should have failed.");

    if(H5Pclose(dcpl) < 0) FAIL_STACK_ERROR

    PASSED();
    return 0;

 error:
    H5E_BEGIN_TRY {
        H5Pclose(dcpl);
    } H5E_END_TRY;
    return 1;
} /* end test_overflow() */


/*-------------------------------------------------------------------------
 * Function:    test_read_file_set
 *
 * Purpose:     Tests reading from an external file set.
 *
 * Return:      Success:    0
 *              Failure:    1
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March  4, 1998
 *
 *-------------------------------------------------------------------------
 */
static int
test_read_file_set(hid_t fapl)
{
    hid_t	file = -1;          /* file to write to                     */
    hid_t	dcpl = -1;          /* dataset creation properties          */
    hid_t	space = -1;         /* data space                           */
    hid_t	dset = -1;          /* dataset                              */
    hid_t	grp = -1;           /* group to emit diagnostics            */
    int		fd = -1;            /* external file descriptors            */
    size_t	i, j;               /* miscellaneous counters               */
    hssize_t	n;              /* bytes of I/O                         */
    char	filename[1024];     /* file names                           */
    int		part[25], whole[100];   /* raw data buffers                 */
    hsize_t	cur_size;           /* current data space size              */
    hid_t	hs_space;           /* hyperslab data space                 */
    hsize_t	hs_start = 30;      /* hyperslab starting offset            */
    hsize_t	hs_count = 25;      /* hyperslab size                       */
	int temparray[10] = {0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f};

    TESTING("read external dataset");

    /* Write the data to external files directly */
    for(i=0; i<4; i++) {
        for(j=0; j<25; j++) {
            part[j] = (int)(i*25+j);
        } /* end for */
        HDsprintf(filename, "extern_%lua.raw", (unsigned long)i+1);
        if((fd = HDopen(filename, O_RDWR|O_CREAT|O_TRUNC, 0666)) < 0)
            TEST_ERROR
        n = HDwrite(fd, temparray, (size_t)i*10);
        if(n < 0 || (size_t)n != i*10)
            TEST_ERROR
        n = HDwrite(fd, part, sizeof(part));
        if(n != sizeof(part))
        TEST_ERROR
        HDclose(fd);
    } /* end for */

    /*
     * Create the file and an initial group.  This causes messages about
     * debugging to be emitted before we start playing games with what the
     * output looks like.
     */
    h5_fixname(FILENAME[1], fapl, filename, sizeof(filename));
    if((file = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl)) < 0)
        FAIL_STACK_ERROR
    if((grp = H5Gcreate2(file, "emit-diagnostics", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0)
        FAIL_STACK_ERROR
    if(H5Gclose(grp) < 0) FAIL_STACK_ERROR

    /* Create the dataset */
    if((dcpl = H5Pcreate(H5P_DATASET_CREATE)) < 0)
        FAIL_STACK_ERROR
    if(H5Pset_external(dcpl, "extern_1a.raw",  (HDoff_t)0, (hsize_t)sizeof part) < 0 ||
            H5Pset_external(dcpl, "extern_2a.raw", (HDoff_t)10, (hsize_t)sizeof part) < 0 ||
            H5Pset_external(dcpl, "extern_3a.raw", (HDoff_t)20, (hsize_t)sizeof part) < 0 ||
            H5Pset_external(dcpl, "extern_4a.raw", (HDoff_t)30, (hsize_t)sizeof part) < 0)
        FAIL_STACK_ERROR
    cur_size = 100;
    if((space = H5Screate_simple(1, &cur_size, NULL)) < 0)
        FAIL_STACK_ERROR
    if((dset = H5Dcreate2(file, "dset1", H5T_NATIVE_INT, space, H5P_DEFAULT, dcpl, H5P_DEFAULT)) < 0)
        FAIL_STACK_ERROR

    /*
     * Read the entire dataset and compare with the original
     */
    HDmemset(whole, 0, sizeof(whole));
    if(H5Dread(dset, H5T_NATIVE_INT, space, space, H5P_DEFAULT, whole) < 0)
        FAIL_STACK_ERROR
    for(i = 0; i < 100; i++)
        if(whole[i] != (signed)i)
            FAIL_PUTS_ERROR("    Incorrect value(s) read.");

        /* Read the middle of the dataset */
        if((hs_space = H5Scopy(space)) < 0)
            FAIL_STACK_ERROR
        if(H5Sselect_hyperslab(hs_space, H5S_SELECT_SET, &hs_start, NULL, &hs_count, NULL) < 0)
            FAIL_STACK_ERROR

        HDmemset(whole, 0, sizeof(whole));
        if(H5Dread(dset, H5T_NATIVE_INT, hs_space, hs_space, H5P_DEFAULT, whole) < 0)
            FAIL_STACK_ERROR

        if(H5Sclose(hs_space) < 0) FAIL_STACK_ERROR

        for(i = hs_start; i<hs_start+hs_count; i++) {
            if(whole[i] != (signed)i)
	            FAIL_PUTS_ERROR("    Incorrect value(s) read.");
        } /* end for */

    if(H5Dclose(dset) < 0) FAIL_STACK_ERROR
    if(H5Pclose(dcpl) < 0) FAIL_STACK_ERROR
    if(H5Sclose(space) < 0) FAIL_STACK_ERROR
    if(H5Fclose(file) < 0) FAIL_STACK_ERROR
    PASSED();
    return 0;

 error:
    H5E_BEGIN_TRY {
        H5Dclose(dset);
        H5Pclose(dcpl);
        H5Sclose(space);
        H5Fclose(file);
    } H5E_END_TRY;
    return 1;
} /* end test_read_file_set() */


/*-------------------------------------------------------------------------
 * Function:    test_write_file_set
 *
 * Purpose:     Tests writing to an external file set.
 *
 * Return:      Success:    0
 *              Failure:    1
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March  4, 1998
 *
 *-------------------------------------------------------------------------
 */
static int
test_write_file_set(hid_t fapl)
{
    hid_t	file = -1;          /* file to which to write               */
    hid_t	dcpl = -1;          /* dataset creation properties          */
    hid_t	mem_space = -1;     /* memory data space                    */
    hid_t	file_space = -1;    /* file data space                      */
    hid_t	dset = -1;          /* dataset                              */
    unsigned i;                 /* miscellaneous counters               */
    int		fd = -1;            /* external file descriptor             */
    int	    part[25], whole[100];   /* raw data buffers                 */
    hsize_t	cur_size = 100;     /* current data space size              */
    hsize_t	max_size = 200;     /* maximum data space size              */
    hsize_t	hs_start = 100;     /* hyperslab starting offset            */
    hsize_t	hs_count = 100;     /* hyperslab size                       */
    char	filename[1024];     /* file name                            */
	int temparray[10] = {0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f,0x0f0f0f0f};

    TESTING("write external dataset");

    /* Create another file */
    h5_fixname(FILENAME[2], fapl, filename, sizeof filename);
    if((file = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl)) < 0)
        FAIL_STACK_ERROR

    /* Create the external file list */
    if((dcpl=H5Pcreate(H5P_DATASET_CREATE)) < 0)
        FAIL_STACK_ERROR
    if(H5Pset_external(dcpl, "extern_1b.raw", (HDoff_t)0, (hsize_t)sizeof part) < 0 ||
            H5Pset_external(dcpl, "extern_2b.raw", (HDoff_t)10, (hsize_t)sizeof part) < 0 ||
            H5Pset_external(dcpl, "extern_3b.raw", (HDoff_t)20, (hsize_t)sizeof part) < 0 ||
            H5Pset_external(dcpl, "extern_4b.raw", (HDoff_t)30, H5F_UNLIMITED) < 0)
        FAIL_STACK_ERROR

    /* Make sure the output files are fresh*/
    for(i=1; i<=4; i++) {
        HDsprintf(filename, "extern_%db.raw", i);
        if((fd= HDopen(filename, O_RDWR|O_CREAT|O_TRUNC, 0666)) < 0) {
            H5_FAILED();
            printf("    cannot open %s: %s\n", filename, HDstrerror(errno));
            goto error;
        } /* end if */

        if(HDwrite(fd, temparray, (i-1)*10) < 0) {
            H5_FAILED();
            printf("    write error to file %s: %s\n", filename, HDstrerror(errno));
            goto error;
        }
        HDclose(fd);
    } /* end for */

    /* Create the dataset */
    if((mem_space = H5Screate_simple(1, &cur_size, &max_size)) < 0)
        FAIL_STACK_ERROR
    if((file_space = H5Scopy(mem_space)) < 0)
        FAIL_STACK_ERROR
    if((dset = H5Dcreate2(file, "dset1", H5T_NATIVE_INT, file_space, H5P_DEFAULT, dcpl, H5P_DEFAULT)) < 0)
        FAIL_STACK_ERROR

    /* Write the entire dataset and compare with the original */
    for(i = 0; i < cur_size; i++)
        whole[i] = (int)i;
    if(H5Dwrite(dset, H5T_NATIVE_INT, mem_space, file_space, H5P_DEFAULT, whole) < 0)
        FAIL_STACK_ERROR
    for(i = 0; i < 4; i++) {
        char name1[64], name2[64];

        HDsprintf(name1, "extern_%da.raw", i + 1);
        HDsprintf(name2, "extern_%db.raw", i + 1);
        if(!files_have_same_contents(name1, name2))
            FAIL_PUTS_ERROR("   Output differs from expected value.")
    } /* end for */

    /* Extend the dataset by another 100 elements */
    if(H5Dset_extent(dset, &max_size) < 0)
        FAIL_STACK_ERROR
    if(H5Sclose(file_space) < 0)
        FAIL_STACK_ERROR
    if((file_space = H5Dget_space(dset)) < 0)
        FAIL_STACK_ERROR

    /* Write second half of dataset */
    for(i = 0; i < hs_count; i++)
        whole[i] = 100 + (int)i;
    if(H5Sselect_hyperslab(file_space, H5S_SELECT_SET, &hs_start, NULL, &hs_count, NULL) < 0)
        FAIL_STACK_ERROR
    if(H5Dwrite(dset, H5T_NATIVE_INT, mem_space, file_space, H5P_DEFAULT, whole) < 0)
        FAIL_STACK_ERROR


    if(H5Dclose(dset) < 0) FAIL_STACK_ERROR
    if(H5Pclose(dcpl) < 0) FAIL_STACK_ERROR
    if(H5Sclose(mem_space) < 0) FAIL_STACK_ERROR
    if(H5Sclose(file_space) < 0) FAIL_STACK_ERROR
    if(H5Fclose(file) < 0) FAIL_STACK_ERROR

    PASSED();
    return 0;

 error:
    H5E_BEGIN_TRY {
        H5Dclose(dset);
        H5Pclose(dcpl);
        H5Sclose(mem_space);
        H5Sclose(file_space);
        H5Fclose(file);
    } H5E_END_TRY;
    return 1;
} /* end test_write_file_set() */


/*-------------------------------------------------------------------------
 * Function:    test_open_ext_link_twice
 *
 * Purpose:     Tests opening an external link twice.  It exposed a bug
 *              in the library.  This function tests the fix.  This test
 *              doesn't work with MULTI driver.
 *
 *              TODO: Fix to work with multiple VFDs instead of blindly
 *                    switching to sec2.
 *
 *              TODO: Move this test to the links test, where it belongs.
 *
 * Return:      Success:    0
 *              Failure:    1
 *
 * Programmer:	Raymond Lu
 *              5 November 2007
 *
 *-------------------------------------------------------------------------
 */
static int
test_open_ext_link_twice(hid_t fapl)
{
    hid_t fid = -1;             /* file ID                              */
    hid_t gid = -1;             /* group ID                             */
    hid_t xid = -1;             /* external link ID                     */
    hid_t xid2 = -1;            /* external link ID (2nd opening)       */
    char  filename[1024];       /* file name                            */
    const char *pathname = H5_get_srcdir_filename(LINKED_FILE); /* Corrected test file name */

    TESTING("opening external link twice");

    /* Make a copy of the FAPL, in order to switch to the sec2 driver */
    /* (useful when running test with another VFD) */
    if((fapl = H5Pcopy(fapl)) < 0)
        FAIL_STACK_ERROR

    /* Switch local copy of the fapl to the sec2 driver */
    if(H5Pset_fapl_sec2(fapl) < 0)
        FAIL_STACK_ERROR

    h5_fixname(FILENAME[3], fapl, filename, sizeof filename);

    if((fid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl)) < 0)
        FAIL_STACK_ERROR

    if((gid = H5Gopen2(fid, "/", H5P_DEFAULT)) < 0)
        FAIL_STACK_ERROR

    /* Create an external link to an existing file */
    if(H5Lcreate_external(pathname, "/group", gid, " link", H5P_DEFAULT, H5P_DEFAULT) < 0)
        FAIL_STACK_ERROR

    if(H5Gclose(gid) < 0)
        FAIL_STACK_ERROR

    if(H5Fclose(fid) < 0)
        FAIL_STACK_ERROR

    /* Reopen the file */
    if((fid = H5Fopen(filename, H5F_ACC_RDONLY, fapl)) < 0)
        FAIL_STACK_ERROR

    /* Open the external link which is "/ link" as created previously via H5Lcreate_external() */
    if((xid = H5Gopen2(fid, "/ link", H5P_DEFAULT)) < 0)
        FAIL_STACK_ERROR

    /* Open the external link twice */
    if((xid2 = H5Gopen2(xid, ".", H5P_DEFAULT)) < 0)
        FAIL_STACK_ERROR

    if(H5Gclose(xid2) < 0)
        FAIL_STACK_ERROR

    if(H5Gclose(xid) < 0)
        FAIL_STACK_ERROR

    if(H5Fclose(fid) < 0)
        FAIL_STACK_ERROR

    if(H5Pclose(fapl) < 0)
        FAIL_STACK_ERROR

    PASSED();

    return 0;

 error:
    H5E_BEGIN_TRY {
        H5Gclose(gid);
        H5Gclose(xid);
        H5Gclose(xid2);
        H5Fclose(fid);
    } H5E_END_TRY;
    return 1;
} /* end test_open_ext_link_twice() */


/*-------------------------------------------------------------------------
 * Function:	main
 *
 * Purpose:	Runs external dataset tests.
 *
 * Return:	Success:	exit(0)
 *
 *		Failure:	exit(non-zero)
 *
 * Programmer:	Robb Matzke
 *              Tuesday, March  3, 1998
 *
 *-------------------------------------------------------------------------
 */
int
main(void)
{
    hid_t	fapl_id = -1;       /* file access properties               */
    hid_t	fid = -1;           /* file for test_1* functions           */
    char	filename[1024];     /* file name for test_1* funcs          */
    hid_t	gid = -1;           /* group to emit diagnostics            */
    int		nerrors = 0;        /* number of errors                     */

    h5_reset();
    fapl_id = h5_fileaccess();
    h5_fixname(FILENAME[0], fapl_id, filename, sizeof filename);

    if((fid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id)) < 0)
        FAIL_STACK_ERROR

    /* Create a group that will be used in the file set read test */
    if((gid = H5Gcreate2(fid, "emit-diagnostics", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0)
        FAIL_STACK_ERROR
    if(H5Gclose(gid) < 0) FAIL_STACK_ERROR

    /* These tests use a common file */
    nerrors += test_non_extendible(fid);
    nerrors += test_too_small(fid);
    nerrors += test_large_enough_current_eventual(fid);
    nerrors += test_large_enough_current_not_eventual(fid);
    nerrors += test_unlimited(fid);
    nerrors += test_multiple_files(fid);

    /* These tests use no file */
    nerrors += test_add_to_unlimited();
    nerrors += test_overflow();

    /* These tests use the VFD-aware fapl */
    nerrors += test_read_file_set(fapl_id);
    nerrors += test_write_file_set(fapl_id);
    nerrors += test_open_ext_link_twice(fapl_id);

    /* Verify symbol table messages are cached */
    nerrors += (h5_verify_cached_stabs(FILENAME, fapl_id) < 0 ? 1 : 0);

    if(nerrors > 0) goto error;

    if(H5Fclose(fid) < 0) FAIL_STACK_ERROR

    HDputs("All external storage tests passed.");

    /* Clean up files used by file set tests */
    if(h5_cleanup(FILENAME, fapl_id)) {
        HDremove("extern_1a.raw");
        HDremove("extern_1b.raw");
        HDremove("extern_2a.raw");
        HDremove("extern_2b.raw");
        HDremove("extern_3a.raw");
        HDremove("extern_3b.raw");
        HDremove("extern_4a.raw");
        HDremove("extern_4b.raw");
    } /* end if */

    return EXIT_SUCCESS;

error:
    H5E_BEGIN_TRY {
        H5Fclose(fid);
        H5Pclose(fapl_id);
        H5Gclose(gid);
    } H5E_END_TRY;
    nerrors = MAX(1, nerrors);
    printf ("%d TEST%s FAILED.\n", nerrors, 1==nerrors?"":"s");
    return EXIT_FAILURE;
} /* end main() */

