#include "c_gen_tests.h"

#include <logger.h>

static int hello_world_tester(struct c_gen *out)
{
	include(out, STDIO_H_PATH);
	finish_line(&out->base_gen);
	declare_function(out, INT_TP, MAIN_FUNC_NAME, 0);

	finish_line(&out->base_gen);
	open_block(out);

	line_gen_write("printf(\"Hello World!\\n\")", &out->base_gen);
	end_statement(out);
	line_gen_write("return 0", &out->base_gen);
	end_statement(out);

	close_block(out);

	return 1;
}

static struct c_gen_tv hello_world = {
	.expected_file = "hello_world.c",
	.tester = hello_world_tester
};

static int deep_block_tester(struct c_gen *out)
{
	size_t indent_i;

	include(out, STDIO_H_PATH);
	finish_line(&out->base_gen);
	declare_function(out, INT_TP, MAIN_FUNC_NAME, 0);

	finish_line(&out->base_gen);
	open_block(out);

	for (indent_i = 1; indent_i < MAX_C_INDENTS; indent_i++) {
		if (open_block(out)) {
			printlg(ERROR_LEVEL,
				"Premature indentation failure at %u.\n",
				(unsigned) indent_i);
			return 0;
		}
	}

	if (!indent(&out->base_gen)) {
		printlg(ERROR_LEVEL, "Uncaught excess indentation.\n");
		return 0;
	}

	line_gen_write("printf(\"Hello World!\\n\")", &out->base_gen);
	end_statement(out);
	line_gen_write("return 0", &out->base_gen);
	end_statement(out);

	for (indent_i = MAX_C_INDENTS; indent_i > 1; indent_i--) {
		if (close_block(out)) {
			printlg(ERROR_LEVEL,
				"Premature unindentation failure at %u.\n",
				(unsigned) indent_i);
			return 0;
		}
	}

	close_block(out);

	if (!unindent(&out->base_gen)) {
		printlg(ERROR_LEVEL, "Uncaught excess unindentation.\n");
		return 0;
	}
	return 1;
}

static struct c_gen_tv deep_block = {
	.expected_file = "deep_block.c",
	.tester = deep_block_tester
};

static int struct_use_tester(struct c_gen *out)
{
	include(out, STDIO_H_PATH);
	finish_line(&out->base_gen);

	/* struct declaration */
	line_gen_printf(&out->base_gen, STRUCT_FMT, "test_struct");
	open_block(out);
	line_gen_printf(&out->base_gen, VAR_DEC_FMT, CHAR_TP, "test_member");
	end_statement(out);
	_close_block(out);
	end_statement(out);
	finish_line(&out->base_gen);

	/* struct instance definition */
	line_gen_printf(&out->base_gen, VAR_DEF_FMT,
			STATIC_KW " " STRUCT_KW " " "test_struct",
			"test_value");
	open_block(out);
	line_gen_printf(&out->base_gen, FIELD_ASSIGN_FMT "'h'" ",",
			"test_member");
	_close_block(out);
	end_statement(out);
	finish_line(&out->base_gen);

	declare_function(out, INT_TP, MAIN_FUNC_NAME, 0);

	finish_line(&out->base_gen);
	open_block(out);

	line_gen_write("printf(" "\"Member of struct is %c\\n\", "
		       "test_value.test_member" ")", &out->base_gen);
	end_statement(out);
	line_gen_write("printf(" "\"Member of pointer is %c\\n\", "
		       "(&test_value)->test_member" ")", &out->base_gen);
	end_statement(out);
	finish_line(&out->base_gen);
	line_gen_write("return 0", &out->base_gen);
	end_statement(out);

	close_block(out);

	return 1;
}

static struct c_gen_tv struct_use = {
	.expected_file = "struct_use.c",
	.tester = struct_use_tester
};

static int array_use_tester(struct c_gen *out)
{
	char char_i;

	include(out, STDIO_H_PATH);
	finish_line(&out->base_gen);

	/* declare the array of capital letters procedurally */
	line_gen_write(STATIC_KW " " CHAR_TP " ", &out->base_gen);
	line_gen_printf(&out->base_gen, ARR_INT_FMT " = ", "letters",
			(int) ('Z' - 'A') + 1);
	open_block(out);
	for (char_i = 'A'; char_i <= 'Z'; char_i++) {
		line_gen_printf(&out->base_gen, "'%c',", char_i);
		finish_line(&out->base_gen);
	}
	_close_block(out);
	end_statement(out);
	finish_line(&out->base_gen);

	declare_function(out, INT_TP, MAIN_FUNC_NAME, 0);

	finish_line(&out->base_gen);
	open_block(out);

	line_gen_write("printf(\"Letters %s!\\n\", letters)", &out->base_gen);
	end_statement(out);
	line_gen_write("return 0", &out->base_gen);
	end_statement(out);

	close_block(out);

	return 1;
}

static struct c_gen_tv array_use = {
	.expected_file = "array_use.c",
	.tester = array_use_tester
};

struct c_gen_tv *c_gen_tvs[N_C_GEN_TESTS] = {
	&hello_world, &deep_block, &struct_use, &array_use
};
