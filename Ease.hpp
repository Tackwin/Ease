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
#include <filesystem>

#ifndef EASE_NAMESPCE
#define EASE_NAMESPACE Ease
#endif

namespace EASE_NAMESPACE {

static std::filesystem::path Working_Directory;

struct Flags {

	// I want to make optional bool because user code might want to knwo if a user set someting in
	// the cmd.
	// For now i'm keeping it as everything is false by default, since you can only set a flag to
	// true it should be enough ?
	bool clean = false;
	bool release = false;
	bool scratch = false;
	bool install = false;
	bool show_help = false;
	bool link_only = false;
	bool no_inline = false;
	bool generate_debug = false;
	bool no_install_path = false;
	bool show_help_install = false;
	bool no_compile_commands = false;
	bool run_after_compilation = false;

	size_t j = 0;
	size_t verbose_level = 0;

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
};

struct State {
	size_t flags_hash = 0;
	std::map<std::filesystem::path, std::uint64_t> files;

	static State load_from_file(const std::filesystem::path& p) noexcept;
	static State get_unchanged(const State& a, const State& b) noexcept;
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
		Static,
		Shared,
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

	Flags flags;

	Cli cli;
	Arch arch;
	Target target;
	std::string_view std_ver;

	bool invert_header_implementation_define = false;

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

	Build_Ptr next;

	static Build get_default(Flags flags = {}) noexcept;

	void add_header(const std::filesystem::path& f) noexcept;
	void add_source(const std::filesystem::path& f) noexcept;
	void add_source_recursively(const std::filesystem::path& f) noexcept;

	void add_library(const std::filesystem::path& f) noexcept;
	void add_library_path(const std::filesystem::path& f) noexcept;

	void add_export(const std::filesystem::path& f) noexcept;
	void add_export(const std::filesystem::path& from, const std::filesystem::path& to) noexcept;

	void add_define(std::string str) noexcept;

	void add_default_win32() noexcept;
};

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
	Lib_Path,
	Define,
	Optimisation,
	No_Optimisation,
	Preprocess,
	Arch,
	Debug_Symbol_Compile,
	Debug_Symbol_Link,
	No_Inline
};


struct Env {
	static constexpr bool Win32 =
		#ifdef _WIN32
		true;
		#else
		false;
		#endif
};

namespace details {
	std::string get_cli_flag(
		Build::Cli cli, Cli_Opts opts, std::string_view param = ""
	) noexcept;
	void install_build(const Build& b) noexcept;
	std::vector<std::filesystem::path> get_installed_dirs(const Build& b) noexcept;
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

#ifndef EASE_HPP
#include "Ease.hpp"
#endif

#define NS EASE_NAMESPACE

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
std::string unique_name(const std::filesystem::path& path) noexcept;

// FILE IO
void dump_to_file(std::string_view str, const std::filesystem::path& p) noexcept;
std::string read_file(const std::filesystem::path& p) noexcept;

// ===================== FLAGS
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
		if (strcmp(it, "--no-install-path") == 0) {
			flags.no_install_path = true;
		}
		if (strcmp(it, "--no-compile-commands") == 0) {
			flags.no_compile_commands = true;
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
	}

	if (flags.j < 1) flags.j = 1;

	return flags;
}

const char* NS::Flags::help_message() noexcept {
	return
	" ====================================== Optional flags ======================================\n"
	"--run                        Will run the program (if the program is runnable) after the\n"
	"                             compilation.\n"
	"                    Exemple: ./Build.exe --run\n\n"

	"--clean                      Will clean every file or directory generated by this software.\n"
	"                    Exemple: ./Build.exe --clean\n\n"
	
	"-h|--help                    Will show this message, then exit.\n"
	"                    Exemple: ./Build.exe -h | ./Build.exe --help\n\n"

	"--help-install               Print informations about the install system.\n"
	"                    Exemple: ./Build.exe --help-install\n\n"
	
	"-j <n>                        Will use n threads.\n"
	"                    Exemple: ./Build.exe -j 4\n\n"

	"-l|--link                    Will only link the program, then exit. There must be a complete\n"
	"                             state available. Will not check for incremental compilation.\n"
	"                    Exemple: ./Build.exe -l | ./Build.exe --link\n\n"

	"--verbose <level>            Change the level of verbosity.\n"
	"                             Level 0 (default) will show a simple progress message.\n"
	"                             Level 1 will show with each stage the commands executed.\n"
	"                             Level 2 will show additional informations.\n"
	"                    Exemple: ./Build.exe --verbose 0 | ./Build.exe --verbose 1\n\n"

	"--release [level]            Will compile the program using default option for release\n"
	"                             (for instance, if the cli choosen is gcc, will compile using -O3)\n"
	"                             if level is specified it will use the appropriate level of\n"
	"                             optimisation."
	"                    Exemple: ./Build.exe --release | ./Build.exe --release 1\n\n"

	"--install                    Will install the program to be referenced by the EASE_PATH\n"
	"                             Environment variable, for more information you can run me with\n"
	"                             the flag --help-env\n"
	"                    Exemple: ./Build.exe --install\n\n"

	"--build-dir <path>           Change the default build path used by Ease to keep state across\n"
	"                             runs. Default to ease_build/"
	"                    Exemple: ./Build.exe --build-dir new_build/path/\n\n"
	
	"--temp-dir <path>            Change the default temp path used by Ease, it is created and \n"
	"                             destroyed each run of the build program. Default to ease_temp/"
	"                    Exemple: ./Build.exe --temp-dir new_temp/path/\n\n"
	
	"--no-install-path            Will *not* search your install variable for files to include\n"
	"                             and or directory to look for librairies.\n"
	"                    Exemple: ./Build.exe --no-install-path\n\n"
	
	"--debug                      Will generate alongside the program debug symbols\n"
	"                             It is not mutually exclusive with --release"
	"                    Exemple: ./Build.exe --debug\n\n"
	
	"--no-inline                  Will set the compiler flag to not inline any function, if\n"
	"                             availabe.\n"
	"                    Exemple: ./Build.exe --no-inline\n\n"
	
	"--scratch                    Disable incremental compilation and will clean the state file\n"
	"                    Exemple: ./Build.exe --scratch\n\n"
	
	"--no-compile-commands        Will not generate a compile_commands.json file.\n"
	"                    Exemple: ./Build.exe --no-compile-commands\n\n"
	
	"--compile-commands <path>    Set the path to generate compile_commands.json file. If <path> is\n"
	"                             a directory the file will be named compile_commands.json else it\n"
	"                             will take the name of the file specified by <path>"
	"                    Exemple: ./Build.exe --compile-commands build_dir/ |\n"
	"                    Exemple: ./Build.exe --compile-commands build_dir/my_file.json \n\n"
	
	"--state-file <path>          Will use <path> as the path to save the state file. If <path>\n"
	"                             represent a file then the state file will take exactly it's name\n"
	"                             but if it represents a directory the state file will be named\n"
	"                             state.txt .  By defualt the state file is located in the\n"
	"                             build directory and is named state.txt\n"
	"                    Exemple: ./Build.exe --state-file ./my_dir/other_file.ext |\n"
	"                    Exemple: ./Build.exe --state-file ./my_dir/ \n\n"
	
	"-o|--output <path>           Will use <path> as the path to save the executable file if\n"
	"                             applicable if <path> represent a file then the output will be\n"
	"                             named Run and be located in <path>. If it represents a file\n"
	"                             Then the ouput will be saved as <path>\n"
	"                    Exemple: ./Build.exe --output ./my_dir/other_file.ext |\n"
	"                    Exemple: ./Build.exe --output ./my_dir/               |\n"
	"                    Exemple: ./Build.exe -o       ./my_dir/other_file.ext |\n"
	"                    Exemple: ./Build.exe -o       ./my_dir/ \n"
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


size_t NS::Flags::hash() const noexcept {
	auto combine = [](auto seed, auto v) {
		std::hash<std::remove_cv_t<decltype(v)>> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
		return seed;
	};

	size_t h = 0;
	h = combine(h, clean);
	h = combine(h, release);
	h = combine(h, scratch);
	h = combine(h, install);
	h = combine(h, no_inline);
	h = combine(h, show_help);
	h = combine(h, link_only);
	h = combine(h, generate_debug);
	h = combine(h, no_install_path);
	h = combine(h, show_help_install);
	h = combine(h, no_compile_commands);
	h = combine(h, run_after_compilation);
	h = combine(h, j);
	h = combine(h, verbose_level);
	h = combine(h, state_file);
	h = combine(h, output);
	h = combine(h, build_path);
	h = combine(h, temp_path);
	h = combine(h, compile_command_path);
	h = combine(h, release_level);

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
	build.std_ver = "c++17";
	build.compiler = "clang++";
	build.archiver = "llvm-ar";
	build.arch = Arch::x64;
	build.name = "Run";
	build.flags = flags;
	return build;
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
void NS::Build::add_header(const std::filesystem::path& f) noexcept {
	auto x = f;
	x = x.lexically_normal();
	header_files.push_back(x);
}
void NS::Build::add_define(std::string str) noexcept {
	defines.push_back(std::move(str));
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

std::filesystem::path NS::details::get_output_path(const Build& b) noexcept {
	std::filesystem::path out = b.name;
	if (b.flags.output) {
		out = *b.flags.output;
		if (std::filesystem::is_directory(*b.flags.output)) {
			out /= b.name;
		}
	}

	return out;
}

void NS::Build::add_default_win32() noexcept {
	add_library("Shell32");
	add_library("Ole32");
}


// ===================== BUILD

// ===================== State

NS::State NS::State::load_from_file(const std::filesystem::path& p) noexcept {
	NS::State state;
	auto text = read_file(p);

	std::istringstream stream(text);
	std::string line;

	if (std::getline(stream, line)) std::sscanf(line.c_str(), "%zu", &state.flags_hash);

	while(std::getline(stream, line)) {
		trim(line);
		std::filesystem::path f = line;
		f = f.lexically_normal();
		auto& p = state.files[f];

		if (std::getline(stream, line)) {
			p = (std::uint64_t)std::stoull(line);
		}
	}
	
	return state;
}
NS::State NS::State::get_unchanged(const State& a, const State& b) noexcept {
	NS::State s;

	for (auto [k, v] : a.files) if (b.files.count(k) > 0 && b.files.at(k) == v) s.files[k] = v;

	return s;
}

void NS::State::save_to_file(const std::filesystem::path& p) noexcept {
	std::string to_dump = "";
	to_dump += std::to_string(flags_hash) + "\n";
	for (auto& [a, b] : files) {
		to_dump += a.generic_string() + "\n";
		to_dump += std::to_string(b) + "\n";
	}

	dump_to_file(to_dump, p);
}
NS::State set_files_hashes(const std::filesystem::path& p, NS::State& s) noexcept {
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

NS::Commands compile_command_object(const NS::State& state, const NS::Build& b) noexcept {
	using namespace NS;
	using namespace details;
	NS::Commands commands;
	std::string command;

	// Compile to -o
	for (auto x : b.source_files) {

		auto c = x;

		std::filesystem::path o = b.flags.get_build_path();
		o += unique_name(x) + ".o";
		o = o.lexically_normal();

		if (b.flags.link_only) continue;

		auto test_file = unique_name(x);

		// Check if this files has not changed
		// (so it must first exists)
		if (std::filesystem::is_regular_file(o) && state.files.count(test_file) > 0) continue;

		command = b.compiler.generic_string();
		command += " " + get_cli_flag(b.cli, Cli_Opts::Compile);
		command += " " + get_cli_flag(b.cli, Cli_Opts::Std_Version, b.std_ver);

		if (b.flags.release){
			std::string param = "3";
			if (b.flags.release_level) param = std::to_string(*b.flags.release_level);
			command += " " + get_cli_flag(b.cli, Cli_Opts::Optimisation, param);
		}
		else
			command += " " + get_cli_flag(b.cli, Cli_Opts::No_Optimisation);

		if (b.flags.no_inline) command += " " + get_cli_flag(b.cli, Cli_Opts::No_Inline);

		command += " " + get_cli_flag(b.cli, Cli_Opts::Object_Output, o.generic_string());

		for (auto& d : b.defines) command += " " + get_cli_flag(b.cli, Cli_Opts::Define, d);

		for (auto& x : b.header_files) if (std::filesystem::is_directory(x))
			command += " " + get_cli_flag(b.cli, Cli_Opts::Include, x.generic_string());

		for (auto& x : b.header_files) if (std::filesystem::is_regular_file(x))
			command += " " + get_cli_flag(b.cli, Cli_Opts::Force_Include, x.generic_string());

		if (b.flags.generate_debug)
			command +=
				" " + get_cli_flag(b.cli, Cli_Opts::Debug_Symbol_Compile, x.generic_string());

		command += " " + c.generic_string();
		
		commands.add_command(command, "Compile " + x.filename().generic_string(), x, o);
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
	
	for (auto x : b.source_files) {
		std::filesystem::path o = b.flags.get_build_path();
		o += unique_name(x) + ".o";
		o = o.lexically_normal();

		command += o.generic_string() + " ";
	}
	for (auto& x : b.link_files)
		command += get_cli_flag(b.cli, Cli_Opts::Link, x.generic_string()) + " ";
	for (auto& x : b.lib_path)
		command += get_cli_flag(b.cli, Cli_Opts::Lib_Path, x.generic_string()) + " ";


	if (b.flags.generate_debug)
		command += get_cli_flag(b.cli, Cli_Opts::Debug_Symbol_Link) + " ";

	auto exe_path = NS::details::get_output_path(b).replace_extension("exe");

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
		o += unique_name(x) + ".o";
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
		p += unique_name(x) + ".pre";
		p = p.lexically_normal();

		command = b.compiler.generic_string();

		command += " " + get_cli_flag(b.cli, Cli_Opts::Preprocess);
		command += " " + get_cli_flag(b.cli, Cli_Opts::Std_Version, b.std_ver);
		command += " " + get_cli_flag(b.cli, Cli_Opts::Preprocessor_Output, p.generic_string());

		for (auto& d : b.defines)
			command += " " + get_cli_flag(b.cli, Cli_Opts::Define, d);

		for (auto& x : b.header_files) if (std::filesystem::is_directory(x))
			command += " " + get_cli_flag(b.cli, Cli_Opts::Include, x.generic_string());

		for (auto& x : b.header_files) if (std::filesystem::is_regular_file(x))
			command += " " + get_cli_flag(b.cli, Cli_Opts::Force_Include, x.generic_string());

		command += " " + f.generic_string();

		commands.add_command(command, "Preprocess " + x.filename().generic_string(), x, p);
	}

	return commands;
}

void execute(const NS::Build& build, const NS::Commands& c) noexcept {
	std::vector<std::thread> threads;
	std::atomic<bool> stop_flag = false;

	for (size_t i = 0; i < build.flags.j; ++i) {
		threads.push_back(std::thread([&, i] {
			for (size_t j = i; j < c.entries.size(); j += build.flags.j) {
				if (stop_flag) return;

				printf(
					"[%*d/%*d] %s\n",
					(int)(1 + std::log10((double)c.entries.size())),
					(int)(1 + j),
					(int)(1 + std::log10((double)c.entries.size())),
					(int)c.entries.size(),
					c.entries[j].short_desc.c_str()
				);
				if (build.flags.verbose_level > 0) printf("%s\n", c.entries[j].command.c_str());
				auto ret = system(c.entries[j].command.c_str());
			
				if (ret != 0) stop_flag = true;
			}
		}));
	}
	for (auto& x : threads) x.join();

	if (stop_flag) {
		printf("There was an error in the build. There should be more informations above.");
	}
}

void add_install_path(NS::Build& b) noexcept {
	auto dirs = NS::details::get_installed_dirs(b);
	for (auto& x : dirs) {
		b.add_header(x);
		b.add_library_path(x);
	}
}

void handle_build(Build& b) noexcept;
void handle_build(Build& b) noexcept {
	printf("Building %s\n", b.name.c_str());

	if (!b.flags.no_install_path) add_install_path(b);

	NS::State old_state = {};
	NS::State new_state = {};

	new_state.flags_hash = b.flags.hash();
	if (!b.flags.scratch && std::filesystem::is_regular_file(b.flags.get_state_path()))
		old_state = NS::State::load_from_file(b.flags.get_state_path());

	if (new_state.flags_hash != old_state.flags_hash) {
		old_state = {};
		b.flags.scratch = true;
	}

	std::string to_dump;
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
	case NS::Build::Target::Exe :
	case NS::Build::Target::Static :
	{
		if (b.source_files.empty()) break;

		std::filesystem::create_directory(b.flags.get_build_path());
		std::filesystem::create_directory(b.flags.get_temp_path());

		for (auto& x : b.pre_compile) execute(b, x);

		::NS::Commands c;

		if (!b.flags.no_compile_commands) {
			c = compile_command_object({}, b);
			auto p = b.flags.get_compile_commands_path();
			if (std::filesystem::is_directory(p)) p /= "compile_commands.json";
			c.save_command_json(p);
		}

		if (!b.flags.link_only) {
			c = compile_command_incremetal_check(b);
			execute(b, c);
			set_files_hashes(b.flags.get_temp_path(), new_state);
			if (!b.flags.scratch) old_state = NS::State::get_unchanged(old_state, new_state);

			for (auto& x : b.pre_compile) execute(b, x);
			c = compile_command_object(old_state, b);
			for (auto& x : b.post_compile) execute(b, x);

			execute(b, c);
		}
		if (b.target == NS::Build::Target::Exe) {
			c = compile_command_link_exe(b);
		} else {
			c = compile_command_link_static(b);
		}
		// in the link phase we add the executable(s) to the install path.
		for (auto& x : c.entries) if (x.output) {
			b.to_install.push_back(*x.output);
		}

		for (auto& x : b.pre_link) execute(b, x);
		execute(b, c);
		for (auto& x : b.post_link) execute(b, x);

		new_state.save_to_file(b.flags.get_state_path());

		if (b.target == NS::Build::Target::Exe && b.flags.run_after_compilation) {
			std::string run =
				NS::details::get_output_path(b).replace_extension("exe").generic_string();
			system(run.c_str());
		}

		std::filesystem::remove_all(b.flags.get_temp_path());
		break;
	}
	default:
		break;
	}

	if (b.flags.install) NS::details::install_build(b);

	if (b.next.b) handle_build(*b.next.b);
}

int main(int argc, char** argv) {
	extern NS::Build build(NS::Flags flags) noexcept;
	auto flags = NS::Flags::parse(argc - 1, argv + 1);

	auto b = build(flags);
	NS::Working_Directory = std::filesystem::absolute(std::filesystem::current_path());

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

	handle_build(b);

	return 0;
}
#define main dummy_main

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


	switch (opts) {
	case NS::Cli_Opts::Preprocess :
		X("-E", "/P");
	case NS::Cli_Opts::No_Optimisation :
		X("-O0", "/O0");
	case NS::Cli_Opts::No_Inline :
		X("-fno-inline", "/Ob0");
	case NS::Cli_Opts::Debug_Symbol_Link :
		X("-g -gcodeview -gno-column-info", "/DEBUG");
	case NS::Cli_Opts::Debug_Symbol_Compile :
		X("-g -gcodeview -gno-column-info", "/Z7");

	case NS::Cli_Opts::Arch :
		X(std::string("-m32"), "");
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
		X(std::string("-I") + param.data(), std::string("/I") + param.data());
	case NS::Cli_Opts::Std_Version :
		X(std::string("-std=") + param.data(), std::string("/std:") + param.data());
	case NS::Cli_Opts::Lib_Path :
		X(std::string("-L") + param.data(), std::string("/LIBPATH:") + param.data());
	case NS::Cli_Opts::Object_Output :
		X(std::string("-o ") + param.data(), std::string("/Fo\"") + param.data() + "\"");
	case NS::Cli_Opts::Exe_Output :
		X(std::string("-o ") + param.data(), std::string("/Fo\"") + param.data() + "\"");
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

std::string unique_name(const std::filesystem::path& path) noexcept {
	std::string ret = path.filename().generic_string();
	auto h = hash(ret);

	ret += "_";
	ret += BASE64_TABLE[h % 64];
	ret += BASE64_TABLE[(h / 64) % 64];
	ret += BASE64_TABLE[(h / (64 * 64)) % 64];

	return ret;
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
