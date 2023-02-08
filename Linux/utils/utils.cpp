#include "utils.hpp"
#include <unistd.h>
#include <cmath>

namespace utils
{

std::string
removePrefix (const std::string &prefix, const std::string &value)
{
  if (value.compare (0, prefix.size (), prefix) == 0)
    {
      return value.substr (prefix.size ());
    }
  else
    {
      return value;
    }
}

bool
hasPrefix (const std::string &value, const std::string &prefix)
{
  if (value.size () < prefix.size ())
    return false;
  const auto cmp = value.compare (0, prefix.size (), prefix);
  return cmp == 0;
}

std::string
parentPath (const std::string &path)
{
  const std::string directory_separator = "/";
  std::string parent_path (path);
  // find first not '/' (from end)
  // find first '/' (from end)
  // find first not '/' (from end)
  auto reverse_start_pos = parent_path.find_last_not_of (directory_separator);

  /*
   * Whole path consists only of '/'.
   */
  if (reverse_start_pos == std::string::npos)
    {
      return std::string ();
    }

  reverse_start_pos
      = parent_path.find_last_of (directory_separator, reverse_start_pos);

  /*
   * No directory separator in the rest of the path.
   */
  if (reverse_start_pos == std::string::npos)
    {
      return std::string ();
    }

  reverse_start_pos
      = parent_path.find_last_not_of (directory_separator, reverse_start_pos);

  /*
   *
   * /foo/bar   => /foo
   * /foo/bar/  => /foo
   * /foo/bar// => /foo
   * /foo       => std::string()
   * /foo/      => std::string()
   * /          => std::string()
   * //foo      => std::string()
   *
   */
  if (reverse_start_pos == std::string::npos)
    {
      return std::string ();
    }

  return path.substr (0, reverse_start_pos + 1);
}


std::string
filenameFromPath (const std::string &filepath, const bool include_extension)
{
  const std::string directory_separator = "/";
  std::vector<std::string> path_tokens;
  tokenizeString (filepath, path_tokens, directory_separator);

  if (path_tokens.size () == 0)
    {
      return std::string ();
    }

  const std::string &filename = path_tokens.back ();

  if (include_extension)
    {
      return filename;
    }

  const size_t substr_to = filename.find_last_of ('.');
  return filename.substr (0, substr_to);
}

bool
hasToken(const std::string& str, const std::string& delimiters, const std::string& pattern)
{
  std::vector<std::string> tokens;
  tokenizeString(str, tokens, delimiters, true);
  if (tokens.empty ())
    return false;
  else
    return std::any_of (
        tokens.begin (), tokens.end (),
        [&pattern] (const std::string &str) { return str == pattern; });
}

template <>
std::string
numberToString (const uint8_t number, const std::string &prefix,
                const int base, const int align, const char align_char)
{
  const uint16_t n = static_cast<uint16_t> (number);
  return numberToString (n, prefix, base, align, align_char);
}

template <>
uint8_t
stringToNumber (const std::string &s, const int base)
{
  const unsigned int num = stringToNumber<unsigned int> (s, base);
  return (uint8_t)num;
}


int
loadFiles (
    const std::string &directory,
    std::function<std::string (const std::string &, const struct dirent *)> filter,
    std::function<int (const std::string &, const std::string &)> loader,
    std::function<bool (const std::pair<std::string, std::string> &,
                        const std::pair<std::string, std::string> &)> sorter,
    bool directory_required)
{
  DIR *dirobj = opendir (directory.c_str ());

  if (dirobj == nullptr)
    {
      if (!directory_required && errno == ENOENT)
        {
          return 0;
        }
      // throw
    }

  int retval{ 0 };

  try
    {
      std::vector<std::pair<std::string, std::string> > loadpaths;
      struct dirent *entry_ptr = nullptr;

      // assume that the readdir is thread-safe and reliable
      while ((entry_ptr = readdir (dirobj)) != nullptr)
        {
          // enumerate the file in dir
          const std::string filename = entry_ptr->d_name;
          if (filename == "." || filename == "..")
            {
              continue;
            }
          // fullpath of current file
          auto fullpath = directory + "/" + filename;
          // if it is a link symbol, analyze it, or return the origin path
          auto loadpath = filter (fullpath, entry_ptr);

          if (!loadpath.empty ())
            {
              loadpaths.emplace_back (
                  std::make_pair (std::move (loadpath), std::move (fullpath)));
            }

          std::sort (loadpaths.begin(), loadpaths.end(), sorter);
        }

      for (const auto &loadpath : loadpaths)
        {
          retval += loader (loadpath.first, loadpath.second);
        }
    }
  catch (const std::exception& e)
    {
        throw e;
    }
  return retval;
}

std::string
symLnkPath (std::string linkpath, struct stat *st_user)
{
  struct stat st;
  struct stat *st_ptr = nullptr;

  if (st_user == nullptr)
    {
      if (lstat (linkpath.c_str (), &st) < 0)
      {
        // throw
        return {};
      }
      st_ptr = &st;
    }
  else
    {
      st_ptr = st_user;
    }

  if (!S_ISLNK (st_ptr->st_mode))
    {
      // todo: throw
      return {};
    }

  if (st_ptr->st_size < 1)
    {
      st_ptr->st_size = 4096;
    }

  /*
   * Check sanity of st_size. min: 1 byte, max: 1 MiB (because 1 MiB should be
   * enough :)
   */
  if (st_ptr->st_size < 1 || st_ptr->st_size > (1024 * 1024))
    {
      // todo: throw out of range
      return {};
    }

  std::string buffer (st_ptr->st_size, 0);
  const ssize_t link_size
      = readlink (linkpath.c_str (), &buffer[0], buffer.capacity ());

  if (link_size <= 0 || link_size > st_ptr->st_size)
    {
      // throw Exception(__func__, __LINE__,"xxx");
      return {};
    }

  buffer.resize (link_size);

  if (buffer[0] == '/')
    {
      /* Absolute path */
      return buffer;
    }
  else
    {
      /* Relative path */
      return utils::parentPath (linkpath) + "/" + buffer;
    }
}

float round(float f, float round_bits)
{
  int round_tail = std::pow(10.0, round_bits);
  float value = (int)(f * round_tail + .5);
  return (float)value / round_tail;
}

}