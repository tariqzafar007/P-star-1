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

#include "matcher.h"
#include "parser.h"
#include "scene.h"
#include "pragma.h"
#include "expression.h"
#include "blocks.h"
#include "types.h"
#include "template.h"
#include "exception.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cerrno>
#include <cstdio>

using namespace std;

/**
 * @brief 
 */
wpl_parser::wpl_parser (wpl_io &io, int num_parents) {
	this->io = &io;
	if (num_parents > 64) {
		throw runtime_error("Max file includes reached");
	}
	this->num_parents = num_parents;
}

/**
 * @brief 
 */
wpl_parser::~wpl_parser () {
}

void wpl_parser::parse_scene(wpl_namespace *parent_namespace) {
	wpl_matcher_position start(get_position());

	char buf[WPL_VARNAME_SIZE];
	get_word(buf);
	wpl_scene *scene = new wpl_scene(buf);
	parent_namespace->register_identifier(scene);

	ignore_string_match(WHITESPACE, 0);

	// Check for base scenes
	if (ignore_letter (':')) {
		do {
			get_word(buf);

			wpl_scene *base = parent_namespace->find_scene(buf);
			if (!base) {
				load_position(start);
				ostringstream msg;
				msg << "Could not find base scene '" << buf << "' " <<
					"while parsing base scene list";
				THROW_ELEMENT_EXCEPTION(msg.str());
			}

			scene->add_base(base);

			ignore_string_match(WHITESPACE, 0);
		} while (ignore_letter (','));
	}

	ignore_blockstart();

	scene->set_parent_namespace(parent_namespace);
	scene->load_position(get_position());
	scene->parse_value(scene);
	load_position(scene->get_position());
}

void wpl_parser::parse_template(wpl_namespace *parent_namespace) {
	char buf[WPL_VARNAME_SIZE];
	get_word(buf);
	wpl_template *my_template = new wpl_template(buf);
	parent_namespace->register_identifier(my_template);

	ignore_blockstart();

	my_template->load_position(get_position());
	my_template->parse_value(parent_namespace);
	load_position(my_template->get_position());
}

void wpl_parser::parse_include(wpl_namespace *parent_namespace) {
	wpl_parser *include = new wpl_parser(*io, num_parents+1);
	includes.emplace_back(include);

	ignore_string_match(WHITESPACE, 0);

	int len = search (NON_SEMICOLON, 0, false);
	if (len <= 0) {
		THROW_ELEMENT_EXCEPTION("Expected a filename after #INCLUDE");
	}

	string name(get_string_pointer(), len);

	if (name.at(0) != '/') {
		// Relative path requested, add root folder
		name.insert(0, string(io->get_env("PSTAR_ROOT")) + "/");
	}

	include->parse_file(parent_namespace, name.c_str());

	ignore_string(len);
	if (!ignore_letter(';')) {
		THROW_ELEMENT_EXCEPTION("Expected ';' after #INCLUDE");
	}
}

void wpl_parser::first_level(wpl_namespace *parent_namespace) {
	if (ignore_string ("#!")) {
		ignore_string_match (NEWLINE, ALL);
	}
	while (!at_end()) {
		ignore_string_match (WHITESPACE, 0);
		if (ignore_string(wpl_blockname_scene)) {
			parse_scene(parent_namespace);
		}
		else if (ignore_string(wpl_blockname_html_template)) {
			parse_template(parent_namespace);
		}
		else if (ignore_string("#INCLUDE")) {
			parse_include(parent_namespace);
		}
		else {
			break;
		}
	}
	if (!at_end()) {
		THROW_ELEMENT_EXCEPTION("Syntax error");
	}
}

void wpl_parser::parse_file (wpl_namespace *parent_namespace, const char *filename) {
	FILE *file = fopen (filename, "r");
	size_t filesize;

	if (!file) {
		ostringstream msg;
		msg << "Could not open file '" << filename << "': " << strerror(errno);
		throw wpl_parser_exception(msg.str());
	}
	try {
		if (	(fseek (file, 0L, SEEK_END) != 0) ||
			(!((filesize = ftell (file)) > 0)) ||
			(fseek (file, 0L, SEEK_SET) != 0)
		) {
			throw ferror(file);
		}

		unique_ptr<char[]> buf(new char[filesize+1]);

		int read_bytes = fread(buf.get(), 1, filesize, file);
		if ((read_bytes <= 0 ) ||
			(fclose (file) != 0)
		) {
			throw ferror(file);
		}

		buf.get()[read_bytes] = '\0';
		/*
		filesizes are funny on windows, can't do this check
		if (strlen (buf) != filesize) {
			delete buf;
			throw wpl_parser_exception("Source file contains NULL characters");
		}*/

		file_content.append(buf.get());
		set_text (file_content.c_str(), filesize, filename);
	}
	catch (int e) {
		fclose (file);
		ostringstream msg;
		msg << "Could not read file: " << strerror(e);
		throw wpl_parser_exception(msg.str());
	}
	
	first_level(parent_namespace);

	// Don't ever use this pointer again
	io = NULL;
}
