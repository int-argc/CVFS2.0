<!DOCTYPE HTML>
<html>
<body>
<?php

$comm = "sudo cat /var/log/samba-audit.log";
$status = exec($comm, $pout);

foreach($pout as $l){
	echo $l . '<br />';
}
echo '<a href="home.html"> HOME </a>';
?>
</body>
</html>
