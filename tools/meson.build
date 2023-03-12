srcs = [
	'eeprom-util.c',
	'command.c',
	'../util.c'
]

extra_args = []

if get_option('buildtype') == 'debug'
	extra_args += '-DENABLE_WRITE'
endif

executable(
	'eeprom-util',
	sources: srcs,
	#version: meson.project_version(),
	c_args : extra_args,
	dependencies: dep_libeeprom, 
	#include_directories : dep_libeeprom,
	install: true
)
