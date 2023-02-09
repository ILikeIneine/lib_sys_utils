#pragma once

namespace scan
{

// todo: 
enum class file_type : unsigned int
{
  LNK,
  DIR,
  ELF,

  // elf type
  DYN,
  EXE
};

}