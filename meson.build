project(
  'slpd-lite',
  'cpp',
  version: '0.1',
  meson_version: '>=1.1.1',
  default_options: [
    'warning_level=3',
    'werror=true',
    'cpp_std=c++23',
  ],
)

libsystemd_dep = dependency('libsystemd')

executable(
  'slpd',
  'main.cpp',
  'slp_message_handler.cpp',
  'slp_parser.cpp',
  'slp_server.cpp',
  'sock_channel.cpp',
  dependencies: [ libsystemd_dep ],
  install: true,
  install_dir: get_option('sbindir'),
)

build_tests = get_option('tests')
gtest = dependency('gtest', main: true, disabler: true, required: build_tests)
gmock = dependency('gmock', disabler: true, required: build_tests)
  test(
      'test_slp_parser',
      executable('test_slp_parser',
          './test/slp_parser_test.cpp',
          'slp_parser.cpp',
          dependencies: [
              gtest,
          ],
          implicit_include_directories: true,
          include_directories: '../'
      )
  )
