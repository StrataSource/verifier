// MIT License
// Copyright (c) 2019 Pranav
//    Permission is hereby granted, free of charge, to any person obtaining a copy
//    of this software and associated documentation files (the "Software"), to deal
//    in the Software without restriction, including without limitation the rights
//    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//    														  copies of the Software, and to permit persons to whom the Software is
//    furnished to do so, subject to the following conditions:
//
//    The above copyright notice and this permission notice shall be included in all
//    copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//    SOFTWARE.
// https://github.com/p-ranav/glob/tree/d72787daee0aecc41d288a7093de054608a77c70/
#pragma once
#include <string>
#include <vector>

#ifdef GLOB_USE_GHC_FILESYSTEM
	#include <ghc/filesystem.hpp>
#else
	#include <filesystem>
#endif

namespace glob {

#ifdef GLOB_USE_GHC_FILESYSTEM
	namespace fs = ghc::filesystem;
#else
	namespace fs = std::filesystem;
#endif

	/// \param pathname string containing a path specification
	/// \return vector of paths that match the pathname
	///
	/// Pathnames can be absolute (/usr/src/Foo/Makefile) or relative (../../Tools/*/*.gif)
	/// Pathnames can contain shell-style wildcards
	/// Broken symlinks are included in the results (as in the shell)
	std::vector<fs::path> glob(const std::string &pathname);

	/// \param pathnames string containing a path specification
	/// \return vector of paths that match the pathname
	///
	/// Globs recursively.
	/// The pattern “**” will match any files and zero or more directories, subdirectories and
	/// symbolic links to directories.
	std::vector<fs::path> rglob(const std::string &pathname);

	/// Runs `glob` against each pathname in `pathnames` and accumulates the results
	std::vector<fs::path> glob(const std::vector<std::string> &pathnames);

	/// Runs `rglob` against each pathname in `pathnames` and accumulates the results
	std::vector<fs::path> rglob(const std::vector<std::string> &pathnames);

	/// Initializer list overload for convenience
	std::vector<fs::path> glob(const std::initializer_list<std::string> &pathnames);

	/// Initializer list overload for convenience
	std::vector<fs::path> rglob(const std::initializer_list<std::string> &pathnames);

	/// Returns true if the input path matche the glob pattern
	bool fnmatch(const fs::path &name, const std::string &pattern);

} // namespace glob
