<?php
/*
    server side code of server decommissioning page
    requires:
        php5-sqlite
*/

// name and path of decommissioning program
$PROGNAME = './decomm';
$ok = 'SUCCESS';

list($iqn, $mountpt) = explode('|', $_POST['node']);

// execute decommissioning program
$comm = $PROGNAME . ' ' . $iqn . ' ' . $mountpt;
$status = exec($comm, $pout);

// we can print blah blah here, but we can really just redirect to fail / success page
// or even just redirect to homepage
if ($status != $ok) {
    echo 'Decommissioning Failed <br />';
    foreach ($pout as $l) {
        echo $l . '<br />';
    }
} else {
    echo 'Decommissioning Successful <br />';
}

echo '<a href="home.html">HOME</a>'

?>
