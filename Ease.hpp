#if 0
echo \" <<'BATCH_SCRIPT' >/dev/null ">NUL "\" \`" <#"

clang++ -o ./Build.exe ./Build.cpp -std=c++17 && Build.exe
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
#include <vector>
#include <string>
#include <filesystem>
#include <map>
#include <optional>

namespace Ease {

struct Flags {

	bool clean = false;
	bool release = false;
	bool scratch = false;
	bool show_help = false;
	bool link_only = false;
	bool generate_debug = false;
	bool run_after_compilation = false;
	
	size_t j = 0;

	std::optional<std::filesystem::path> state_file;
	std::optional<std::filesystem::path> output;

	static Flags parse(int argc, char** argv) noexcept;
	static const char* help_message() noexcept;
};

struct State {
	std::map<std::filesystem::path, std::uint64_t> files;

	static State load_from_file(const std::filesystem::path& p) noexcept;
	static State get_unchanged(const State& a, const State& b) noexcept;
	void save_to_file(const std::filesystem::path& p) noexcept;
};

struct Commands {
	std::vector<std::string> commands;
};

struct Build {
	enum class Target {
		Header_Only,
		Exe
	};

	enum class Cli {
		Cl = 0,
		Gcc,
		Count
	};

	enum class Std_Ver {
		Cpp17 = 0,
		Cpp20 = 1,
		Count
	};

	std::string name;

	Flags flags;
	std::filesystem::path state_file;

	Cli cli;
	Target target;
	Std_Ver std_ver;

	bool invert_header_implementation_define = false;

	std::filesystem::path compiler;
	std::vector<std::filesystem::path> source_files;
	std::vector<std::filesystem::path> header_files;
	std::vector<std::filesystem::path> link_files;

	std::vector<Commands> pre_compile;
	std::vector<Commands> post_compile;
	std::vector<Commands> pre_link;
	std::vector<Commands> post_link;

	std::vector<std::string> defines;

	static Build get_default(Flags flags = {}) noexcept;

	void add_source(const std::filesystem::path& f) noexcept;
	void add_source_recursively(const std::filesystem::path& f) noexcept;
	void add_library(const std::filesystem::path& f) noexcept;
	void add_header(const std::filesystem::path& f) noexcept;
	void add_include(const std::filesystem::path& f) noexcept;
	void add_define(std::string str) noexcept;
};


};

#endif
#ifndef Ease_header_only
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <cctype>
#include <locale>
#include <thread>

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

#define NS Ease

std::filesystem::path Default_State_File = "build/state.txt";
std::filesystem::path Working_Directory;

std::string get_compile_flag(NS::Build::Cli cli, NS::Build::Std_Ver std_ver) noexcept;
std::string get_exe_output_flag(NS::Build::Cli cli, std::string_view str) noexcept;
std::string get_object_output_flag(NS::Build::Cli cli, std::string_view str) noexcept;
std::string get_preprocessor_output_flag(NS::Build::Cli cli, std::string_view str) noexcept;
std::string get_force_include_flag(NS::Build::Cli cli, std::string_view str) noexcept;
std::string get_compile_flag(NS::Build::Cli cli) noexcept;
std::string get_include_flag(NS::Build::Cli cli, std::string_view str) noexcept;
std::string get_link_flag(NS::Build::Cli cli, std::string_view str) noexcept;
std::string get_define_flag(NS::Build::Cli cli, std::string_view str) noexcept;
std::string get_optimisation_flag(NS::Build::Cli cli) noexcept;
std::string get_prepocessor_flag(NS::Build::Cli cli) noexcept;
std::string get_debug_symbol_compile_flag(NS::Build::Cli cli) noexcept;
std::string get_debug_symbol_link_flag(NS::Build::Cli cli) noexcept;

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
		if (strcmp(it, "-l") == 0 || strcmp(it, "--link") == 0) {
			flags.link_only = true;
		}
		if (strcmp(it, "-h") == 0 || strcmp(it, "--help") == 0) {
			flags.show_help = true;
		}
		if (strcmp(it, "--release") == 0) {
			flags.release = true;
		}
		if (strcmp(it, "--scratch") == 0) {
			flags.scratch = true;
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

	"-j <n>                        Will use n threads.\n"
	"                    Exemple: ./Build.exe -j 4\n\n"

	"-l|--link                    Will only link the program, then exit. There must be a complete\n"
	"                             state available. Will not check for incremental compilation.\n"
	"                    Exemple: ./Build.exe -l | ./Build.exe --link\n\n"

	"--release                    Will compile the program using default option for release\n"
	"                             (for instance, if the cli choosen is gcc, will compile using -O3)\n"
	"                    Exemple: ./Build.exe --release\n\n"
	
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

// ===================== FLAGS

// ===================== BUILD
NS::Build NS::Build::get_default(::NS::Flags flags) noexcept {
	NS::Build build;
	build.cli = Cli::Gcc;
	build.target = Target::Exe;
	build.std_ver = Std_Ver::Cpp17;
	build.compiler = "clang++";
	build.name = "Run";
	build.flags = flags;
	return build;
}
void NS::Build::add_library(const std::filesystem::path& f) noexcept {
	auto x = f;
	x = x.lexically_normal();
	link_files.push_back(x);
}
void NS::Build::add_source(const std::filesystem::path& f) noexcept {
	auto x = f;
	x = x.lexically_normal();
	source_files.push_back(x);
}
void NS::Build::add_source_recursively(const std::filesystem::path& f) noexcept {
	for (auto& x : std::filesystem::recursive_directory_iterator(f)) {
		if (!std::filesystem::is_regular_file(x)) continue;
		if (x.path().extension() != ".c" && x.path().extension() != ".cpp") continue;

		auto y = x.path();
		y = y.lexically_normal();
		add_source(y);
	}
}

void NS::Build::add_include(const std::filesystem::path& f) noexcept {
	auto x = f;
	x = x.lexically_normal();
	header_files.push_back(x);
}
void NS::Build::add_header(const std::filesystem::path& f) noexcept {
	auto x = f;
	x = x.lexically_normal();
	header_files.push_back(x);
}
void NS::Build::add_define(std::string str) noexcept {
	defines.push_back(std::move(str));
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

std::string compile_script_header_only(const NS::Build& b) noexcept {
	std::string script;

	for (auto& x : b.header_files) script += read_file(x) + "\n";
	if (b.invert_header_implementation_define) script += "#ifndef " + b.name + "_header_only\n";
	else script += "#ifdef " + b.name + "_implementation\n";
	for (auto& x : b.source_files) {
		script += read_file(x) + "\n";
	}
	script += "#endif\n";
	return script;
}

NS::Commands compile_command_object(NS::State& state, const NS::Build& b) noexcept {
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
		command += " " + get_compile_flag(b.cli);

		if (b.flags.release)
			command += " " + get_optimisation_flag(b.cli);

		command += " " + get_object_output_flag(b.cli, o.generic_string());

		for (auto& d : b.defines) command += " " + get_define_flag(b.cli, d);
			command += " " + get_compile_flag(b.cli, b.std_ver);

		for (auto& x : b.header_files) if (std::filesystem::is_directory(x))
			command += " " + get_include_flag(b.cli, x.generic_string());

		for (auto& x : b.header_files) if (std::filesystem::is_regular_file(x))
			command += " " + get_force_include_flag(b.cli, x.generic_string());

		if (b.flags.generate_debug)
			command += " " + get_debug_symbol_compile_flag(b.cli);

		command += " " + c.generic_string();

		commands.commands.push_back(command);
	}
	return commands;
}

NS::Commands compile_command_link_exe(const NS::Build& b) noexcept {
	NS::Commands commands;
	std::string command;

	command = b.compiler.generic_string() + " ";
	for (auto x : b.source_files) {
		std::filesystem::path o = "build/";
		o += unique_name(x) + ".o";
		o = o.lexically_normal();

		command += o.generic_string() + " ";
	}
	for (auto& x : b.link_files) {
		command += get_link_flag(b.cli, x.generic_string()) + " ";
	}

	if (b.flags.generate_debug)
		command += " " + get_debug_symbol_link_flag(b.cli);

	std::string exe_name = b.name + ".exe";
	if (b.flags.output) {
		exe_name = b.flags.output->generic_string();
		if (std::filesystem::is_directory(*b.flags.output)) {
			exe_name += b.name + ".exe";
		}
	}

	command += " " + get_exe_output_flag(b.cli, exe_name);
	commands.commands.push_back(command);
}

NS::Commands compile_command_incremetal_check(const NS::Build& b) noexcept {
	NS::Commands commands;
	std::string command;

	// First run preprocessor to chech for need to recompilation
	for (auto x : b.source_files) {
		auto f = x;
		std::filesystem::path p = "temp/";
		p += unique_name(x) + ".pre";
		p = p.lexically_normal();

		command = b.compiler.generic_string();

		command += " " + get_prepocessor_flag(b.cli);

		command += " " + get_preprocessor_output_flag(b.cli, p.generic_string());

		for (auto& d : b.defines)
			command += " " + get_define_flag(b.cli, d);

		command += " " + get_compile_flag(b.cli, b.std_ver);

		for (auto& x : b.header_files) if (std::filesystem::is_directory(x))
			command += " " + get_include_flag(b.cli, x.generic_string());

		for (auto& x : b.header_files) if (std::filesystem::is_regular_file(x))
			command += " " + get_force_include_flag(b.cli, x.generic_string());

		command += " " + f.generic_string();

		commands.commands.push_back(command);
	}

	return commands;
}

void execute(const NS::Build& build, const NS::Commands& c) noexcept {
	std::vector<std::thread> threads;
	for (size_t i = 0; i < build.flags.j; ++i) {
		threads.push_back(std::thread([&, i] {
			for (size_t j = i; j < c.commands.size(); j += build.flags.j) {
				printf("%s\n", c.commands[j].c_str());
				system(c.commands[j].c_str());
			}
		}));
	}
	for (auto& x : threads) x.join();
}

int main(int argc, char** argv) {
	extern NS::Build build(NS::Flags flags) noexcept;
	auto flags = NS::Flags::parse(argc - 1, argv + 1);

	auto b = build(flags);

	if (flags.show_help) {
		printf("%s", NS::Flags::help_message());
		return 0;
	}
	if (flags.clean) {
		std::filesystem::remove_all("build");
		std::filesystem::remove_all("temp");
		return 0;
	}

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

		to_dump = compile_script_header_only(b);
		dump_to_file(to_dump, b.name + ".hpp");

		for (auto& x : b.post_compile) execute(b, x);
		for (auto& x : b.pre_link) execute(b, x);
		for (auto& x : b.post_link) execute(b, x);
		break;
	}
	case NS::Build::Target::Exe : {
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
		c = compile_command_link_exe(b);
		for (auto& x : b.pre_link) execute(b, x);
		execute(b, c);
		for (auto& x : b.post_link) execute(b, x);

		new_state.save_to_file(b.state_file);

		if (flags.run_after_compilation) {
			std::string run = b.name + ".exe";
			system(run.c_str());
		}

		std::filesystem::remove_all("temp");
		break;
	}
	default:
		break;
	}


	return 0;
}
#define main dummy_main

std::string get_compile_flag(NS::Build::Cli cli, NS::Build::Std_Ver std_ver) noexcept {
	const char* lookup[(size_t)NS::Build::Cli::Count][(size_t)NS::Build::Std_Ver::Count];
	lookup[(size_t)NS::Build::Cli::Gcc][(size_t)NS::Build::Std_Ver::Cpp17] = "-std=c++17";
	lookup[(size_t)NS::Build::Cli::Cl][(size_t)NS::Build::Std_Ver::Cpp17] = "/std:c++17";
	lookup[(size_t)NS::Build::Cli::Gcc][(size_t)NS::Build::Std_Ver::Cpp20] = "-std=c++20";
	lookup[(size_t)NS::Build::Cli::Cl][(size_t)NS::Build::Std_Ver::Cpp20] = "/std:c++20";
	return lookup[(size_t)cli][(size_t)std_ver];
}

std::string get_prepocessor_flag(NS::Build::Cli cli) noexcept {
	switch (cli) {
	case NS::Build::Cli::Gcc :
		return "-E";
		break;
	case NS::Build::Cli::Cl :
		return "/P";
		break;
	default:
		break;
	}
	return "";
}
std::string get_debug_symbol_compile_flag(NS::Build::Cli cli) noexcept {
	switch (cli) {
	case NS::Build::Cli::Gcc :
		return "-g";
		break;
	case NS::Build::Cli::Cl :
		return "/Z7";
		break;
	default:
		break;
	}
	return "";
}
std::string get_debug_symbol_compile_flag(NS::Build::Cli cli) noexcept {
	switch (cli) {
	case NS::Build::Cli::Gcc :
		return "";
		break;
	case NS::Build::Cli::Cl :
		return "/DEBUG";
		break;
	default:
		break;
	}
	return "";
}
std::string get_optimisation_flag(NS::Build::Cli cli) noexcept {
	switch (cli) {
	case NS::Build::Cli::Gcc :
		return "-O3";
		break;
	case NS::Build::Cli::Cl :
		return "/O3";
		break;
	default:
		break;
	}
	return "";
}
std::string get_define_flag(NS::Build::Cli cli, std::string_view str) noexcept {
	std::string ret = "";
	switch (cli) {
	case NS::Build::Cli::Gcc :
		ret += "-D";
		ret += str.data();
		break;
	case NS::Build::Cli::Cl :
		ret += "/D";
		ret += str.data();
		break;
	default:
		break;
	}
	return ret;
}
std::string get_force_include_flag(NS::Build::Cli cli, std::string_view str) noexcept {
	switch (cli) {
	case NS::Build::Cli::Gcc :
		return std::string("-include ") + str.data();
	case NS::Build::Cli::Cl :
		return std::string("/FI\"") + str.data() + "\"";
	default:
		return "";
	}
}
std::string get_compile_flag(NS::Build::Cli cli) noexcept {
	switch (cli) {
	case NS::Build::Cli::Gcc :
		return "-c";
		break;
	case NS::Build::Cli::Cl :
		return "/c";
		break;
	default:
		return "";
		break;
	}
}
std::string get_link_flag(NS::Build::Cli cli, std::string_view str) noexcept {
	switch (cli) {
	case NS::Build::Cli::Gcc :
		return std::string("-l") + str.data();
		break;
	case NS::Build::Cli::Cl :
		return "";
		break;
	default:
		return "";
		break;
	}
}

std::string get_include_flag(NS::Build::Cli cli, std::string_view str) noexcept {
	switch (cli) {
	case NS::Build::Cli::Gcc :
		return std::string("-I") + str.data();
		break;
	case NS::Build::Cli::Cl :
		return std::string("/I ") + str.data();
		break;
	default:
		return "";
		break;
	}
}
std::string get_object_output_flag(NS::Build::Cli cli, std::string_view str) noexcept {
	switch (cli) {
	case NS::Build::Cli::Gcc :
		return std::string("-o ") + str.data();
		break;
	case NS::Build::Cli::Cl :
		return std::string("/Fo") + "\"" + str.data() + "\"";
		break;
	default:
		return "";
		break;
	}
}
std::string get_exe_output_flag(NS::Build::Cli cli, std::string_view str) noexcept {
	switch (cli) {
	case NS::Build::Cli::Gcc :
		return std::string("-o ") + str.data();
		break;
	case NS::Build::Cli::Cl :
		return std::string("/Fe") + "\"" + str.data() + "\"";
		break;
	default:
		return "";
		break;
	}
}
std::string get_preprocessor_output_flag(NS::Build::Cli cli, std::string_view str) noexcept {
	switch (cli) {
	case NS::Build::Cli::Gcc :
		return std::string("-o ") + str.data();
		break;
	case NS::Build::Cli::Cl :
		return std::string("/Fi") + "\"" + str.data() + "\"";
		break;
	default:
		return "";
		break;
	}
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

#ifndef BS_NO_NAMESPACE
using namespace Ease;
#endif

#endif
