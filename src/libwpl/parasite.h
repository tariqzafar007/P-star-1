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

#include <memory>
#include <list>

class wpl_parasite_deleter {
	public:
	template<class T> void operator() (T *p) {
		p->suicide();
	}
};

template<class T> class wpl_parasite {
	protected:
	T *host;

	public:
	wpl_parasite (T *host) : host(host) {}
	virtual ~wpl_parasite() {}
	virtual void notify() = 0;
	virtual void suicide() = 0;
};

template<class T> class wpl_parasite_host {
	list<unique_ptr<wpl_parasite<T>,wpl_parasite_deleter>> parasites;

	public:
	wpl_parasite_host(): parasites() {}
	wpl_parasite_host(const wpl_parasite_host<T> &copy): parasites() {}

	void register_parasite(wpl_parasite<T> *parasite) {
		parasites.emplace_back(parasite);
	}

	void notify_parasites() {
		for (auto parasite : parasites) {
			parasite->notify();
		}
	}
};
