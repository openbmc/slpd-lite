project('slpd-lite',
	'cpp',
	version: '0.1',
	meson_version: '>=0.49.0',
	default_options: [ 'warning_level=3',
			    'werror=true',
	  		    'cpp_std=c++17' ])

add_project_arguments('-Wno-psabi','-Wno-missing-field-initializers', language: 'cpp')
add_project_link_arguments('-lsystemd',language:'cpp')

conf_data = configuration_data()
conf_data.set_quoted('SERVICE_DIR', '/etc/slp/services/')

configure_file(output: 'config.h',
	       configuration: conf_data)

deps = dependency('systemd',version:'>=221')
compiler = meson.get_compiler('cpp')

if compiler.has_header('systemd/sd-bus.h')
	### Nothing to do
else
	warning('Systemd Development Package Required')
endif

slpd_sources = [ 'main.cpp',
		 'sock_channel.cpp',
		 'slp_server.cpp',
                 'slp_parser.cpp',
                 'slp_message_handler.cpp']

executable('slpd',
	    slpd_sources,
	    dependencies:deps,
	    install:true)
