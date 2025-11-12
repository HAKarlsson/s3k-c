project(
    'app0',
    'c',
    version: '0.1',
    meson_version: '>= 1.5.1',
    default_options: [
        'buildtype=release',
        'warning_level=2',
        'c_std=c99',
        'b_ndebug=if-release',
    ],
)

plat = get_option('plat')

plat_c = meson.current_source_dir() / 'plat' / plat + '.c'
plat_h = meson.current_source_dir() / 'plat' / plat + '.h'
plat_ld = meson.current_source_dir() / 'plat' / plat + '.ld'

elf = executable(
    'app0.elf',
    sources: files(plat_c, 'head.S', 'main.c'),
    c_args: ['-ffreestanding', '-include', plat_h],
    link_args: ['-nostdlib', '-T', plat_ld],
    dependencies: [
        subproject('libserio').get_variable('dep'),
        subproject('libs3k').get_variable('dep'),
    ],
)
