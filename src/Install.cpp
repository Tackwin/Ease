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
#include <ShlObj.h>
#include <Windows.h>
std::filesystem::path get_user_data_path() noexcept {
	PWSTR buffer;
	auto result = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &buffer);

	if (result != 0) {
		return std::filesystem::current_path();
	}

	auto str = std::wstring{ buffer };
	std::filesystem::path p{ str };

	CoTaskMemFree(buffer);
	return p;
}
#else
std::filesystem::path get_user_data_path() noexcept {
	return "~";
}
#endif

struct Install_State {
	std::unordered_set<std::filesystem::path, fs_hash> dirs;
};

void NS::details::install_build(const NS::Build& b) noexcept {
	std::filesystem::create_directories("install");

	Install_State state;

	std::ifstream in_file;
	in_file.open(get_user_data_path() / ".ease/install.txt");
	if (in_file.good()) {
		for (std::string line; std::getline(in_file, line);) if (!line.empty()) {
			state.dirs.insert(line);
		}
	} else {
		std::filesystem::create_directories(get_user_data_path() / ".ease/");
	}
	in_file.close();

	for (auto& x : b.to_install) if (std::filesystem::is_regular_file(x)) {

		auto d = x.parent_path();
		d = d.lexically_normal();
		d = Working_Directory / "install" / x;
		d = d.lexically_normal();

		std::filesystem::create_directories(Working_Directory / "install" / x.parent_path());
		if (b.flags.verbose_level > 0) {
			printf(
				"Installing %s to %s.\n", x.generic_string().c_str(), d.generic_string().c_str()
			);
		}
		std::filesystem::copy_file(
			x,
			d,
			std::filesystem::copy_options::overwrite_existing
		);

		state.dirs.insert(Working_Directory / "install");
	}

	std::ofstream out_file;
	out_file.open(get_user_data_path() / ".ease/install.txt");
	for (auto& x : state.dirs) {
		out_file << x.generic_string() << "\n";
	}
	out_file.close();
}
#undef NS