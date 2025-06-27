project(
    's3k',
    'c',
    version: '0.1',
    meson_version: '>= 1.4.1',
    default_options: [
        'buildtype=debug',
        'warning_level=2',
        'c_std=c99',
        'b_ndebug=if-release',
        'werror=true',
    ],
)

if get_option('platdir') == ''
    platdir = meson.current_source_dir() / 'plat' / get_option('plat')
else
    platdir = get_option('platdir') / get_option('plat')
endif

sources = files(
    platdir / 'plat.c',
    'src/cap/ops.c',
    'src/cap/util.c',
    'src/csr.c',
    'src/head.S',
    'src/kprint.c',
    'src/kstate.c',
    'src/proc.c',
    'src/rtc.c',
    'src/sched.c',
    'src/machine.c',
    'src/trap.S',
    'src/trap.c',
)

c_args = [
    '-ffreestanding',
    '-Wno-unused-parameter',
    '-Wno-unused-but-set-variable',
    '-Wno-parentheses',
    '-nostdlib',
    '-fno-builtin',
    '-include',
    platdir / 'config.h',
    '-include',
    's3k_conf.h',
]

link_args = [
    '-nostartfiles',
    '-nostdlib',
    '-fno-builtin',
    '-T' + platdir / 'linker.ld',
]

machine_header = 'machine.h'

subdir('brc')

include_directories = include_directories('include')

elf = executable(
    's3k.elf',
    sources: sources + libkernel_c,
    include_directories: include_directories,
    c_args: c_args,
    link_args: link_args,
)

if not meson.is_subproject()
    subdir('test')
endif
