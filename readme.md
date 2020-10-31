# Ease
> Build C++ projects with Ease.

Ease is a Build System for C++ that strive to acheive simplicity. There is no dependancies, no installation you drop off BS.hpp in your project and can start writing a build function.

The build function will be called and the build will start according to the return value of this function.

<!-- ![](header.png) -->

## Installation

Copy paste [BS.hpp][BS] in your directory.

## Usage example

You can look for example in the examples directory.
Here is a quick and true snippet to build a simple project.
```c++
#include "BS.hpp"

Build build(Flags flags) noexcept {
	Build build = Build::get_default(flags);

	build.name = "Hello";

	build.add_source("src/Main.cpp");
	build.add_source("src/Other_file.cpp");

	build.add_header("include/");
	build.add_library("lib/awesomelib");

	return build;
}
```
Then just build and run this file with your favorite compiler !
```sh
clang++ -std=c++17 Build.cpp && ./a.out
```


One more example, the actual Build.cpp used to build BuildSelf itself ! :)
```c++
#include "src/BuildSelf.hpp" // Can't use BS.hpp since we are building it
#include "src/BuildSelf.cpp"

Build build(Flags flags) noexcept {
	Build build = Build::get_default(flags);

	build.name = "BS";
	build.target = Build::Target::Header_Only; // We don't want to build a .exe
	                                           // Just a single heade
	build.invert_header_implementation_define = true; // To not have to define
	                                                  // a stupid macro

	build.add_source("./src/BuildSelf.cpp");
	build.add_header("./src/BuildSelf.hpp");

	return build;
}
```


## Release History

* 0.0.0
    * Intial release

<!-- Markdown link & img dfn's -->
[BS]: https://github.com/Tackwin/BuildSelf/blob/master/BS.hpp
