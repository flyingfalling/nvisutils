#include <string_tokenizer.hpp>

std::vector<std::string> tokenize_string(const std::string& src, const std::string& delim, bool include_empty_repeats)
{
  std::vector<std::string> retval;
  boost::char_separator<char> sep( delim.c_str() );
  boost::tokenizer<boost::char_separator<char>> tokens(src, sep);
  for(const auto& t : tokens)
    {
      retval.push_back( t );
    }
  return retval;
}
