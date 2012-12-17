<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN"
"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html>
  <head>
    <title>Enlightenment Screenshots</title> 
    <link href="index.css" rel="stylesheet" type="text/css"></link>
  </head>
  <body bgcolor=#ffffff alink=#88bbff link=#000000 vlink=#888888>
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

   $auth_file = "/var/www/www/ss/ip-" . $img;
   $auth_expire = 60 * 60; // You have one hour to remove your content

   if (time() - filemtime($auth_file) < $auth_expire)
     {
       $auth = md5($img . get_ip());

       $fh = fopen($auth_file, "r");
       $head = fgets($fh);
       fclose($fh);
     }
   else
     {
       $auth = false;
       $head = true;
     }

   if ($head == $auth || get_ip() == "140.211.167.168")
     {
       print "<span><a href=remove.php?image=" . $img . ">Remove content</a></span>\n";
     }
   print "<span><a href=ban.php?image=" . $img . ">Alert content</a></span>\n";

   print "<a href=http://www.enlightenment.org/ss/" . $img . ">";
   print "<img src=" . $img . " border=1>";
   print "</a>\n";
   ?>
  </body>
</html>