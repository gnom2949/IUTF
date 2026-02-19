PHP_ARG_ENABLE(iutf, whether to enable iutf support,
[ --enable-iutf 	Enable iutf support])

if test "$PHP_IUTF" != "no"; then

	PHP_ADD_INCLUDE([../../includes])
	PHP_ADD_INCLUDE([../../core])

	PHP_NEW_EXTENSION(iutf,
		iutfCaller.c \
		../../core/iutf-api.c \
		../../core/iutf-ast.c \
		../../core/iutf-import.c \
		../../core/iutf-parser.c \
		../../core/iutf-lexer.c \
		../../core/iutf-validator.c,
		$ext_shared,, -Wall -Wextra -Wno-unused-parameter)

	# if you needed a something library for test, you can connect with this
	# PHP_ADD_LIBRARY(m, 1, IUTF_SHARED_LIBADD)
	# PHP_SUBST(IUTF_SHARED_LIBADD)

fi