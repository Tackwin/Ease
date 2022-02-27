#if 0
# In cmd "type Ease.hpp | cmd"
# In bash "type Ease.hpp | bash"
#endif

#if 0
echo \" <<'BATCH_SCRIPT' >/dev/null ">NUL "\" \`" <#"

clang++ -o ./Build.exe ./Build.cpp -std=c++17 && Build.exe %*
exit
REM ====== Batch Script End ======
GOTO :eof
TYPE CON >NUL
BATCH_SCRIPT
#> | Out-Null


echo \" <<'POWERSHELL_SCRIPT' >/dev/null # " | Out-Null
# ===== PowerShell Script Begin =====
clang++ -o ./Build.exe ./Build.cpp -std=c++17; Build.exe
exit
# ====== PowerShell Script End ======
while ( ! $MyInvocation.MyCommand.Source ) { $input_line = Read-Host }
exit
<#
POWERSHELL_SCRIPT


set +o histexpand 2>/dev/null
# ===== Bash Script Begin =====
clang++ -o ./Build.exe ./Build.cpp -std=c++17 && Build.exe
exit
# ====== Bash Script End ======
case $- in *"i"*) cat /dev/stdin >/dev/null ;; esac
exit
#>
#endif


#ifndef EASE_HPP
#define EASE_HPP

#define _CRT_SECURE_NO_WARNINGS
#include <map>
#include <cmath>
#include <atomic>
#include <vector>
#include <string>
#include <optional>
#include <functional>
#include <filesystem>
#include <unordered_map>

#ifndef EASE_NAMESPCE
#define EASE_NAMESPACE Ease
#endif

namespace EASE_NAMESPACE {

static std::filesystem::path Working_Directory;

struct Flags {

	// >FLAGS
	// I want to make optional bool because user code might want to knwo if a user set someting in
	// the cmd.
	// For now i'm keeping it as everything is false by default, since you can only set a flag to
	// true it should be enough ?
	bool clean = false;
	bool openmp = false;
	bool silent = false;
	bool release = false;
	bool scratch = false;
	bool install = false;
	bool no_simd = false;
	bool assembly = false;
	bool fast_math = false;
	bool show_help = false;
	bool link_only = false;
	bool no_inline = false;
	bool profile_build = false;
	bool no_default_lib = false;
	bool compile_native = false;
	bool generate_debug = false;
	bool no_install_path = false;
	bool show_help_install = false;
	bool no_compile_commands = false;
	bool run_after_compilation = false;
	bool recompile_build_script = false;
	bool no_watch_source_changed = false;

	size_t j = 0;
	int verbose_level = 0;

	std::vector<std::string> defines;
	
	std::vector<std::string> rest_args;

	std::optional<size_t> release_level = std::nullopt;

	inline static std::filesystem::path Default_State_Path = "state.txt";
	inline static std::filesystem::path Default_Build_Path = "ease_build/";
	inline static std::filesystem::path Default_Temp_Path = "ease_temp/";

	inline static std::filesystem::path Default_Compile_Command_Path = "compile_commands.json";

	std::optional<std::filesystem::path> state_file;
	std::optional<std::filesystem::path> output;
	std::optional<std::filesystem::path> build_path;
	std::optional<std::filesystem::path> temp_path;
	std::optional<std::filesystem::path> compile_command_path;

	std::filesystem::path get_compile_commands_path() const noexcept {
		if (compile_command_path) return *compile_command_path;
		else return get_build_path() / Default_Compile_Command_Path;
	}
	std::filesystem::path get_build_path() const noexcept {
		if (build_path) return *build_path;
		return Default_Build_Path;
	}
	std::filesystem::path get_temp_path() const noexcept {
		if (temp_path) return *temp_path;
		return Default_Temp_Path;
	}

	std::filesystem::path get_state_path() const noexcept {
		if (state_file) return *state_file;
		else return get_build_path() / Default_State_Path;
	}

	static Flags parse(int argc, char** argv) noexcept;
	static const char* help_message() noexcept;
	static const char* help_install_message() noexcept;

	size_t hash() const noexcept;
	size_t rebuild_hash() const noexcept;
};

struct Build_State {
	size_t flags_hash = 0;
	size_t rebuild_flag_hash = 0;
	std::map<std::filesystem::path, std::uint64_t> files;

	static Build_State get_unchanged(const Build_State& a, const Build_State& b) noexcept;
};

struct States {
	std::unordered_map<std::string, Build_State> states;
	size_t last_write_build_script = 0;

	static States load_from_file(const std::filesystem::path& p) noexcept;
	void save_to_file(const std::filesystem::path& p) noexcept;
};

struct Commands {
	struct Entry {
		std::string command;
		std::string short_desc;
		std::filesystem::path file;
		std::filesystem::path directory;

		std::optional<std::filesystem::path> output;
	};

	std::vector<Entry> entries;

	void add_command(
		std::string command,
		std::string desc,
		std::filesystem::path file,
		std::filesystem::path out
	) noexcept;

	void add_command(
		std::string command,
		std::string desc,
		std::filesystem::path file,
		std::filesystem::path out,
		std::filesystem::path dir
	) noexcept;

	void save_command_json(const std::filesystem::path& p) noexcept;
};

struct Build;
struct Build_Ptr {
	Build* b = nullptr;
	Build_Ptr() noexcept {}
	Build_Ptr(const Build& b_) noexcept;
	~Build_Ptr() noexcept;
	Build_Ptr(const Build_Ptr& other) noexcept;
	Build_Ptr(Build_Ptr&& other) noexcept;
	Build_Ptr& operator=(const Build_Ptr& other) noexcept;
	Build_Ptr& operator=(Build_Ptr&& other) noexcept;
};

struct Build {
	enum class Target {
		Header_Only,
		Assembly,
		Static,
		Shared,
		Custom,
		None, // Use for export only for instance.
		Exe
	};

	enum class Arch {
		x86,
		x64
	};

	enum class Cli {
		Cl = 0,
		Gcc,
		Count
	};

	std::string name;
	std::optional<std::string> triplet;

	Flags flags;

	Cli cli;
	Arch arch;
	Target target;
	std::string_view std_ver;

	bool invert_header_implementation_define = false;

	size_t stack_size = 8196;

	std::filesystem::path compiler;
	std::filesystem::path archiver; // I don't really like to need llvm-ar or something
	                                // i feel like we could do this ourself, it's just concatenating
	                                // it mustn't be that complicated.

	std::vector<std::filesystem::path> source_files;
	std::vector<std::filesystem::path> header_files;

	std::vector<std::filesystem::path> link_files;
	std::vector<std::filesystem::path> lib_path;

	std::vector<std::filesystem::path> static_files;

	std::vector<std::filesystem::path> export_files;
	std::vector<std::filesystem::path> export_dest_files;

	std::vector<Commands> pre_compile;
	std::vector<Commands> post_compile;
	std::vector<Commands> pre_link;
	std::vector<Commands> post_link;

	std::vector<std::string> defines;

	std::vector<std::filesystem::path> to_install;

	std::function<void(void)> run_function = nullptr;

	Build_Ptr next;
	Build_State current_state;

	static Build get_default(Flags flags = {}) noexcept;
	static Build sequentials(std::vector<Build> builds) noexcept;

	void add_header(const std::filesystem::path& f) noexcept;
	void add_source(const std::filesystem::path& f) noexcept;
	void add_source_recursively(const std::filesystem::path& f) noexcept;

	void del_source(const std::filesystem::path& f) noexcept;
	void del_source_recursively(const std::filesystem::path& f) noexcept;

	void add_library(const std::filesystem::path& f) noexcept;
	void add_library_path(const std::filesystem::path& f) noexcept;

	void add_export(const std::filesystem::path& f) noexcept;
	void add_export(const std::filesystem::path& from, const std::filesystem::path& to) noexcept;

	void add_define(std::string str) noexcept;
	void add_debug_defines() noexcept;

	void add_default_win32() noexcept;
	void no_warnings_win32() noexcept;
};

// >FLAGS
enum class Cli_Opts {
	Compile,
	Link_Shared,
	Exe_Output,
	Object_Output,
	Preprocessor_Output,
	Force_Include,
	Std_Version,
	Include,
	Link,
	No_SIMD,
	Lib_Path,
	Time_Trace,
	Define,
	Optimisation,
	Fast_Math,
	No_Optimisation,
	Preprocess,
	Arch_32,
	Assembly_Output,
	Debug_Symbol_Compile,
	Debug_Symbol_Link,
	No_Default_Lib,
	OpenMP,
	Native,
	Stack_Size,
	No_Inline
};


struct Env {
	static constexpr bool Win32 =
		#ifdef _WIN32
		true;
		#else
		false;
		#endif

	static const char* Build_Script_Path;
};

#define EASE_WATCH_ME const char* EASE_NAMESPACE::Env::Build_Script_Path = __FILE__;

namespace details {
	std::string escape(std::string_view in) noexcept;
	
	std::string get_cli_flag(
		Build::Cli cli, Cli_Opts opts, std::string_view param = ""
	) noexcept;
	void install_build(const Build& b) noexcept;
	std::vector<std::filesystem::path> get_installed_dirs(const Build& b) noexcept;
	std::filesystem::path get_output_dir(const Build& b) noexcept;
	std::filesystem::path get_output_path(const Build& b) noexcept;
};

};

#ifndef EASE_NO_NAMESPACE
using namespace Ease;
#endif

#endif
#ifndef Ease_header_only
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <cstdlib>
#include <vector>
#include <cctype>
#include <locale>
#include <thread>
#include <cstdarg>

#ifndef EASE_HPP
#include "Ease.hpp"
#endif

#define NS EASE_NAMESPACE

// >TODO(Tackwin): We need to move from using clang++ -Wl style options to pass
// args to the linker to invoking directly ld or mold or lld or link.exe or whatever

// trim from start (in place)
void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
}

// trim from end (in place)
void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

// trim from both ends (in place)
void trim(std::string &s) {
	ltrim(s);
	rtrim(s);
}

// trim from start (copying)
std::string ltrim_copy(std::string s) {
	ltrim(s);
	return s;
}


std::uint64_t hash(const std::string& str) noexcept;
std::string unique_name(const Build& build, const std::filesystem::path& path) noexcept;

// FILE IO
void dump_to_file(std::string_view str, const std::filesystem::path& p) noexcept;
std::string read_file(const std::filesystem::path& p) noexcept;

// ===================== FLAGS
// >FLAGS
NS::Flags NS::Flags::parse(int argc, char** argv) noexcept {
	NS::Flags flags;

	for (int i = 0; i < argc; ++i) {
		auto it = argv[i];

		if (strcmp(it, "--run") == 0) {
			flags.run_after_compilation = true;
		}
		if (strcmp(it, "--clean") == 0) {
			flags.clean = true;
		}
		if (strcmp(it, "--native") == 0) {
			flags.compile_native = true;
		}
		if (strcmp(it, "--fast-math") == 0) {
			flags.fast_math = true;
		}
		if (strcmp(it, "--debug") == 0) {
			flags.generate_debug = true;
		}
		if (strcmp(it, "--install") == 0) {
			flags.install = true;
		}
		if (strcmp(it, "-l") == 0 || strcmp(it, "--link") == 0) {
			flags.link_only = true;
		}
		if (strcmp(it, "-h") == 0 || strcmp(it, "--help") == 0) {
			flags.show_help = true;
		}
		if (strcmp(it, "--help-install") == 0) {
			flags.show_help_install = true;
		}
		if (strcmp(it, "--verbose") == 0) {
			if (i + 1 >= argc) continue;
			++i;
			flags.verbose_level = std::stoi(argv[i]);
		}
		if (strcmp(it, "--silent") == 0) {
			flags.silent = true;
		}
		if (strcmp(it, "--no-simd") == 0) {
			flags.no_simd = true;
		}
		if (strcmp(it, "--release") == 0) {
			flags.release = true;
			if (i + 1 < argc) {
				if (std::isdigit(argv[i + 1][0])) {
					i++;
					flags.release_level = std::stoull(argv[i]);
				}
			}
		}
		if (strcmp(it, "--scratch") == 0) {
			flags.scratch = true;
		}
		if (strcmp(it, "--no-inline") == 0) {
			flags.no_inline = true;
		}
		if (strcmp(it, "--assembly") == 0) {
			flags.assembly = true;
		}
		if (strcmp(it, "--no-install-path") == 0) {
			flags.no_install_path = true;
		}
		if (strcmp(it, "--profile-build") == 0) {
			flags.profile_build = true;
		}
		if (strcmp(it, "--no-default-lib") == 0) {
			flags.no_default_lib = true;
		}
		if (strcmp(it, "--recompile") == 0) {
			flags.recompile_build_script = true;
		}
		if (strcmp(it, "--openmp") == 0) {
			flags.openmp = true;
		}
		if (strcmp(it, "--no-compile-commands") == 0) {
			flags.no_compile_commands = true;
		}
		if (strcmp(it, "--no-watch-source-changed") == 0) {
			flags.no_watch_source_changed = true;
		}
		if (strcmp(it, "--compile-commands") == 0) {
			if (i + 1 >= argc) continue;
			i++;
			flags.compile_command_path = argv[i];
			flags.compile_command_path = flags.compile_command_path->lexically_normal();
		}
		if (strcmp(it, "--state-file") == 0) {
			if (i + 1 >= argc) continue;
			i++;
			flags.state_file = argv[i];
			flags.state_file = flags.state_file->lexically_normal();
		}
		if (strcmp(it, "--build-dir") == 0) {
			if (i + 1 >= argc) continue;
			i++;
			flags.build_path = argv[i];
			flags.build_path = flags.build_path->lexically_normal();
		}
		if (strcmp(it, "--temp-dir") == 0) {
			if (i + 1 >= argc) continue;
			i++;
			flags.temp_path = argv[i];
			flags.temp_path = flags.temp_path->lexically_normal();
		}
		if (strcmp(it, "-j") == 0) {
			if (i + 1 >= argc) continue;
			i++;
			flags.j = std::stoi(argv[i]);
		}
		if (strcmp(it, "-o") == 0 || strcmp(it, "--output") == 0) {
			if (i + 1 >= argc) continue;
			i++;
			flags.output = argv[i];
			flags.output = flags.output->lexically_normal();
		}
		if (strcmp(it, "-D") == 0 || strcmp(it, "--define") == 0) {
			if (i + 1 >= argc) continue;
			i++;
			flags.defines.push_back(argv[i]);
		}
		if (strcmp(it, "--") == 0) {
			for (++i; i < argc; ++i) flags.rest_args.push_back(argv[i]);
			break;
		}
	}

	if (flags.j < 1) flags.j = 1;

	return flags;
}

// >FLAGS
const char* NS::Flags::help_message() noexcept {
	return
	" ====================================== Optional flags ======================================\n"
	"--run                        Will run the program (if the program is runnable) after the\n"
	"                             compilation.\n"
	"                    Exemple: ./Build.exe --run\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"--clean                      Will clean every file or directory generated by this software.\n"
	"                    Exemple: ./Build.exe --clean\n\n"
	
	" --------------------------------------------------------------------------------------------\n"
	"-h|--help                    Will show this message, then exit.\n"
	"                    Exemple: ./Build.exe -h | ./Build.exe --help\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"--help-install               Print informations about the install system.\n"
	"                    Exemple: ./Build.exe --help-install\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"--no-simd                    Disable the generation of simd instruction and usage of vector.\n"
	"                             registers.\n"
	"                    Exemple: ./Build.exe --no-simd\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"-j <n>                        Will use n threads.\n"
	"                    Exemple: ./Build.exe -j 4\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"--openmp                     Will compile with openmp if available.\n"
	"                    Exemple: ./Build.exe --openmp\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"--fast-math                  Enable fast math optimisation.\n"
	"                    Exemple: ./Build.exe --fast-math\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"--silent                     Will disable any write to the stdout.\n"
	"                    Exemple: ./Build.exe --silent\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"--no-watch-source-changed    Will disable watching for change in Build.cpp.\n"
	"                    Exemple: ./Build.exe --no-watch-source-changed\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"--assembly                   Will compile to output assembly file.\n"
	"                    Exemple: ./Build.exe --assembly\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"-l|--link                    Will only link the program, then exit. There must be a complete\n"
	"                             state available. Will not check for incremental compilation.\n"
	"                    Exemple: ./Build.exe -l | ./Build.exe --link\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"--profile-build              Will produce an output file that contains information about the\n"
	"                             time spent on each aspect of the compilation.\n"
	"                    Exemple: ./Build.exe --profile-build\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"--verbose <level>            Change the level of verbosity.\n"
	"                             Level 0 (default) will show a simple progress message.\n"
	"                             Level 1 will show with each stage the commands executed.\n"
	"                             Level 2 will show additional informations.\n"
	"                    Exemple: ./Build.exe --verbose 0 | ./Build.exe --verbose 1\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"-D|--define <arg>            Add <arg> to the list of C macro definitions.\n"
	"                    Exemple: ./Build.exe --define N_THREADS=5 | ./Build.exe --define A=1 -D B=2\n"
	"                    Exemple: ./Build.exe --define FLAG -D V=1 -D \"un deux\"\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"--release [level]            Will compile the program using default option for release\n"
	"                             (for instance, if the cli choosen is gcc, will compile using -O3)\n"
	"                             if level is specified it will use the appropriate level of\n"
	"                             optimisation.\n"
	"                    Exemple: ./Build.exe --release | ./Build.exe --release 1\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"--install                    Will install the program to be referenced by the EASE_PATH\n"
	"                             Environment variable, for more information you can run me with\n"
	"                             the flag --help-env\n"
	"                    Exemple: ./Build.exe --install\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"--native                     Will target the host cpu for compilation where applicable.\n"
	"                    Exemple: ./Build.exe --native\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"--recompile                   Will recompile the build script before running it.\n"
	"                    Exemple: ./Build.exe --recompile-build-script\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"--build-dir <path>           Change the default build path used by Ease to keep state across\n"
	"                             runs. Default to ease_build/\n"
	"                    Exemple: ./Build.exe --build-dir new_build/path/\n\n"
	
	" --------------------------------------------------------------------------------------------\n"
	"--temp-dir <path>            Change the default temp path used by Ease, it is created and \n"
	"                             destroyed each run of the build program. Default to ease_temp/\n"
	"                    Exemple: ./Build.exe --temp-dir new_temp/path/\n\n"
	
	" --------------------------------------------------------------------------------------------\n"
	"--no-install-path            Will *not* search your install variable for files to include\n"
	"                             and or directory to look for librairies.\n"
	"                    Exemple: ./Build.exe --no-install-path\n\n"
	
	" --------------------------------------------------------------------------------------------\n"
	"--debug                      Will generate alongside the program debug symbols\n"
	"                             It is not mutually exclusive with --release\n"
	"                    Exemple: ./Build.exe --debug\n\n"
	
	" --------------------------------------------------------------------------------------------\n"
	"--no-default-lib             Will not link against the default libraries.\n"
	"                    Exemple: ./Build.exe --debug\n\n"
	
	" --------------------------------------------------------------------------------------------\n"
	"--no-inline                  Will set the compiler flag to not inline any function, if\n"
	"                             availabe.\n"
	"                    Exemple: ./Build.exe --no-inline\n\n"
	
	" --------------------------------------------------------------------------------------------\n"
	"--scratch                    Disable incremental compilation and will clean the state file\n"
	"                    Exemple: ./Build.exe --scratch\n\n"
	
	" --------------------------------------------------------------------------------------------\n"
	"--no-compile-commands        Will not generate a compile_commands.json file.\n"
	"                    Exemple: ./Build.exe --no-compile-commands\n\n"

	" --------------------------------------------------------------------------------------------\n"
	"-- <rest_of_args>...         The rest of the arguments after -- will be forwared to the next\n"
	"                             stage of building. It will commonly be running. --run must be\n"
	"                             specified altough.\n"
	"                    Exemple: ./Build.exe --run -- arg1 arg2 arg3\n\n"
	
	" --------------------------------------------------------------------------------------------\n"
	"--compile-commands <path>    Set the path to generate compile_commands.json file. If <path> is\n"
	"                             a directory the file will be named compile_commands.json else it\n"
	"                             will take the name of the file specified by <path>\n"
	"                    Exemple: ./Build.exe --compile-commands build_dir/ |\n"
	"                    Exemple: ./Build.exe --compile-commands build_dir/my_file.json \n\n"
	
	" --------------------------------------------------------------------------------------------\n"
	"--state-file <path>          Will use <path> as the path to save the state file. If <path>\n"
	"                             represent a file then the state file will take exactly it's name\n"
	"                             but if it represents a directory the state file will be named\n"
	"                             state.txt .  By defualt the state file is located in the\n"
	"                             build directory and is named state.txt\n"
	"                    Exemple: ./Build.exe --state-file ./my_dir/other_file.ext |\n"
	"                    Exemple: ./Build.exe --state-file ./my_dir/ \n\n"
	
	" --------------------------------------------------------------------------------------------\n"
	"-o|--output <path>           Will use <path> as the path to save the executable file if\n"
	"                             applicable if <path> represent a file then the output will be\n"
	"                             named Run and be located in <path>. If it represents a file\n"
	"                             Then the ouput will be saved as <path>\n"
	"                    Exemple: ./Build.exe --output ./my_dir/other_file.ext |\n"
	"                    Exemple: ./Build.exe --output ./my_dir/               |\n"
	"                    Exemple: ./Build.exe -o       ./my_dir/other_file.ext |\n"
	"                    Exemple: ./Build.exe -o       ./my_dir/ \n\n"
	;
}

const char* NS::Flags::help_install_message() noexcept {
	return
	"Idk man... figure it out :/.\n"
	;
}




namespace std {
	template<>
	struct hash<std::filesystem::path> {
		std::size_t operator()(const std::filesystem::path& p) const {
			return std::filesystem::hash_value(p);
		}
	};
};


// >FLAGS
size_t NS::Flags::hash() const noexcept {
	auto combine = [](auto seed, auto v) {
		std::hash<std::remove_cv_t<decltype(v)>> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
		return seed;
	};

	size_t h = 0;
	h = combine(h, clean);
	h = combine(h, openmp);
	h = combine(h, silent);
	h = combine(h, release);
	h = combine(h, scratch);
	h = combine(h, install);
	h = combine(h, no_simd);
	h = combine(h, assembly);
	h = combine(h, no_inline);
	h = combine(h, fast_math);
	h = combine(h, show_help);
	h = combine(h, link_only);
	h = combine(h, profile_build);
	h = combine(h, compile_native);
	h = combine(h, generate_debug);
	h = combine(h, no_default_lib);
	h = combine(h, no_install_path);
	h = combine(h, show_help_install);
	h = combine(h, no_compile_commands);
	h = combine(h, run_after_compilation);
	h = combine(h, recompile_build_script);
	h = combine(h, no_watch_source_changed);
	h = combine(h, j);
	h = combine(h, verbose_level);
	h = combine(h, state_file);
	h = combine(h, output);
	h = combine(h, build_path);
	h = combine(h, temp_path);
	h = combine(h, compile_command_path);
	h = combine(h, release_level);
	for (auto& x : defines) h = combine(h, x);
	for (auto& x : rest_args) h = combine(h, x);

	return h;
}

// >FLAGS
size_t NS::Flags::rebuild_hash() const noexcept {
	auto combine = [](auto seed, auto v) {
		std::hash<std::remove_cv_t<decltype(v)>> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
		return seed;
	};

	size_t h = 0;
	h = combine(h, clean);
	h = combine(h, openmp);
	h = combine(h, release);
	h = combine(h, no_simd);
	h = combine(h, scratch);
	h = combine(h, assembly);
	h = combine(h, no_inline);
	h = combine(h, link_only);
	h = combine(h, fast_math);
	h = combine(h, compile_native);
	h = combine(h, generate_debug);
	h = combine(h, state_file);
	h = combine(h, release_level);
	h = combine(h, recompile_build_script);
	for (auto& x : defines) h = combine(h, x);

	return h;
}


// ===================== FLAGS

// ===================== BUILD_PTR
Build_Ptr::Build_Ptr(const Build& b_) noexcept {
	b = new Build(b_);
}
Build_Ptr::~Build_Ptr() noexcept {
	delete b;
}
Build_Ptr::Build_Ptr(const Build_Ptr& other) noexcept {
	*this = other;
}
Build_Ptr::Build_Ptr(Build_Ptr&& other) noexcept {
	*this = std::move(other);
}
Build_Ptr& Build_Ptr::operator=(const Build_Ptr& other) noexcept {
	delete b;
	this->b = nullptr;
	if (other.b) this->b = new Build(*other.b);
	return *this;
}
Build_Ptr& Build_Ptr::operator=(Build_Ptr&& other) noexcept {
	delete b;
	this->b = other.b;
	other.b = nullptr;
	return *this;
}
// ===================== BUILD_PTR

// ===================== BUILD
NS::Build NS::Build::get_default(::NS::Flags flags) noexcept {
	NS::Build build;
	build.cli = Cli::Gcc;
	build.target = Target::Exe;
	build.std_ver = "c++20";
	build.compiler = "clang++";
	build.archiver = "llvm-ar";
	build.arch = Arch::x64;
	build.name = "Run";
	build.flags = flags;
	return build;
}
NS::Build NS::Build::sequentials(std::vector<::NS::Build> builds) noexcept {
	if (builds.empty()) return Build::get_default({});

	Build curr = builds.back();

	for (size_t i = builds.size() - 2; i + 1 > 0; --i) {
		auto n = builds[i];
		n.next = curr;
		curr = n;
	}

	return curr;
}

void NS::Build::add_library(const std::filesystem::path& f) noexcept {
	auto x = f;
	x = x.lexically_normal();
	link_files.push_back(x);
}
void NS::Build::add_library_path(const std::filesystem::path& f) noexcept {
	auto x = f;
	x = x.lexically_normal();
	lib_path.push_back(x);
}
void NS::Build::add_source(const std::filesystem::path& f) noexcept {
	auto x = f;
	x = x.lexically_normal();
	if (!std::filesystem::is_regular_file(x)) {
		printf(
			"Can't add source %s. %s does not point to a source.\n",
			f.filename().generic_string().c_str(),
			f.generic_string().c_str()
		);
		return;
	}
	source_files.push_back(x);
}
void NS::Build::add_source_recursively(const std::filesystem::path& f) noexcept {
	if (!std::filesystem::is_directory(f)) {
		printf(
			"Can't search recursively for %s, it is not a directory.\n",
			f.generic_string().c_str()
		);
		return;
	}
	for (auto& x : std::filesystem::recursive_directory_iterator(f)) {
		if (!std::filesystem::is_regular_file(x)) continue;
		if (x.path().extension() != ".c" && x.path().extension() != ".cpp") continue;

		auto y = x.path();
		y = y.lexically_normal();
		add_source(y);
	}
}

void NS::Build::del_source(const std::filesystem::path& f) noexcept {
	auto x = f;
	x = x.lexically_normal();

	for (size_t i = source_files.size() - 1; i + 1 > 0; --i) {
		if (source_files[i] == x) source_files.erase(std::begin(source_files) + i);
	}
}

void NS::Build::del_source_recursively(const std::filesystem::path& f) noexcept {
	if (!std::filesystem::is_directory(f)) {
		printf(
			"Can't search recursively for %s, it is not a directory.\n",
			f.generic_string().c_str()
		);
		return;
	}
	for (auto& x : std::filesystem::recursive_directory_iterator(f)) {
		if (!std::filesystem::is_regular_file(x)) continue;
		if (x.path().extension() != ".c" && x.path().extension() != ".cpp") continue;

		auto y = x.path();
		y = y.lexically_normal();
		del_source(y);
	}
}


void NS::Build::add_header(const std::filesystem::path& f) noexcept {
	auto x = f;
	x = x.lexically_normal();
	header_files.push_back(x);
}
void NS::Build::add_define(std::string str) noexcept {
	defines.push_back(std::move(str));
}
void NS::Build::add_debug_defines() noexcept {
	add_define("_DEBUG");
	add_define("_ITERATOR_DEBUG_LEVEL=2");
}

void NS::Build::add_export(const std::filesystem::path& f) noexcept {
	add_export(f, f);
}
void NS::Build::add_export(
	const std::filesystem::path& from, const std::filesystem::path& to
) noexcept {
	auto x = from;
	auto y = to;
	x = x.lexically_normal();
	y = y.lexically_normal();
	export_files.push_back(x);
	export_dest_files.push_back(y);
}

std::filesystem::path NS::details::get_output_dir(const Build& b) noexcept {
	if (b.flags.output) return *b.flags.output;
	return "";
}

std::filesystem::path NS::details::get_output_path(const Build& b) noexcept {
	std::filesystem::path out = b.name;
	if (b.flags.output) {
		out = *b.flags.output;
		if (std::filesystem::is_directory(*b.flags.output)) {
			out /= b.name;
		}
	}
	if constexpr (Env::Win32) out = out.replace_extension(".exe");

	return out;
}

void NS::Build::add_default_win32() noexcept {
	add_library("Shell32");
	add_library("Ole32");
}
void NS::Build::no_warnings_win32() noexcept {
	add_define("_CRT_SECURE_NO_WARNINGS");
}


// ===================== BUILD

// ===================== State

NS::States NS::States::load_from_file(const std::filesystem::path& p) noexcept {
	NS::States states;
	auto text = read_file(p);

	std::istringstream stream(text);
	std::string line;

	if (std::getline(stream, line)) states.last_write_build_script = std::stoull(line.c_str());

	// each build begins with a pair of line <name, number_of_line>
	while(std::getline(stream, line)) {
		trim(line);
		std::string name = line;
		std::getline(stream, line);
		size_t l = std::stoull(line);

		auto& s = states.states[name];
		if (std::getline(stream, line)) std::sscanf(line.c_str(), "%zu", &s.flags_hash);
		if (std::getline(stream, line)) std::sscanf(line.c_str(), "%zu", &s.rebuild_flag_hash);
		
		for (size_t i = 0; i < l && std::getline(stream, line); ++i) {
			trim(line);
			std::filesystem::path f = line;
			f = f.lexically_normal();
			auto& p = s.files[f];

			if (std::getline(stream, line)) p = (std::uint64_t)std::stoull(line);
		}
	}

	return states;
}
NS::Build_State NS::Build_State::get_unchanged(const Build_State& a, const Build_State& b) noexcept {
	NS::Build_State s;

	for (auto [k, v] : a.files) if (b.files.count(k) > 0 && b.files.at(k) == v) s.files[k] = v;

	return s;
}

void NS::States::save_to_file(const std::filesystem::path& p) noexcept {
	std::string to_dump = "";
	to_dump += std::to_string(last_write_build_script) + "\n";

	for (auto& [name, s] : states) {
		to_dump += name + "\n";
		to_dump += std::to_string(s.files.size()) + "\n";

		to_dump += std::to_string(s.flags_hash) + "\n";
		to_dump += std::to_string(s.rebuild_flag_hash) + "\n";
		for (auto& [a, b] : s.files) {
			to_dump += a.generic_string() + "\n";
			to_dump += std::to_string(b) + "\n";
		}
	}

	dump_to_file(to_dump, p);
}
NS::Build_State set_files_hashes(const std::filesystem::path& p, NS::Build_State& s) noexcept {
	s.files.clear();
	for (auto& x : std::filesystem::recursive_directory_iterator(p)) {
		if (!x.is_regular_file()) continue;
		if (x.path().extension() != ".pre") continue;

		auto file = read_file(x);
		auto f = x.path();
		f = f.lexically_relative(p);
		f = f.replace_extension("");
		std::uint64_t hash = std::hash<std::string>()(file);
		s.files[f] = hash;
	}
	return s;
}

// ===================== State
// ===================== Commands


void NS::Commands::add_command(
	std::string command,
	std::string desc,
	std::filesystem::path file,
	std::filesystem::path out,
	std::filesystem::path dir
) noexcept {
	Entry e;
	e.command = std::move(command);
	e.short_desc = std::move(desc);
	e.file = std::move(file);
	e.output = std::move(out);
	e.directory = std::move(dir);
	entries.push_back(e);
}

void NS::Commands::add_command(
	std::string command,
	std::string desc,
	std::filesystem::path file,
	std::filesystem::path out
) noexcept {
	add_command(
		std::move(command),
		std::move(desc),
		std::move(file),
		std::move(out),
		std::filesystem::current_path()
	);
}

void NS::Commands::save_command_json(const std::filesystem::path& p) noexcept {
	std::string json;
	json = "[";

	auto escape = [] (const std::string& str) noexcept {
		static std::string n;
		n.clear();
		for (auto& c : str) {
			if (c == '"') n += "\\";
			n += c;
		}

		return n;
	};

	char separator = ' ';
	for (auto& x : entries) {
		auto dir = std::filesystem::absolute(x.directory);
		auto com = x.command;
		auto fil = std::filesystem::relative(x.file, dir);
		auto out = std::filesystem::relative(x.output ? *x.output : "", dir);

		json += separator;

		json += "{\n";
		json += "\t\"directory\": \"" + escape(dir.generic_string()) + "\",\n";
		json += "\t\"command\":   \"" + escape(com) + "\",\n";
		json += "\t\"output\":    \"" + escape(out.generic_string()) + "\",\n";
		json += "\t\"file\":      \"" + escape(fil.generic_string()) + "\"\n";
		json += "}";

		separator = ',';
	}

	json += "]";

	dump_to_file(json, p);
}

// ===================== Commands



void concat(
	const std::filesystem::path& dst, const std::vector<std::filesystem::path>& src
) noexcept {
	std::string to_dump = read_file(dst);
	for (auto& x : src) to_dump += read_file(x);
	dump_to_file(to_dump, dst);
}
void append(
	const std::filesystem::path& dst, std::string_view src
) noexcept {
	std::string to_dump = read_file(dst);
	to_dump += src;
	dump_to_file(to_dump, dst);
}

std::string produce_single_header(NS::Build& b) noexcept {
	std::string single;

	for (auto& x : b.header_files) if (std::filesystem::is_regular_file(x))
		single += read_file(x) + "\n";
	if (b.invert_header_implementation_define) single += "#ifndef " + b.name + "_header_only\n";
	else single += "#ifdef " + b.name + "_implementation\n";
	for (auto& x : b.source_files) {
		single += read_file(x) + "\n";
	}
	single += "#endif\n";

	dump_to_file(single, b.name + ".hpp");

	return single;
}

NS::Commands compile_command_object(const NS::Build_State& state, const NS::Build& b) noexcept {
	using namespace NS;
	using namespace details;
	NS::Commands commands;
	std::string command;

	// Compile to -o
	for (auto x : b.source_files) {

		auto c = x;

		std::filesystem::path o = b.flags.get_build_path();
		o += unique_name(b, x) + ".o";
		o = o.lexically_normal();

		if (b.flags.link_only) continue;

		auto test_file = unique_name(b, x);

		// Check if this files has not changed
		// (so it must first exists)
		if (std::filesystem::is_regular_file(o) && state.files.count(test_file) > 0) continue;

		command = b.compiler.generic_string();
		command += " " + get_cli_flag(b.cli, Cli_Opts::Compile);
		command += " " + get_cli_flag(b.cli, Cli_Opts::Std_Version, b.std_ver);

		// >TODO(Tackwin): Must this be here ?
		if (b.triplet)              command += " -target " + *b.triplet;

		if (b.flags.openmp)         command += " " + get_cli_flag(b.cli, Cli_Opts::OpenMP);
		if (b.flags.no_simd)        command += " " + get_cli_flag(b.cli, Cli_Opts::No_SIMD);
		if (b.flags.no_inline)      command += " " + get_cli_flag(b.cli, Cli_Opts::No_Inline);
		if (b.flags.profile_build)  command += " " + get_cli_flag(b.cli, Cli_Opts::Time_Trace);
		if (b.flags.compile_native) command += " " + get_cli_flag(b.cli, Cli_Opts::Native);

		if (b.flags.release) {
			std::string param = "3";
			if (b.flags.release_level) param = std::to_string(*b.flags.release_level);
			command += " " + get_cli_flag(b.cli, Cli_Opts::Optimisation, param);
		}
		else
			command += " " + get_cli_flag(b.cli, Cli_Opts::No_Optimisation);

		if (b.flags.fast_math) command += " " + get_cli_flag(b.cli, Cli_Opts::Fast_Math);

		command += " " + get_cli_flag(b.cli, Cli_Opts::Object_Output, o.generic_string());

		for (auto& d : b.defines)       command += " " + get_cli_flag(b.cli, Cli_Opts::Define, d);
		for (auto& d : b.flags.defines) command += " " + get_cli_flag(b.cli, Cli_Opts::Define, d);

		for (auto& x : b.header_files) if (std::filesystem::is_directory(x))
			command += " " + get_cli_flag(b.cli, Cli_Opts::Include, x.generic_string());

		for (auto& x : b.header_files) if (std::filesystem::is_regular_file(x))
			command += " " + get_cli_flag(b.cli, Cli_Opts::Force_Include, x.generic_string());

		if (b.flags.generate_debug)
			command +=
				" " + get_cli_flag(b.cli, Cli_Opts::Debug_Symbol_Compile, x.generic_string());

		if (b.arch == Build::Arch::x86) command += " " + get_cli_flag(b.cli, Cli_Opts::Arch_32);

		command += " " + c.generic_string();
		
		commands.add_command(command, "Compile " + x.filename().generic_string(), x, o);
	}
	return commands;
}


NS::Commands compile_assembly(const NS::Build_State& state, const NS::Build& b) noexcept {
	using namespace NS;
	using namespace details;
	NS::Commands commands;
	std::string command;

	// Compile to -o
	for (auto x : b.source_files) {

		auto c = x;

		std::filesystem::path o = NS::details::get_output_path(b).parent_path();

		// >SEE(Tackwin):
		// Here we ideally want to preverse the tree structure of the sources.
		// But i don't want to just spam std::filesystem::cretate_directories here.
		// so i don't know what i should do...
		o += unique_name(b, x) + ".s";
		o = o.lexically_normal();

		if (b.flags.link_only) continue;

		auto test_file = unique_name(b, x);

		// Check if this files has not changed
		// (so it must first exists)
		if (std::filesystem::is_regular_file(o) && state.files.count(test_file) > 0) continue;

		command = b.compiler.generic_string();
		command += " " + get_cli_flag(b.cli, Cli_Opts::Compile);
		command += " " + get_cli_flag(b.cli, Cli_Opts::Std_Version, b.std_ver);
		command += " " + get_cli_flag(b.cli, Cli_Opts::Assembly_Output, o.generic_string());

		if (b.flags.compile_native) command += " " + get_cli_flag(b.cli, Cli_Opts::Native);
		if (b.flags.no_simd)        command += " " + get_cli_flag(b.cli, Cli_Opts::No_SIMD);
		if (b.flags.openmp) command += " " + get_cli_flag(b.cli, Cli_Opts::OpenMP);
		if (b.flags.release) {
			std::string param = "3";
			if (b.flags.release_level) param = std::to_string(*b.flags.release_level);
			command += " " + get_cli_flag(b.cli, Cli_Opts::Optimisation, param);
		}
		else
			command += " " + get_cli_flag(b.cli, Cli_Opts::No_Optimisation);

		if (b.flags.no_inline) command += " " + get_cli_flag(b.cli, Cli_Opts::No_Inline);
		if (b.flags.fast_math) command += " " + get_cli_flag(b.cli, Cli_Opts::Fast_Math);


		for (auto& d : b.defines) command += " " + get_cli_flag(b.cli, Cli_Opts::Define, d);
		for (auto& d : b.flags.defines) command += " " + get_cli_flag(b.cli, Cli_Opts::Define, d);

		for (auto& x : b.header_files) if (std::filesystem::is_directory(x))
			command += " " + get_cli_flag(b.cli, Cli_Opts::Include, x.generic_string());

		for (auto& x : b.header_files) if (std::filesystem::is_regular_file(x))
			command += " " + get_cli_flag(b.cli, Cli_Opts::Force_Include, x.generic_string());

		if (b.flags.generate_debug)
			command +=
				" " + get_cli_flag(b.cli, Cli_Opts::Debug_Symbol_Compile, x.generic_string());
	
		if (b.arch == Build::Arch::x86) command += " " + get_cli_flag(b.cli, Cli_Opts::Arch_32);

		command += " " + c.generic_string();
		
		commands.add_command(command, "Assemble " + x.filename().generic_string(), x, o);
	}
	return commands;
}

NS::Commands compile_command_link_exe(const NS::Build& b) noexcept {
	using namespace NS;
	using namespace details;
	NS::Commands commands;
	std::string command;

	command = b.compiler.generic_string() + " ";

	if (b.target == Build::Target::Shared)
		command += get_cli_flag(b.cli, Cli_Opts::Link_Shared) + " ";
	
	command += get_cli_flag(b.cli, Cli_Opts::Stack_Size, std::to_string(b.stack_size)) + " ";

	for (auto x : b.source_files) {
		std::filesystem::path o = b.flags.get_build_path();
		o += unique_name(b, x) + ".o";
		o = o.lexically_normal();

		command += o.generic_string() + " ";
	}
	for (auto& x : b.link_files)
		command += get_cli_flag(b.cli, Cli_Opts::Link, x.generic_string()) + " ";
	for (auto& x : b.lib_path)
		command += get_cli_flag(b.cli, Cli_Opts::Lib_Path, x.generic_string()) + " ";

	if (b.flags.openmp) command += get_cli_flag(b.cli, Cli_Opts::OpenMP) + " ";
	if (b.flags.generate_debug)
		command += get_cli_flag(b.cli, Cli_Opts::Debug_Symbol_Link) + " ";
	if (b.flags.no_default_lib) command += get_cli_flag(b.cli, Cli_Opts::No_Default_Lib) + " ";

	if (b.arch == Build::Arch::x86) command += get_cli_flag(b.cli, Cli_Opts::Arch_32) + " ";

	auto exe_path = NS::details::get_output_path(b);
	if (b.target == Build::Target::Shared)
		exe_path = exe_path.replace_extension(".dll");

	command += get_cli_flag(b.cli, Cli_Opts::Exe_Output, exe_path.generic_string()) + " ";
	commands.add_command(command, "Link " + b.name, "", exe_path);
	return commands;
}

NS::Commands compile_command_link_static(const NS::Build& b) noexcept {
	NS::Commands commands;
	std::string command;

	command = b.archiver.generic_string() + " rcs ";
	auto static_path = NS::details::get_output_path(b).replace_extension("lib");
	command += static_path.generic_string() + " ";

	for (auto x : b.source_files) {
		std::filesystem::path o = b.flags.get_build_path();
		o += unique_name(b, x) + ".o";
		o = o.lexically_normal();

		command += o.generic_string() + " ";
	}

	commands.add_command(command, "Archive " + b.name, "", static_path);
	return commands;
}

NS::Commands compile_command_incremetal_check(const NS::Build& b) noexcept {
	using namespace NS;
	using namespace details;
	NS::Commands commands;
	std::string command;

	// First run preprocessor to chech for need to recompilation
	for (auto x : b.source_files) {
		auto f = x;
		std::filesystem::path p = b.flags.get_temp_path();
		p += unique_name(b, x) + ".pre";
		p = p.lexically_normal();

		command = b.compiler.generic_string();

		command += " " + get_cli_flag(b.cli, Cli_Opts::Preprocess);
		command += " " + get_cli_flag(b.cli, Cli_Opts::Std_Version, b.std_ver);
		command += " " + get_cli_flag(b.cli, Cli_Opts::Preprocessor_Output, p.generic_string());

		command += " " + get_cli_flag(b.cli, Cli_Opts::Native);
		if (b.flags.openmp) command += " " + get_cli_flag(b.cli, Cli_Opts::OpenMP);
		for (auto& d : b.defines) command += " " + get_cli_flag(b.cli, Cli_Opts::Define, d);
		for (auto& d : b.flags.defines) command += " " + get_cli_flag(b.cli, Cli_Opts::Define, d);

		for (auto& x : b.header_files) if (std::filesystem::is_directory(x))
			command += " " + get_cli_flag(b.cli, Cli_Opts::Include, x.generic_string());

		for (auto& x : b.header_files) if (std::filesystem::is_regular_file(x))
			command += " " + get_cli_flag(b.cli, Cli_Opts::Force_Include, x.generic_string());

		command += " " + f.generic_string();

		commands.add_command(command, "Preprocess " + x.filename().generic_string(), x, p);
	}

	return commands;
}

bool execute(const NS::Build& build, const NS::Commands& c) noexcept {
	#ifndef NO_THREAD
	std::vector<std::thread> threads;
	std::atomic<bool> stop_flag = false;

	for (size_t i = 0; i < build.flags.j; ++i) {
		threads.push_back(std::thread([&, i] {
			for (size_t j = i; j < c.entries.size(); j += build.flags.j) {
				if (stop_flag) return;

				if (build.flags.verbose_level >= 0 && !build.flags.silent) {
					printf(
						"[%*d/%*d] %s\n",
						(int)(1 + std::log10((double)c.entries.size())),
						(int)(1 + j),
						(int)(1 + std::log10((double)c.entries.size())),
						(int)c.entries.size(),
						c.entries[j].short_desc.c_str()
					);
				}
				if (build.flags.verbose_level > 0 && !build.flags.silent)
					printf("%s\n", c.entries[j].command.c_str());
				auto ret = system(c.entries[j].command.c_str());
			
				if (ret != 0) stop_flag = true;
			}
		}));
	}
	for (auto& x : threads) x.join();

	if (stop_flag && !build.flags.silent) {
		printf("There was an error in the build. There should be more informations above.");
	}
#else
	bool stop_flag = false;
	for (size_t i = 0; i < c.entries.size(); i++) {
		if (stop_flag) return;

		if (build.flags.verbose_level >= 0 && !build.flags.silent) {
			printf(
				"[%*d/%*d] %s\n",
				(int)(1 + std::log10((double)c.entries.size())),
				(int)(1 + i),
				(int)(1 + std::log10((double)c.entries.size())),
				(int)c.entries.size(),
				c.entries[i].short_desc.c_str()
			);
		}
		if (build.flags.verbose_level > 0 && !build.flags.silent)
			printf("%s\n", c.entries[i].command.c_str());
		auto ret = system(c.entries[i].command.c_str());
	
		if (ret != 0) stop_flag = true;
	}
	
	if (stop_flag) {
		printf("There was an error in the build. There should be more informations above.");
	}
#endif
	return !stop_flag;
}

void add_install_path(NS::Build& b) noexcept {
	auto dirs = NS::details::get_installed_dirs(b);
	for (auto& x : dirs) {
		b.add_header(x);
		b.add_library_path(x);
	}
}

void handle_build(Build& b, NS::States& new_states) noexcept {
	if (!b.flags.silent) printf("Building %s\n", b.name.c_str());

	if (!b.flags.no_install_path) add_install_path(b);

	NS::Build_State new_state = {};

	new_state.flags_hash = b.flags.hash();
	new_state.rebuild_flag_hash = b.flags.rebuild_hash();

	if (b.flags.scratch || new_state.rebuild_flag_hash != b.current_state.rebuild_flag_hash) {
		b.current_state = {};
		b.flags.scratch = true;
	}

	bool successful = true;
	switch (b.target) {
	case NS::Build::Target::Header_Only : {
		for (auto& x : b.pre_compile) execute(b, x);

		produce_single_header(b);
		b.to_install.push_back(NS::details::get_output_path(b).replace_extension("hpp"));

		for (auto& x : b.post_compile) execute(b, x);
		for (auto& x : b.pre_link) execute(b, x);
		for (auto& x : b.post_link) execute(b, x);
		break;
	}
	case NS::Build::Target::Assembly : {
		if (b.source_files.empty()) break;

		std::filesystem::create_directory(b.flags.get_build_path());

		for (auto& x : b.pre_compile) execute(b, x);

		::NS::Commands c;
		c = compile_assembly({}, b);
		execute(b, c);
		for (auto& x : b.post_compile) execute(b, x);

		break;
	}
	case NS::Build::Target::Custom :
	{
		for (auto& x : b.pre_compile) execute(b, x);
		for (auto& x : b.post_compile) execute(b, x);
		for (auto& x : b.pre_link) execute(b, x);
		for (auto& x : b.post_link) execute(b, x);
		if (b.flags.run_after_compilation && b.run_function) b.run_function();
		break;
	}
	case NS::Build::Target::Shared :
	case NS::Build::Target::Exe :
	case NS::Build::Target::Static :
	{
		if (b.source_files.empty()) break;

		std::filesystem::create_directory(b.flags.get_build_path());
		std::filesystem::create_directory(b.flags.get_temp_path());

		for (auto& x : b.pre_compile) successful &= execute(b, x);

		::NS::Commands c;

		if (!b.flags.no_compile_commands) {
			c = compile_command_object({}, b);
			auto p = b.flags.get_compile_commands_path();
			if (std::filesystem::is_directory(p)) p /= "compile_commands.json";
			c.save_command_json(p);
		}

		if (!b.flags.link_only) {
			if (b.flags.assembly) {
				c = compile_assembly({}, b);
				successful &= execute(b, c);
			}

			c = compile_command_incremetal_check(b);
			successful &= execute(b, c);
			set_files_hashes(b.flags.get_temp_path(), new_state);
			if (!b.flags.scratch)
				b.current_state = NS::Build_State::get_unchanged(b.current_state, new_state);

			c = compile_command_object(b.current_state, b);

			successful &= execute(b, c);
			for (auto& x : b.post_compile) successful &= execute(b, x);
		}
		if (b.target == NS::Build::Target::Static) {
			c = compile_command_link_static(b);
		} else {
			c = compile_command_link_exe(b);
		}
		// in the link phase we add the executable(s) to the install path.
		for (auto& x : c.entries) if (x.output) {
			b.to_install.push_back(*x.output);
		}

		for (auto& x : b.pre_link) successful &= execute(b, x);
		successful &= execute(b, c);
		for (auto& x : b.post_link) successful &= execute(b, x);

		if (b.target == NS::Build::Target::Exe && b.flags.run_after_compilation && successful)
		if (!b.run_function) {
			std::string run = NS::details::get_output_path(b).generic_string();
			for (auto& x : b.flags.rest_args) run += " " + x;
			if (!Env::Win32) run = "./" + run;
			if (!b.flags.silent) printf("Running %s\n", run.c_str());
			system(run.c_str());
		}

		std::filesystem::remove_all(b.flags.get_temp_path());
		break;
	}
	default:
		break;
	}
	if (b.flags.run_after_compilation && b.run_function && successful) b.run_function();

	if (b.flags.install) NS::details::install_build(b);

	if (b.next.b) handle_build(*b.next.b, new_states);
	
	new_states.states[b.name] = new_state;
}

std::string details::escape(std::string_view in) noexcept {
	std::string out;

	for (auto& c : in) {
		if (c == '"') out += '\\';
		out += c;
	}

	return '"' + out + '"';
}

// Just...
// https://stackoverflow.com/questions/61030383/how-to-convert-stdfilesystemfile-time-type-to-time-t
template <typename TP>
auto stupid_function_that_convert_between_unspecified_file_clock_to_system_clock_thank_cpp17(TP tp)
{
	using namespace std::chrono;
	return time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
}


#ifndef NAME
#define NAME build
#endif

int main(int argc, char** argv) {
	extern NS::Build NAME(NS::Flags flags) noexcept;

	if (argc > 2 && strcmp(argv[1], "__EASE__STAGE_1__") == 0) {
		std::string move_original = "mv ";
		move_original += details::escape(argv[2]) + " ";
		move_original += " __ease__old_exe";
		system(move_original.c_str());

		std::string command_copy = "mv ";
		command_copy += details::escape(argv[0]);
		command_copy += " ";
		command_copy += details::escape(argv[2]);

		if (system(command_copy.c_str()) == 0) {
			std::string stage2_command = details::escape(argv[2]);
			stage2_command += " __EASE__STAGE_2__ ";
			for (size_t i = 3; i < argc; ++i) {
				stage2_command += " ";
				stage2_command += details::escape(argv[i]);
			}

			stage2_command = "\"" + stage2_command + "\"";
			return system(stage2_command.c_str());
		}
		return -1;
	}
	if (argc > 1 && strcmp(argv[1], "__EASE__STAGE_2__") == 0) {
		std::string command_remove = "rm __ease__old_exe";

		system(command_remove.c_str());

		// Shift the command to continue as-if __EASE__STAGE_2__ were not passed
		// as args.
		argc -= 1;
		for (size_t i = 1; i < argc; ++i) argv[i] = argv[i + 1];
	}

	NS::Working_Directory = std::filesystem::absolute(std::filesystem::current_path());
	auto flags = NS::Flags::parse(argc - 1, argv + 1);

	auto b = NAME(flags);

	if (flags.show_help) {
		printf("%s", NS::Flags::help_message());
		return 0;
	}
	if (flags.show_help_install) {
		printf("%s", NS::Flags::help_install_message());
		return 0;
	}

	if (flags.verbose_level > 1) {
		printf("Running in %s.\n", NS::Working_Directory.generic_string().c_str());
	}

	if (flags.clean) {
		std::filesystem::remove_all(flags.get_build_path());
		std::filesystem::remove_all(flags.get_temp_path());
		return 0;
	}
	std::filesystem::create_directory(flags.get_build_path());
	std::filesystem::create_directory(flags.get_temp_path());

	NS::States old_states;
	NS::States new_states;
	if (!b.flags.scratch && std::filesystem::is_regular_file(flags.get_state_path()))
		old_states = NS::States::load_from_file(flags.get_state_path());

	if (!flags.no_watch_source_changed || flags.recompile_build_script) {

		// I don't want to comment on these stupid lines...
		// at least it's portable :^)
		auto stupid_inermediary_chrono_type_last_write =
			std::filesystem::last_write_time(NS::Env::Build_Script_Path);
		auto stupidier_inermediary_chrono_type_last_write =
			stupid_function_that_convert_between_unspecified_file_clock_to_system_clock_thank_cpp17(
				stupid_inermediary_chrono_type_last_write
			).time_since_epoch();
		auto last_write = std::chrono::duration_cast<std::chrono::seconds>(
			stupidier_inermediary_chrono_type_last_write
		).count();

		// WHEN C++ WILL SORT ITS SHIT OUT... We will be able to just use the UNIX_TIMESTAMP macro
		// here, and save us a variable in the state.txt

		if (last_write > old_states.last_write_build_script || flags.recompile_build_script) {
			if (!b.flags.silent)
				printf("Detected change in %s... Recompiling.\n", NS::Env::Build_Script_Path);

			old_states.last_write_build_script = last_write;
			old_states.save_to_file(flags.get_state_path());

			std::string command_stage;
			std::string compile_command;

			compile_command += "clang++ -std=c++17 -o __ease__stage1.exe ";
			compile_command += NS::Env::Build_Script_Path;
			
			if (system(compile_command.c_str()) == 0) {
				command_stage += "__ease__stage1.exe";
				command_stage += " __EASE__STAGE_1__";
				command_stage += " ";
				command_stage += details::escape(argv[0]);

				for (size_t i = 1; i < argc; ++i) {
					command_stage += " ";
					command_stage += details::escape(argv[i]);
				}

				return system(command_stage.c_str());
			} else if (!b.flags.silent) {
				printf(
					"There was a problem with the automatic recompilation of the build script: \n"
				);
				printf("    %s\n", NS::Env::Build_Script_Path);
				printf("Proceeding without recompilation...\n");
			}
		}

		new_states.last_write_build_script = old_states.last_write_build_script;
	}

	auto next_ptr = &b;
	while(next_ptr) {
		next_ptr->current_state = old_states.states[next_ptr->name];
		next_ptr = next_ptr->next.b;
	}

	handle_build(b, new_states);

	new_states.save_to_file(b.flags.get_state_path());

	return 0;
}
#define main __unused__;\
	const char* EASE_NAMESPACE::Env::Build_Script_Path = __FILE__;\
	Build build

std::string NS::details::get_cli_flag(
	NS::Build::Cli cli, NS::Cli_Opts opts, std::string_view param
) noexcept {
	#define X(a, b) \
	switch(cli) { \
		case NS::Build::Cli::Gcc : \
			return (a); \
		case NS::Build::Cli::Cl : \
			return (b); \
		default: \
			return "???"; \
	}

	using details::escape;

	switch (opts) {
	case NS::Cli_Opts::Preprocess :
		X("-E", "/P");
	case NS::Cli_Opts::No_Optimisation :
		X("-O0", "/O0");
	case NS::Cli_Opts::No_Inline :
		X("-fno-inline", "/Ob0");
	case NS::Cli_Opts::OpenMP :
		X("-fopenmp", "/OpenMP");
	case NS::Cli_Opts::Fast_Math :
		X("-ffast-math", "/fp:fast");
	case NS::Cli_Opts::Debug_Symbol_Link :
		X("-g -gno-column-info", "/DEBUG");
	case NS::Cli_Opts::Debug_Symbol_Compile :
		X("-g -gcodeview -gno-column-info", "/Z7");

	case NS::Cli_Opts::No_Default_Lib : {
		// >TODO(Tackwin): here we need to check the linker's cli instead of the compiler.
		X("-Xlinker /NODEFAULTLIB", "/NODEFAULTLIB");
	}

	// >TODO(Tackwin): The stack size is fucked
	// >TODO(Tackwin): I think there is a bug with the arch 32 cli opts.

	case NS::Cli_Opts::No_SIMD:
		X("-mno-sse", "");
	case NS::Cli_Opts::Time_Trace :
		X(std::string("-ftime-trace"), "");
	case NS::Cli_Opts::Stack_Size :
		// X(std::string("-Wl,-stack_size -Wl,") + param.data(), "");
	case NS::Cli_Opts::Arch_32 :
		// X(std::string("-m32"), "");
	case NS::Cli_Opts::Native :
		X(std::string("-march=native"), "");
	case NS::Cli_Opts::Compile :
		X(std::string("-c"), std::string("/c"));
	case NS::Cli_Opts::Link_Shared :
		X(std::string("-shared"), std::string("/DLL"));
	case NS::Cli_Opts::Link :
		X(std::string("-l") + param.data(), std::string("???"));
	case NS::Cli_Opts::Optimisation :
		X(std::string("-O") + param.data(), std::string("/O") + param.data());
	case NS::Cli_Opts::Define :
		X(std::string("-D") + param.data(), std::string("/D") + param.data());
	case NS::Cli_Opts::Include :
		X(std::string("-I") + escape(param), std::string("/I") + escape(param));
	case NS::Cli_Opts::Std_Version :
		X(std::string("-std=") + param.data(), std::string("/std:") + param.data());
	case NS::Cli_Opts::Lib_Path :
		X(std::string("-L") + escape(param), std::string("/LIBPATH:") + escape(param));
	case NS::Cli_Opts::Object_Output :
		X(std::string("-o ") + param.data(), std::string("/Fo\"") + param.data() + "\"");
	case NS::Cli_Opts::Exe_Output :
		X(std::string("-o ") + param.data(), std::string("/Fo\"") + param.data() + "\"");
	case NS::Cli_Opts::Assembly_Output :
		X(std::string("-S -o ") + param.data(), "??");
	case NS::Cli_Opts::Preprocessor_Output :
		X(std::string("-o ") + param.data(), std::string("/Fi\"") + param.data() + "\"");
	case NS::Cli_Opts::Force_Include :
		X(std::string("-include ") + param.data(), std::string("/FI\"") + param.data() + "\"");
	default:
		break;
	}

	#undef X
}

void dump_to_file(std::string_view str, const std::filesystem::path& p) noexcept {
	FILE* f = fopen(p.generic_string().c_str(), "w");
	fprintf(f, "%s", str.data());
	fclose(f);
}

std::string read_file(const std::filesystem::path& p) noexcept {
	FILE *f = fopen(p.generic_string().c_str(), "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

	std::string string;
	string.resize(fsize);
	fread(string.data(), 1, fsize, f);
	fclose(f);

	return string;
}

std::uint64_t hash(const std::string& str) noexcept {
	return (std::uint64_t)std::hash<std::string>()(str);
}

constexpr char BASE64_TABLE[] =  {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                  'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                  '4', '5', '6', '7', '8', '9', '+', '-'};

std::string unique_name(const Build& build, const std::filesystem::path& path) noexcept {
	std::string ret = path.filename().generic_string();
	auto h = hash(path.generic_string());

	ret += "_";
	ret += BASE64_TABLE[h % 64];
	ret += BASE64_TABLE[(h / 64) % 64];
	ret += BASE64_TABLE[(h / (64 * 64)) % 64];

	return build.name + ret;
}

#undef NS
#ifndef EASE_HPP
#include "Ease.hpp"
#endif

#include <stdlib.h>
#include <unordered_set>
#include <fstream>

#ifdef _WIN32
#define setenv(x, y, z) ((int)_putenv_s((x), (y)));
#endif

#define NS EASE_NAMESPACE

struct fs_hash {
	std::size_t operator()(const std::filesystem::path& p) const {
		return std::filesystem::hash_value(p);
	}
};

std::filesystem::path get_user_path() noexcept;

#ifdef _WIN32
std::filesystem::path get_user_data_path() noexcept {
	auto path = std::getenv("LOCALAPPDATA");
	if (!path) {
		printf("Error retrieving env variable.\n");
		return "/";
	} else {
		return path;
	}
}
#else
std::filesystem::path get_user_data_path() noexcept {
	return "~";
}
#endif

struct Install_State {
	std::unordered_set<std::filesystem::path, fs_hash> dirs;

	static std::optional<Install_State> open(const std::filesystem::path& path) noexcept {
		Install_State state;
		std::ifstream in_file;
		in_file.open(path);
		if (in_file.good()) {
			for (std::string line; std::getline(in_file, line);) if (!line.empty()) {
				state.dirs.insert(line);
			}
		} else {
			return std::nullopt;
		}
		in_file.close();
		return state;
	}

	void save(const std::filesystem::path& path) noexcept {
		std::ofstream out_file;
		out_file.open(path);
		for (auto& x : dirs) {
			out_file << x.generic_string() << "\n";
		}
		out_file.close();
	}
};



void NS::details::install_build(const NS::Build& b) noexcept {
	std::filesystem::create_directories("install");

	auto opt_state = Install_State::open(get_user_data_path() / ".ease/install.txt");
	if (!opt_state) {
		std::filesystem::create_directories(get_user_data_path() / ".ease/");
		opt_state = Install_State{};
	}
	Install_State state = *opt_state;


	state.dirs.insert(Working_Directory / "install");
	for (auto& x : b.to_install) if (std::filesystem::is_regular_file(x)) {
		
		auto d = x;
		d = d.lexically_normal();
		d = Working_Directory / "install" / d;
		d = d.lexically_normal();
		if (b.flags.verbose_level > 0) {
			printf(
				"Installing %s to %s.\n", x.generic_string().c_str(), d.generic_string().c_str()
			);
		}

		std::filesystem::create_directories(d.parent_path());
		std::filesystem::copy_file(
			x,
			d,
			std::filesystem::copy_options::overwrite_existing
		);
	}

	for (size_t i = 0; i < b.export_files.size(); ++i) {
		auto d = b.export_dest_files[i];
		d = d.lexically_normal();
		d = Working_Directory / "install" / d;
		d = d.lexically_normal();
		if (b.flags.verbose_level > 0) {
			printf(
				"Installing %s to %s\n",
				b.export_files[i].generic_string().c_str(),
				d.generic_string().c_str()
			);
		}

		auto opts = std::filesystem::copy_options::overwrite_existing;
		if (std::filesystem::is_directory(b.export_files[i]))
			opts = std::filesystem::copy_options::recursive;

		std::filesystem::create_directories(d.parent_path());
		std::filesystem::copy(b.export_files[i], d, opts);
	} 

	state.save(get_user_data_path() / ".ease/install.txt");
}


std::vector<std::filesystem::path> NS::details::get_installed_dirs(const Build& b) noexcept {
	auto opt_state = Install_State::open(get_user_data_path() / ".ease/install.txt");
	if (opt_state) {
		return std::vector<std::filesystem::path>(
			std::begin(opt_state->dirs), std::end(opt_state->dirs)
		);
	}
	return {};
}

#undef NS
#endif
