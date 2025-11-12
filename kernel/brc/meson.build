barocq = find_program('barocq', required: true)
fs = import('fs')
br_srcs = files(fs.read('modorder').strip().split())

libkernel_c = custom_target(
    'libkernel_c',
    input: br_srcs,
    output: ['libkernel.c', 'libkernel.h'],
    command: [barocq, '@INPUT@', '-o', '@OUTPUT0@', '-gen-header'],
)
