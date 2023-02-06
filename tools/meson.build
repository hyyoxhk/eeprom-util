srcs = [
	'eeprom-util.c',
	'command.c'
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


# project('eeprom-util',
# 	'c',
# 	version: '3.2.0',
# 	meson_version: '>= 0.52.1',
# 	license : 'MIT',
# )

# cc = meson.get_compiler('c')

# global_args = []
# global_args_maybe = [
# 	'-Wmissing-prototypes',
# 	'-Wno-unused-parameter',
# 	'-Wno-shift-negative-value', # required due to Pixman
# 	'-Wno-missing-field-initializers',
# 	'-Wno-pedantic',
# 	'-Wundef',
# 	'-fvisibility=hidden',
# ]
# foreach a : global_args_maybe
# 	if cc.has_argument(a)
# 		global_args += a
# 	endif
# endforeach
# add_global_arguments(global_args, language: 'c')

# extra_args = []

# if get_option('buildtype') == 'debug'
# 	extra_args += '-DENABLE_WRITE'
# endif

# dep_eeprom = dependency('eeprom')

# srcs = [
# 	'eeprom-util.c',
# 	'command.c'
# ]

# inc = include_directories('.')

# executable(
# 	'eeprom-util',
# 	srcs,
# 	c_args : extra_args,
# 	include_directories: inc,
# 	dependencies: dep_eeprom,
# )
