/*

-------------------------------------------------------------

Copyright (c) MMXIV Atle Solbakken
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

#pragma once

#include "exception.h"

#include <fstream>

class wpl_file {
	private:
	fstream file;
	ostringstream error;

	public:
	wpl_file() {}
	~wpl_file();

	void reset_error() {
		error.str("");
		error.clear();
	}

	string get_error() {
		return error.str();
	}

	bool check_error();
	bool open (const char *filename, ios_base::openmode mode);
	bool close();
};