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


function main_table_begin()
{
echo "
<!-- Begin of main table -->
<table border=\"0\" cellpadding=\"3\" cellspacing=\"2\" width=\"100%\" class=\"main\">
  <tr valign=\"top\">";
}

function main_table_end()
{
echo "
<!-- End of main table -->
  </tr>
</table>";
}

function main_area_begin()
{
echo "
<!-- Begin of main area -->
<td width=\"100%\" class=\"mainarea\">";
}

function main_area_end()
{
echo "
<!-- End of main area -->
</td>";
}

?>
