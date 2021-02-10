#include "src/Ease.hpp"
#include "src/Ease.cpp"
#include "src/Install.cpp"

/*
clang++ -o ./Build.exe .\Build.cpp  -std=c++17
*/

Build main(Flags flags) noexcept {
	Build build = Build::get_default(flags);

	build.name = "Ease";
	build.target = Build::Target::Header_Only;
	build.invert_header_implementation_define = true;

	build.add_source("./src/Ease.cpp");
	build.add_source("./src/Install.cpp");
	build.add_header("./src/Ease.hpp");

	Commands copies;
	copies.add_command(
		"cp Ease.hpp test/Ease.hpp",
		"Copy Ease to other directories",
		"Ease.hpp",
		"test/Ease.hpp"
	);
	build.post_link.push_back(copies);

	return build;
}