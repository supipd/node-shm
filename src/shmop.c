/*
 * SHM ... shared memory module
 *  Copyright (c) 2013 Jan Supuka <sbmintegral@sbmintegral.sk>
 *	MIT Licensed
*/

#include "shmop.h"

/*****************************DEBUGGING*************************/

static char errmsg[1024];
char * shm_err() {	return errmsg;	}

static int activateDebug = 0;
void shm_setDbg( int on )  { activateDebug = on; if(on){ fprintf(stderr,"\nDebugging ON\n");} }

void shm_dbg(const char * format, ...) { 
  va_list args;
  va_start (args, format);
  if (activateDebug) {
	vfprintf (stderr, format, args);
	if (errno) {
		printf(" ERRNO: %d ... %s \n", errno, strerror(errno));
		errno = 0;
	}
  }
  va_end (args);
}

static char outputXT[128];
char *firstXhxtx(void *data, int size) {
	int i, cnt = size > 8 ? 8 : size;
	outputXT[0] = 0;	// strlen = 0
	if (data && (cnt > 0)) {
		for (i = 0; i < cnt; i++) {	sprintf(outputXT + strlen(outputXT), "0x%02X,", ((unsigned char*)data)[i]); 	}
		strcat(outputXT, " ... ");
		for (i = 0; i < cnt; i++) {	sprintf(outputXT + strlen(outputXT), "%c", ((unsigned char*)data)[i]); 	}
	} else {
		sprintf(outputXT, "NULL");
	}
	return outputXT;
}


/*****************************SHM POOL*************************/
static php_shmop shmPOOL[MAX_SHM_IDs];
static int shmPOOLcnt = 0;
static php_shmop *shm_wrk_copy;

php_shmop *shm_pool(int handle, int key, void *addr) {
	php_shmop *ptr;
	int i; 
	int i_free = -1, i_shmid = -1, i_key = -1, i_addr = -1;

	for (i = 0; i < MAX_SHM_IDs; i++) {
		ptr = &(shmPOOL[i]);
		if ((i_free == -1) && (!ptr->in_use))	{
			i_free  =i; 		//oznacim si prvy volny
		}
		if (key)	{
			if (ptr->key == key)	{
				if (i_key != -1)	
					return NULL;	//CHYBA, nemozu byt 2 polozky s rovnakym KEY
				i_key = i;			//poznacim 'key' sa zhoduje
			} 
			if (ptr->shmid == key)	{
				if (i_shmid != -1)	
					return NULL;	//CHYBA, nemozu byt 2 polozky s rovnakym SHMID
				i_shmid = i;		//poznacim 'shmid' sa zhoduje
			} 
		}
		if (addr && (ptr->addr == addr))	{
			if (i_addr != -1)	
				return NULL;	//CHYBA, nemozu byt 2 polozky s rovnakym ADDR
			i_addr = i;			//poznacim 'addr' sa zhoduje
		} 
	}
	ptr = NULL;
	switch (handle & ~DELETE_IF_EXISTS)	{
		case CREATE_NEW_IF_NOT_EXISTS:	//vzdy podla 'key'
			if (i_key != -1)
				ptr = &(shmPOOL[i_key]);		//pokial som nasiel potrebnu zhodu, odovzdavam
			else if (i_free != -1) {
				shmPOOL[i_free].in_use = 1;
shmPOOL[i_free].key = key;
				ptr = &(shmPOOL[i_free]);		//pokial neexistuje rovnaky 'key' a je volne miesto, odovzdavam
shmPOOLcnt++;
			}
			break;
		case GET_IF_EXISTS_BY_KEY:
			if (i_key != -1)
				ptr = &(shmPOOL[i_key]);		//pokial som nasiel potrebnu zhodu, odovzdavam
			break;
		case GET_IF_EXISTS_BY_SHMID:
			if (i_shmid != -1)
				ptr = &(shmPOOL[i_shmid]);		//pokial som nasiel potrebnu zhodu, odovzdavam
			break;
		case GET_IF_EXISTS_BY_ADDR:
			if (i_addr != -1)
				ptr = &(shmPOOL[i_addr]);		//pokial som nasiel potrebnu zhodu, odovzdavam
			break;
	}
	if ( ptr && (handle & DELETE_IF_EXISTS)) {
		memset(ptr, 0, sizeof(php_shmop));
shmPOOLcnt--;
	}
	shm_wrk_copy = ptr;
	return ptr;
}

php_shmop * getFromPool(int shmid) {
	if ( ! shm_wrk_copy || (shm_wrk_copy->shmid != shmid) ) {
	/*return */	shm_pool(GET_IF_EXISTS_BY_SHMID, shmid, NULL);		//auto update  shm_wrk_copy
	}
shm_dbg("getFromPool shmid : %d %s \n", shmid, shm_wrk_copy ? "success" : "not found");		
	return shm_wrk_copy;
};
php_shmop * removeFromPool(int shmid) {
	return shm_pool(GET_IF_EXISTS_BY_KEY | DELETE_IF_EXISTS, shmid, NULL);
};

php_shmop shmop_list(int index) {	//maybe in future usefull 
	php_shmop ps;
	memset(&ps, 0, sizeof(php_shmop));
	if ((index < 0) || (index >= MAX_SHM_IDs)) {
		ps.in_use = -1;
		ps.size = MAX_SHM_IDs;
	} else {
		ps = shmPOOL[index];
	}
	return ps;
}
void printSHMpool() {
	int i,c;
	php_shmop shmop;
	
	shmop = shmop_list(-1);
	c = shmop.size;	//defined size of POOL
	printf("\rPOOL[%d]: ", c );
	for (i = 0; i < c; i++) {
		shmop = shmop_list(i);
		if (shmop.in_use) {
			printf(" _%d_ %d / %d, ", i, shmop.key, shmop.shmid);
		}
	}
	printf("\n");
}


/*****************************OS DEPENDANT FUNCTIONS*************************/
#if ! defined _WIN32

php_shmop * saveToPool(php_shmop *shmop) {
	php_shmop * ps = shm_pool(CREATE_NEW_IF_NOT_EXISTS, shmop->key, NULL);
	if (ps) {
		shmop->in_use = 1;
		memcpy(ps, shmop, sizeof(php_shmop));
	}
shm_dbg("saveToPool shmid : %d %s \n", shmop->shmid, ps ? "success" : "not found");		
	return ps;
};
//______________________________________________________________________________________________
#else	//#if defined _WIN32

php_shmop * saveToPool(php_shmop *shmop) {
	php_shmop * ps = shm_pool(GET_IF_EXISTS_BY_KEY, shmop->key, NULL);
	if (ps) {
		shmop->in_use = 1;
#if defined _WIN32
		memcpy( &shmop->i_shm_pair, &ps->i_shm_pair, sizeof(ps->i_shm_pair) );
#endif
		memcpy(ps, shmop, sizeof(php_shmop));
	}
	return ps;
};

shm_pair *shm_get(int handle, int key, void *addr) {
	php_shmop *psmo;
	
	psmo = shm_pool(handle, key, addr);
	if (psmo == NULL)
		return NULL;
	return &(psmo->i_shm_pair);	
}

/*
In order to create a new message queue, or access an existing queue, the shmget() system call is used.

  SYSTEM CALL: shmget();                                                          

  PROTOTYPE: int shmget ( key_t key, int size, int shmflg );                                             
    RETURNS: shared memory segment identifier on success                                                       
             -1 on error: errno = EINVAL (Invalid segment size specified)
                                  EEXIST (Segment exists, cannot create)
                                  EIDRM (Segment is marked for deletion, or was removed)
                                  ENOENT (Segment does not exist)
                                  EACCES (Permission denied)
                                  ENOMEM (Not enough memory to create segment)
  NOTES:
This particular call should almost seem like old news at this point. 
It is strikingly similar to the corresponding get calls for message queues and semaphore sets.
The first argument to shmget() is the key value (in our case returned by a call to ftok()). 
This key value is then compared to existing key values that exist within the kernel for other shared memory segments. 
At that point, the open or access operation is dependent upon the contents of the shmflg argument.

IPC_CREAT
Create the segment if it doesn't already exist in the kernel.

IPC_EXCL
When used with IPC_CREAT, fail if segment already exists.

If IPC_CREAT is used alone, shmget() either returns the segment identifier for a newly created segment, 
or returns the identifier for a segment which exists with the same key value. 
If IPC_EXCL is used along with IPC_CREAT, then either a new segment is created, or if the segment exists, the call fails with -1. 
IPC_EXCL is useless by itself, but when combined with IPC_CREAT, it can be used as a facility to guarantee that no existing segment is opened for access.

Once again, an optional octal mode may be OR'd into the mask.

Let's create a wrapper function for locating or creating a shared memory segment :

int open_segment( key_t keyval, int segsize )
{
        int     shmid;
        
        if((shmid = shmget( keyval, segsize, IPC_CREAT | 0660 )) == -1)
        {
                return(-1);
        }
        
        return(shmid);
}
Note the use of the explicit permissions of 0660. 
This small function either returns a shared memory segment identifier (int), or -1 on error. 
The key value and requested segment size (in bytes) are passed as arguments.
Once a process has a valid IPC identifier for a given segment, the next step is for the process to attach or map the segment into its own addressing space.
*/ 
TSRM_API int shmget ( key_t key, int size, int shmflg ) { //int shmget(int key, int size, int flags) {
	shm_pair *shm;
	char shm_segment[26], shm_info[29];
	HANDLE shm_handle, info_handle;
	BOOL created = FALSE;
	DWORD errnum;	//for DEBUG

	if (size < 0) {
		return -1;
	}

	sprintf(shm_segment, "TSRM_SHM_SEGMENT:%d", key);
	sprintf(shm_info, "TSRM_SHM_DESCRIPTOR:%d", key);

	shm_handle  = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, shm_segment);
	info_handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, shm_info);

	if ((!shm_handle && !info_handle)) {
		if (shmflg & IPC_CREAT) {
			shm_handle	= CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, shm_segment);
			info_handle	= CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(shm->descriptor), shm_info);
			created		= TRUE;
		}
		if ((!shm_handle || !info_handle)) {
			errnum = GetLastError();
			return -1;
		}
	} else {
		if (shmflg & IPC_EXCL) {
			return -1;
		}
	}
	
	if ((shm = shm_get(CREATE_NEW_IF_NOT_EXISTS, key, NULL)) == NULL)	return -1;
	shm->segment = shm_handle;
	shm->info	 = info_handle;
	shm->descriptor = MapViewOfFileEx(shm->info, FILE_MAP_ALL_ACCESS, 0, 0, 0, NULL);

	if (created) {
		shm->descriptor->shm_perm.key	= key;
		shm->descriptor->shm_segsz		= size;
		shm->descriptor->shm_ctime		= time(NULL);
		shm->descriptor->shm_cpid		= getpid();
		shm->descriptor->shm_perm.mode	= shmflg;

		shm->descriptor->shm_perm.cuid	= shm->descriptor->shm_perm.cgid= 0;
		shm->descriptor->shm_perm.gid	= shm->descriptor->shm_perm.uid = 0;
		shm->descriptor->shm_atime		= shm->descriptor->shm_dtime	= 0;
		shm->descriptor->shm_lpid		= shm->descriptor->shm_nattch	= 0;
		shm->descriptor->shm_perm.mode	= shm->descriptor->shm_perm.seq	= 0;
	}
	if (shm->descriptor->shm_perm.key != key || size > shm->descriptor->shm_segsz ) {
		CloseHandle(shm->segment);
		UnmapViewOfFile(shm->descriptor);
		CloseHandle(shm->info);
		return -1;
	}	
	return key;
}

/*
  SYSTEM CALL: shmat();                                                          

  PROTOTYPE: int shmat ( int shmid, char *shmaddr, int shmflg);
    RETURNS: address at which segment was attached to the process, or
             -1 on error: errno = EINVAL (Invalid IPC ID value or attach address passed)
                                  ENOMEM (Not enough memory to attach segment)
                                  EACCES (Permission denied)
  NOTES:
If the addr argument is zero (0), the kernel tries to find an unmapped region. 
This is the recommended method. 
An address can be specified, but is typically only used to facilitate proprietary hardware 
or to resolve conflicts with other apps. 
The SHM_RND flag can be OR'd into the flag argument to force a passed address to be page aligned (rounds down to the nearest page size).
In addition, if the SHM_RDONLY flag is OR'd in with the flag argument, then the shared memory segment will be mapped in, but marked as readonly.

This call is perhaps the simplest to use. 
Consider this wrapper function, which is passed a valid IPC identifier for a segment, and returns the address that the segment was attached to:

char *attach_segment( int shmid )
{
        return(shmat(shmid, 0, 0));
}
Once a segment has been properly attached, and a process has a pointer to the start of that segment, 
reading and writing to the segment become as easy as simply referencing or dereferencing the pointer! 
Be careful not to lose the value of the original pointer! If this happens, you will have no way of accessing the base (start) of the segment.
*/
TSRM_API void *shmat(int shmid, const void *shmaddr, int shmflg) {	//void *shmat(int key, const void *shmaddr, int flags) {
	shm_pair *shm;
	if ((shm = shm_get(GET_IF_EXISTS_BY_KEY, shmid, NULL)) == NULL)	return (void*)-1;
	
	if (!shm->segment) {
		return (void*)-1;
	}

	shm->descriptor->shm_atime = time(NULL);
	shm->descriptor->shm_lpid  = getpid();
	shm->descriptor->shm_nattch++;

	shm->addr = MapViewOfFileEx(shm->segment, FILE_MAP_ALL_ACCESS, 0, 0, 0, NULL);

	return shm->addr;
}

/*
  SYSTEM CALL: shmdt();                                                          

  PROTOTYPE: int shmdt ( char *shmaddr );
    RETURNS: -1 on error: errno = EINVAL (Invalid attach address passed)
	 
After a shared memory segment is no longer needed by a process, it should be detached by calling this system call. 
As mentioned earlier, this is not the same as removing the segment from the kernel! 
After a detach is successful, the shm_nattch member of the associates shmid_ds structure is decremented by one. 
When this value reaches zero (0), the kernel will physically remove the segment.
*/
TSRM_API int shmdt(const void *shmaddr) {
	int ret;
	shm_pair *shm;
	if ((shm = shm_get(GET_IF_EXISTS_BY_ADDR, 0, (void*)shmaddr)) == NULL)	return -1;

	if (!shm->segment) {
		return -1;
	}
	shm->descriptor->shm_dtime = time(NULL);
	shm->descriptor->shm_lpid  = getpid();
	shm->descriptor->shm_nattch--;

	//return UnmapViewOfFile(shm->addr) ? 0 : -1;
//nestaci, nakolko nemam ZEND garbage collector, musim explicitne sam uzavriet vsetko potrebne:
//(A JE VLASTNE JEDNO, ci som creator, alebo nie - system si to osetri sam (pokial mu NATVRDO nedam 		shm_nattch = 0; ?!	))
	ret = UnmapViewOfFile(shm->addr);
	CloseHandle(shm->segment);
	UnmapViewOfFile(shm->descriptor);
	CloseHandle(shm->info);
	return ret;
}

/*
  SYSTEM CALL: shmctl();
  PROTOTYPE: int shmctl ( int shmqid, int cmd, struct shmid_ds *buf );
    RETURNS: 0 on success
             -1 on error: errno = EACCES (No read permission and cmd is IPC_STAT)
                                  EFAULT (Address pointed to by buf is invalid with IPC_SET and
                                          IPC_STAT commands)
                                  EIDRM  (Segment was removed during retrieval)
                                  EINVAL (shmqid invalid)
                                  EPERM  (IPC_SET or IPC_RMID command was issued, but
                                          calling process does not have write (alter)
                                          access to the segment)
      NOTES:
This particular call is modeled directly after the msgctl call for message queues. 
In light of this fact, it won't be discussed in too much detail. Valid command values are:

IPC_STAT
Retrieves the shmid_ds structure for a segment, and stores it in the address of the buf argument

IPC_SET
Sets the value of the ipc_perm member of the shmid_ds structure for a segment. Takes the values from the buf argument.

IPC_RMID
Marks a segment for removal.

The IPC_RMID command doesn't actually remove a segment from the kernel. Rather, it marks the segment for removal. 
The actual removal itself occurs when the last process currently attached to the segment has properly detached it. 
Of course, if no processes are currently attached to the segment, the removal seems immediate.

To properly detach a shared memory segment, a process calls the shmdt system call.
*/ 
TSRM_API int shmctl ( int shmqid, int cmd, struct shmid_ds *buf )	{	//int shmctl(int key, int cmd, struct shmid_ds *buf) {
	shm_pair *shm;
	if ((shm = shm_get(GET_IF_EXISTS_BY_KEY, shmqid, NULL)) == NULL)		return -1;

	if (!shm->segment) {
		return -1;
	}
	switch (cmd) {
		case IPC_STAT:
			memcpy(buf, shm->descriptor, sizeof(struct shmid_ds));
			return 0;

		case IPC_SET:
			shm->descriptor->shm_ctime		= time(NULL);
			shm->descriptor->shm_perm.uid	= buf->shm_perm.uid;
			shm->descriptor->shm_perm.gid	= buf->shm_perm.gid;
			shm->descriptor->shm_perm.mode	= buf->shm_perm.mode;
			return 0;

		case IPC_RMID:
			if (shm->descriptor->shm_nattch < 1) {
				shm->descriptor->shm_perm.key = -1;
			}
			return 0;

		default:
			return -1;
	}
}

#endif


/*****************************SHM CORE FUNCTIONS*************************/

/* {{{ proto int shmop_open (int key, string flags, int mode, int size)
   gets and attaches a shared memory segment */
int shmop_open (int key, char* flags, int mode, int size)	//PHP_FUNCTION(shmop_open)
{
	size_t flags_len =  strlen(flags);
	int shmatflg = 0, shmflg = 0, shmid = 0;
	struct shmid_ds shm;
	php_shmop  *shmop, i_shmop;
	memset(&i_shmop, 0, sizeof(php_shmop));	// !! new instance for Linux and OsX
	shmop = &i_shmop;
//	shmop->key = key;
//	shmop->size = size;
//	shmop->shmflg = mode;

//	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lsll", &key, &flags, &flags_len, &mode, &size) == FAILURE) {
//		return;
//	}
shm_dbg("\n FUNCTION shmop_open (int key, char* flags, int mode, int size) : %d, %s, %d, %d \n", key, flags, mode, size);	
	if (shmPOOLcnt >= MAX_SHM_IDs) {
		sprintf(errmsg, "SHM POLL full");	
		goto err;
	}
		
	if (flags_len != 1) {
		sprintf(errmsg, "Shared memory - %s is not a valid flag", flags);	//php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s is not a valid flag", flags);
		RETURN_FALSE;
	}
	switch (flags[0]) {
		case 'a':
			shmatflg |= SHM_RDONLY;
			break;
		case 'c':
			shmflg |= IPC_CREAT;
			break;
		case 'n':
			shmflg |= (IPC_CREAT | IPC_EXCL);
			break;	
		case 'w':
			/* noop 
				shm segment is being opened for read & write
				will fail if segment does not exist
			*/
			break;
		default:
		sprintf(errmsg, "Shared memory - invalid access mode");	//php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid access mode");
			goto err;
	}
	if (shmflg & IPC_CREAT && size < 1) {
		sprintf(errmsg, "Shared memory segment size must be greater than zero");	//php_error_docref(NULL TSRMLS_CC, E_WARNING, "Shared memory segment size must be greater than zero");
		goto err;
	}
shm_dbg("CALL shmget(key, size, shmflg) : %d, %d, %d \n", key, size, shmflg );		
		//WIN creates shmop in pool, Linux, OsX in their's own system space
		//it must be maintained pool - check shm_wrk_copy
	shmid = shmget(key, size, shmflg);
	if (shmop->shmid == -1) {
		sprintf(errmsg, "unable to attach or create shared memory segment");	//php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to attach or create shared memory segment");
		goto errKill;
	}
	if (! shm_wrk_copy) {	// WINDOWS
		shm_wrk_copy = &i_shmop;		
	}
	memcpy (shmop, shm_wrk_copy, sizeof(php_shmop));
	shmop->key = key;
	shmop->shmid = shmid;
	shmop->shmatflg = shmatflg;
	shmop->shmflg = shmflg;
	shmop->size = size;
//____________________________any OS ... save info here too__________________________
shmop = saveToPool(shmop);	if (! shmop) { RETURN_FALSE; }
//___________________________________________________________________________________
shm_dbg("CALL shmctl(shmop->shmid, IPC_STAT, &shm) : %d, %d, %p \n", shmop->shmid, IPC_STAT, &shm );		
	if (shmctl(shmop->shmid, IPC_STAT, &shm)) {
		sprintf(errmsg, "unable to get shared memory segment information");	//php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to get shared memory segment information");
		goto errKill;
	}	
shm_dbg("CALL shmop->addr = shmat(shmop->shmid, 0, shmop->shmatflg) : %d, %d, %p \n", shmop->shmid, 0, shmop->shmatflg );		
	shmop->addr = shmat(shmop->shmid, 0, shmop->shmatflg);
	if (shmop->addr == (char*) -1) {
		sprintf(errmsg, "unable to attach to shared memory segment");			//php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to attach to shared memory segment");
		goto errKill;
	}
	shmop->size = shm.shm_segsz;
		//shmop* already in stack
	//zend_list_insert(shmop, shm_type);
	return shmop->shmid;	//RETURN_LONG(rsid);
errKill:
removeFromPool(shmop->key);	//efree(shmop);
err:
shm_dbg("%s errno %d \n", errmsg, errno );		
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto string shmop_read (int shmid, int start, int count)
   reads from a shm segment */
unsigned char*  shmop_read (int shmid, int start, int count)	//PHP_FUNCTION(shmop_read)
{
	unsigned char *startaddr;
	php_shmop *shmop; 
shm_dbg("\n FUNCTION shmop_read (int shmid, int start, int count) : %d, %d, %d \n",shmid, start, count);		
	shmop = getFromPool(shmid);	if (! shmop) { RETURN_NULL; }
//	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll", &shmid, &start, &count) == FAILURE) {
//		return;
//	}
	if ( (start < 0) || (start > shmop->size) ) {
		sprintf(errmsg, "Shared memory - start is out of range");			//php_error_docref(NULL TSRMLS_CC, E_WARNING, "start is out of range");
shm_dbg("%s\n",errmsg);		
		RETURN_NULL;
	}	
	if ( ( (start + count) > shmop->size) || ( count < 0 ) ) {
		sprintf(errmsg, "Shared memory - count is out of range");			//php_error_docref(NULL TSRMLS_CC, E_WARNING, "count is out of range");
shm_dbg("%s\n",errmsg);		
		RETURN_NULL;
	}
	startaddr = (unsigned char*)shmop->addr + start;
shm_dbg("FUNCTION shmop_read 	return unsigned char *startaddr %p : %s \n", startaddr, firstXhxtx(startaddr, count));
	return startaddr;
}
/* }}} */

/* {{{ proto int shmop_size (int shmid)
   returns the shm size */
int  shmop_size (int shmid)	//PHP_FUNCTION(shmop_size)
{
	php_shmop *shmop; 
shm_dbg("\n FUNCTION shmop_size (int shmid) : %d \n",shmid);		
	shmop = getFromPool(shmid);	if (! shmop) { RETURN_FALSE; }
//	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &shmid) == FAILURE) {
//		return;
//	}
shm_dbg("FUNCTION shmop_size	return shmop->size : %d \n", shmop->size );		
	return shmop->size;	//RETURN_LONG(shmop->size);
}
/* }}} */

/* {{{ proto int shmop_write (int shmid, string data, int offset)
   writes to a shared memory segment */
int  shmop_write (int shmid, char * data, int offset, int data_len)	//PHP_FUNCTION(shmop_write)
{
	int writesize;
	php_shmop *shmop; 
shm_dbg("\n FUNCTION shmop_write (int shmid, char * data, int offset, int data_len) : %d %d %s %d %d \n", shmid, firstXhxtx(data, data_len), offset, data_len);		
	shmop = getFromPool(shmid);	if (! shmop) { RETURN_FALSE; }
//	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lsl", &shmid, &data, &data_len, &offset) == FAILURE) {
//		return;
//	}
shm_dbg("CALL shmdt(shmop->addr) : %p \n", shmop->addr );		
	if ((shmop->shmatflg & SHM_RDONLY) == SHM_RDONLY) {
		sprintf(errmsg, "trying to write to a read only segment");			//php_error_docref(NULL TSRMLS_CC, E_WARNING, "trying to write to a read only segment");
shm_dbg("%s\n",errmsg);		
		RETURN_FALSE;
	}

	if (offset < 0 || offset > shmop->size) {
		sprintf(errmsg, "Shared memory - offset out of range");			//php_error_docref(NULL TSRMLS_CC, E_WARNING, "offset out of range");
shm_dbg("%s\n",errmsg);		
		RETURN_FALSE;
	}

	writesize = (data_len < shmop->size - offset) ? data_len : shmop->size - offset;
	memcpy(shmop->addr + offset, data, writesize);
	
shm_dbg("FUNCTION shmop_write 	return int writesize %d\n", writesize);
	return writesize;	//RETURN_LONG(writesize);
}
/* }}} */

/* {{{ proto bool shmop_delete (int shmid)
   mark segment for deletion */
int shmop_delete (int shmid)	//PHP_FUNCTION(shmop_delete)
{
	php_shmop *shmop; 
shm_dbg("\n FUNCTION shmop_delete (int shmid) : %d \n",shmid);		
	shmop = getFromPool(shmid);	if (! shmop) { RETURN_FALSE; }
// A helpful hint, although when using shmop on windows (yes you can use shmop on windows in IIS or Apache when used as isapi/module) 
// you can delete a segment and later open a new segment with the SAME KEY in the same script no problem, 
// YOU CANNOT IN LINUX/UNIX - the segment will still exist - it's not immediately deleted - 
// but will be marked for deletion and you'll get errors when trying to create the new one - just a reminder.

// There is an important factor to keep in mind here, the shmop_delete function doesn't actually "delete" the memory segment per say, 
// it MARKS the segment for deletion. An shm segment can not be deleted while there are processes attached to it, 
// so a call to shm_delete will two things, first no other processes will be allowed to attach to the segment after this call, 
// and also, it will mark the segment for deletion, which means that when all current processes mapping the segment detach (shmop_close) 
// the system will automatically delete the segment. 

//	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &shmid) == FAILURE) {
//		return;
//	}
shm_dbg("CALL shmctl(shmop->shmid, IPC_RMID, NULL) : %d %d %s \n", shmop->shmid, IPC_RMID, "NULL" );		
	if (shmctl(shmop->shmid, IPC_RMID, NULL)) {
		sprintf(errmsg, "can't mark segment for deletion (are you the owner?)");			//php_error_docref(NULL TSRMLS_CC, E_WARNING, "can't mark segment for deletion (are you the owner?)");
shm_dbg("%s\n",errmsg);		
		RETURN_FALSE;
	}
//???? NONO, wait for close	!		removeFromPool(shmop->key);	// zend_list_delete(shmid);
shm_dbg("FUNCTION shmop_delete : OK \n");		
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void shmop_close (int shmid)
   closes a shared memory segment */
int  shmop_close (int shmid)	//PHP_FUNCTION(shmop_close)
{
	php_shmop *shmop; 
shm_dbg("\n FUNCTION shmop_close (int shmid) : %d \n",shmid);		
	shmop = getFromPool(shmid);	if (! shmop) { RETURN_FALSE; }
// shmop_close doesn't delete the memory segment, it just detaches from it. 
// If you have created the block and need to delete it you must call shmop_delete **BEFORE** calling shmop_close 
// (for reasons outlined in shmop_delete help page notes).

//	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &shmid) == FAILURE) {
//		return;
//	}
shm_dbg("CALL shmdt(shmop->addr) : %p \n", shmop->addr );		
	if (shmdt(shmop->addr)) {	//	detach memory segment		
		sprintf(errmsg, "can't detached segment (are you the owner?)");			//php_error_docref(NULL TSRMLS_CC, E_WARNING, "can't mark segment for deletion (are you the owner?)");
shm_dbg("%s\n",errmsg);		
removeFromPool(shmop->key);	//zend_list_delete(shmid);
		RETURN_FALSE;
	};			
removeFromPool(shmop->key);	//zend_list_delete(shmid);
shm_dbg("FUNCTION shmop_close : OK or NOT ... closed \n");		
	RETURN_TRUE;
}
/* }}} */



