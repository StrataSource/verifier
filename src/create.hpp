//
// Created by ENDERZOMBI102 on 15/10/2023.
//
#pragma once

#include <string_view>
#include <vector>

auto create( std::string_view root, std::string_view indexLocation, const std::vector<std::string>& excluded, bool overwrite ) noexcept -> int;
