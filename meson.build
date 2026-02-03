project('iutf', 'c',
	version: '0.6.2',
	license: 'Apache-2.0',
	default_options: ['warning_level=3', 'werror=false'])

#dependencies
glib_dep = dependency('glib-2.0', required: true)
idep = dependency('iutf', required: false)

# Compilation parameters
c_args = ['-D_GNU_SOURCE']

# SOURCE
isource = [
	'src/core/iutf-api.c',
	'src/core/iutf-ast.c',
	'src/core/iutf-parser.c',
	#'src/core/iutf-stream.h', # In develop
	'src/core/iutf-lexer.c',
	'src/core/iutf-validator.c',
	'src/core/iutf-import.c',
]

iheader = [
	'src/includes/colors.h',
	'src/includes/iutf-api.h',
	'src/includes/iutf-ast.h',
	'src/includes/iutf-import.h',
	'src/includes/iutf-lexer.h',
	#'src/includes/iutf-stream.h', # Experimental file, in develop.
	'src/includes/iutf-parser.h',
	'src/includes/iutf-validator.h',
]

# Library Creation (libiutf.so)
libiutfC = library('libiutf',
	isource,
	include_directories: include_directories('src/includes'),
	c_args: c_args,
	install: true,
)

# installing headers into /usr/include/iutf/ except iutf-import.h
install_headers(iheader, subdir: 'iutf')

# main iutf-parser file, binary or other executable file
iutf_parser = executable('iutf-parser',
	'src/core/main.c',
	link_with: libiutfC,
	include_directories: include_directories('src/includes'),
	c_args: c_args,
	install: true,
)

# Tests

test_Source = [
	'tests/test-lexer.c',
	'tests/test-parser.c',
	'tests/test-api.c',
	'tests/test-import.c',
]

foreach test_src : test_Source
	test_name = test_src.split('/')[-1].replace('.c', '')
	exe = executable(test_name,
		test_src,
		link_with: libiutfC,
		include_directories: include_directories('src/includes'),
		dependencies: glib_dep,
		c_args: c_args,
	)
	test(test_name, exe)
endforeach

pkg = import('pkgconfig')

pkg.generate(libiutfC,
  name: 'iutf',
  filebase: 'iutf',
  description: 'A IUTF shared library',
  subdirs: 'iutf',
  version: meson.project_version(),
  extra_cflags: c_args
)