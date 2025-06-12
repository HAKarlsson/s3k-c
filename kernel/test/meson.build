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


test_runner = gen_test_runner.process('main.c')

test_sources = files(
		'../src/cap/ops.c',
		'../src/cap/ipc.c',
		'../src/cap/monitor.c',
		'../src/cap/ops.c',
		'../src/cap/pmp.c',
		'../src/cap/table.c',
		'../src/cap/util.c',
		'../src/exception.c',
		'../src/interrupt.c',
		'../src/kprint.c',
		'../src/proc.c',
		'../src/sched.c',
		'../src/syscall.c',
		'../src/kstate.c',
		'main.c',
		'utils.c',
) + test_runner + kernel_c

s3k_test = executable('s3k_test',
	sources : test_sources,
	include_directories : include_directories,
	c_args : [
		'-include', 'config.h',
		'-DNDEBUG',
	],
	native : true,
	dependencies : [
	    unity_dep,
	],
)

test('s3k_test', s3k_test)
