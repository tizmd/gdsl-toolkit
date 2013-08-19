/*
 * bbgraph_node.cpp
 *
 *  Created on: Aug 9, 2013
 *      Author: jucs
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory>
#include "bbgraph_node.h"
#include "../expression/expression.h"

bool bbgraph_node::has_subgraph() {
	for(size_t i = 0; i < children.size(); ++i)
		if(children[i].dst.lock()->id->get_address_machine() == id->get_address_machine())
			return true;
	return false;
}

//void bbgraph_node::print_dot_label() {
//	printf("\t\"%s\"", id->to_string().c_str());
//	if(id->get_inner())
//		printf(" [label=%zu];\n", id->get_inner());
//	else
//		printf(";\n");
//}

void bbgraph_node::print_dot_subgraph() {
	if(is_marked())
		return;
	mark();
	printf("\t\t\"%s\" [label=%zu];\n", id->to_string().c_str(), id->get_inner());
	for(size_t i = 0; i < children.size(); ++i) {
		if(children[i].dst.lock()->id->get_address_machine() == id->get_address_machine()) {
			printf("\t\t\"%s\" -> \"%s\" [label=\"", id->to_string().c_str(), children[i].dst.lock()->id->to_string().c_str());
			children[i].condition->print();
			printf("\"];\n");
			children[i].dst.lock()->print_dot_subgraph();
		}
	}
}

void bbgraph_node::add_child(shared_ptr<bbgraph_node> child, shared_ptr<expression> condition) {
	struct bbgraph_branch branch = { child, condition };

	children.push_back(branch);
}

void bbgraph_node::add_parent(shared_ptr<bbgraph_node> parent, shared_ptr<expression> condition) {
	struct bbgraph_pref pref = { parent, condition };

	parents.push_back(pref);
}

void bbgraph_node::add_expression(shared_ptr<expression> expression) {
	if(uexp == NULL)
		uexp = shared_ptr<union_expression>(new union_expression(expression->get_size()));
	uexp->add(expression);
}

void bbgraph_node::unmark_all() {
	if(marked) {
		marked = false;
		for (size_t i = 0; i < children.size(); ++i)
			children[i].dst.lock()->unmark_all();
	}
}

void bbgraph_node::print_dot() {
//	printf("\t\"%s\" [label=\"%p\"];\n", id->to_string().c_str(), (void*)id->get_address_machine());
	if(is_marked())
		return;
	if(has_subgraph()) {
		printf("\tsubgraph cluster%p {\n", (void*)id->get_address_machine());
		printf("\t\tlabel=\"%p\";\n", (void*)id->get_address_machine());
		print_dot_subgraph();
		printf("\t}\n");
	} else {
		printf("\t\"%s\";\n", id->to_string().c_str());
	}
	mark();
	for(size_t i = 0; i < children.size(); ++i) {
		if(children[i].dst.lock()->is_marked())
			continue;
//		if(children[i].dst->id->get_address_machine() != id->get_address_machine())
//			printf("\t\"%p\" -> %zu;\n", (void*)id->get_address_machine(), children[i]->id->get_inner());
//		else
//			printf("\t\"%p\" -> \"%p\";\n", (void*)id->get_address_machine(), (void*)children[i]->id->get_address_machine());
		printf("\t\"%s\" -> \"%s\";\n", id->to_string().c_str(), children[i].dst.lock()->id->to_string().c_str());
		children[i].dst.lock()->print_dot();
	}
}