project(
    'hello',
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

s3k = subproject(
    's3k',
    default_options: ['s3k_conf=' + meson.current_source_dir() / 's3k_conf.h'],
)

app0 = subproject('app0')
