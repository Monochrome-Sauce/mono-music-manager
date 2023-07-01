project('mono-music-manager', 'cpp',
	default_options: [
		'b_sanitize=address,undefined',
		'buildtype=debug',
		'c_std=gnu17',
		'cpp_std=gnu++20',
		'optimization=s',
		'warning_level=2',
		'werror=true'
	],
	license : 'GPL-3.0-or-later',
	meson_version: '>= 0.61',
	version: '0.1.1',
)


################################################################################
# Version

version_array = meson.project_version().split('.')
version_major = version_array[0].to_int()
version_minor = version_array[1].to_int()
version_patch = version_array[2].to_int()


################################################################################
# Default paths

source_code_root = meson.current_source_dir()
message(f'Source code dir = @source_code_root@')


################################################################################
# Options

cxx = meson.get_compiler('cpp')
is_debug_build = (get_option('buildtype') == 'debug')


################################################################################
# Config

config_h = configuration_data()
config_h.set('MOMUMA__VERSION_MAJOR', version_major)
config_h.set('MOMUMA__VERSION_MINOR', version_minor)
config_h.set('MOMUMA__VERSION_PATCH', version_patch)
config_h.set_quoted('MOMUMA__NAME', meson.project_name())
config_h.set_quoted('MOMUMA__VERSION', meson.project_version())
configure_file(configuration: config_h, output: 'build-config.h')


################################################################################
# Dependencies

asan_dep = cxx.find_library('asan')
ubsan_dep = cxx.find_library('ubsan')

catch2_dep = dependency('catch2', include_type: 'system', version: '>= 2.10')
glibmm_dep = dependency('glibmm-2.4', include_type: 'system', version: '>= 2.60')
mpv_dep = dependency('mpv', include_type: 'system', version: '>= 1.109')
sigc_dep = dependency('sigc++-2.0', include_type: 'system', version: '>= 2.10')
spdlog_dep = dependency('spdlog', include_type: 'system', version: '>= 1.9')
sqlite_dep = dependency('sqlite3', include_type: 'system', version: '>= 3.37')


################################################################################
# Compiler flags

extra_flags = [
	'-DFMT_USE_CONSTEXPR', # let FMT/SPDLOG functions be type-checked at compile-time
	'-DMPV_ENABLE_DEPRECATED=0',
	'-DSQLITE_THREADSAFE=1',
	'-Wcast-align',
	'-Wcast-qual',
	'-Wchar-subscripts',
	'-Wconversion',
	'-Wdouble-promotion',
	'-Wformat=2',
	'-Wmissing-declarations',
	'-Wnull-dereference',
	'-Wold-style-cast',
	'-Woverflow',
	'-Wpointer-arith',
	'-Wshadow',
	'-Wsign-compare',
	'-Wundef',
	'-Wunused-result',
	'-Wunused-variable',
]

cxx_flags = extra_flags
add_project_arguments(cxx.get_supported_arguments(cxx_flags), language: 'cpp')


################################################################################
# Subdirectories

include_directory = include_directories('./include/')
internal_directory = include_directories('./include/momuma/internal/')

subdir('src')
libmomuma = static_library(meson.project_name(),
	cpp_args: cxx_flags + extra_flags,
	dependencies: [
		asan_dep, ubsan_dep,
		
		glibmm_dep, mpv_dep, sigc_dep, spdlog_dep, sqlite_dep,
	],
	implicit_include_directories: true,
	include_directories: [ include_directory, internal_directory ],
	sources: momuma_sources,
)
momuma_dep = declare_dependency(
	link_with: libmomuma,
	include_directories: [ include_directory, internal_directory ],
)
subdir('tests')
