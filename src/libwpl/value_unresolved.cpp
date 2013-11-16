/*

-------------------------------------------------------------

Copyright (c) MMIII Atle Solbakken
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

#include "value_unresolved.h"
#include "value_function_ptr.h"
#include "function.h"
#include "variable.h"

#include <iostream>

wpl_value *wpl_value_unresolved_identifier::resolve(wpl_namespace_session *nss) {
	if (wpl_variable *variable = nss->find_variable(value.c_str())) {
		return variable->get_value();
	}
	cerr << "While resolving name '" << value << "':\n";
	throw runtime_error("Could not resolve name");
}

int wpl_value_unresolved_identifier::do_fastop (
		wpl_expression_state *exp_state,
		wpl_value *final_result,
		const wpl_operator_struct *op
		)
{
	if ((op->flags & WPL_OP_F_HAS_BOTH) == WPL_OP_F_HAS_BOTH) {
		cerr << "While doing operator '" << op->name << endl;
		throw ("Too few operands for operator");
	}

	if (wpl_variable *variable = exp_state->find_variable(value.c_str())) {
		wpl_value *value = variable->get_value();

		/* Optimization
		   To prevent more than one lookup, replace ourselves
		   with the value from the variable.
		 */
		int my_pos = exp_state->pos()+1;
		shunting_yard_carrier ca(value, op);
		exp_state->replace(my_pos, ca);

		return value->do_fastop(exp_state, final_result, op);
	}

	return wpl_value::do_fastop(exp_state, final_result, op);
}

int wpl_value_unresolved_identifier::do_operator_recursive (
		wpl_expression_state *exp_state,
		wpl_value *final_result
		)
{
	if (wpl_variable *variable = exp_state->find_variable(value.c_str())) {
		wpl_value *value = variable->get_value();

		/* Optimization
		   To prevent more than one lookup, replace ourselves
		   with the value from the variable.
		 */
		int my_pos = exp_state->pos()+1;
		shunting_yard_carrier ca(value);
		exp_state->replace(my_pos, ca);

		return value->do_operator_recursive(exp_state, final_result);
	}

	if (wpl_function *function = exp_state->find_function(value.c_str())) {
		wpl_value_function_ptr function_ptr(function, NULL, exp_state->get_nss());
		return function_ptr.do_operator_recursive(exp_state, final_result);
	}

	return wpl_value::do_operator_recursive(exp_state, final_result);
}
