!
! This file contains Fortran90 interfaces for H5F functions.
! 
      MODULE H5G
      USE H5GLOBAL
 
        CONTAINS
          
!----------------------------------------------------------------------
! Name:		h5gcreate_f 
!
! Purpose:	Creates a new group. 
!
! Inputs:  
!		loc_id		- location identifier
!		name		- group name at the specified location
! Outputs:  
!		grp_id		- group identifier
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!		size_hint	- a parameter indicating the number of bytes 
!				  to reserve for the names that will appear 
!				  in the group
!
! Programmer:	Elena Pourmal
!		August 12, 1999	
!
! Modifications: 	Explicit Fortran interfaces were added for 
!			called C functions (it is needed for Windows
!			port).  March 5, 2001 
!
! Comment:		
!----------------------------------------------------------------------
          SUBROUTINE h5gcreate_f(loc_id, name, grp_id, hdferr, size_hint)
           
            IMPLICIT NONE
            INTEGER(HID_T), INTENT(IN) :: loc_id   ! File or group identifier 
            CHARACTER(LEN=*), INTENT(IN) :: name   ! Name of the group 
            INTEGER(HID_T), INTENT(OUT) :: grp_id  ! Group identifier 
            INTEGER, INTENT(OUT) :: hdferr         ! Error code 
            INTEGER(SIZE_T), OPTIONAL, INTENT(IN) :: size_hint 
                                                   ! Parameter indicating
                                                   ! the number of bytes
                                                   ! to reserve for the
                                                   ! names that will appear
                                                   ! in the group  
            INTEGER :: namelen ! Length of the name character string
            INTEGER(SIZE_T) :: size_hint_default 

!            INTEGER, EXTERNAL :: h5gcreate_c
!  MS FORTRAN needs explicit interface for C functions called here.
!
            INTERFACE
              INTEGER FUNCTION h5gcreate_c(loc_id, name, namelen, &
                               size_hint_default, grp_id)
              USE H5GLOBAL
              !MS$ATTRIBUTES C,reference,alias:'_H5GCREATE_C'::h5gcreate_c
              !DEC$ATTRIBUTES reference :: name
              INTEGER(HID_T), INTENT(IN) :: loc_id
              CHARACTER(LEN=*), INTENT(IN) :: name
              INTEGER :: namelen
              INTEGER(SIZE_T) :: size_hint_default
              INTEGER(HID_T), INTENT(OUT) :: grp_id
              END FUNCTION h5gcreate_c
            END INTERFACE

            size_hint_default = OBJECT_NAMELEN_DEFAULT_F 
            if (present(size_hint)) size_hint_default = size_hint 
            namelen = LEN(name)
            hdferr = h5gcreate_c(loc_id, name, namelen, size_hint_default, &
                                 grp_id)

          END SUBROUTINE h5gcreate_f

!----------------------------------------------------------------------
! Name:		h5gopen_f 
!
! Purpose: 	Opens an existing group. 	
!
! Inputs:  
!		loc_id		- location identifier
!		name 		- name of the group to open
! Outputs:  
!		grp_id		- group identifier
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!				NONE
!
! Programmer:	Elena Pourmal
!		August 12, 1999	
!
! Modifications: 	Explicit Fortran interfaces were added for 
!			called C functions (it is needed for Windows
!			port).  March 5, 2001 
!
! Comment:		
!----------------------------------------------------------------------
          SUBROUTINE h5gopen_f(loc_id, name, grp_id, hdferr)
           
            IMPLICIT NONE
            INTEGER(HID_T), INTENT(IN) :: loc_id   ! File or group identifier 
            CHARACTER(LEN=*), INTENT(IN) :: name   ! Name of the group 
            INTEGER(HID_T), INTENT(OUT) :: grp_id  ! File identifier 
            INTEGER, INTENT(OUT) :: hdferr         ! Error code 

            INTEGER :: namelen ! Length of the name character string

!            INTEGER, EXTERNAL :: h5gopen_c
!  MS FORTRAN needs explicit interface for C functions called here.
!
            INTERFACE
              INTEGER FUNCTION h5gopen_c(loc_id, name, namelen, grp_id)
              USE H5GLOBAL
              !MS$ATTRIBUTES C,reference,alias:'_H5GOPEN_C'::h5gopen_c
              !DEC$ATTRIBUTES reference :: name
              INTEGER(HID_T), INTENT(IN) :: loc_id
              CHARACTER(LEN=*), INTENT(IN) :: name
              INTEGER :: namelen
              INTEGER(HID_T), INTENT(OUT) :: grp_id
              END FUNCTION h5gopen_c
            END INTERFACE
  
            namelen = LEN(name)
            hdferr = h5gopen_c(loc_id, name, namelen, grp_id) 

          END SUBROUTINE h5gopen_f

!----------------------------------------------------------------------
! Name:		h5gclose_f 
!
! Purpose:	Closes the specified group. 	
!
! Inputs:  
!		grp_id		- group identifier
! Outputs:  
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!				NONE
!
! Programmer:	Elena Pourmal
!		August 12, 1999	
!
! Modifications: 	Explicit Fortran interfaces were added for 
!			called C functions (it is needed for Windows
!			port).  March 5, 2001 
!
! Comment:		
!----------------------------------------------------------------------
          SUBROUTINE h5gclose_f(grp_id, hdferr)

            IMPLICIT NONE
            INTEGER(HID_T), INTENT(IN) :: grp_id  ! Group identifier
            INTEGER, INTENT(OUT) :: hdferr        ! Error code

!            INTEGER, EXTERNAL :: h5gclose_c
!  MS FORTRAN needs explicit interface for C functions called here.
!
            INTERFACE
              INTEGER FUNCTION h5gclose_c(grp_id)
              USE H5GLOBAL
              !MS$ATTRIBUTES C,reference,alias:'_H5GCLOSE_C'::h5gclose_c
              INTEGER(HID_T), INTENT(IN) :: grp_id
              END FUNCTION h5gclose_c
            END INTERFACE

            hdferr = h5gclose_c(grp_id)

          END SUBROUTINE h5gclose_f


!----------------------------------------------------------------------
! Name:		h5gget_obj_info_idx_f 
!
! Purpose:	Returns name and type of the group member identified by 
!		its index. 
!
! Inputs:  
!		loc_id		- location identifier
!		name		- name of the group at the specified location
!		idx		- object index (zero-based)
! Outputs:  
!		obj_name	- object name
!		obj_type	- object type
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!				NONE
!
! Programmer:	Elena Pourmal
!		August 12, 1999	
!
! Modifications: 	Explicit Fortran interfaces were added for 
!			called C functions (it is needed for Windows
!			port).  March 5, 2001 
!
! Comment:		
!----------------------------------------------------------------------
          SUBROUTINE h5gget_obj_info_idx_f(loc_id, name, idx, &
                                           obj_name, obj_type, hdferr)
           
            IMPLICIT NONE
            INTEGER(HID_T), INTENT(IN) :: loc_id   ! File or group identifier 
            CHARACTER(LEN=*), INTENT(IN) :: name   ! Name of the group 
            INTEGER, INTENT(IN) :: idx             ! Index of member object 
            CHARACTER(LEN=*), INTENT(OUT) :: obj_name   ! Name of the object 
            INTEGER, INTENT(OUT) :: obj_type       ! Object type 
            INTEGER, INTENT(OUT) :: hdferr         ! Error code 

            INTEGER :: namelen ! Length of the name character string
            INTEGER :: obj_namelen ! Length of the obj_name character string

!            INTEGER, EXTERNAL :: h5gget_obj_info_idx_c
!  MS FORTRAN needs explicit interface for C functions called here.
!
            INTERFACE
              INTEGER FUNCTION h5gget_obj_info_idx_c(loc_id, name, &
                               namelen, idx, &
                               obj_name, obj_namelen, obj_type)
              USE H5GLOBAL
              !MS$ATTRIBUTES C,reference,alias:'_H5GGET_OBJ_INFO_IDX_C'::h5gget_obj_info_idx_c
              !DEC$ATTRIBUTES reference :: name
              !DEC$ATTRIBUTES reference :: obj_name
              INTEGER(HID_T), INTENT(IN) :: loc_id
              CHARACTER(LEN=*), INTENT(IN) :: name
              INTEGER :: namelen
              INTEGER, INTENT(IN) :: idx
              CHARACTER(LEN=*), INTENT(OUT) :: obj_name
              INTEGER :: obj_namelen
              INTEGER, INTENT(OUT) :: obj_type
              END FUNCTION h5gget_obj_info_idx_c
            END INTERFACE

            namelen = LEN(name)
            obj_namelen = LEN(obj_name)
            hdferr = h5gget_obj_info_idx_c(loc_id, name, namelen, idx, &
                                           obj_name, obj_namelen, obj_type)     

          END SUBROUTINE h5gget_obj_info_idx_f

!----------------------------------------------------------------------
! Name:		h5gn_members_f 
!
! Purpose: 	Returns the number of group members. 	
!
! Inputs:  
!		loc_id		- location identifier
!		name		- name of the group at the specified location
! Outputs:  
!		nmembers	- number of group members
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!				NONE
!
! Programmer:	Elena Pourmal
!		August 12, 1999	
!
! Modifications: 	Explicit Fortran interfaces were added for 
!			called C functions (it is needed for Windows
!			port).  March 5, 2001 
!
! Comment:		
!----------------------------------------------------------------------

          SUBROUTINE h5gn_members_f(loc_id, name, nmembers, hdferr)
           
            IMPLICIT NONE
            INTEGER(HID_T), INTENT(IN) :: loc_id   ! File or group identifier 
            CHARACTER(LEN=*), INTENT(IN) :: name   ! Name of the group 
            INTEGER, INTENT(OUT) :: nmembers       ! Number of members in the
                                                   ! group 
            INTEGER, INTENT(OUT) :: hdferr         ! Error code 

            INTEGER :: namelen ! Length of the name character string
  
!            INTEGER, EXTERNAL :: h5gn_members_c
!  MS FORTRAN needs explicit interface for C functions called here.
!
            INTERFACE
              INTEGER FUNCTION h5gn_members_c(loc_id, name, namelen, nmembers)
              USE H5GLOBAL
              !MS$ATTRIBUTES C,reference,alias:'_H5GN_MEMBERS_C'::h5gn_members_c
              !DEC$ATTRIBUTES reference :: name
              INTEGER(HID_T), INTENT(IN) :: loc_id
              CHARACTER(LEN=*), INTENT(IN) :: name
              INTEGER :: namelen
              INTEGER, INTENT(OUT) :: nmembers
              END FUNCTION h5gn_members_c
            END INTERFACE

            namelen = LEN(name)
            hdferr = h5gn_members_c(loc_id, name, namelen, nmembers) 

          END SUBROUTINE h5gn_members_f

!----------------------------------------------------------------------
! Name:		h5glink_f 
!
! Purpose: 	Creates a link of the specified type from new_name 
!		to current_name. 	
!
! Inputs:  
!		loc_id		- location identifier
!		link_type	- link type
!				  Possible values are:
!				  H5G_LINK_HARD_F (0) or
!				  H5G_LINK_SOFT_F (1) 
!		current_name	- name of the existing object if link is a 
!				  hard link. Can be anything for the soft link. 
!		new_name	- new name for the object
! Outputs:  
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!				NONE
!
! Programmer:	Elena Pourmal
!		August 12, 1999	
!
! Modifications: 	Explicit Fortran interfaces were added for 
!			called C functions (it is needed for Windows
!			port).  March 5, 2001 
!
! Comment:		
!----------------------------------------------------------------------

          SUBROUTINE h5glink_f(loc_id, link_type, current_name, &
                                                   new_name, hdferr)

            IMPLICIT NONE
            INTEGER(HID_T), INTENT(IN) :: loc_id   ! File or group identifier 
            INTEGER, INTENT(IN) :: link_type       ! link type
                                                   ! Possible values are:
                                                   ! H5G_LINK_HARD_F (0) or
                                                   ! H5G_LINK_SOFT_F (1) 
            
            CHARACTER(LEN=*), INTENT(IN) :: current_name   
                                                   ! Current name of an object 
            CHARACTER(LEN=*), INTENT(IN) :: new_name ! New name of an object
            INTEGER, INTENT(OUT) :: hdferr         ! Error code
            
            INTEGER :: current_namelen ! Lenghth of the current_name string
            INTEGER :: new_namelen     ! Lenghth of the new_name string

!            INTEGER, EXTERNAL :: h5glink_c
!  MS FORTRAN needs explicit interface for C functions called here.
!
            INTERFACE
              INTEGER FUNCTION h5glink_c(loc_id, link_type, current_name, &
                               current_namelen, new_name, new_namelen)
                              
              USE H5GLOBAL
              !MS$ATTRIBUTES C,reference,alias:'_H5GLINK_C'::h5glink_c
              !DEC$ATTRIBUTES reference :: current_name
              !DEC$ATTRIBUTES reference :: new_name
              INTEGER(HID_T), INTENT(IN) :: loc_id 
              INTEGER, INTENT(IN) :: link_type
              CHARACTER(LEN=*), INTENT(IN) :: current_name
              INTEGER :: current_namelen
              CHARACTER(LEN=*), INTENT(IN) :: new_name
              INTEGER :: new_namelen
              END FUNCTION h5glink_c
            END INTERFACE
            
            current_namelen = LEN(current_name)
            new_namelen = LEN(new_name)
            hdferr = h5glink_c(loc_id, link_type, current_name, &
                               current_namelen, new_name, new_namelen)
          END SUBROUTINE h5glink_f

!----------------------------------------------------------------------
! Name:		h5gunlink_f 
!
! Purpose: 	Removes the specified name from the group graph and 
!		decrements the link count for the object to which name 
!		points	
!
! Inputs:  
!		loc_id		- location identifier
!		name		- name of the object to unlink
! Outputs:  
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!				NONE
!
! Programmer:	Elena Pourmal
!		August 12, 1999	
!
! Modifications: 	Explicit Fortran interfaces were added for 
!			called C functions (it is needed for Windows
!			port).  March 5, 2001 
!
! Comment:		
!----------------------------------------------------------------------

          SUBROUTINE h5gunlink_f(loc_id, name, hdferr)

            IMPLICIT NONE
            INTEGER(HID_T), INTENT(IN) :: loc_id   ! File or group identifier 
            CHARACTER(LEN=*), INTENT(IN) :: name   ! Name of an object 
            INTEGER, INTENT(OUT) :: hdferr         ! Error code
            
            INTEGER :: namelen ! Lenghth of the name character string
            
!            INTEGER, EXTERNAL :: h5gunlink_c
!  MS FORTRAN needs explicit interface for C functions called here.
!
            INTERFACE
              INTEGER FUNCTION h5gunlink_c(loc_id, name, namelen)
              USE H5GLOBAL
              !MS$ATTRIBUTES C,reference,alias:'_H5GUNLINK_C'::h5gunlink_c
              !DEC$ATTRIBUTES reference :: name
              INTEGER(HID_T), INTENT(IN) :: loc_id
              CHARACTER(LEN=*), INTENT(IN) :: name
              INTEGER :: namelen
              END FUNCTION h5gunlink_c
            END INTERFACE
            
            namelen = LEN(name)
            hdferr = h5gunlink_c(loc_id, name, namelen)
          END SUBROUTINE h5gunlink_f

!----------------------------------------------------------------------
! Name:		h5gmove_f 
!
! Purpose:	Renames an object within an HDF5 file.  	
!
! Inputs:  
!		loc_id		- location identifier
!		name		- object's name at specified location
!		new_name	- object's new name
! Outputs:  
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!				NONE
!
! Programmer:	Elena Pourmal
!		August 12, 1999	
!
! Modifications: 	Explicit Fortran interfaces were added for 
!			called C functions (it is needed for Windows
!			port).  March 5, 2001 
!
! Comment:		
!----------------------------------------------------------------------

          
          SUBROUTINE h5gmove_f(loc_id, name, new_name, hdferr)

            IMPLICIT NONE
            INTEGER(HID_T), INTENT(IN) :: loc_id   ! File or group identifier 
            CHARACTER(LEN=*), INTENT(IN) :: name   ! Current name of an object 
            CHARACTER(LEN=*), INTENT(IN) :: new_name ! New name of an object
            INTEGER, INTENT(OUT) :: hdferr         ! Error code
            
            INTEGER :: namelen         ! Lenghth of the current_name string
            INTEGER :: new_namelen     ! Lenghth of the new_name string

!            INTEGER, EXTERNAL :: h5gmove_c
!  MS FORTRAN needs explicit interface for C functions called here.
!
            INTERFACE
              INTEGER FUNCTION h5gmove_c(loc_id, name, namelen, new_name, new_namelen)
              USE H5GLOBAL
              !MS$ATTRIBUTES C,reference,alias:'_H5GMOVE_C'::h5gmove_c
              !DEC$ATTRIBUTES reference :: name
              !DEC$ATTRIBUTES reference :: new_name
              INTEGER(HID_T), INTENT(IN) :: loc_id
              CHARACTER(LEN=*), INTENT(IN) :: name
              INTEGER :: namelen
              CHARACTER(LEN=*), INTENT(IN) :: new_name
              INTEGER :: new_namelen
              END FUNCTION h5gmove_c
            END INTERFACE
            
            namelen = LEN(name)
            new_namelen = LEN(new_name)
            hdferr = h5gmove_c(loc_id, name, namelen, new_name, new_namelen)
          END SUBROUTINE h5gmove_f

!----------------------------------------------------------------------
! Name:		h5gget_linkval_f 
!
! Purpose: 	Returns the name of the object that the symbolic link 
! 		points to. 	
!
! Inputs:  
!		loc_id		- location identifier
!		name		- symbolic link to the object whose name 
!				  is to be returned.  
!		size		- maximum number of characters to be returned
! Outputs:  
!		buffer		- a buffer to hold the name of the object 
!				  being sought 
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!				NONE
!
! Programmer:	Elena Pourmal
!		August 12, 1999	
!
! Modifications: 	Explicit Fortran interfaces were added for 
!			called C functions (it is needed for Windows
!			port).  March 5, 2001 
!
! Comment:		
!----------------------------------------------------------------------

          SUBROUTINE h5gget_linkval_f(loc_id, name, size, buffer, hdferr)

            IMPLICIT NONE
            INTEGER(HID_T), INTENT(IN) :: loc_id   ! File or group identifier 
            CHARACTER(LEN=*), INTENT(IN) :: name   ! Current name of an object 
            INTEGER(SIZE_T), INTENT(IN) :: size    ! Maximum number of buffer
            CHARACTER(LEN=size), INTENT(OUT) :: buffer 
                                                   ! Buffer to hold a name of
                                                   ! the object symbolic link
                                                   ! points to
            INTEGER, INTENT(OUT) :: hdferr         ! Error code
            
            INTEGER :: namelen ! Lenghth of the current_name string

!            INTEGER, EXTERNAL :: h5gget_linkval_c
!  MS FORTRAN needs explicit interface for C functions called here.
!
            INTERFACE
              INTEGER FUNCTION h5gget_linkval_c(loc_id, name, namelen, size, buffer)
              USE H5GLOBAL
              !MS$ATTRIBUTES C,reference,alias:'_H5GGET_LINKVAL_C'::h5gget_linkval_c
              !DEC$ATTRIBUTES reference :: name
              !DEC$ATTRIBUTES reference :: buffer 
              INTEGER(HID_T), INTENT(IN) :: loc_id
              CHARACTER(LEN=*), INTENT(IN) :: name 
              INTEGER :: namelen
              INTEGER(SIZE_T), INTENT(IN) :: size
              CHARACTER(LEN=*), INTENT(OUT) :: buffer
              END FUNCTION h5gget_linkval_c
            END INTERFACE
            
            namelen = LEN(name)
            hdferr = h5gget_linkval_c(loc_id, name, namelen, size, buffer)
          END SUBROUTINE h5gget_linkval_f

!----------------------------------------------------------------------
! Name:		h5gset_comment_f 
!
! Purpose: 	Sets comment for specified object. 	
!
! Inputs:  
!		loc_id		- location identifier
!		name		- name of the object
!		comment		- comment to set for the object
! Outputs:  
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!				NONE
!
! Programmer:	Elena Pourmal
!		August 12, 1999	
!
! Modifications: 	Explicit Fortran interfaces were added for 
!			called C functions (it is needed for Windows
!			port).  March 5, 2001 
!
! Comment:		
!----------------------------------------------------------------------

           SUBROUTINE h5gset_comment_f(loc_id, name, comment, hdferr)

            IMPLICIT NONE
            INTEGER(HID_T), INTENT(IN) :: loc_id   ! File or group identifier 
            CHARACTER(LEN=*), INTENT(IN) :: name   ! Current name of an object 
            CHARACTER(LEN=*), INTENT(IN) :: comment ! New name of an object
            INTEGER, INTENT(OUT) :: hdferr         ! Error code
            
            INTEGER :: namelen ! Lenghth of the current_name string
            INTEGER :: commentlen     ! Lenghth of the comment string

!            INTEGER, EXTERNAL :: h5gset_comment_c
!  MS FORTRAN needs explicit interface for C functions called here.
!
            INTERFACE
              INTEGER FUNCTION h5gset_comment_c(loc_id, name, namelen, &
                                                comment, commentlen)
              USE H5GLOBAL
              !MS$ATTRIBUTES C,reference,alias:'_H5GSET_COMMENT_C'::h5gset_comment_c
              !DEC$ATTRIBUTES reference :: name 
              !DEC$ATTRIBUTES reference :: comment 
              INTEGER(HID_T), INTENT(IN) :: loc_id
              CHARACTER(LEN=*), INTENT(IN) :: name
              INTEGER :: namelen
              CHARACTER(LEN=*), INTENT(IN) :: comment
              INTEGER :: commentlen
              END FUNCTION h5gset_comment_c
            END INTERFACE
            
            namelen = LEN(name)
            commentlen = LEN(comment)
            hdferr = h5gset_comment_c(loc_id, name, namelen, comment, commentlen)
          END SUBROUTINE h5gset_comment_f

!----------------------------------------------------------------------
! Name:		h5gget_comment_f 
!
! Purpose:	Retrieves comment for specified object.  	
!
! Inputs:  
!		loc_id		- location identifier
!		name		- name of the object at specified location
!		size		- size of the buffer required to hold comment
! Outputs:  
!		buffer		- buffer to hold object's comment
!		hdferr:		- error code		
!				 	Success:  0
!				 	Failure: -1   
! Optional parameters:
!				NONE
!
! Programmer:	Elena Pourmal
!		August 12, 1999	
!
! Modifications: 	Explicit Fortran interfaces were added for 
!			called C functions (it is needed for Windows
!			port).  March 5, 2001 
!
! Comment:		
!----------------------------------------------------------------------

          SUBROUTINE h5gget_comment_f(loc_id, name, size, buffer, hdferr)

            IMPLICIT NONE
            INTEGER(HID_T), INTENT(IN) :: loc_id   ! File or group identifier 
            CHARACTER(LEN=*), INTENT(IN) :: name   ! Current name of an object 
            INTEGER(SIZE_T), INTENT(IN) :: size    ! Maximum number of buffer
            CHARACTER(LEN=size), INTENT(OUT) :: buffer 
                                                   ! Buffer to hold a comment
            INTEGER, INTENT(OUT) :: hdferr         ! Error code
            
            INTEGER :: namelen ! Lenghth of the current_name string

!            INTEGER, EXTERNAL :: h5gget_comment_c
!  MS FORTRAN needs explicit interface for C functions called here.
!
            INTERFACE
              INTEGER FUNCTION h5gget_comment_c(loc_id, name, namelen, &
                                                size, buffer)
              USE H5GLOBAL
              !MS$ATTRIBUTES C,reference,alias:'_H5GGET_COMMENT_C'::h5gget_comment_c
              !DEC$ATTRIBUTES reference :: name 
              !DEC$ATTRIBUTES reference :: buffer 
              INTEGER(HID_T), INTENT(IN) :: loc_id
              CHARACTER(LEN=*), INTENT(IN) :: name
              INTEGER :: namelen
              INTEGER(SIZE_T), INTENT(IN) :: size
              CHARACTER(LEN=*), INTENT(OUT) :: buffer
              END FUNCTION h5gget_comment_c
            END INTERFACE
            
            namelen = LEN(name)
            hdferr = h5gget_comment_c(loc_id, name, namelen, size, buffer)
          END SUBROUTINE h5gget_comment_f


      END MODULE H5G
