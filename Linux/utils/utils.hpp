#pragma once
#include <algorithm>
#include <chrono>
#include <dirent.h>
#include <functional>
#include <iomanip>
#include <string>
#include <type_traits>
#include <unordered_map>

#include <sys/stat.h>

namespace utils
{

bool hasPrefix (const std::string &value, const std::string &prefix);

auto removePrefix (const std::string &prefix, const std::string &value)
    -> std::string;

auto parentPath (const std::string &path) -> std::string;

/* Return the filename part of a path. If include_extension is set to
 * false, then any characters after the last dot character '.' will be
 * removed including the dot.
 *
 * Examples: "/foo/bar/baz" => "baz"
 *           "/foo/bar/baz.txt" (include_extension=false) => "baz"
 *           "/foo/bar/baz.txt" (include_extension=true) => "baz.txt"
 *           "/foo/bar/baz.woo.txt" (include_extension=false) => "baz.woo"
 *           "foo.txt" (include_extension=true) => "foo.txt"
 */
auto filenameFromPath (const std::string &filepath,
                       const bool include_extensions = false) -> std::string;

bool hasToken (const std::string &str, const std::string &delimiters,
               const std::string &pattern);

int loadFiles (
    const std::string &directory,
    std::function<std::string (const std::string &, const struct dirent *)>
        filter,
    std::function<int (const std::string &, const std::string &)> loader,
    std::function<bool (const std::pair<std::string, std::string> &,
                        const std::pair<std::string, std::string> &)>
        sorter,
    bool directory_required = true);

// if have return symbol's linkpath
auto symLnkPath (std::string filepath, struct stat *st_user = nullptr)
    -> std::string;

float round(float f, float round_bits = 2.0f);

// ignore_empty = true
// "aa:bb::cc" ==> ["aa", "bb", "cc"]
// ignore_empty = false
// "aa:bb::cc:" ==> ["aa", "bb", "", "cc", ""]
// !!! if no delimiters found, tokens = 0 !!!
template <typename StringType>
void
tokenizeString (const StringType &str, std::vector<StringType> &tokens,
                const typename std::vector<StringType>::value_type delimiters,
                const bool ignore_empty = false)
{
  typename StringType::size_type pos, last_pos = 0;
  while (true)
    {
      pos = str.find_first_of (delimiters, last_pos);

      if (pos == StringType::npos)
        {
          pos = str.length ();
          // enable?
          // if (last_pos == 0)
          //     break;

          // trim empty : two delimiters continuse
          if (pos != last_pos || !ignore_empty)
            {
              tokens.emplace_back (
                  StringType (str.data () + last_pos, pos - last_pos));
            }
          break;
        }
      else
        {
          if (pos != last_pos || !ignore_empty)
            {
              tokens.emplace_back (
                  StringType (str.data () + last_pos, pos - last_pos));
            }
        }
      last_pos = pos + 1;
    }
}

template< typename Enumeration>
auto as_integral(Enumeration const value) -> std::underlying_type_t<Enumeration>
{
  return static_cast<std::underlying_type_t<Enumeration>>(value);
}

// Thread Safe
template < class DurTy = std::chrono::seconds, class IntTy = int64_t >
IntTy
timestamp_since_epoch ()
{
  auto nowT = std::chrono::time_point_cast<DurTy> (
      std::chrono::system_clock::now ());
  return static_cast<IntTy> (nowT.time_since_epoch ().count ());
}

/**
 * Convert a number of type T to its string
 * representation.
 */
template <typename T>
std::string
numberToString (const T number, const std::string &prefix = std::string (),
                const int base = 10, const int align = -1,
                const char align_char = ' ')
{
  std::ostringstream ss;
  ss << std::setbase (base);
  ss << number;
  const std::string number_string = ss.str ();
  std::string result;
  result.append (prefix);

  if (align > 0 && number_string.size () < (size_t)align)
    {
      size_t chars_to_add = (size_t)align - number_string.size ();

      for (; chars_to_add > 0; --chars_to_add)
        {
          result += align_char;
        }
    }

  result.append (number_string);
  return result;
}

template <>
std::string numberToString (const uint8_t number, const std::string &prefix,
                            const int base, const int align,
                            const char align_char);


/**
 * Convert a string representation of a number
 * to a number of type T.
 */
template <typename T>
T
stringToNumber (const std::string &s, const int base = 10)
{
  std::istringstream ss (s);
  T num;
  ss >> std::setbase (base) >> num;
  return num;
}

template <> uint8_t stringToNumber (const std::string &s, const int base);



template<typename E>
constexpr 
auto toUType(E enumerator) noexcept
{
    return static_cast<std::underlying_type_t<E>>(enumerator);
}

} // namespace utils
