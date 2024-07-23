//
// Created by ENDERZOMBI102 on 15/10/2023.
//
#pragma once

#include <string_view>
#include <vector>

auto createFromRoot( std::string_view root_, std::string_view indexLocation, bool skipArchives,
					 const std::vector<std::string>& fileExcludes, const std::vector<std::string>& fileIncludes,
					 const std::vector<std::string>& archiveExcludes, const std::vector<std::string>& archiveIncludes) -> int;
