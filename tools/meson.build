srcs = [
	'eeprom-util.c',
	'command.c'
]

executable(
	'eeprom-util',
	sources: srcs,
	#version: meson.project_version(),
	dependencies: dep_libeeprom, 
	#include_directories : dep_libeeprom,
	install: true
)
