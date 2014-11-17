#define DEBUG

#include <c_gen.h>

int declare_function(struct c_gen *to_declare, const char *type,
		     const char *name, size_t n_args, ...)
{
	va_list args;
	size_t arg_i;
	int ret;

	if ((ret = line_gen_write(type, &to_declare->base_gen))) {
		printlg(WARNING_LEVEL,
			"Could not write function return type.\n");
		return ret;
	}
	if ((ret = line_gen_write(" ", &to_declare->base_gen))) {
		printlg(WARNING_LEVEL,
			"Could not write space after function return type.\n");
		return ret;
	}
	if ((ret = line_gen_write(name, &to_declare->base_gen))) {
		printlg(WARNING_LEVEL,
			"Could not write function name.\n");
		return ret;
	}
	if ((ret = line_gen_write(PAREN_OPEN, &to_declare->base_gen))) {
		printlg(WARNING_LEVEL,
			"Could not start arguments.\n");
		return ret;
	}

	va_start(args, n_args);
	for (arg_i = 0; arg_i < n_args; arg_i++) {
		struct typed_var *arg = va_arg(args, struct typed_var *);
		if (arg_i > 0) {
			if ((ret = line_gen_write(NEW_ARG,
						  &to_declare->base_gen))) {
				printlg(WARNING_LEVEL,
					"Could not write delimiter before "
					"%u.\n",
					(unsigned) arg_i);
				return ret;
			}
		}
		if ((ret = line_gen_printf(&to_declare->base_gen, VAR_DEC_FMT,
					   arg->type, arg->name))) {
			printlg(WARNING_LEVEL,
				"Could not write argument %u.\n",
				(unsigned) arg_i);
			return ret;
		}
	}
	va_end(args);

	if ((ret = line_gen_write(PAREN_CLOSE, &to_declare->base_gen))) {
		printlg(WARNING_LEVEL,
			"Could not close arguments.\n");
		return ret;
	}

	return 0;
}
