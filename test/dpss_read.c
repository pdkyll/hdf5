/*
 * Copyright � 1998 NCSA
 *                  All rights reserved.
 *
 * Programmer:  Saurabh Bagchi (bagchi@uiuc.edu)
 *              Wednesday, August 11, 1999.
 *
 * Modifications: Saurabh Bagchi (Aug 17, 1999)
 *                Modified to work with VFL (HDF51.3).
 */

/* Test the following functionality of the DPSS driver. 
   1. Open a remote file for read (the dataset was written using dpss_write.c).
   2. Create a memory buffer to hold the dataset.
   3. Read the dataset into the memory buffer.
   4. Get some information about the dataset from the file.
*/
#include <h5test.h>
#include <strings.h>

#ifndef H5_HAVE_DPSS
int main(void)
{
    printf("Test skipped because DPSS driver not available\n");
    return 0;
}
#else

#define DATASETNAME "IntArray"
#define NX_SUB  98           /* hyperslab dimensions */ 
#define NY_SUB  98 
#define NX 100           /* output buffer dimensions */ 
#define NY 100 
#define NZ  3 
#define RANK         2
#define RANK_OUT     3

int
main (int argc, char **argv)
{
    hid_t       fapl =  -1, file, dataset;         /* handles */
    hid_t       datatype, dataspace;   
    hid_t       memspace; 
    H5T_class_t class;                 /* data type class */
    H5T_order_t order;                 /* data order */
    size_t      size;                  /*
				        * size of the data element	       
				        * stored in file
				        */
    hsize_t     dimsm[3];              /* memory space dimensions */
    hsize_t     dims_out[2];           /* dataset dimensions */      
    herr_t      status;                             

    int         data_out[NX][NY][NZ ]; /* output buffer */
   
    hsize_t      count[2];              /* size of the hyperslab in the file */
    hssize_t     offset[2];             /* hyperslab offset in the file */
    hsize_t      count_out[3];          /* size of the hyperslab in memory */
    hssize_t     offset_out[3];         /* hyperslab offset in memory */
    int          i, j, k, status_n, rank;
    char         *url;
    
    if (argc != 2) {
      printf ("Incorrect command line. \n");
      printf ("Correct command line: %s <url>\n\n", argv[0]);
      exit(1);
    }
    
    url = argv [1];

    printf ("\n Reading dataset %s \n\n", DATASETNAME);
    
    for (j = 0; j < NX; j++) {
	for (i = 0; i < NY; i++) {
	    for (k = 0; k < NZ ; k++)
		data_out[j][i][k] = 0;
	}
    } 
 
     /* Create access property list and set the driver to DPSS */
    fapl = H5Pcreate (H5P_FILE_ACCESS);
    if (fapl < 0) {
      printf (" H5Pcreate failed. \n");
      return -1;
    }
    
    status = H5Pset_fapl_dpss (fapl);
    if (status < 0) {
      printf ("H5Pset_fapl_dpss failed. \n");
      return -1;
    }

    /*
     * Open the file and the dataset.
     */
    file = H5Fopen(url, H5F_ACC_RDONLY, fapl);
    if (file < 0) {
      printf ("Could not open file '%s'\n", url);
      return -1;
    }
    dataset = H5Dopen(file, DATASETNAME);

    /*
     * Get datatype and dataspace handles and then query
     * dataset class, order, size, rank and dimensions.
     */
    datatype  = H5Dget_type(dataset);     /* datatype handle */ 
    class     = H5Tget_class(datatype);
    if (class == H5T_INTEGER) printf("Data set has INTEGER type \n");
    order     = H5Tget_order(datatype);
    if (order == H5T_ORDER_LE) printf("Little endian order \n");

    size  = H5Tget_size(datatype);
    printf(" Data size is %d \n", size);

    dataspace = H5Dget_space(dataset);    /* dataspace handle */
    rank      = H5Sget_simple_extent_ndims(dataspace);
    status_n  = H5Sget_simple_extent_dims(dataspace, dims_out, NULL);
    printf("rank %d, dimensions %lu x %lu \n", rank,
	   (unsigned long)(dims_out[0]), (unsigned long)(dims_out[1]));

    /* 
     * Define hyperslab in the dataset. 
     */
    offset[0] = 0;
    offset[1] = 0;
    count[0]  = NX_SUB;
    count[1]  = NY_SUB;
    status = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, 
				 count, NULL);

    /*
     * Define the memory dataspace.
     */
    dimsm[0] = NX;
    dimsm[1] = NY;
    dimsm[2] = NZ ;
    memspace = H5Screate_simple(RANK_OUT,dimsm,NULL);   

    /* 
     * Define memory hyperslab. 
     */
    offset_out[0] = 3;
    offset_out[1] = 0;
    offset_out[2] = 0;
    count_out[0]  = NX_SUB;
    count_out[1]  = NY_SUB;
    count_out[2]  = 1;
    status = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, offset_out, NULL, 
				 count_out, NULL);

    /*
     * Read data from hyperslab in the file into the hyperslab in 
     * memory and display.
     */
    status = H5Dread(dataset, H5T_NATIVE_INT, memspace, dataspace,
		     H5P_DEFAULT, data_out);
#if 0
    for (j = 0; j < NX; j++) {
	for (i = 0; i < NY; i++) printf("%d ", data_out[j][i][0]);
	printf("\n");
    }
#endif
    /*
     * 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0
     * 3 4 5 6 0 0 0  
     * 4 5 6 7 0 0 0
     * 5 6 7 8 0 0 0
     * 0 0 0 0 0 0 0
     */

    /*
     * Close/release resources.
     */
    H5Tclose(datatype);
    H5Dclose(dataset);
    H5Sclose(dataspace);
    H5Sclose(memspace);
    H5Fclose(file);
    H5Pclose(fapl);
    
    return 0;
}     
#endif
  
