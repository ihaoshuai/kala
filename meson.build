project('kala', 'cpp')

incl = include_directories('include')

src = files(
    'src/main.cpp',
    'src/signal_handle.cpp',
    'src/share_memory.cpp',
    'src/lockfile.cpp',
)

incls = [ incl ]
deps = [ ]

extra_args = []
if get_option('buildtype') == 'release'
    extra_args += [ '-DRELEASE_BUILD' ]
endif

executable(
    'kala',
    src,
    cpp_args: extra_args,
    include_directories: incls,
    dependencies: deps
)