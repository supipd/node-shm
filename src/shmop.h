/*
 * SHM ... shared memory module
 *  Copyright (c) 2013 Jan Supuka <sbmintegral@sbmintegral.sk>
 *	MIT Licensed
*/

#ifndef __SHMOP_H_INCLUDED__
#define __SHMOP_H_INCLUDED__


										// NOTICE !   convention returning booleans true = 1, false = 0
								#define RETURN_TRUE		return 1
								#define RETURN_FALSE	return 0	
								#define RETURN_NULL		return NULL	

								#define PHP_SHMOP_GET_RES
								#define MAX_SHM_IDs	32
								
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>


/*****************************OS DEPENDANT HEADERS*************************/
#if !defined _WIN32
	#include <sys/ipc.h> 
	#include <sys/shm.h>
	#include <errno.h>
	
//	typedef void * PVOID;
//	typedef PVOID HANDLE;
	typedef void * HANDLE;
	
#else	//#if !defined _WIN32
	#include <windows.h>
	#include <process.h>

	#define IPC_PRIVATE	0
	#define IPC_CREAT	00001000
	#define IPC_EXCL	00002000
	#define IPC_NOWAIT	00004000

	#define IPC_RMID	0
	#define IPC_SET		1
	#define IPC_STAT	2
	#define IPC_INFO	3

	//#define SHM_R		2	
					//PAGE_READONLY
	//#define SHM_W		4	
					//PAGE_READWRITE

	#define	SHM_RDONLY	4
					//FILE_MAP_READ
	//#define	SHM_RND	2
					//FILE_MAP_WRITE
	//#define	SHM_REMAP	FILE_MAP_COPY ... = FILE_MAP_READ 	
	
	struct ipc_perm {
		int			key;
		unsigned short	uid;	/* owner euid and egid */
		unsigned short	gid;
		unsigned short	cuid;	/* creator euid and egid */
		unsigned short	cgid;
		unsigned short	mode;	/* access modes see mode flags below */
		unsigned short	seq;	/* slot usage sequence number */
	};
	struct shmid_ds {	//kernel structure
		struct	ipc_perm	shm_perm;	/* operation perms */
		int				shm_segsz;	/* size of segment (bytes) */
		time_t			shm_atime;	/* last attach time */
		time_t			shm_dtime;	/* last detach time */
		time_t			shm_ctime;	/* last change time */
		unsigned short	shm_cpid;	/* pid of creator */
		unsigned short	shm_lpid;	/* pid of last operator */
		short			shm_nattch;	/* no. of current attaches */
                                                /* the following are private */
//                unsigned short   shm_npages;     /* size of segment (pages) */
//                unsigned long   *shm_pages;      /* array of ptrs to frames -> SHMMAX */ 
//                struct vm_area_struct *attaches; /* descriptors for attaches */	
	};
	#define TSRM_API 
		
#endif	//#if !defined _WIN32

typedef struct {
	void	*addr;
	HANDLE	info;
	HANDLE	segment;
	struct	shmid_ds	*descriptor;
} shm_pair;

#define CREATE_NEW_IF_NOT_EXISTS	0
#define GET_IF_EXISTS_BY_KEY		1
#define GET_IF_EXISTS_BY_SHMID		2
#define GET_IF_EXISTS_BY_ADDR		3
#define DELETE_IF_EXISTS			32

typedef int key_t;

typedef struct {
	int in_use;
	int shmid;
	key_t key;
	int shmflg;
	int shmatflg;
	char *addr;
	int size;
	shm_pair	i_shm_pair;
} php_shmop;



/***************************** FUNCTIONS *************************/

char * shm_err() ;
void shm_setDbg( int on ) ;
void shm_dbg(const char * format, ...) ; 
char *firstXhxtx(void *data, int size) ;

php_shmop *shm_pool(int handle, int key, void *addr) ;
php_shmop * getFromPool(int shmid) ;
php_shmop * removeFromPool(int shmid) ;
php_shmop shmop_list(int index) ;
void printSHMpool() ;
php_shmop * saveToPool(php_shmop *shmop) ;

int shmop_open (int key, char* flags, int mode, int size);	//PHP_FUNCTION(shmop_open)
unsigned char*  shmop_read (int shmid, int start, int count);	//PHP_FUNCTION(shmop_read)
int  shmop_size (int shmid);	//PHP_FUNCTION(shmop_size)
int  shmop_write (int shmid, char * data, int offset, int data_len);	//PHP_FUNCTION(shmop_write)
int shmop_delete (int shmid);	//PHP_FUNCTION(shmop_delete)
int  shmop_close (int shmid);	//PHP_FUNCTION(shmop_close)


#endif	//#ifndef __SHMOP_H_INCLUDED__

