compiler = meson.get_compiler('cpp')

test_sources = files(
	'catch_main.cpp',
	'ctest_mpv-player.cpp',
)

# if compiler.get_id() == 'gcc'
# 	message('Compiler = GCC')
# 	libs_compiler = ['-lstdc++fs']
# 	libs_linker   = ['-lstdc++fs']
# elif compiler.get_id() == 'clang'
# 	message('Compiler = LLVM/clang')
# 	libs_compiler = ['-stdlib=libc++']
# 	libs_linker   = ['-stdlib=libc++', '-lstdc++fs']
# endif

exe = executable('test__catch2',
	cpp_args: cxx_flags + extra_flags,
	dependencies: [
		momuma_dep,
		glibmm_dep, sigc_dep, catch2_dep,
	],
	implicit_include_directories : true,
	include_directories: [ include_directory ],
	sources: test_sources,
)

test('catch2', exe, timeout: 120)
