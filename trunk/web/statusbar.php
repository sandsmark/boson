<?php

function statusbar($latestversion, $latestupdate)
{
echo "
<!-- Status bar -->
<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" bgcolor=\"#d0d0ff\" width=\"95%\">
  <tr>
    <td align=\"left\">
      Latest stable version: <b>$latestversion</b>
    </td>
    <td align=\"right\">
      This page last updated: <b>$latestupdate</b>
    </td>
  </tr>
</table>
<br>";
}

?>
