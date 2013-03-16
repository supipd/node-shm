

//require("./consoling.js");
var shm = require('../build/Release/shm');


function buff2hex(buff) {
	var r='';
	var i=0;
	var h;
	while(i<buff.length){
		h=buff[i++].toString(16);
		while(h.length<2){
			h='0'+h;
		}
		r+=h;
	}return r;
}





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



/*
var cns = new Rconsole(
	"Hallo>"	,{
		evalFnc : function(code) {
			return eval(code);
		}
	,	remote : { port: 8082 }
	} );
//activate it	
cns.asker();
*/

