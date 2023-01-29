project('eeprom-util',
	'c',
	version: '4.0',
	meson_version: '>= 0.52.1',
	license : 'GPL-2.0-only',
)

auto_generated_h = configuration_data()

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

version = meson.project_version() + run_command('setversion', check: false).stdout().strip()
date = run_command('date', '+%d %b %C%y', check: false).stdout().strip()
time = run_command('date', '+%T', check: false).stdout().strip()

auto_generated_h.set_quoted('VERSION', version)
auto_generated_h.set_quoted('BUILD_DATE', date)
auto_generated_h.set_quoted('BUILD_TIME', time)

configure_file(output: 'auto_generated.h', configuration: auto_generated_h)

extra_args = []

if get_option('enable-write')
	extra_args += '-DENABLE_WRITE'
endif

if get_option('buildtype') == 'debug'
	extra_args += '-DENABLE_WRITE'
endif

dep_json_c = dependency('json-c', version: '>= 0.13.1')

inc_libeeprom = include_directories('include')

srcs_libeeprom = [
	'eeprom.c',
	'field.c',
	'hal.c',
	'layout.c',
	'parsing_libjson.c'
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

subdir('tools')
