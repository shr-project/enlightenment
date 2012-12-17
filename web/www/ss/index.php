<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" 
"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<!--
  -- index.php
  --
  -- Modified By: Nicholas "mekius" Hughart
  --       Added: Pagination Support
  --        Date: 2012-08-28
  -->
<html>
  <head>
    <title>Enlightenment Screenshots</title>
    <link href="index.css" rel="stylesheet" type="text/css"></link>
  </head>
  <body bgcolor=#ffffff alink=#88bbff link=#000000 vlink=#888888>
  <?php
    define('IMAGES_PER_PAGE', 24);
    define('MAX_PAGE_LINKS', 9);

    $page = 0;
    if (isset($_GET['page']))
      $page = (int)($_GET['page']);

    $files = glob("e-*");
    array_multisort(
                     array_map( 'filemtime', $files ),
                     SORT_NUMERIC,
                     SORT_DESC,
                     $files
                   );

    $pages = (int)(count($files) / IMAGES_PER_PAGE);
    if (count($files) % IMAGES_PER_PAGE)
      $pages++;
  ?>

    <div id="PagesWrapper">
      <div id="Pages">
      <?php
        echo '<span class="PreviousArrows">';
        if ($page != 0)
          echo "<span><a href='?page=0'>&laquo;</a></span>";
        else
          echo "<span>&laquo;</span>";

        if ($page > 0)
          echo "<span><a href='?page=".($page-1)."' accesskey='p'>&lt;</a></span>";
        else
          echo "<span>&lt;</span>";
        echo '</span>';

        $i = 0;
        if (
            ($pages > MAX_PAGE_LINKS) && 
            ($page > (MAX_PAGE_LINKS/2))
           )
        {
          if ($page > ($pages - (MAX_PAGE_LINKS/2)))
            $i = $pages - MAX_PAGE_LINKS;
          else
            $i = $page - (int)(MAX_PAGE_LINKS/2);
        }

        for ($j = 0; ($i < $pages) && ($j < MAX_PAGE_LINKS); ++$i, ++$j)
        {
          if ($i == $page)
            echo "<a href=\"?page=$i\" class='highlight'>".sprintf("%02u", $i+1)."</a>\n";
          else
            echo "<a href=\"?page=$i\">".sprintf("%02u", $i+1)."</a>\n";
        }

        echo '<span class="NextArrows">';
        if ($page < ($pages-1))
          echo "<span><a href='?page=".($page+1)."' accesskey='n'>&gt;</a></span>";
        else
          echo "<span>&gt;</span>";

        if ($page != ($pages-1))
          echo "<span><a href='?page=".($pages-1)."' >&raquo;</a></span>";
        else
          echo "<span>&raquo;</span>";
        echo '</span>';
      ?>
      </div>
    </div>

    <div id="Images">
    <?php
      $skip = $page * IMAGES_PER_PAGE;
      foreach ($files as &$f) {
          if ($skip-- > 0)
            continue;

          if ($skip < -IMAGES_PER_PAGE)
            break;

          $thumb = "th-" . $f;
          if (!file_exists($thumb)) {
	    continue;
          }
          print "<a href=display.php?image=" . urlencode($f) . "><img src=" . $thumb . " border=1 hspace=10 vspace=10></a>\n";
      }
    ?>
    </div>
  </body>
</html>
