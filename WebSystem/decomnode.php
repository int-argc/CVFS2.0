<!DOCTYPE html>
<html>
    <head>
        <meta charset="utf-8">
        <title>Server Decommissioning</title>
        <style>
            .details {
                margin-left: 50px;
            }
        </style>
    </head>
    <body>
        <h1>Select from available nodes</h1>
        <form action="decom_serv.php" method="post">
        <?php
        $config = 'cvfsweb.conf';
        $cvfs_dir = file_get_contents($config);
        $DBNAME = trim($cvfs_dir) . '/Database/cvfs_db';
	echo "DBNAME = " . $DBNAME;
        $db = new SQLite3($DBNAME);
        $res = $db->query('SELECT * from Target;');
        while ($row = $res->fetchArray()) {
            echo '<input type="radio" name="node" value="' . $row['iqn'] . '|' . $row['mountpt'] . '|' . $row['assocvol'] . '" />' . $row['iqn'] . '<br />';
            echo '<div class="details">' . 'IP address: ' . $row['ipadd'] . '<br />'
            . 'Volume: ' . $row['assocvol'] . '<br />'
            . 'Mount point: ' . $row['mountpt'] . '<br />'
            . 'Available Space: ' . $row['avspace'] . '<br />';
            echo '</div>';
        }
        ?>

        <input type="submit" value="Decommission">
        </form>
    </body>
</html>
