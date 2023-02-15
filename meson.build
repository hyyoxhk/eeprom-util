project('eeprom-util',
	'c',
	version: '4.0',
	meson_version: '>= 0.52.1',
	license : 'MIT',
)

cc = meson.get_compiler('c')

global_args = []
global_args_maybe = [
	'-Wmissing-prototypes',
	'-Wno-unused-parameter',
	'-Wno-shift-negative-value', # required due to Pixman
	'-Wno-missing-field-initializers',
	'-Wno-pedantic',
	'-Wundef',
	'-fvisibility=hidden',
]
foreach a : global_args_maybe
	if cc.has_argument(a)
		global_args += a
	endif
endforeach
add_global_arguments(global_args, language: 'c')

extra_args = []

pkgconfig = import('pkgconfig')

if get_option('enable-write')
	extra_args += '-DENABLE_WRITE'
endif

if get_option('buildtype') == 'debug'
	extra_args += '-DENABLE_WRITE'
endif

dep_json_c = dependency('json-c', version: '>= 0.13.1')

inc_libeeprom = include_directories('.')

srcs_libeeprom = [
	'eeprom.c',
	'field.c',
	'hal.c',
	'layout.c',
	'libjson.c',
	'util.c'
]

lib_libeeprom = library(
	'eeprom',
	sources: srcs_libeeprom,
	version: meson.project_version(),
	dependencies: dep_json_c, 
	include_directories : inc_libeeprom,
	install: true
)

dep_libeeprom = declare_dependency(
	link_with: lib_libeeprom,
	include_directories: inc_libeeprom
)

install_headers(
	'eeprom.h',
	'util.h',
	subdir: 'eeprom'
	)

pkgconfig.generate(
	lib_libeeprom,
	name: 'eeprom',
	description : 'eeprom layout parser',
	version: meson.project_version(),
	subdirs: 'eeprom'
)

subdir('tools')
