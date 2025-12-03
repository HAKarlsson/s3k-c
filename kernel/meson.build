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
    'src/cap_ops.c',
    'src/csr.c',
    'src/head.S',
    'src/kstate.c',
    'src/machine.c',
    'src/proc.c',
    'src/rtc.c',
    'src/sched.c',
    'src/trap.S',
    'src/trap.c',
)

c_args = [
    '-ffreestanding',
    '-Wno-unused-parameter',
    '-Wno-unused-but-set-variable',
    '-nostdlib',
    '-include',
    platdir / 'config.h',
    '-include',
    's3k_conf.h',
]

link_args = ['-nostartfiles', '-nostdlib', '-T', platdir / 'linker.ld']

if get_option('debug') == true
    sources += files('src/kprint.c')
else
    c_args += ['-flto']
    link_args += ['-flto']
endif

compiler = meson.get_compiler('c')

if compiler.get_id() != 'ccomp'
    c_args += ['-Wno-parentheses', '-fno-builtin']
    link_args += ['-fno-builtin']
endif

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

objdump = find_program('riscv64-unknown-elf-objdump')
da = custom_target(
    'da',
    command: [objdump, '-d', elf.full_path()],
    capture: true,
    depends: [elf],
    output: ['s3k.da'],
)

# if not meson.is_subproject()
#     subdir('test')
# endif
