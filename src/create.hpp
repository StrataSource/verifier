//
// Created by ENDERZOMBI102 on 15/10/2023.
//
#pragma once

#include <string_view>
#include <vector>

auto createFromRoot( std::string_view root, std::string_view indexLocation, bool skipArchives, const std::vector<std::string>& excluded, const std::vector<std::string>& included ) -> int;
