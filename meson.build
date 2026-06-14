project('iutf', 'c',
	version: '0.6.3',
	license: 'Apache-2.0',
	default_options: ['warning_level=3', 'werror=false'])

#dependencies
#### Int Memory Manager Fetching as subproject because imm has Makefile.
imm_src_dir = meson.project_source_root() / 'subprojects' / 'imm' 

imm_build = custom_target('imm_build',
    output: 'libimm.so',
    command: ['make', '-C', imm_src_dir],
    build_by_default: true,
    build_always_stale: false,
    env: {'PREFIX': imm_src_dir},
)

imm = declare_dependency(
    compile_args: [],
    include_directories: include_directories('subprojects/imm/src'),
    link_args: [
        '-L' + imm_src_dir,
        '-limm',
        '-Wl,-rpath,' + imm_src_dir,
    ],
)
imm = imm.as_system() 
#### ENDOF
glib_dep = dependency('glib-2.0', required: true)
#idep = dependency('iutf', required: false)

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
	'src/core/iutf-errtypes.c',
	'src/core/iutf-var.c',
]

iheader = [
	'src/includes/colors.h',
	'src/includes/iutf-api.h',
	'src/includes/iutf-ast.h',
	'src/includes/iutf-import.h',
	'src/includes/iutf-lexer.h',
	'src/includes/iutf-var.h',
	'src/includes/iutf.h',
	'src/includes/types.h',
	'src/includes/errtypes.h',
	#'src/includes/iutf-stream.h', # Experimental file, in develop.
	'src/includes/iutf-parser.h',
	'src/includes/iutf-validator.h',
]

# Library Creation (libiutf.so)
libiutfC = library('iutf',
	isource,
	include_directories: include_directories('src/includes'),
	dependencies: [glib_dep, imm],
	c_args: c_args,
	install: true,
)

# installing headers into /usr/include/iutf/ except iutf-import.h
install_headers(iheader, subdir: 'iutf')

# main iutf-parser file, binary or other executable file
iutf_parser = executable('iutf-parser',
	'src/core/main.c',
	link_with: libiutfC,
	dependencies: [glib_dep, imm],
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
        dependencies: [glib_dep, imm],
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
  extra_cflags: '-I${includedir}/iutf',
  libraries_private: '-L${prefix}/lib/iutf',
)
