//
// Created by ENDERZOMBI102 on 15/10/2023.
//
#pragma once

#include <vector>
#include <string_view>

#include "output/Output.hpp"


auto create( std::string_view root, std::string_view indexLocation, const std::vector<std::string>& excluded, bool overwrite, const Output* out ) -> int;
