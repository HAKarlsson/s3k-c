project(
    's3k',
    'c',
    version: '0.1',
    meson_version: '>= 1.1',
    default_options: [
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
    platdir / 'plat.c',
    'src/cap/ipc.c',
    'src/cap/monitor.c',
    'src/cap/ops.c',
    'src/cap/pmp.c',
    'src/cap/table.c',
    'src/cap/util.c',
    'src/csr.c',
    'src/exception.c',
    'src/head.S',
    'src/interrupt.c',
    'src/kernel.c',
    'src/proc.c',
    'src/rtc.c',
    'src/sched.c',
    'src/syscall.c',
    'src/trap.S',
    'src/trap.c',
)

if get_option('debug') == true
    sources += files('src/kprint.c')
endif

include_directories = include_directories('include')

c_args = [
    '-ffreestanding',
    '-include',
    platdir / 'config.h',
    '-include',
    's3k_conf.h',
]

link_args = ['-nostartfiles', '-nostdlib', '-T', platdir / 'linker.ld']

elf = executable(
    's3k.elf',
    sources: sources,
    include_directories: include_directories,
    c_args: c_args,
    link_args: link_args,
)

objdump = find_program('riscv64-unknown-elf-objdump')
da = custom_target(
    'da',
    command: [objdump, '-d', elf.full_path()],
    capture: true,
    depends: [elf],
    output: ['s3k.da'],
)
