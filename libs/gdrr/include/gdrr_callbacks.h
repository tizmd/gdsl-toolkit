/*
 * gdrr.h
 *
 *  Created on: Feb 22, 2013
 *      Author: jucs
 */

#ifndef GDRR_CALLBACKS_H_
#define GDRR_CALLBACKS_H_

#include <gdrr_types.h>
#include <gdsl-x86.h>

struct gdrr_sem_id_callbacks {
	gdrr_sem_id_t *(*virt_na)(void *closure, int_t con);
//	gdrr_sem_id_t *(*virt_eq)(void *closure);
//	gdrr_sem_id_t *(*virt_neq)(void *closure);
//	gdrr_sem_id_t *(*virt_les)(void *closure);
//	gdrr_sem_id_t *(*virt_leu)(void *closure);
//	gdrr_sem_id_t *(*virt_lts)(void *closure);
//	gdrr_sem_id_t *(*virt_ltu)(void *closure);
	gdrr_sem_id_t *(*virt_t)(void *closure, int_t this);
};

struct gdrr_sem_address_callbacks {
	gdrr_sem_address_t *(*sem_address)(void *closure, int_t size,
			gdrr_sem_linear_t *address);
};

struct gdrr_sem_var_callbacks {
	gdrr_sem_var_t *(*sem_var)(void *closure, gdrr_sem_id_t *id, int_t offset);
};

struct gdrr_sem_linear_callbacks {
	gdrr_sem_linear_t *(*sem_lin_var)(void *closure, gdrr_sem_var_t *this);
	gdrr_sem_linear_t *(*sem_lin_imm)(void *closure, int_t imm);
	gdrr_sem_linear_t *(*sem_lin_add)(void *closure, gdrr_sem_linear_t *opnd1,
			gdrr_sem_linear_t *opnd2);
	gdrr_sem_linear_t *(*sem_lin_sub)(void *closure, gdrr_sem_linear_t *opnd1,
			gdrr_sem_linear_t *opnd2);
	gdrr_sem_linear_t *(*sem_lin_scale)(void *closure, int_t imm,
			gdrr_sem_linear_t *opnd);
};

struct gdrr_sem_sexpr_callbacks {
	gdrr_sem_sexpr_t *(*sem_sexpr_lin)(void *closure, gdrr_sem_linear_t *this);
	gdrr_sem_sexpr_t *(*sem_sexpr_cmp)(void *closure, gdrr_sem_op_cmp_t *this);
};

struct gdrr_sem_op_cmp_callbacks {
	gdrr_sem_op_t *(*sem_cmpeq)(void *closure, int_t size,
			gdrr_sem_linear_t *opnd1, gdrr_sem_linear_t *opnd2);
	gdrr_sem_op_t *(*sem_cmpneq)(void *closure, int_t size,
			gdrr_sem_linear_t *opnd1, gdrr_sem_linear_t *opnd2);
	gdrr_sem_op_t *(*sem_cmples)(void *closure, int_t size,
			gdrr_sem_linear_t *opnd1, gdrr_sem_linear_t *opnd2);
	gdrr_sem_op_t *(*sem_cmpleu)(void *closure, int_t size,
			gdrr_sem_linear_t *opnd1, gdrr_sem_linear_t *opnd2);
	gdrr_sem_op_t *(*sem_cmplts)(void *closure, int_t size,
			gdrr_sem_linear_t *opnd1, gdrr_sem_linear_t *opnd2);
	gdrr_sem_op_t *(*sem_cmpltu)(void *closure, int_t size,
			gdrr_sem_linear_t *opnd1, gdrr_sem_linear_t *opnd2);
};

struct gdrr_sem_op_callbacks {
	gdrr_sem_op_t *(*sem_lin)(void *closure, int_t size,
			gdrr_sem_linear_t *opnd1);
	gdrr_sem_op_t *(*sem_mul)(void *closure, int_t size,
			gdrr_sem_linear_t *opnd1, gdrr_sem_linear_t *opnd2);
	gdrr_sem_op_t *(*sem_div)(void *closure, int_t size,
			gdrr_sem_linear_t *opnd1, gdrr_sem_linear_t *opnd2);
	gdrr_sem_op_t *(*sem_divs)(void *closure, int_t size,
			gdrr_sem_linear_t *opnd1, gdrr_sem_linear_t *opnd2);
	gdrr_sem_op_t *(*sem_mod)(void *closure, int_t size,
			gdrr_sem_linear_t *opnd1, gdrr_sem_linear_t *opnd2);
	gdrr_sem_op_t *(*sem_shl)(void *closure, int_t size,
			gdrr_sem_linear_t *opnd1, gdrr_sem_linear_t *opnd2);
	gdrr_sem_op_t *(*sem_shr)(void *closure, int_t size,
			gdrr_sem_linear_t *opnd1, gdrr_sem_linear_t *opnd2);
	gdrr_sem_op_t *(*sem_shrs)(void *closure, int_t size,
			gdrr_sem_linear_t *opnd1, gdrr_sem_linear_t *opnd2);
	gdrr_sem_op_t *(*sem_and)(void *closure, int_t size,
			gdrr_sem_linear_t *opnd1, gdrr_sem_linear_t *opnd2);
	gdrr_sem_op_t *(*sem_or)(void *closure, int_t size, gdrr_sem_linear_t *opnd1,
			gdrr_sem_linear_t *opnd2);
	gdrr_sem_op_t *(*sem_xor)(void *closure, int_t size,
			gdrr_sem_linear_t *opnd1, gdrr_sem_linear_t *opnd2);
	gdrr_sem_op_t *(*sem_sx)(void *closure, int_t size, int_t fromsize,
			gdrr_sem_linear_t *opnd1);
	gdrr_sem_op_t *(*sem_zx)(void *closure, int_t size, int_t fromsize,
			gdrr_sem_linear_t *opnd1);
	gdrr_sem_op_t *(*sem_cmp)(void *closure, gdrr_sem_op_cmp_t *this);
	gdrr_sem_op_t *(*sem_arb)(void *closure, int_t size);
};

struct gdrr_sem_stmt_callbacks {
	gdrr_sem_stmt_t *(*sem_assign)(void *closure, gdrr_sem_var_t *lhs,
			gdrr_sem_op_t *rhs);
	gdrr_sem_stmt_t *(*sem_load)(void *closure, gdrr_sem_var_t *lhs, int_t size,
			gdrr_sem_address_t *address);
	gdrr_sem_stmt_t *(*sem_store)(void *closure, gdrr_sem_address_t *address,
			gdrr_sem_op_t *rhs);
	gdrr_sem_stmt_t *(*sem_ite)(void *closure, gdrr_sem_sexpr_t *cond,
			gdrr_sem_stmts_t *then_branch, gdrr_sem_stmts_t *else_branch);
	gdrr_sem_stmt_t *(*sem_while)(void *closure, gdrr_sem_sexpr_t *cond,
			gdrr_sem_stmts_t *body);
	gdrr_sem_stmt_t *(*sem_cbranch)(void *closure, gdrr_sem_sexpr_t *cond,
			gdrr_sem_address_t *target_true, gdrr_sem_address_t *target_false);
	gdrr_sem_stmt_t *(*sem_branch)(void *closure,
			gdrr_branch_hint_t *branch_hint, gdrr_sem_address_t *target);
};

struct gdrr_branch_hint_callbacks {
	gdrr_branch_hint_t *(*branch_hint)(void *closure, int_t con);
//	gdrr_sem_branch_hint_t *(*hint_jump)(void *closure);
//	gdrr_sem_branch_hint_t *(*hint_call)(void *closure);
//	gdrr_sem_branch_hint_t *(*hint_ret)(void *closure);
};

struct gdrr_sem_stmts_callbacks {
	gdrr_sem_stmts_t *(*sem_cons)(void *closure, gdrr_sem_stmt_t *hd,
			gdrr_sem_stmts_t *tl);
	gdrr_sem_stmts_t *(*sem_nil)(void *closure);
};

struct gdrr_sem_stmts_list_callbacks {
	gdrr_sem_stmts_t *(*list_next)(void *closure, gdrr_sem_stmt_t *next,
			gdrr_sem_stmts_t *list);
	gdrr_sem_stmts_t *(*list_init)(void *closure);
};

#endif /* GDRR_CALLBACKS_H_ */