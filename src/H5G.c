/*-------------------------------------------------------------------------
 * Copyright (C) 1997	National Center for Supercomputing Applications.
 *			All rights reserved.
 *
 *-------------------------------------------------------------------------
 *
 * Created:	H5G.c
 *		Jul 18 1997
 *		Robb Matzke <matzke@llnl.gov>
 *
 * Purpose:	Symbol table functions.	 The functions that begin with
 *		`H5G_stab_' don't understand the naming system; they operate
 * 		on a single symbol table at a time.
 *
 *		The functions that begin with `H5G_node_' operate on the leaf
 *		nodes of a symbol table B-tree.  They should be defined in
 *		the H5Gnode.c file. 
 *
 *		The remaining functions know how to traverse the group
 *		directed graph.
 *
 * Names:	Object names are a slash-separated list of components.  If
 *		the name begins with a slash then it's absolute, otherwise
 *		it's relative ("/foo/bar" is absolute while "foo/bar" is
 *		relative).  Multiple consecutive slashes are treated as
 *		single slashes and trailing slashes are ignored.  The special
 *		case `/' is the root group.  Every file has a root group.
 *
 *		API functions that look up names take a location ID and a
 *		name.  The location ID can be a file ID or a group ID and the
 *		name can be relative or absolute.
 *
 *              +--------------+----------- +--------------------------------+
 * 		| Location ID  | Name       | Meaning                        |
 *              +--------------+------------+--------------------------------+
 * 		| File ID      | "/foo/bar" | Find `foo' within `bar' within |
 *		|              |            | the root group of the specified|
 *		|              |            | file.                          |
 *              +--------------+------------+--------------------------------+
 * 		| File ID      | "foo/bar"  | Find `foo' within `bar' within |
 *		|              |            | the current working group of   |
 *		|              |            | the specified file.            |
 *              +--------------+------------+--------------------------------+
 * 		| File ID      | "/"        | The root group of the specified|
 *		|              |            | file.                          |
 *              +--------------+------------+--------------------------------+
 * 		| File ID      | "."        | The current working group of   |
 *		|              |            | the specified file.            |
 *              +--------------+------------+--------------------------------+
 * 		| Group ID     | "/foo/bar" | Find `foo' within `bar' within |
 *		|              |            | the root group of the file     |
 *		|              |            | containing the specified group.|
 *              +--------------+------------+--------------------------------+
 * 		| Group ID     | "foo/bar"  | File `foo' within `bar' within |
 *		|              |            | the specified group.           |
 *              +--------------+------------+--------------------------------+
 * 		| Group ID     | "/"        | The root group of the file     |
 *		|              |            | containing the specified group.|
 *              +--------------+------------+--------------------------------+
 * 		| Group ID     | "."        | The specified group.           |
 *              +--------------+------------+--------------------------------+
 *
 *
 * Modifications:
 *
 *	Robb Matzke, 5 Aug 1997
 *	Added calls to H5E.
 *
 *	Robb Matzke, 30 Aug 1997
 *	Added `Errors:' field to function prologues.
 *
 *-------------------------------------------------------------------------
 */
#define H5G_PACKAGE /*suppress error message about including H5Gpkg.h */

/* Packages needed by this file... */
#include <H5private.h>
#include <H5Iprivate.h>
#include <H5Bprivate.h>
#include <H5Eprivate.h>
#include <H5Gpkg.h>
#include <H5HLprivate.h>
#include <H5MMprivate.h>
#include <H5Oprivate.h>

#define H5G_INIT_HEAP		8192
#define H5G_RESERVED_ATOMS	0
#define PABLO_MASK		H5G_mask

/* Interface initialization */
static hbool_t interface_initialize_g = FALSE;
#define INTERFACE_INIT	H5G_init_interface
static herr_t H5G_init_interface(void);
static void H5G_term_interface(void);


/*-------------------------------------------------------------------------
 * Function:	H5Gcreate
 *
 * Purpose:	Creates a new group relative to LOC_ID and gives it the
 *		specified NAME.  The group is opened for write access
 *		and it's object ID is returned.
 *
 *		The optional SIZE_HINT specifies how much file space to
 *		reserve to store the names that will appear in this
 *		group. If a non-positive value is supplied for the SIZE_HINT
 *		then a default size is chosen.
 *
 * See also:	H5Gset(), H5Gpush(), H5Gpop()
 *
 * Errors:
 *
 * Return:	Success:	The object ID of a new, empty group open for
 *				writing.  Call H5Gclose() when finished with
 *				the group.
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Wednesday, September 24, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Gcreate(hid_t loc_id, const char *name, size_t size_hint)
{
    H5G_t		   *loc = NULL;
    H5G_t		   *grp = NULL;
    hid_t		    ret_value = FAIL;

    FUNC_ENTER(H5Gcreate, FAIL);

    /* Check arguments */
    if (NULL==(loc=H5G_loc (loc_id))) {
	HRETURN_ERROR (H5E_ARGS, H5E_BADTYPE, FAIL, "not a location");
    }
    if (!name || !*name) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name given");
    }
    
    /* Create the group */
    if (NULL == (grp = H5G_create(loc, name, size_hint))) {
	HRETURN_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "unable to create group");
    }
    if ((ret_value = H5I_register(H5_GROUP, grp)) < 0) {
	H5G_close(grp);
	HRETURN_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL,
		      "unable to register group");
    }
    FUNC_LEAVE(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5Gopen
 *
 * Purpose:	Opens an existing group for modification.  When finished,
 *		call H5Gclose() to close it and release resources.
 *
 * Errors:
 *
 * Return:	Success:	Object ID of the group.
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Wednesday, December 31, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
hid_t
H5Gopen(hid_t loc_id, const char *name)
{
    hid_t	ret_value = FAIL;
    H5G_t	*grp = NULL;
    H5G_t	*loc = NULL;

    FUNC_ENTER(H5Gopen, FAIL);

    /* Check args */
    if (NULL==(loc=H5G_loc(loc_id))) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location");
    }
    if (!name || !*name) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name");
    }
    
    /* Open the group */
    if (NULL == (grp = H5G_open(loc, name))) {
	HRETURN_ERROR(H5E_SYM, H5E_CANTOPENOBJ, FAIL, "unable to open group");
    }
    /* Register an atom for the group */
    if ((ret_value = H5I_register(H5_GROUP, grp)) < 0) {
	H5G_close(grp);
	HRETURN_ERROR(H5E_ATOM, H5E_CANTREGISTER, FAIL,
		      "unable to register group");
    }
    FUNC_LEAVE(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5Gclose
 *
 * Purpose:	Closes the specified group.  The group ID will no longer be
 *		valid for accessing the group.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Wednesday, December 31, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Gclose(hid_t grp_id)
{
    FUNC_ENTER(H5Gclose, FAIL);

    /* Check args */
    if (H5_GROUP != H5I_group(grp_id) ||
	NULL == H5I_object(grp_id)) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a group");
    }
    /*
     * Decrement the counter on the group atom.	 It will be freed if the count
     * reaches zero.
     */
    if (H5I_dec_ref(grp_id) < 0) {
	HRETURN_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "unable to close group");
    }
    FUNC_LEAVE(SUCCEED);
}


/*-------------------------------------------------------------------------
 * Function:	H5Gset
 *
 * Purpose:	Sets the working group for file handle FILE to the
 *		specified group.
 *
 *		Each file handle maintains its own notion of the current
 *		working group.	That is, if a single file is opened with
 *		multiple calls to H5Fopen(), which returns multiple file
 *		handles, then each handle's current working group can be
 *		set independently of the other file handles for that file.
 *
 * 		The initial current working group is the root group.
 *
 * See also:	H5Gpush(), H5Gpop()
 *
 * Errors:
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Wednesday, September 24, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Gset(hid_t loc_id, const char *name)
{
    H5G_t	*grp = NULL;
    H5G_t	*loc = NULL;

    FUNC_ENTER(H5Gset, FAIL);

    /* Check/fix arguments */
    if (NULL==(loc=H5G_loc(loc_id))) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location");
    }
    if (!name || !*name) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name");
    }
    
    /* Set the current working group */
    if (NULL == (grp = H5G_open(loc, name))) {
	HRETURN_ERROR(H5E_ARGS, H5E_NOTFOUND, FAIL, "no such group");
    }
    if (H5G_set(grp) < 0) {
	HRETURN_ERROR(H5E_SYM, H5E_CANTINIT, FAIL,
		      "unable to change current working group");
    }
    
    /* Close the handle */
    if (H5G_close(grp)<0) {
	HRETURN_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "unable to close group");
    }
    FUNC_LEAVE(SUCCEED);
}


/*-------------------------------------------------------------------------
 * Function:	H5Gpush
 *
 * Purpose:	Similar to H5Gset() except the new working group is pushed
 *		on a stack.
 *
 *		Each file handle maintains its own notion of the current
 *		working group.	That is, if a single file is opened with
 *		multiple calls to H5Fopen(), which returns multiple file
 *		handles, then each handle's current working group can be
 *		set independently of the other file handles for that file.
 *
 * See also:	H5Gset(), H5Gpop()
 *
 * Errors:
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Wednesday, September 24, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Gpush(hid_t loc_id, const char *name)
{
    H5G_t	*grp = NULL;
    H5G_t	*loc = NULL;

    FUNC_ENTER(H5Gpush, FAIL);

    /* Check arguments */
    if (NULL == (loc = H5G_loc(loc_id))) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location");
    }
    if (!name || !*name) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "no name");
    }

    /* Push group onto stack */
    if (NULL == (grp = H5G_open(loc, name))) {
	HRETURN_ERROR(H5E_ARGS, H5E_NOTFOUND, FAIL, "no such group");
    }
    if (H5G_push(grp) < 0) {
	HRETURN_ERROR(H5E_SYM, H5E_CANTINIT, FAIL,
		      "can't change current working group");
    }
    /* Close the handle */
    if (H5G_close(grp) < 0) {
	HRETURN_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "unable to close group");
    }
    FUNC_LEAVE(SUCCEED);
}


/*-------------------------------------------------------------------------
 * Function:	H5Gpop
 *
 * Purpose:	Removes the top (latest) entry from the working group stack
 *		and sets the current working group to the previous value.
 *
 *		Each file handle maintains its own notion of the current
 *		working group.	That is, if a single file is opened with
 *		multiple calls to H5Fopen(), which returns multiple file
 *		handles, then each handle's current working group can be
 *		set independently of the other file handles for that file.
 *
 *		If LOC_ID is a group ID then it's used only to determine the
 *		file from which to pop.
 *
 * See also:	H5Gset(), H5Gpush()
 *
 * Errors:
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL.  The final entry cannot be popped from
 *				the group stack (but it can be changed
 *				with H5Gset()).
 *
 * Programmer:	Robb Matzke
 *		Wednesday, September 24, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Gpop(hid_t loc_id)
{
    H5G_t	*loc = NULL;

    FUNC_ENTER(H5Gpop, FAIL);

    /* Check arguments */
    if (NULL == (loc = H5G_loc(loc_id))) {
	HRETURN_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a location");
    }

    /* pop */
    if (H5G_pop(H5G_fileof (loc))<0) {
	HRETURN_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "stack is empty");
    }
    FUNC_LEAVE(SUCCEED);
}


/*-------------------------------------------------------------------------
 * Function:	H5Giterate
 *
 * Purpose:	Iterates over the entries of a group.  The LOC_ID and NAME
 *		identify the group over which to iterate and IDX indicates
 *		where to start iterating (zero means at the beginning).	 The
 *		OPERATOR is called for each member and the iteration
 *		continues until the operator returns non-zero or all members
 *		are processed. The operator is passed a group ID for the
 *		group being iterated, a member name, and OP_DATA for each
 *		member.
 *
 * Return:	Success:	The return value of the first operator that
 *				returns non-zero, or zero if all members were
 *				processed with no operator returning non-zero.
 *
 *		Failure:	FAIL if something goes wrong within the
 *				library, or a negative value returned by one
 *				of the operators.
 *
 * Programmer:	Robb Matzke
 *		Monday, March 23, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Giterate (hid_t loc_id, const char *name, int *idx,
	    H5G_iterate_t op, void *op_data)
{
    int			_idx = 0;
    H5G_bt_ud2_t	udata;
    herr_t		ret_value = FAIL;
    H5G_t		*loc = NULL;
    
    FUNC_ENTER (H5Giterate, FAIL);

    /* Check args */
    if (NULL==(loc=H5G_loc (loc_id))) {
	HRETURN_ERROR (H5E_ARGS, H5E_BADTYPE, FAIL, "not a location");
    }
    if (!name || !*name) {
	HRETURN_ERROR (H5E_ARGS, H5E_BADVALUE, FAIL, "no name specified");
    }
    if (!idx) idx = &_idx;
    if (!op) {
	HRETURN_ERROR (H5E_ARGS, H5E_BADVALUE, FAIL, "no operator specified");
    }

    /*
     * Open the group on which to operate.  We also create a group ID which
     * we can pass to the application-defined operator.
     */
    if (NULL==(udata.group = H5G_open (loc, name))) {
	HRETURN_ERROR (H5E_SYM, H5E_CANTINIT, FAIL, "unable to open group");
    }
    if ((udata.group_id=H5I_register (H5_GROUP, udata.group))<0) {
	H5G_close (udata.group);
	HRETURN_ERROR (H5E_SYM, H5E_CANTINIT, FAIL,
		       "unable to register group");
    }
    
    /* Build udata to pass through H5B_iterate() to H5G_node_iterate() */
    udata.skip = *idx;
    udata.op = op;
    udata.op_data = op_data;

    /* Iterate over the group members */
    if ((ret_value = H5B_iterate (H5G_fileof(udata.group), H5B_SNODE,
				  &(udata.group->ent.cache.stab.btree_addr),
				  &udata))<0) {
	HERROR (H5E_SYM, H5E_CANTINIT, "iteration operator failed");
    }
    H5I_dec_ref (udata.group_id); /*also closes udata.group*/
    FUNC_LEAVE (ret_value);
}

/*
 *-------------------------------------------------------------------------
 *-------------------------------------------------------------------------
 *   N O   A P I   F U N C T I O N S   B E Y O N D   T H I S   P O I N T
 *-------------------------------------------------------------------------
 *------------------------------------------------------------------------- 
 */

/*-------------------------------------------------------------------------
 * Function:	H5G_init_interface
 *
 * Purpose:	Initializes the H5G interface.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Monday, January	 5, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5G_init_interface(void)
{
    FUNC_ENTER(H5G_init_interface, FAIL);

    /* Initialize the atom group for the group IDs */
    if (H5I_init_group(H5_GROUP, H5I_GROUPID_HASHSIZE, H5G_RESERVED_ATOMS,
		       (herr_t (*)(void *)) H5G_close) < 0 ||
	H5_add_exit(H5G_term_interface) < 0) {
	HRETURN_ERROR(H5E_SYM, H5E_CANTINIT, FAIL,
		      "unable to initialize interface");
    }
    FUNC_LEAVE(SUCCEED);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_term_interface
 *
 * Purpose:	Terminates the H5G interface
 *
 * Return:	void
 *
 * Programmer:	Robb Matzke
 *		Monday, January	 5, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
H5G_term_interface(void)
{
    H5I_destroy_group(H5_GROUP);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_component
 *
 * Purpose:	Returns the pointer to the first component of the
 *		specified name by skipping leading slashes.  Returns
 *		the size in characters of the component through SIZE_P not
 *		counting leading slashes or the null terminator.
 *
 * Errors:
 *
 * Return:	Success:	Ptr into NAME.
 *
 *		Failure:	Ptr to the null terminator of NAME.
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Aug 11 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static const char *
H5G_component(const char *name, size_t *size_p)
{
    assert(name);

    while ('/' == *name) name++;
    if (size_p) *size_p = HDstrcspn(name, "/");
    return name;
}


/*-------------------------------------------------------------------------
 * Function:	H5G_namei
 *
 * Purpose:	Translates a name to a symbol table entry.
 *
 *		If the specified name can be fully resolved, then this
 *		function returns the symbol table entry for the named object
 *		through the OBJ_ENT argument. The symbol table entry for the
 *		group containing the named object is returned through the
 *		GRP_ENT argument if it is non-null.  However, if the name
 *		refers to the root object then the GRP_ENT will be
 *		initialized with an undefined object header address.  The
 *		REST argument, if present, will point to the null terminator
 *		of NAME.
 *
 *		If the specified name cannot be fully resolved, then OBJ_ENT
 *		is initialized with the undefined object header address. The
 *		REST argument will point into the NAME argument to the start
 *		of the component that could not be located.  The GRP_ENT will
 *		contain the entry for the symbol table that was being
 *		searched at the time of the failure and will have an
 *		undefined object header address if the search failed at the
 *		root object. For instance, if NAME is `/foo/bar/baz' and the
 *		root directory exists and contains an entry for `foo', and
 *		foo is a group that contains an entry for baz, but baz is not
 *		a group, then the results will be that REST points to `baz',
 *		GRP_ENT has an undefined object header address, and GRP_ENT
 *		is the symbol table entry for `bar' in `/foo'.
 *
 *		Every file has a root group whose name is `/'.  Components of
 *		a name are separated from one another by one or more slashes
 *		(/).  Slashes at the end of a name are ignored.  If the name
 *		begins with a slash then the search begins at the root group
 *		of the file containing LOC_ENT. Otherwise it begins at
 *		LOC_ENT.  The component `.' is a no-op, but `..' is not
 *		understood by this function (unless it appears as an entry in
 *		the symbol table).
 *		
 * Errors:
 *
 * Return:	Success:	SUCCEED if name can be fully resolved.	See
 *				above for values of REST, GRP_ENT, and
 *				OBJ_ENT.
 *
 *		Failure:	FAIL if the name could not be fully resolved.
 *				See above for values of REST, GRP_ENT, and
 *				OBJ_ENT.
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Aug 11 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5G_namei(H5G_entry_t *loc_ent, const char *name, const char **rest/*out*/,
	  H5G_entry_t *grp_ent/*out*/, H5G_entry_t *obj_ent/*out*/)
{
    H5G_entry_t		_grp_ent;	/*entry for current group	*/
    H5G_entry_t		_obj_ent;	/*entry found			*/
    size_t		nchars;		/*component name length		*/
    char		comp[1024];	/*component name buffer		*/

    /* clear output args before FUNC_ENTER() in case it fails */
    if (rest) *rest = name;
    if (!grp_ent) grp_ent = &_grp_ent;
    if (!obj_ent) obj_ent = &_obj_ent;
    memset(grp_ent, 0, sizeof(H5G_entry_t));
    H5F_addr_undef(&(grp_ent->header));
    memset(obj_ent, 0, sizeof(H5G_entry_t));
    H5F_addr_undef(&(obj_ent->header));

    FUNC_ENTER(H5G_namei, FAIL);

    /*
     * Where does the searching start?  For absolute names it starts at the
     * root of the file; for relative names it starts at CWG.
     */
    if (!name || !*name) {
	HRETURN_ERROR (H5E_SYM, H5E_NOTFOUND, FAIL, "no name given");
    } else if (!loc_ent) {
	HRETURN_ERROR (H5E_SYM, H5E_NOTFOUND, FAIL,
		       "no current working group");
    } else if ('/' == *name) {
	*obj_ent = loc_ent->file->shared->root_grp->ent;
    } else {
	*obj_ent = *loc_ent;
    }

    /* traverse the name */
    while ((name = H5G_component(name, &nchars)) && *name) {
	if (rest) *rest = name;

	/*
	 * Copy the component name into a null-terminated buffer so
	 * we can pass it down to the other symbol table functions.
	 */
	if (nchars+1 > sizeof(comp)) {
	    HRETURN_ERROR (H5E_SYM, H5E_COMPLEN, FAIL,
			   "component is too long");
	}
	HDmemcpy(comp, name, nchars);
	comp[nchars] = '\0';

	/*
	 * The special name `.' is a no-op.
	 */
	if ('.' == comp[0] && !comp[1]) {
	    name += nchars;
	    continue;
	}

	/*
	 * Advance to the next component of the name.
	 */
	*grp_ent = *obj_ent;
	HDmemset(obj_ent, 0, sizeof(H5G_entry_t));
	H5F_addr_undef(&(obj_ent->header));

	if (H5G_stab_find(grp_ent, comp, obj_ent/*out*/)<0) {
	    /*
	     * Component was not found in the current symbol table, possibly
	     * because GRP_ENT isn't a symbol table.
	     */
	    HRETURN_ERROR(H5E_SYM, H5E_NOTFOUND, FAIL, "component not found");
	}
	/* next component */
	name += nchars;
    }
    if (rest) *rest = name; /*final null */

    FUNC_LEAVE(SUCCEED);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_mkroot
 *
 * Purpose:	Creates a root group in an empty file and opens it.  If a
 *		root group is already open then this function immediately
 *		returnes.   If ENT is non-null then it's the symbol table
 *		entry for an existing group which will be opened as the root
 *		group.  Otherwise a new root group is created and then
 *		opened.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Aug 11 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5G_mkroot (H5F_t *f, H5G_entry_t *ent)
{
    H5G_entry_t	new_root;		/*new root object		*/
    H5O_stab_t	stab;			/*symbol table message		*/

    FUNC_ENTER(H5G_mkroot, FAIL);

    /* check args */
    assert(f);
    if (f->shared->root_grp) return SUCCEED;

    /*
     * If there is no root object then create one. The root group always has
     * a hard link count of one since it's pointed to by the boot block.
     */
    if (!ent) {
	ent = &new_root;
	if (H5G_stab_create (f, 16, ent/*out*/)<0) {
	    HRETURN_ERROR (H5E_SYM, H5E_CANTINIT, FAIL,
			   "unable to create root group");
	}
	if (1 != H5O_link (ent, 1)) {
	    HRETURN_ERROR (H5E_SYM, H5E_LINK, FAIL,
			   "internal error (wrong link count)");
	}
    } else {
	/*
	 * Open the root object as a group.
	 */
	if (H5O_open (ent)<0) {
	    HRETURN_ERROR (H5E_SYM, H5E_CANTOPENOBJ, FAIL,
			   "unable to open root group");
	}
	if (NULL==H5O_read (ent, H5O_STAB, 0, &stab)) {
	    H5O_close (ent);
	    HRETURN_ERROR (H5E_SYM, H5E_NOTFOUND, FAIL,
			   "root object is not a group");
	}
	H5O_reset (H5O_STAB, &stab);
    }

    /*
     * Create the group pointer.  Also decrement the open object count so we
     * don't count the root group as an open object.  The root group will
     * never be closed.
     */
    f->shared->root_grp = H5MM_xcalloc (1, sizeof(H5G_t));
    f->shared->root_grp->ent = *ent;
    f->shared->root_grp->nref = 1;
    assert (1==f->nopen);
    f->nopen = 0;

    FUNC_LEAVE(SUCCEED);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_create
 *
 * Purpose:	Creates a new empty group with the specified name. The name
 *		is either an absolute name or is relative to LOC.
 *
 * Errors:
 *
 * Return:	Success:	A handle for the group.	 The group is opened
 *				and should eventually be close by calling
 *				H5G_close().
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Aug 11 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
H5G_t *
H5G_create(H5G_t *loc, const char *name, size_t size_hint)
{
    const char	*rest = NULL;		/*the base name			*/
    H5G_entry_t	grp_ent;		/*group containing new group	*/
    char	_comp[1024];		/*name component		*/
    size_t	nchars;			/*number of characters in compon*/
    H5G_t	*grp = NULL;		/*new group			*/

    FUNC_ENTER(H5G_create, NULL);

    /* check args */
    assert(loc);
    assert(name && *name);

    /* lookup name */
    if (0 == H5G_namei(H5G_entof(loc), name, &rest, &grp_ent, NULL)) {
	HRETURN_ERROR(H5E_SYM, H5E_EXISTS, NULL, "already exists");
    }
    H5E_clear(); /*it's OK that we didn't find it */
    assert(H5F_addr_defined(&(grp_ent.header)));

    /* should be one null-terminated component left */
    rest = H5G_component(rest, &nchars);
    assert(rest && *rest);
    if (rest[nchars]) {
	if (H5G_component(rest + nchars, NULL)) {
	    HRETURN_ERROR(H5E_SYM, H5E_NOTFOUND, NULL, "missing component");
	} else if (nchars + 1 > sizeof _comp) {
	    HRETURN_ERROR(H5E_SYM, H5E_COMPLEN, NULL, "component is too long");
	} else {
	    /* null terminate */
	    HDmemcpy(_comp, rest, nchars);
	    _comp[nchars] = '\0';
	    rest = _comp;
	}
    }
    
    /* create an open group */
    grp = H5MM_xcalloc(1, sizeof(H5G_t));
    if (H5G_stab_create(grp_ent.file, size_hint, &(grp->ent)/*out*/) < 0) {
	grp = H5MM_xfree(grp);
	HRETURN_ERROR(H5E_SYM, H5E_CANTINIT, NULL, "can't create grp");
    }
    
    /* insert child name into parent */
    if (H5G_stab_insert(&grp_ent, rest, &(grp->ent)) < 0) {
	H5O_close(&(grp->ent));
	grp = H5MM_xfree(grp);
	HRETURN_ERROR(H5E_SYM, H5E_CANTINIT, NULL, "can't insert");
    }
    grp->nref = 1;
    FUNC_LEAVE(grp);
}

/*-------------------------------------------------------------------------
 * Function:	H5G_open
 *
 * Purpose:	Opens an existing group.  The group should eventually be
 *		closed by calling H5G_close().
 *
 * Return:	Success:	Ptr to a new group.
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *		Monday, January	 5, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
H5G_t *
H5G_open(H5G_t *loc, const char *name)
{
    H5G_t		*grp = NULL;
    H5G_t		*ret_value = NULL;
    H5O_stab_t		mesg;

    FUNC_ENTER(H5G_open, NULL);

    /* Check args */
    assert(loc);
    assert(name && *name);

    /* Open the object, making sure it's a group */
    grp = H5MM_xcalloc(1, sizeof(H5G_t));
    if (H5G_find(loc, name, NULL, &(grp->ent)/*out*/) < 0) {
	HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, NULL, "group not found");
    }
    if (H5O_open(&(grp->ent)) < 0) {
	HGOTO_ERROR(H5E_SYM, H5E_CANTOPENOBJ, NULL, "unable to open group");
    }
    if (NULL==H5O_read (&(grp->ent), H5O_STAB, 0, &mesg)) {
	H5O_close (&(grp->ent));
	HGOTO_ERROR (H5E_SYM, H5E_CANTOPENOBJ, NULL, "not a group");
    }
    grp->nref = 1;
    ret_value = grp;

  done:
    if (!ret_value && grp) {
	H5MM_xfree(grp);
    }
    FUNC_LEAVE(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_reopen
 *
 * Purpose:	Reopens a group by incrementing the open count.
 *
 * Return:	Success:	The GRP argument.
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *		Monday, January	 5, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
H5G_t *
H5G_reopen(H5G_t *grp)
{
    FUNC_ENTER(H5G_reopen, NULL);

    assert(grp);
    assert(grp->nref > 0);

    grp->nref++;

    FUNC_LEAVE(grp);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_close
 *
 * Purpose:	Closes the specified group.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Monday, January	 5, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5G_close(H5G_t *grp)
{
    FUNC_ENTER(H5G_close, FAIL);

    /* Check args */
    assert(grp);
    assert(grp->nref > 0);

    if (1 == grp->nref) {
	assert (grp!=H5G_fileof(grp)->shared->root_grp);
	if (H5O_close(&(grp->ent)) < 0) {
	    HRETURN_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "unable to close");
	}
    }
    --grp->nref;

    FUNC_LEAVE(SUCCEED);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_set
 *
 * Purpose:	Sets the current working group to be the specified group.
 *		This affects only the top item on the group stack for the
 *		specified file as accessed through this file handle.  If the
 *		file is opened multiple times, then the current working group
 *		for this file handle is the only one that is changed.
 *
 * Note:	The group is re-opened and held open until it is removed from
 *		the current working group stack.
 *
 * Errors:
 *		SYM	  CWG		Can't open group. 
 *		SYM	  CWG		Couldn't close previous c.w.g. 
 *		SYM	  CWG		Not a group. 
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Wednesday, September 24, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5G_set (H5G_t *grp)
{
    H5F_t	*f;
    
    FUNC_ENTER(H5G_set, FAIL);

    /* check args */
    assert(grp);
    f = H5G_fileof (grp);

    /*
     * If there is no stack then create one, otherwise close the current
     * working group.
     */
    if (!f->cwg_stack) {
	f->cwg_stack = H5MM_xcalloc(1, sizeof(H5G_cwgstk_t));
    } else if (H5G_close(f->cwg_stack->grp) < 0) {
	HRETURN_ERROR(H5E_SYM, H5E_CWG, FAIL,
		      "couldn't close previous current working group");
    }
    f->cwg_stack->grp = H5G_reopen (grp);

    FUNC_LEAVE(SUCCEED);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_getcwg
 *
 * Purpose:	Returns the current working group.
 *		
 * Return:	Success:	The current working group.  This group should
 *				not* be closed with H5G_close() when the
 *				caller is done with it.
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *		Wednesday, September 24, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
H5G_t *
H5G_getcwg(H5F_t *f)
{
    H5G_t	*ret_value = NULL;

    FUNC_ENTER(H5G_getcwg, NULL);

    /* check args */
    assert(f);

    if (f->cwg_stack) {
	ret_value = f->cwg_stack->grp;
    } else {
	ret_value = f->shared->root_grp;
    }
    
    FUNC_LEAVE(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_push
 *
 * Purpose:	Pushes a new current working group onto the stack.  The GRP
 *		is reopened and held open until it is removed from the stack.
 *
 * Errors:
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Friday, September 19, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5G_push (H5G_t *grp)
{
    H5G_cwgstk_t	*stack = NULL;

    FUNC_ENTER(H5G_push, FAIL);

    /* check args */
    assert(grp);

    /*
     * Push a new entry onto the stack.
     */
    stack = H5MM_xcalloc(1, sizeof(H5G_cwgstk_t));
    stack->grp = H5G_reopen(grp);
    stack->next = H5G_fileof(grp)->cwg_stack;
    H5G_fileof(grp)->cwg_stack = stack;

    FUNC_LEAVE(SUCCEED);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_pop
 *
 * Purpose:	Pops the top current working group off the stack.  If the
 *		stack becomes empty then the current working group is
 *		implicitly the root group.
 *
 * Errors:
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL if the stack is empty.
 *
 * Programmer:	Robb Matzke
 *		Friday, September 19, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5G_pop (H5F_t *f)
{
    H5G_cwgstk_t	   *stack = NULL;

    FUNC_ENTER(H5G_pop, FAIL);

    /* check args */
    assert(f);

    if ((stack = f->cwg_stack)) {
	if (H5G_close(stack->grp) < 0) {
	    HRETURN_ERROR(H5E_SYM, H5E_CWG, FAIL,
			  "can't close current working group");
	}
	f->cwg_stack = stack->next;
	stack->grp = NULL;
	H5MM_xfree(stack);
    } else {
	HRETURN_ERROR(H5E_SYM, H5E_CWG, FAIL, "stack is empty");
    }

    FUNC_LEAVE(SUCCEED);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_insert
 *
 * Purpose:	Inserts a symbol table entry into the group graph.
 *
 * Errors:
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Friday, September 19, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5G_insert(H5G_t *loc, const char *name, H5G_entry_t *ent)
{
    const char	*rest = NULL;	/*part of name not existing yet */
    H5G_entry_t	grp;		/*entry for group to contain obj */
    size_t	nchars;		/*number of characters in name	*/
    char	_comp[1024];	/*name component		*/

    FUNC_ENTER(H5G_insert, FAIL);

    /* Check args. */
    assert (loc);
    assert (name && *name);
    assert (ent);

    /*
     * Look up the name -- it shouldn't exist yet.
     */
    if (H5G_namei(H5G_entof(loc), name, &rest, &grp, NULL) >= 0) {
	HRETURN_ERROR(H5E_SYM, H5E_EXISTS, FAIL, "already exists");
    }
    H5E_clear(); /*it's OK that we didn't find it */
    rest = H5G_component(rest, &nchars);

    /*
     * There should be one component left.  Make sure it's null
     * terminated.
     */
    if (rest[nchars]) {
	if (H5G_component(rest + nchars, NULL)) {
	    HRETURN_ERROR(H5E_SYM, H5E_NOTFOUND, FAIL, "component not found");
	} else if (nchars + 1 > sizeof _comp) {
	    HRETURN_ERROR(H5E_SYM, H5E_COMPLEN, FAIL, "component is too long");
	} else {
	    /* null terminate */
	    HDmemcpy(_comp, rest, nchars);
	    _comp[nchars] = '\0';
	    rest = _comp;
	}
    }

    /*
     * The object into a symbol table.
     */
    if (H5O_link(ent, 1) < 0) {
	HRETURN_ERROR(H5E_SYM, H5E_LINK, FAIL, "link inc failure");
    }
    if (H5G_stab_insert(&grp, rest, ent) < 0) {
	HRETURN_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "can't insert");
    }
    FUNC_LEAVE(SUCCEED);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_find
 *
 * Purpose:	Finds an object with the specified NAME at location LOC.  On
 *		successful return, GRP_ENT (if non-null) will be initialized
 *		with the symbol table information for the group in which the
 *		object appears (it will have an undefined object header
 *		address if the object is the root object) and OBJ_ENT will be
 *		initialized with the symbol table entry for the object
 *		(OBJ_ENT is optional when the caller is interested only in
 * 		the existence of the object).
 *
 * Errors:
 *
 * Return:	Success:	SUCCEED, see above for values of GRP_ENT and
 *				OBJ_ENT.
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Aug 12 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5G_find(H5G_t *loc, const char *name,
	 H5G_entry_t *grp_ent/*out*/, H5G_entry_t *obj_ent/*out*/)
{
    FUNC_ENTER(H5G_find, FAIL);

    /* check args */
    assert (loc);
    assert (name && *name);

    if (H5G_namei(H5G_entof(loc), name, NULL, grp_ent, obj_ent) < 0) {
	HRETURN_ERROR(H5E_SYM, H5E_NOTFOUND, FAIL, "object not found");
    }
    FUNC_LEAVE(SUCCEED);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_entof
 *
 * Purpose:	Returns a pointer to the entry for a group.
 *
 * Return:	Success:	Ptr to group entry
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *              Tuesday, March 24, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
H5G_entry_t *
H5G_entof (H5G_t *grp)
{
    return grp ? &(grp->ent) : NULL;
}


/*-------------------------------------------------------------------------
 * Function:	H5G_fileof
 *
 * Purpose:	Returns the file to which the specified group belongs.
 *
 * Return:	Success:	File pointer.
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *              Tuesday, March 24, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
H5F_t *
H5G_fileof (H5G_t *grp)
{
    assert (grp);
    return grp->ent.file;
}


/*-------------------------------------------------------------------------
 * Function:	H5G_loc
 *
 * Purpose:	Given a location ID return a group.  The group should not be
 *		closed.
 *
 * Return:	Success:	Group pointer.
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *              Tuesday, March 24, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
H5G_t *
H5G_loc (hid_t loc_id)
{
    H5F_t	*f;
    H5G_t	*ret_value = NULL;

    FUNC_ENTER (H5G_loc, NULL);
    
    if (H5_FILE==H5I_group (loc_id)) {
	if (NULL==(f=H5I_object (loc_id))) {
	    HRETURN_ERROR (H5E_ARGS, H5E_BADVALUE, NULL, "invalid file ID");
	}
	if (NULL==(ret_value=H5G_getcwg (f))) {
	    HRETURN_ERROR (H5E_ARGS, H5E_BADVALUE, NULL,
			   "unable to get current working directory");
	}
    } else if (H5_GROUP==H5I_group (loc_id)) {
	if (NULL==(ret_value=H5I_object (loc_id))) {
	    HRETURN_ERROR (H5E_ARGS, H5E_BADVALUE, NULL, "invalid group ID");
	}
    } else {
	HRETURN_ERROR (H5E_ARGS, H5E_BADTYPE, NULL, "not a location");
    }

    FUNC_LEAVE (ret_value);
}
