<?php
	session_start();	//musi byt, aby som mal k dispozicii (pre HTML ani GET/POST pakety neviditelne) stavove premenne

/*	http://www.webmasterworld.com/linux/3476321.htm
As I suggested before, "ipcs -m" will display the shared-memory segments currently in use.
Perhaps this will help you debug your application.
Still not sure what you mean by "used or left out".
The OS doesn't know how much of a shared-memory segment has been "used". Your app makes a call, say, 
asking for a shared-memory segment of 10,000 bytes. At some point you tell the OS you are done, 
and it takes the memory back. But the OS has no idea if you've stuffed 1 byte, 5000, or 10,000 into the memory.

Should you attempt to write outside of an allocated memory segment (e.g. in the above example, to byte 10,001) 
you will get a segmentation violation.
	You *are* giving the shared memory segments back eventually, right? ;)

Now, there are some limits placed on shared memory segment size, number of shared memory segments, etc. 
This varies from system to system. 
There are a number of kernel variables that you can view to determine the limits on your system. 
You can "cat" the variable numbers in /proc/sys/kernel. For example, to view the maximum size of a shared-memory segment:
	cat /proc/sys/kernel/shmmax
Here are the values on my Fedora Core 6 system:
	shmmax - max size of a shared-memory segment: 33554432
	shmall - total shared memory avail: 2097152
	shmseg - max # of shared memory segs per process: (doesn't exist in FC6)
	shmmni - max # of shared memory segs system-wide: 4096

Note my previous caveat about interpreting "available memory" statistics in Linux. They aren't really meaningful, 
as you need to add the memory being used for disk caching and tmpfs to any "available" feature.
*/
if (function_exists("xdebug_break"))	xdebug_break();
echo "VOLANIE<br/>";
foreach (array('shma','id','size','len','txt','offset','hexa') as $k) {
	echo "<b>$k</b> ".(isset($_GET[$k])?$_GET[$k]:'nezadane')." <br/>";
}
/*
echo "<b>shma</b> ".isset($_GET['shma'])?$_GET['shma']:'nezadane'." <br/>";
echo "<b>id</b> ".$_GET['id']." <br/>";
echo "<b>zise</b> ".$_GET['size']." <br/>";
echo "<b>len</b> ".$_GET['len']." <br/>";
echo "<b>txt</b> ".$_GET['txt']." <br/>";
*/
echo "<br/>";
?>
<pre>
/* poprerabal som to tak, aby sa dala step-by-step skusat funkcionalita SHARED MEMORY
	teraz je mozne spustit v browser-i sekvenciu volani stranok napr.:
( PRE SKUSANIE SCADA_CTRL_SHM priklad volania (zavesenia sa na SHM):
	http://localhost/LIBRARY/TESTY_A_POZNAMKY/sharedmem_basic.php?XDEBUG_SESSION_START=test&shma=r&id=0xCADA
	http://localhost/LIBRARY/TESTY_A_POZNAMKY/sharedmem_basic.php?XDEBUG_SESSION_START=test&shma=r&id=51930
)
				id DEFAULT 4083(0xff3)
shma=o	http://localhost/..../sharedmem_basic.php?shma=<b>o</b>&size=1024&id=1234
			Create <b>?size=xxxxx</b> byte shared memory block with system id of <b>?id=xxxx</b>
shma=s	http://localhost/..../sharedmem_basic.php?shma=<b>s</b>
			Get shared memory block's size
shma=w	http://localhost/..../sharedmem_basic.php?shma=<b>w</b>
			Lets write a test string into shared memory <b>?offset=100</b> (default:0) <b>&txt=aaaaaaa</b> (default:"my shared memory block")
shma=r	http://localhost/..../sharedmem_basic.php?shma=<b>r</b>
			Now lets read the string back <b>?offset=100</b> (default:0) <b>&len=xxx</b> (default:shm_size) <b>&hexa=16</b> (default:0)
shma=d	http://localhost/..../sharedmem_basic.php?shma=<b>d</b>
			Mark shared memory block for deletion (needed by creator before close SHM)
shma=c	http://localhost/..../sharedmem_basic.php?shma=<b>c</b>
			Closed shared memory block
	a takto krok za krokom overit fungovanie
		OVERENE, funguje to aj cross-browser, aj cross-klient
*/
</pre>
<?php
$pid = getmypid();
echo "PID: $pid \n<br/>";
if (isset($_GET['shma']))	{   

	$id = 0xff3;	if (isset($_GET['id']))	{	//ani neviem, ci treba riesit formu zadania "0xCADA"
		if (is_string($_GET['id']))	{
			if (strtolower(substr($_GET['id'],0,2)) == "0x") {
				$id = hexdec(substr($_GET['id'],2));
			} else {
				$id = intval($_GET['id']);
			}
		} else {
			$id = $_GET['id'];
		}
	}
	$size = 1024;	if (isset($_GET['size']))	$size = $_GET['size'];
	$offset = 0;	if (isset($_GET['offset']))	$offset = $_GET['offset'];

	switch($_GET['shma'])	{
		case 'o':
			// Create 100 byte shared memory block with system id of 0xff3
			$shm_id = @shmop_open($id, "c", 0644, $size);
			if (!$shm_id) {
				echo "Couldn't create shared memory segment\n";
			}
			else	{
				echo "Created shared memory segment\n";
			}
			break;
		case 's':
			$shm_id = @shmop_open($id, "a", 0, 0);
			if (!$shm_id) {
				echo "Couldn't access shared memory segment\n";
			}
			else	{
				// Get shared memory block's size
				$shm_size = shmop_size($shm_id);
				echo "SHM Block Size: " . $shm_size . " has been created.\n";
			}
			break;
		case 'w':
			$shm_id = @shmop_open($id, "w", 0, 0);
			if (!$shm_id) {
				echo "Couldn't access shared memory segment\n";
			}
			else	{
				if (! isset($_GET['txt']))	{
					// Lets write a test string into shared memory
					$shm_bytes_written = shmop_write($shm_id, "my shared memory block", 0);
					$ln = strlen("my shared memory block");
				}
				else	{
					$shm_bytes_written = shmop_write($shm_id, $_GET['txt'], $offset);
					$ln = strlen($_GET['txt']);
				}
				if ($shm_bytes_written != $ln) {
					echo "Couldn't write the entire length of data\n";
				}
				else {
					echo "Writen the entire length of data\n";
				}
			}
			break;
		case 'r':
//			$shm_id = @shmop_open("0xCADA", "a", 0, 0);
			$shm_id = @shmop_open($id, "a", 0, 0);
			if (!$shm_id) {
				echo "Couldn't access shared memory segment\n";
			}
			else	{
				// Get shared memory block's size
				$shm_size = shmop_size($shm_id);
				if (! isset($_GET['len']))	{
					$data_len = $shm_size;
				} else {
					$data_len = $_GET['len'];
				}
				// Now lets read the string back
				$my_string = shmop_read($shm_id, $offset, $data_len);
				if (!$my_string) {
					echo "Couldn't read from shared memory block\n";
				}
				echo 'The data inside shared memory was: <br/><textarea style="margin: 2px; width: 100%; height: 50%; "><![CDATA['."\n";
				if (isset($_GET['hexa'])) {
					 $lms = strlen($my_string); $sms = 0+$_GET['hexa']; 
					 if ($sms <= 0) {
						echo  bin2hex($my_string) . "\n";
					 } else {
						for ($i = 0; $i < $lms; $i+=$sms) {
							echo  bin2hex(substr($my_string,$i,$sms)) . "\n";
						}
						echo  bin2hex(substr($my_string,$i)) . "\n";
					 }
				} else {
					 echo  $my_string . "\n";
				}
				echo "]]></textarea>";
			}
			break;
		case 'd':
			$shm_id = @shmop_open($id, "a", 0, 0);
			if (!$shm_id) {
				echo "Couldn't access shared memory segment\n";
			}
			else	{
				//Now lets delete the block and close the shared memory segment
				if (!@shmop_delete($shm_id)) {
					echo "Couldn't mark shared memory block for deletion.";
				}
				else	{
					echo "shared memory block deleted.";
				}
			}
			break;
		case 'c':
			$shm_id = @shmop_open($id, "a", 0, 0);
			if (!$shm_id) {
				echo "Couldn't access shared memory segment\n";
			}
			else	{
				shmop_close($shm_id);
				echo "closed shared memory block.";
				
				unset($shm_id);
			}
			break;
	}
	if (isset($shm_id))	{
		@shmop_close($shm_id);
	}
}   

?>