#############################
Expected output for 'h5dump -H -p -d nbit tfilters.h5'
#############################
HDF5 "tfilters.h5" {
DATASET "nbit" {
   DATATYPE  undefined integer
   DATASPACE  SIMPLE { ( 20, 10 ) / ( 20, 10 ) }
   STORAGE_LAYOUT {
      CHUNKED ( 10, 5 )
      SIZE 76
    }
   FILTERS {
      COMPRESSION NBIT
   }
   FILLVALUE {
      FILL_TIME H5D_FILL_TIME_IFSET
      VALUE  0   
   }
   ALLOCATION_TIME {
      H5D_ALLOC_TIME_INCR
   }
}
}
