project('slpd-lite',
        'cpp',
        version: '0.1',
        meson_version: '>=0.49.0',
        default_options: [ 'warning_level=3',
                           'werror=true',
                           'cpp_std=c++17' ])

add_project_arguments('-Wno-missing-field-initializers', language: 'cpp')

conf_data = configuration_data()
conf_data.set_quoted('SERVICE_DIR', '/etc/slp/services/')

configure_file(output: 'config.h',
               configuration: conf_data)

systemd = dependency('systemd',version:'>=221')
libsystemd = dependency('libsystemd', required:true)

deps = [systemd, libsystemd]

compiler = meson.get_compiler('cpp')

slpd_sources = [ 'main.cpp',
                 'sock_channel.cpp',
                 'slp_server.cpp',
                 'slp_parser.cpp',
                 'slp_message_handler.cpp']

executable('slpd',
           slpd_sources,
           dependencies:deps,
           install:true)
