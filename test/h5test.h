/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have     *
 * access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu. *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Programmer:  Robb Matzke <matzke@llnl.gov>
 *              Friday, November 20, 1998
 *
 * Purpose:     Test support stuff.
 */
#ifndef _H5TEST_H
#define _H5TEST_H

#undef NDEBUG
#include "hdf5.h"
#include "H5private.h"

#ifdef H5_STDC_HEADERS
#   include <signal.h>
#   include <stdarg.h>
#endif

/*
 * Predefined test verbosity levels.
 *
 * Convention:
 * 
 * The higher the verbosity value, the more information printed.
 * So, output for higher verbosity also include output of all lower
 * verbosity.
 * 
 *  Value     Description
 *  0         None:   No informational message.
 *  1                 "All tests passed"
 *  2                 Header of overall test
 *  3         Default: header and results of individual test
 *  4         
 *  5         Low:    Major category of tests.
 *  6
 *  7         Medium: Minor category of tests such as functions called.
 *  8
 *  9         High:   Highest level.  All information.
 */
#define VERBO_NONE 0     /* None    */
#define VERBO_DEF  3     /* Default */
#define VERBO_LO   5     /* Low     */
#define VERBO_MED  7     /* Medium  */
#define VERBO_HI   9     /* High    */

/*
 * Verbose queries
 * Only None needs an exact match.  The rest are at least as much.
 */
#define VERBOSE_NONE	(GetTestVerbosity()==VERBO_NONE)
#define VERBOSE_DEF	(GetTestVerbosity()>=VERBO_DEF)
#define VERBOSE_LO	(GetTestVerbosity()>=VERBO_LO)
#define VERBOSE_MED	(GetTestVerbosity()>=VERBO_MED)
#define VERBOSE_HI	(GetTestVerbosity()>=VERBO_HI)

/*
 * This contains the filename prefix specificied as command line option for
 * the parallel test files. 
 */
extern char *paraprefix;
#ifdef H5_HAVE_PARALLEL
extern MPI_Info h5_io_info_g;         /* MPI INFO object for IO */
#endif

/*
 * Print the current location on the standard output stream.
 */
#define AT() 		printf ("	 at %s:%d in %s()...\n",	      \
				__FILE__, __LINE__, __FUNCTION__);

/*
 * The name of the test is printed by saying TESTING("something") which will
 * result in the string `Testing something' being flushed to standard output.
 * If a test passes, fails, or is skipped then the PASSED(), H5_FAILED(), or
 * SKIPPED() macro should be called.  After H5_FAILED() or SKIPPED() the caller
 * should print additional information to stdout indented by at least four
 * spaces.  If the h5_errors() is used for automatic error handling then
 * the H5_FAILED() macro is invoked automatically when an API function fails.
 */
#define TESTING(WHAT)	{printf("Testing %-62s",WHAT); fflush(stdout);}
#define PASSED()	{puts(" PASSED");fflush(stdout);}
#define H5_FAILED()	{puts("*FAILED*");fflush(stdout);}
#define SKIPPED()	{puts(" -SKIP-");fflush(stdout);}
#define TEST_ERROR      {H5_FAILED(); AT(); goto error;}

#ifdef __cplusplus
extern "C" {
#endif

/* Generally useful testing routines */
H5TEST_DLL int h5_cleanup(const char *base_name[], hid_t fapl);
H5TEST_DLL char *h5_fixname(const char *base_name, hid_t fapl, char *fullname,
		 size_t size);
H5TEST_DLL hid_t h5_fileaccess(void);
H5TEST_DLL void h5_no_hwconv(void);
H5TEST_DLL void h5_reset(void);
H5TEST_DLL void h5_show_hostname(void);
H5TEST_DLL off_t h5_get_file_size(const char *filename);
H5TEST_DLL int print_func(const char *format, ...);

/* Routines for operating on the list of tests (for the "all in one" tests) */
H5TEST_DLL void TestUsage(void);
H5TEST_DLL void AddTest(const char *TheName, void (*TheCall) (void),
	     void (*Cleanup) (void), const char *TheDescr, 
	     const void *Parameters);
H5TEST_DLL void TestInfo(const char *ProgName);
H5TEST_DLL void TestParseCmdLine(int argc, char *argv[], int *Summary, int *CleanUp);
H5TEST_DLL void PerformTests(void);
H5TEST_DLL void TestSummary(void);
H5TEST_DLL void TestCleanup(void);
H5TEST_DLL void TestInit(void);
H5TEST_DLL int  GetTestVerbosity(void);
H5TEST_DLL int  SetTestVerbosity(int newval);
H5TEST_DLL void ParseTestVerbosity(char *argv);
H5TEST_DLL int  GetTestNumErrs(void);
H5TEST_DLL void *GetTestParameters(void);
H5TEST_DLL int  TestErrPrintf(const char *format, ...);

#ifdef H5_HAVE_PARALLEL
H5TEST_DLL int h5_set_info_object(void);
H5TEST_DLL void h5_dump_info_object(MPI_Info info);
#endif

#ifdef __cplusplus
}
#endif
#endif
