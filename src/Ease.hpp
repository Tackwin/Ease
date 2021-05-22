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
#include <unordered_map>

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
	bool openmp = false;
	bool release = false;
	bool scratch = false;
	bool install = false;
	bool assembly = false;
	bool show_help = false;
	bool link_only = false;
	bool no_inline = false;
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
	Arch_32,
	Assembly_Output,
	Debug_Symbol_Compile,
	Debug_Symbol_Link,
	No_Default_Lib,
	OpenMP,
	Native,
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
	std::filesystem::path get_output_path(const Build& b) noexcept;
};

};

#ifndef EASE_NO_NAMESPACE
using namespace Ease;
#endif

#endif