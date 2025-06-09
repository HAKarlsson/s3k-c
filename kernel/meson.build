project(
	's3k',
	'c',
	version : '0.1',
	meson_version : '>= 1.4.1',
	default_options : [
		'buildtype=debug',
		'warning_level=2',
		'c_std=c99',
		'b_ndebug=if-release',
	],
)

if get_option('platdir') == ''
	platdir = meson.current_source_dir() / 'plat' / get_option('plat')
else
	platdir = get_option('platdir') / get_option('plat')
endif

sources = files(
	'src/head.S',
	'src/trap.S',
	'src/cap/ipc.c',
	'src/cap/monitor.c',
	'src/cap/ops.c',
	'src/cap/pmp.c',
	'src/cap/table.c',
	'src/cap/util.c',
	'src/exception.c',
	'src/interrupt.c',
	'src/kprint.c',
	'src/proc.c',
	'src/sched.c',
	'src/syscall.c',
	'src/trap.c',
	'src/rtc.c',
	'src/csr.c',
	'src/kinit.c',
	platdir / 'plat.c',
)

c_args = [
	'-ffreestanding',
	'-include', platdir / 'config.h',
	'-include', 's3k_conf.h',
]

link_args = [
	'-nostartfiles',
	'-nodefaultlibs',
	'-nostdlib',
	'-T', platdir / 'linker.ld',
]

barocq = find_program('barocq', required : true)

kernel_br = files(
	'brc/config.br',
	'brc/error.br',
	'brc/machine.br',
	'brc/types.br',
	'brc/capgen/cap.br',
	'brc/vreg.br',
	'brc/proc.br',
	'brc/sched.br',
	'brc/ctable.br',
	'brc/ptable.br',
	'brc/cap_ops.br',
	'brc/pmp.br',
	'brc/monitor.br',
	'brc/channels.br',
	'brc/ipc.br',
	'brc/syscall.br'
)

kernel_c = custom_target(
	'kernel_c',
	input : kernel_br,
	output : ['libkernel.c', 'libkernel.h'],
	command : [
		barocq,
		'@INPUT@',
		'-o', '@OUTPUT0@',
		'-gen-header',
	],
)

include_directories = include_directories(
	'include',
)

elf = executable('s3k.elf',
	sources : sources + kernel_c,
	include_directories : include_directories,
	c_args : c_args,
	link_args : link_args,
)

unity_proj = subproject('unity')

unity_lib = static_library(meson.project_name(),
  sources: unity_proj.get_variable('unity_src'),
  c_args: unity_proj.get_variable('unity_args'),
  include_directories: unity_proj.get_variable('unity_inc'),
  install: false,
  native: true,
)

unity_dep = declare_dependency(
  link_with: unity_lib,
  include_directories: unity_proj.get_variable('unity_inc')
)

gen_test_runner = unity_proj.get_variable('gen_test_runner')

test_runner = gen_test_runner.process(
	'test/test.c',
)

test_sources = files(
		'src/cap/ops.c',
		'src/cap/ipc.c',
		'src/cap/monitor.c',
		'src/cap/ops.c',
		'src/cap/pmp.c',
		'src/cap/table.c',
		'src/cap/util.c',
		'src/exception.c',
		'src/interrupt.c',
		'src/kprint.c',
		'src/proc.c',
		'src/sched.c',
		'src/syscall.c',
		'src/trap.c',
		'src/kinit.c'
) + kernel_c

s3k_test = executable('s3k_test',
	sources : test_sources + files(
		'test/test.c',
		'test/rtc.c',
		'test/csr.c',
		'test/trap.c',
		'test/kputc.c',
	) + test_runner,
	include_directories : include_directories,
	c_args : [
		'-include', platdir / 'config.h',
		'-include', 's3k_conf.h',
	],
	native : true,
	dependencies : [
	    unity_dep,
	],
)

test('s3k_test', s3k_test)
