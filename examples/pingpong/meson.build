project(
	'hello',
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

s3k = subproject('s3k',
	default_options : [
		's3k_conf=' + meson.current_source_dir() / 's3k_conf.h',
	],
)

app0 = subproject('app0')
app1 = subproject('app1')

s3k_elf = s3k.get_variable('elf')
app0_elf = app0.get_variable('elf')
app1_elf = app1.get_variable('elf')

qemu_riscv64 = find_program('qemu-system-riscv64')
run_target('qemu',
  command : [
    qemu_riscv64,
    '-machine', 'virt',
    '-bios', 'none',
    '-kernel', s3k_elf.full_path(),
    '-nographic',
    '-smp', '1',
    '-m', '128M',
    '-device', 'loader,file=' + app0_elf.full_path(),
    '-device', 'loader,file=' + app1_elf.full_path(),
  ],
  depends : [
    s3k_elf,
    app0_elf,
    app1_elf
  ],
)
