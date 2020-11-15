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

	bool clean = false;
	bool release = false;
	bool scratch = false;
	bool install = false;
	bool show_help = false;
	bool link_only = false;
	bool generate_debug = false;
	bool no_install_path = false;
	bool show_help_install = false;
	bool run_after_compilation = false;

	size_t j = 0;
	size_t verbose_level = 0;

	std::optional<std::filesystem::path> state_file;
	std::optional<std::filesystem::path> output;

	static Flags parse(int argc, char** argv) noexcept;
	static const char* help_message() noexcept;
	static const char* help_install_message() noexcept;
};

struct State {
	std::map<std::filesystem::path, std::uint64_t> files;

	static State load_from_file(const std::filesystem::path& p) noexcept;
	static State get_unchanged(const State& a, const State& b) noexcept;
	void save_to_file(const std::filesystem::path& p) noexcept;
};

struct Commands {
	std::vector<std::string> commands;
	std::vector<std::string> short_desc;

	std::vector<std::filesystem::path> file_output;

	void add_command(std::string c) noexcept;
	void add_command(std::string c, std::string desc) noexcept;
	void add_command(std::string c, std::string desc, std::filesystem::path out) noexcept;
};

struct Build {
	enum class Target {
		Header_Only,
		Static,
		Exe
	};

	enum class Cli {
		Cl = 0,
		Gcc,
		Count
	};

	std::string name;

	Flags flags;
	std::filesystem::path state_file;

	Cli cli;
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
	Preprocess,
	Debug_Symbol_Compile,
	Debug_Symbol_Link
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

std::filesystem::path Default_State_File = "build/state.txt";

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
		}
		if (strcmp(it, "--scratch") == 0) {
			flags.scratch = true;
		}
		if (strcmp(it, "--no-install-path") == 0) {
			flags.no_install_path = true;
		}
		if (strcmp(it, "--state-file") == 0) {
			if (i + 1 >= argc) continue;
			i++;
			flags.state_file = argv[i];
			flags.state_file = flags.state_file->lexically_normal();
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

	"--release                    Will compile the program using default option for release\n"
	"                             (for instance, if the cli choosen is gcc, will compile using -O3)\n"
	"                    Exemple: ./Build.exe --release\n\n"

	"--install                    Will install the program to be referenced by the EASE_PATH\n"
	"                             Environment variable, for more information you can run me with\n"
	"                             the flag --help-env\n"
	"                    Exemple: ./Build.exe --install\n\n"
	
	"--no-install-path            Will *not* search your install variable for files to include\n"
	"                             and or directory to look for librairies.\n"
	"                    Exemple: ./Build.exe --no-install-path\n\n"
	
	"--debug                      Will generate alongside the program debug symbols\n"
	"                             It is not mutually exclusive with --release"
	"                    Exemple: ./Build.exe --debug\n\n"
	
	"--scratch                    Disable incremental compilation and will clean the state file\n"
	"                    Exemple: ./Build.exe --scratch\n\n"
	
	"--state-file <path>          Will use <path> as the path to save the state file. If <path>\n"
	"                             represent a file then the state file will take exactly it's name\n"
	"                             but if it represents a directory the state file will be named\n"
	"                             state.txt\n"
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

// ===================== FLAGS

// ===================== BUILD
NS::Build NS::Build::get_default(::NS::Flags flags) noexcept {
	NS::Build build;
	build.cli = Cli::Gcc;
	build.target = Target::Exe;
	build.std_ver = "c++17";
	build.compiler = "clang++";
	build.archiver = "llvm-ar";
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
	for (auto& [a, b] : files) {
		to_dump += a.generic_string() + "\n";
		to_dump += std::to_string(b) + "\n";
	}

	dump_to_file(to_dump, p);
}
NS::State construct_new_state(const std::filesystem::path& p) noexcept {
	NS::State s;
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

void NS::Commands::add_command(std::string c) noexcept {
	add_command(c, std::move(c));
}

void NS::Commands::add_command(std::string c, std::string desc) noexcept {
	commands.emplace_back(std::move(c));
	short_desc.emplace_back(std::move(desc));
}

void NS::Commands::add_command(
	std::string c, std::string desc, std::filesystem::path out
) noexcept {
	commands.emplace_back(std::move(c));
	short_desc.emplace_back(std::move(desc));
	file_output.emplace_back(std::move(out));
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

NS::Commands compile_command_object(NS::State& state, const NS::Build& b) noexcept {
	using namespace NS;
	using namespace details;
	NS::Commands commands;
	std::string command;

	// Compile to -o
	for (auto x : b.source_files) {

		auto c = x;

		std::filesystem::path o = "build/";
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

		if (b.flags.release)
			command += " " + get_cli_flag(b.cli, Cli_Opts::Optimisation);

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

		commands.add_command(command, "Compile " + x.filename().generic_string());
	}
	return commands;
}

NS::Commands compile_command_link_exe(const NS::Build& b) noexcept {
	using namespace NS;
	using namespace details;
	NS::Commands commands;
	std::string command;

	command = b.compiler.generic_string() + " ";
	for (auto x : b.source_files) {
		std::filesystem::path o = "build/";
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
	commands.add_command(command, "Link " + b.name, exe_path);
	return commands;
}

NS::Commands compile_command_link_static(const NS::Build& b) noexcept {
	NS::Commands commands;
	std::string command;

	command = b.archiver.generic_string() + " rcs ";
	auto static_path = NS::details::get_output_path(b).replace_extension("lib");
	command += static_path.generic_string() + " ";

	for (auto x : b.source_files) {
		std::filesystem::path o = "build/";
		o += unique_name(x) + ".o";
		o = o.lexically_normal();

		command += o.generic_string() + " ";
	}

	commands.add_command(command, "Archive " + b.name, static_path);
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
		std::filesystem::path p = "temp/";
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

		commands.add_command(command, "Preprocess " + x.filename().generic_string());
	}

	return commands;
}

void execute(const NS::Build& build, const NS::Commands& c) noexcept {
	std::vector<std::thread> threads;
	std::atomic<bool> stop_flag = false;

	for (size_t i = 0; i < build.flags.j; ++i) {
		threads.push_back(std::thread([&, i] {
			for (size_t j = i; j < c.commands.size(); j += build.flags.j) {
				if (stop_flag) return;

				printf(
					"[%*d/%*d] %s\n",
					(int)(1 + std::log10((double)c.commands.size())),
					(int)(1 + j),
					(int)(1 + std::log10((double)c.commands.size())),
					(int)c.commands.size(),
					c.short_desc[j].c_str()
				);
				if (build.flags.verbose_level > 0) printf("%s\n", c.commands[j].c_str());
				auto ret = system(c.commands[j].c_str());
			
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
		std::filesystem::remove_all("build");
		std::filesystem::remove_all("temp");
		return 0;
	}

	if (!flags.no_install_path) add_install_path(b);

	NS::State old_state = {};
	NS::State new_state = {};

	if (!b.flags.state_file) {
		b.state_file = Default_State_File;
	}

	if (!b.flags.scratch && std::filesystem::is_regular_file(b.state_file))
		old_state = NS::State::load_from_file(b.state_file);

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

		std::filesystem::create_directory("build");
		std::filesystem::create_directory("temp");

		for (auto& x : b.pre_compile) execute(b, x);

		::NS::Commands c;
		if (!b.flags.link_only) {
			c = compile_command_incremetal_check(b);
			execute(b, c);
			new_state = construct_new_state("temp");
			if (!flags.scratch) old_state = NS::State::get_unchanged(old_state, new_state);

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
		for (auto& x : c.file_output) {
			b.to_install.push_back(x);
		}

		for (auto& x : b.pre_link) execute(b, x);
		execute(b, c);
		for (auto& x : b.post_link) execute(b, x);

		new_state.save_to_file(b.state_file);

		if (b.target == NS::Build::Target::Exe && flags.run_after_compilation) {
			std::string run = b.name + ".exe";
			system(run.c_str());
		}

		std::filesystem::remove_all("temp");
		break;
	}
	default:
		break;
	}

	if (b.flags.install) NS::details::install_build(b);

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
	case NS::Cli_Opts::Optimisation :
		X("-O3", "/O3");
	case NS::Cli_Opts::Debug_Symbol_Link :
		X("-g", "/DEBUG");
	case NS::Cli_Opts::Debug_Symbol_Compile :
		X("-g", "/Z7");

	case NS::Cli_Opts::Compile :
		X(std::string("-c"), std::string("/c"));
	case NS::Cli_Opts::Link :
		X(std::string("-l") + param.data(), std::string("???"));
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

	for (size_t i = 0; i < b.export_files.size(); ++i)
		if (std::filesystem::is_regular_file(b.export_files[i]))
	{
		auto d = b.export_dest_files[i];
		d = d.lexically_normal();
		d = Working_Directory / "install" / d;
		d = d.lexically_normal();
		if (b.flags.verbose_level > 0) {
			printf(
				"Installing %s to %s.\n",
				b.export_files[i].generic_string().c_str(),
				d.generic_string().c_str()
			);
		}

		std::filesystem::create_directories(d.parent_path());
		std::filesystem::copy_file(
			b.export_files[i],
			d,
			std::filesystem::copy_options::overwrite_existing
		);
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
