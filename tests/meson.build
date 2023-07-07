# if compiler.get_id() == 'gcc'
# 	message('Compiler = GCC')
# 	libs_compiler = ['-lstdc++fs']
# 	libs_linker   = ['-lstdc++fs']
# elif compiler.get_id() == 'clang'
# 	message('Compiler = LLVM/clang')
# 	libs_compiler = ['-stdlib=libc++']
# 	libs_linker   = ['-stdlib=libc++', '-lstdc++fs']
# endif


################################################################################
# Variables

compiler = meson.get_compiler('cpp')

test_deps = [ momuma_dep, sigc_dep, catch2_dep ]

extra_flags += '-DTESTING_PATH="@0@"'.format(meson.current_source_dir())

test_env = environment({
	'LSAN_OPTIONS': f'suppressions=@source_code_root@/sanitizer-blacklist.txt'
})


################################################################################
# Executable tests

mpv_player_test_exe = executable('mpv_player',
	cpp_args: cxx_flags + extra_flags,
	dependencies: test_deps,
	implicit_include_directories : true,
	include_directories: include_directory,
	sources: 'ctest__mpv_player.cpp',
)

database_test_exe = executable('database',
	cpp_args: cxx_flags + extra_flags,
	dependencies: test_deps,
	implicit_include_directories : true,
	include_directories: include_directory,
	sources: 'ctest__database.cpp',
)


################################################################################
# Tests

test('mpv_player', mpv_player_test_exe, env: test_env, timeout: 120)
test('database', database_test_exe, env: test_env)
