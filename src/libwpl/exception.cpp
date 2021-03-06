/*

-------------------------------------------------------------

Copyright (c) MMXIII Atle Solbakken
atle@goliathdns.no

-------------------------------------------------------------

This file is part of P* (P-star).

P* is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

P* is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with P*.  If not, see <http://www.gnu.org/licenses/>.

-------------------------------------------------------------

*/

#include "exception.h"

void wpl_element_exception::output(wpl_io &io) const {
	char tmp[40+1];

	wpl_matcher line_counter(pos);

	int line = line_counter.get_linepos();
	int col = line_counter.get_colpos();

	line_counter.get_string_unsafe (tmp, 40);
	tmp[40] = '\0';

	ostringstream final_msg;

	final_msg << "In file '" << pos.filename <<
		"' line " << line <<
		" column " << col <<
		" near '" << tmp << "': " << text << endl;

	io.debug(final_msg.str().c_str());
}
