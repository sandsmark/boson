<?php
/*
    This file is part of the Boson game
    Copyright (C) 2002-2003 The Boson Team (boson-devel@lists.sourceforge.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


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
