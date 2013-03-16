/*
 * SHM ... shared memory module
 *  Copyright (c) 2013 Jan Supuka <sbmintegral@sbmintegral.sk>
 *	MIT Licensed
*/

#include "shmop.h"

#if !defined USE_AS_CONSOLE && !defined TEST_JS_SHMOP
	#define TEST_JS_SHMOP
#endif 



static char sgetted[1024];

char * s_gets() {
	int len;
	fgets(sgetted, sizeof(sgetted), stdin); // read a line from stdin
	len = (int)strlen(sgetted);
	if (len > 0 && sgetted[len-1] == '\n')  // if there's a newline
		sgetted[len-1] = '\0';          	// truncate the string	
	return sgetted;
}

void strTOupr(char *p) { while (*p) { *(p++) = toupper(*p); } }
 
void g_printf(const char * format, ...) { 
  va_list args;
  va_start (args, format);
	vprintf (format, args);
	vprintf(" press ENTER to continue \r", args);
	s_gets();
	vprintf("                         \r", args);
  va_end (args);
}


#if defined TEST_JS_SHMOP
/*************************************************************************/
/*************************************************************************/
	#define USE_AS_CONSOLE


void testing()	{
	char *line = "\n\n____________________________________________________________\n";
	char SHM_read_ERROR[] = " SHM read ERROR ";
//SIMLE_TEST
	php_shmop *shmop;
	int shmid;
	int i, ss;
	unsigned char* content;
//MULTI_TEST
	int ret;
	int shid4,shid5,shid6,shid7;
//POOL_TEST
	int j, cnt = 0, shmK = 1234;
	php_shmop psh;

	
//SHM API:	
//	php_shmop * shmop_open (int key, char* flags, int mode, int size)	
//	unsigned char*  shmop_read (php_shmop *shmop, int start, int count)
//	int  shmop_close (php_shmop *shmop)
//	int  shmop_size (php_shmop *shmop)
//	int  shmop_write (php_shmop *shmop, char * data, int offset, int data_len)
//	int shmop_delete (php_shmop *shmop)
//


//SIMLE_TEST:
printf("%s 1.: simple test  %s", line, line);
	shm_setDbg( 1 );
	shmid = shmop_open (1233, "c", 0666, 1024);
	if (shmid) {
		shmop =  getFromPool(shmid);
		ss = shmop_size (shmop->shmid);
		content = shmop_read (shmop->shmid, 0, ss);
		if (content) {
			for(i = 0; i < ss; i++) {	printf("%02X",content[i]);	}
		} else {
			printf(" SHM read ERROR ");
		}
	}
	shmop_read (44444, -1, 5);
	shm_setDbg( 1 );
	shmop_read (shmid, -1, 5);
	shmop_delete (shmid);
	shmop_close (shmid);
	shm_setDbg( 0 );
g_printf("\nAloha SHM\n");

//MULTI_TEST:
printf("%s 2.: test multiple(4) SHM instances %s", line, line);
	printf("Test SHM\n");
	
	shm_setDbg( 0 );
//	shmop_init();
g_printf("\n\n    Open 4,5,6,7 \n%s", line);
														printSHMpool();
	shid4 = shmop_open (1234, "c", 0, 1024*1024);		printSHMpool();
	shid5 = shmop_open (1235, "c", 0, 1024*1024);		printSHMpool();
	shid6 = shmop_open (1236, "c", 0, 1024*1024);		printSHMpool();
	shid7 = shmop_open (1237, "c", 0, 1024*1024);		printSHMpool();
g_printf("\n\n    Size 4,5,6,7 \n%s", line);	
	printf("%d : %d\n",shid4,shmop_size (shid4));
	printf("%d : %d\n",shid5,shmop_size (shid5));
	printf("%d : %d\n",shid6,shmop_size (shid6));
	printf("%d : %d\n",shid7,shmop_size (shid7));
g_printf("\n\n    Write 4,5,6,7 \n%s", line);	
	ret = shmop_write (shid4, "POZOR na SHM 4", 0, 64);
	ret = shmop_write (shid5, "POZOR na SHM 5", 0, 64);
	ret = shmop_write (shid6, "POZOR na SHM 6", 0, 64);
	ret = shmop_write (shid7, "POZOR na SHM 7", 0, 64);
g_printf("\n\n    Read 4,5,6,7 \n%s", line);		
	content = shmop_read (shid4, 0, 64); printf("'%s'\n", content ? (char*)content : SHM_read_ERROR);
	content = shmop_read (shid5, 0, 64); printf("'%s'\n", content ? (char*)content : SHM_read_ERROR);
	content = shmop_read (shid6, 0, 64); printf("'%s'\n", content ? (char*)content : SHM_read_ERROR);
	content = shmop_read (shid7, 0, 64); printf("'%s'\n", content ? (char*)content : SHM_read_ERROR);
g_printf("\n\n    Close 4,5  Delete 6,7 \n%s", line);	
								printSHMpool();
	shmop_close (shid4);		printSHMpool();		//kill
	shmop_close (shid5);		printSHMpool();		//kill
	shmop_delete (shid6);		printSHMpool();				//only detach
	shmop_delete (shid7);		printSHMpool();				//only detach
g_printf("\n\n    Read closed 4,5  Read deleted 6,7 \n%s", line);	
	content = shmop_read (shid4, 0, 64); printf("'%s'\n", content ? (char*)content : SHM_read_ERROR);	//uz neexistuje
	content = shmop_read (shid5, 0, 64); printf("'%s'\n", content ? (char*)content : SHM_read_ERROR);	//uz neexistuje
	content = shmop_read (shid6, 0, 64); printf("'%s'\n", content ? (char*)content : SHM_read_ERROR);	//?
	content = shmop_read (shid7, 0, 64); printf("'%s'\n", content ? (char*)content : SHM_read_ERROR);	//?
g_printf("\n\n    ReOpen closed 4,5  Read deleted 6,7 \n%s", line);	
													printSHMpool();
	shid4 = shmop_open (1234, "c", 0, 2*1024);		printSHMpool();	//mal by otvorit nove
	shid5 = shmop_open (1235, "c", 0, 2*1024);		printSHMpool();	//mal by otvorit nove
	shid6 = shmop_open (1236, "c", 0, 2*1024);		printSHMpool();	// mal by najst existujuci (iba deleted(deattached), nie closed=destroyed) )
	shid7 = shmop_open (1237, "c", 0, 2*1024);		printSHMpool();	// mal by najst existujuci (iba deleted(deattached), nie closed=destroyed) )
g_printf("\n\n    Size again 4,5,6,7 \n%s", line);	
	printf("%d : %d\n",shid4,shmop_size (shid4));	//2k
	printf("%d : %d\n",shid5,shmop_size (shid5));	//2k
	printf("%d : %d\n",shid6,shmop_size (shid6));	//1M
	printf("%d : %d\n",shid7,shmop_size (shid7));	//1M
		//all detach & destroy
g_printf("\n\n    Close all 4,5,6,7 \n%s", line);	
								printSHMpool();
	shmop_close (shid4);		printSHMpool();	
	shmop_close (shid5);		printSHMpool();
	shmop_close (shid6);		printSHMpool();
	shmop_close (shid7);		printSHMpool();
	
//	shmop_deinit();
	shm_setDbg( 0 );

//POOL_TEST:
printf("%s 3.: test full pool SHM %s", line, line);
	g_printf("Test SHM pool\n");
	
	while(cnt < MAX_SHM_IDs + 2) {	
		shmop_open (shmK, "c", 0, 1024);		printSHMpool();
		cnt++; 
		shmK++;
	}
	for (j = cnt-1; j>=0; j--) {
		psh = shmop_list(j);
		shmop_delete (psh.shmid);
		shmop_close (psh.shmid);		printSHMpool();
	}
}
#endif	//#if defined TEST_JS_SHMOP


#if defined USE_AS_CONSOLE
/*************************************************************************/
/*************************************************************************/

void iconsole() {
	int looper = 1;
	int id, shm_id = -1, size = 1024*1024;
	int shm_size, shm_bytes_written, ln;
	char a, *txt;
	unsigned char *my_string;
	
	a='?'; 
	
	printf("\n\n==========================================================\n\
                 Interactive SHM console\n\
==========================================================\n");
	
	do {
		switch(a)	{
		case '?':
			printf("\n\
?	this help                         \n\
i	set key=(I)d                      \n\
n	create (N)ew SHM                  \n\
o	(O)pen SHM                        \n\
s	get (S)ize SHM                    \n\
s	(W)rite to SHM                    \n\
r	(R)ead from SHM                   \n\
d	mark for (D)eleting SHM           \n\
c	(C)lose SHM                       \n\
p	(P)rint SHM pool                  \n\
q	(Q)uit console testing SHM        \n\
			\n");
			break;
		case 'i':
			printf("Enter ID: "); txt = s_gets();
			sscanf(txt,"%d",&id);
			break;
		case 'n':
			printf("Enter ID: "); txt = s_gets();
			sscanf(txt,"%d",&id);
			printf("Enter size: "); txt = s_gets();
			sscanf(txt,"%d",&size);
			shm_id = shmop_open(id, "n", 0644, size);
			if (shm_id == -1) {
				printf( "%s", "Couldn't create NEW shared memory segment\n" );
			}
			else	{
				printf( "%s", "Created NEW shared memory segment\n" );
			}
			break;
		case 'o':
			printf("Enter ID: "); txt = s_gets();
			sscanf(txt,"%d",&id);
			printf("Enter size: "); txt = s_gets();
			sscanf(txt,"%d",&size);
			shm_id = shmop_open(id, "c", 0644, size);
			if (shm_id == -1) {
				printf( "%s", "Couldn't create shared memory segment\n" );
			}
			else	{
				printf( "%s", "Created shared memory segment\n" );
			}
			break;
		case 's':
			shm_id = shmop_open(id, "a", 0, 0);
			if (shm_id == -1) {
				printf( "%s", "Couldn't access shared memory segment\n" );
			}
			else	{
				// Get shared memory block's size
				shm_size = shmop_size(shm_id);
				printf( "%s%d%s", "SHM Block Size: " , shm_size , " has been created.\n" );
			}
			break;
		case 'w':
			shm_id = shmop_open(id, "w", 0, 0);
			if (shm_id == -1) {
				printf( "%s", "Couldn't access shared memory segment\n" );
			}
			else	{
				printf("Enter text: "); txt = s_gets();
				if ( ! strlen(txt))	{
					// Lets write a test string into shared memory
					ln = (int)strlen("my shared memory block")+1;
					shm_bytes_written = shmop_write(shm_id, "my shared memory block", 0, ln);
				}
				else	{
					ln = (int)strlen(txt)+1;
					shm_bytes_written = shmop_write(shm_id, txt, 0, ln);
				}
				if (shm_bytes_written != ln) {
					printf( "%s", "Couldn't write the entire length of data\n" );
				}
				else {
					printf( "%s", "Writen the entire length of data\n" );
				}
			}
			break;
		case 'r':
			shm_id = shmop_open(id, "a", 0, 0);
			if (shm_id == -1) {
				printf( "%s", "Couldn't access shared memory segment\n" );
			}
			else	{
				// Get shared memory block's size
				shm_size = shmop_size(shm_id);
				
				// Now lets read the string back
				my_string = shmop_read(shm_id, 0, shm_size);
				if (!my_string) {
					printf( "%s", "Couldn't read from shared memory block\n" );
				}
				printf( "%s%s", "The data inside shared memory was: ", my_string);
			}
			break;
		case 'd':
			shm_id = shmop_open(id, "a", 0, 0);
			if (shm_id == -1) {
				printf( "%s", "Couldn't access shared memory segment\n" );
			}
			else	{
				//Now lets delete the block and close the shared memory segment
				if (!shmop_delete(shm_id)) {
					printf( "%s", "Couldn't mark shared memory block for deletion.\n" );
				}
				else	{
					printf( "%s", "shared memory block deleted.\n" );
				}
			}
			break;
		case 'c':
			shm_id = shmop_open(id, "a", 0, 0);
			if (shm_id == -1) {
				printf( "%s", "Couldn't access shared memory segment\n" );
			}
			else	{
				shmop_close(shm_id);
				printf( "%s", "closed shared memory block.\n" );
				
				shm_id = -1;	//unset(shm_id);
			}
			break;
		case 'p':
			printSHMpool();
			break;
		case 'q':
			looper = 0;
			break;
		}
		if (looper) {
			printf("Waiting command for SHM (?inoswrdcpq): ");
			txt = s_gets();
			a = txt[0];
		}
		
	} while (looper);

}
#endif	//#if defined USE_AS_CONSOLE
/*************************************************************************/


/*************************************************************************/
/*************************************************************************/
int main(int argc, char **argv)	{

#if defined TEST_JS_SHMOP
	char param[4];
	
	if (argc > 1) {
		sprintf(param, "%3.3s", argv[1]);	strTOupr(param);
		if ( ! strcmp(param,"CON") )	{	
			iconsole();		
		}	
		else {
			testing();	
		}
	} else {	// run test:
#if defined _WIN32
printf("Hallo SHM under Windows\n");
#else
printf("Hallo SHM\n");
#endif
printf("\
shm_cli.exe      ... no arguments ... run tests, then interactive console \n\
shm_cli.exe CON  ...	only interactive SHM console \n\
");
		testing();
		iconsole();	
	}
	
#else	//#if defined TEST_JS_SHMOP

	#if defined USE_AS_CONSOLE
		iconsole();
	#endif

#endif	//#if defined TEST_JS_SHMOP
	
	return 0;
}

