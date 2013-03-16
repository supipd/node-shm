#include "shm.h";

#if defined DEBUG_SHM
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
#else
	#define shm_dbg
#endif

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
			printf(" %i:%d, ", i, shmop.shmid);
		}
	}
	printf("\n");
}

