
#pragma once

#include <cstdlib>
#include <string>
#include <vector>
#include <stack>

#include <boost/tokenizer.hpp>

std::vector<std::string> tokenize_string(const std::string& src, const std::string& delim, bool include_empty_repeats=false);


