<?php
function get_ip()
{
  if (getenv("REMOTE_ADDR")) $ip = getenv("REMOTE_ADDR");
  else $ip = "UNKNOWN";
  return $ip;
}

$img = "";
if (isset($_GET['image'])) 
  $img = $_GET['image'];

$file = "/var/www/www/ss/" . $img;
$ignore_file = "/var/www/www/ss/ignore-" . $img;
$abuse_file = "/var/www/www/ss/abuse-" . $img;

if ($img[0] == "e" && file_exists($file) && !file_exists($ignore_file))
  {
    $already = false;
    $auth = md5($img . get_ip());

    $count = 0;
    $fh = fopen($abuse_file, "r");
    if ($fh)
      {
	while (!feof($fh))
	  {
	    $tmp = fgets($fh);
	    if ($auth == $tmp)
	      {
		// Don't let people vote multiple time somehow
		$already = true;
	      }
	    else
	      {
		$count++;
	      }
	  }
	fclose($fh);
      }

    if ($count > 10)
      {
	rename("/var/www/www/ss/ip-" . $img, 
	       "/var/www/www/ss/ban/ip-" . $img);
	rename("/var/www/www/ss/abuse-" . $img,
	       "/var/www/www/ss/ban/abuse-" . $img);
	rename("/var/www/www/ss/th-" . $img,
	       "/var/www/www/ss/ban/th-" . $img);
	rename("/var/www/www/ss/" . $img,
	       "/var/www/www/ss/ban/" . $img);
      }
    else
      {
	if (!already)
	  {
	    $fh = fopen($abuse_file, "a");
	    fwrite($fh, $auth . "\n");
	    fclose($fh);
	  }
      }
  }

header("Location: http://www.enlightenment.org/ss/");
die();
?>
