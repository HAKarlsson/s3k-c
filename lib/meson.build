project(
    'libs3k',
    'c',
    version: '0.1',
    meson_version: '>= 1.5.1',
    default_options: [
        'buildtype=debug',
        'warning_level=2',
        'c_std=c99',
        'b_ndebug=if-release',
    ],
)

lib = static_library(
    's3k',
    sources: files('src/s3k.c', 'src/syscall.c'),
    include_directories: include_directories('include'),
)

dep = declare_dependency(
    link_with: lib,
    include_directories: include_directories('include'),
)
