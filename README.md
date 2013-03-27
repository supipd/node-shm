nodejs-shm  (WORK IN PROGRESS)
==========

Node.js native addon for using shared memory compatible with PHP shmop

Description
-----------

Working on bigger project with several autonomous long-running processes I need communication between them. 
This processes ( workers, drivers ) need share relatively huge amount of data ( 10k - 100k bytes ) 
and I feel, that communication through TCP sockets is not so fast, as I want. 

I have PHP workers with shared memory used with shmop_XXXX commands as "data sources", and another workers, 
based on node.js, which I need connect with them - that's why I started this work.  

Working with shared memory need some discipline, my solution of potential concurrency and race condition problem 
is simple (not included in sources, only for imagination) :

My shm system is based on one big (10 MB) allocated shared memory block, divided to sections, where can write 
only exactly one worker. Information about sections (addressOffset, sectionSize, owner) are readed from database
and saved in first - header section.
SHM creates one process - centralManager, which writes header section.
Workers connects to SHM with well-known KEY, reads header section, finds info, where they have memory space
(offset and size) and where are spaces of another workers, whose data they maybe need.

And communication between processes can start.


TODO: simple SHM manager. 


The module works on Windows and Linux, OsX not tested (but has the same native SHM call as Linux, maybe works). 

#####Because a newbie in nodejs, this native addon is maybe not very pretty ... any help invited


Instalation
--------------

You can install with `npm`:

``` bash
$ npm install -g nodejs-shm
```

How to Use
----------

API
---
``` 
    C
//	int shmid  .... int shmop_open (int key, char* flags, int mode, int size)
//	int  shmop_size (int shmid)
//	int  shmop_write (int shmid, char * data, int offset, int data_len)
//	unsigned char*  shmop_read (int shmid, int start, int length)
//	int shmop_delete (int shmid)
//	int  shmop_close (int shmid)

    Javascript
        module 	shm
        function openSHM ( key [Number], flags [String], mode [Number], size [Number]) ... returns shmid [Number]
        function sizeSHM ( shmid [Number] ) ... returns allocated size [Number]
        function writeSHM ( shmid [Number], data [Buffer], offset [Number], data_len [Number] ) ... returns written bytes [Number]
        function readSHM ( shmid [Number], start [Number], length [Number] )  ... returns Buffer
        function deleteSHM ( shmid [Number] ) ... return 0 or 1 [Number]
        function closeSHM ( shmid [Number] ) ... return 0 or 1 [Number]
```

EXAMPLE (test/test.js)
----------------------

``` js
var shm = require('./build/Release/shm');

console.log("Object:\n", shm); 

var shmkey = 1236;

var shmid  = shm.openSHM(shmkey,'c',0,1024);		
console.log("opened shmkey:",shmkey, " shmid:",shmid);

var bubu = new Buffer("1234567890");
console.log("writing '1234567890', written:",shm.writeSHM(shmid,bubu,2,bubu.length));		//10

xxxx = shm.readSHM(shmid,2,10);
console.log("readed(HEXA):", buff2hex(xxxx));

console.log("waiting 10 secs to detach and close");
var timerD = setTimeout( function() {
	console.log("detatch retval:", shm.deleteSHM(shmid) );
	console.log("close retval:", shm.closeSHM(shmid) );
}, 10000);
```

License
-------

Copyright (c) 2013 Jan Supuka sbmintegral.sk

The MIT License

