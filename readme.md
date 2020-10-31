# Ease
> Build C++ projects with Ease.

Ease is a Build System for C++ that strive to acheive simplicity. There is no dependancies, no installation you drop off BS.hpp in your project and can start writing a build function.

The build function will be called and the build will start according to the return value of this function.

<!-- ![](header.png) -->

## Installation

Copy paste [Ease.hpp][Ease_Header] in your directory.

## Usage example

You can look for example in the examples directory.
Here is a quick and true snippet to build a simple project.
```c++
// This is Build.cpp
#include "Ease.hpp"

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


One more example, the actual Build.cpp used to build Ease itself ! :)
```c++
// This is Build.cpp
#include "src/Ease.hpp" // Can't use Ease.hpp since we are building it
#include "src/Ease.cpp"

Build build(Flags flags) noexcept {
	Build build = Build::get_default(flags);

	build.name = "Ease";
	build.target = Build::Target::Header_Only; // We don't want to build a .exe
	                                           // Just a single heade
	build.invert_header_implementation_define = true; // To not have to define
	                                                  // a stupid macro

	build.add_source("./src/Ease.cpp");
	build.add_header("./src/Ease.hpp");

	return build;
}
```

## Running

You get right off the bat some flags and parameter that you can use to configure your build.

`./Build.exe -h` will show you an exhaustive list of flags and options.

## Feature

- Incremental Compiling (By running the preprocessor only and comparing hashes)
- Can compile Header Only and Executables
- Support for cl and gcc-family cli

## Gallery

Fantastic project that will tune your RGB led to get you in the mood for crewmate killing or some
good tasking.
[AmongUsXArduino][Inopio]

## Release History

* 0.0.0
    * Intial release

<!-- Markdown link & img dfn's -->
[Ease_Header]: https://github.com/Tackwin/BuildSelf/blob/master/Ease.hpp
[Inopio]: https://github.com/Inopio/AmongUsXArduino

