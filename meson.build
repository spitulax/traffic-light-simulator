project(
  'traffic-light-simulator',
  'c', 'cpp',
  version : run_command('cat', meson.project_source_root() / 'VERSION', check: true).stdout().strip(),
  license: 'MIT',
  default_options : [
    'c_std=c11',
    'cpp_std=c++11',
    'warning_level=3',
  ],
)

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

if get_option('buildtype').startswith('debug')
  add_project_arguments(['-Og', '-DDEBUG'], language: ['c', 'cpp'])
endif

add_project_arguments('-DPROG_VERSION="' + meson.project_version() + '"', language: ['c', 'cpp'])
add_project_arguments(['-Wconversion', '-Wsign-conversion', '-Wpedantic'], language: ['c', 'cpp'])
add_project_arguments(['-Wshadow', '-Wnon-virtual-dtor'], language: 'cpp')

subdir('src')

include = [
  'src',
]

deps = [
  dependency('raylib'),
]

executable(
  'traffic-light-simulator',
  src,
  include_directories : include,
  dependencies : [deps],
  install : true,
)
