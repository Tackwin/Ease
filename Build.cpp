#include "src/Ease.hpp"
#include "src/Ease.cpp"
#include "src/Install.cpp"

// clang++ -o ./Build.exe .\Build.cpp  -std=c++17 -lShell32 -lOle32

Build build(Flags flags) noexcept {
	Build build = Build::get_default(flags);

	build.name = "Ease";
	build.target = Build::Target::Header_Only;
	build.invert_header_implementation_define = true;

	build.add_source("./src/Ease.cpp");
	build.add_source("./src/Install.cpp");
	build.add_header("./src/Ease.hpp");

	return build;
}