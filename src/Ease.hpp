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