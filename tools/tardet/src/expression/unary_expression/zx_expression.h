/*
 * zx_expression.h
 *
 *  Created on: Aug 7, 2013
 *      Author: jucs
 */

#ifndef ZX_EXPRESSION_H_
#define ZX_EXPRESSION_H_

#include "../expression.h"

class zx_expression: public expression {
private:
	shared_ptr<expression> operand;

public:
	zx_expression(shared_ptr<expression> operand, size_t to_size);
	virtual ~zx_expression();

	char contains(struct rreil_variable *variable);
	bool substitute(struct rreil_variable *old,
				shared_ptr<expression> &new_);
	char evaluate(uint64_t *result);
	void print_inner();
};
#endif /* ZX_EXPRESSION_H_ */
