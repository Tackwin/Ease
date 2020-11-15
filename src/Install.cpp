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