<?php
function get_ip()
{
    if (getenv("REMOTE_ADDR")) $ip = getenv("REMOTE_ADDR");
    else $ip = "UNKNOWN";
    return $ip;
}

function extn($str) {
  $i = strrpos($str,".");
  if (!$i) { return ""; }
  $l = strlen($str) - $i;
  $ext = substr($str,$i+1,$l);
  return $ext;
}

function dothumb($f, $thumb, $new_w, $new_h) {
  $ext = extn($f);
  if (!strcmp("jpg", $ext))
    $src_img = imagecreatefromjpeg($f);
  if (!strcmp("png", $ext))
    $src_img = imagecreatefrompng($f);
  $old_x = imageSX($src_img);
  $old_y = imageSY($src_img);
  $ratio1 = $old_x / $new_w;
  $ratio2 = $old_y / $new_h;
  if ($ratio1 > $ratio2) {
    $thumb_w = $new_w;
    $thumb_h = $old_y / $ratio1;
  }
  else {
    $thumb_h = $new_h;
    $thumb_w = $old_x / $ratio2;
  }
  $dst_img = ImageCreateTrueColor($thumb_w, $thumb_h);
  imagecopyresampled($dst_img, $src_img, 0, 0, 0, 0,
		     $thumb_w, $thumb_h, $old_x, $old_y); 
  if (!strcmp("png", $ext))
    imagepng($dst_img, $thumb); 
  else
    imagejpeg($dst_img, $thumb);
  imagedestroy($dst_img);
  imagedestroy($src_img);
}

ob_start();
############ limit - 6 mb.
$data = file_get_contents('php://input', NULL, NULL, 0, 6 * 1024 * 1024);
############ magic jpeg signature
$jpeg_match = "\xff\xd8\xff\xe0";
$jpeg_magic = substr($data, 0, 4);
############ magic png signature
$png_match = "\x89\x50\x4e\x47";
$png_magic = substr($data, 0, 4);

############ base on signaure, add file extension
$ext = ".unknown";
if ($jpeg_match == $jpeg_magic) $ext = ".jpg";
else if ($png_match == $png_magic) $ext = ".png";
############ not a correct matching file - abort
else {
  header("HTTP/1.1 400 Bad Request");
  echo "Invalid File Format";
  ob_end_flush();
  die();
}

############ get a unique name
$dest = uniqid("e-", true) . $ext;
$temp =  "/var/www/www/ss/tmp/" . $dest;
$thumb = "/var/www/www/ss/tmp/th-" . $dest;
$temp_ip = "/var/www/www/ss/tmp/ip-" . $dest;
############ store the file
$fh = fopen($temp, 'wb');
fwrite($fh, $data);
fclose($fh);

$fh = fopen($temp_ip, 'w');
fwrite($fh, md5($dest . get_ip()));
fclose($fh);
############ prepare url to get file from
$loc = "http://www.enlightenment.org/ss/" . $dest;

## Generate thumb
dothumb($temp, $thumb, 320, 240);

if (!rename($thumb, "/var/www/www/ss/th-" . $dest))
{
  header("HTTP/1.1 400 Bad Request");
  echo "Invalid File Format";
  ob_end_flush();
  die(); 
}
rename($temp, "/var/www/www/ss/" . $dest);
rename($temp_ip, "/var/www/www/ss/ip-" . $dest));

############ respond!
header("HTTP/1.1 200 OK");
header("Content-Type: text/plain");
header("X-Enlightenment-Service: Pants On");
print $loc;
ob_end_flush();
?>
