#pragma once

#include <elf.h>

#include <type_traits>
#include <string>
#include <memory>

#include <cstdio>
#include <cstring>
#include <cctype>

#if defined(__LP64__)
    typedef uint64_t bl_uintptr;
#else
    typedef uint32_t bl_uintptr;
#endif

namespace utils {
namespace detail
{

// 64 bit def
template <typename EquivalentPointerType> struct ElfTypeTraits
{
    static constexpr unsigned char Class = ELFCLASS64;
    static constexpr unsigned char Class32 = ELFCLASS32;

    // integer types
    using Half = Elf64_Half;
    using Word = Elf64_Word;
    using Addr = Elf64_Addr;
    using Off = Elf64_Off;

    // structure types
    using Ehdr = Elf64_Ehdr;
    using Shdr = Elf64_Shdr;
    using Phdr = Elf64_Phdr;
    using Nhdr = Elf64_Nhdr;
};

// 32 bit def
template <> struct ElfTypeTraits<uint32_t>
{
    static constexpr unsigned char Class = ELFCLASS32;

    // integer types
    using Half = Elf32_Half;
    using Word = Elf32_Word;
    using Addr = Elf32_Addr;
    using Off = Elf32_Off;

    // structure types
    using Ehdr = Elf32_Ehdr;
    using Shdr = Elf32_Shdr;
    using Phdr = Elf32_Phdr;
    using Nhdr = Elf32_Nhdr;
};

struct ElfHeaderCommonCheck
{
   static_assert(std::is_same<decltype(Elf32_Ehdr::e_ident), decltype(Elf64_Ehdr::e_ident)>::value,
        "e_ident field is not the same in both Elf32_Ehdr and Elf64_Ehdr");

    // bytes 0-3
    static bool checkElfMagic(const unsigned char* ident)
    {
        return ::memcmp(ident, ELFMAG, SELFMAG) == 0;
    }

    // bytes 6th
    static bool checkElfVersion(const unsigned char* ident)
    {
        unsigned char elfversion = ident[EI_VERSION];
        return elfversion == EV_CURRENT;
    }
};

template<typename EquivalentPointerType = bl_uintptr>
struct ElfHeaderCheck : public ElfHeaderCommonCheck
{
    using TypeTraits = ElfTypeTraits<EquivalentPointerType>;
    using Ehdr = typename TypeTraits::Ehdr;

    static bool checkClass(const unsigned char* ident)
    {
        auto klass = ident[EI_CLASS];
        return (klass == TypeTraits::Class || klass == TypeTraits::Class32);
    }

    static bool checkIdent(const Ehdr& header)
    {
        return checkElfMagic(header.e_ident)
                && checkElfVersion(header.e_ident)
                && checkClass(header.e_ident);
    }

    static bool checkType(const Ehdr& header)
    {
        return header.e_type == ET_DYN
                || header.e_type == ET_EXEC;
    }

    static int getType(const Ehdr& header)
    {
        return header.e_type;
    }

    static bool checkFileVersion(const Ehdr& header)
    {
        return header.e_version == EV_CURRENT;
    }

    static bool checkHeader(const Ehdr& header)
    {
        // check elf
        if(!checkIdent(header))
            return false;
        
        // check type
        return checkType(header)
                && checkFileVersion(header);
    }
};

} // namespace detail

using T = detail::ElfHeaderCheck<>::TypeTraits;


static inline
bool 
check_if_valid_elf(std::string const& path, int& elf_type)
{
    T::Ehdr header;
    std::unique_ptr<std::FILE, decltype(&::fclose)> file(::fopen(path.c_str(), "rb"), &::fclose);

    if(!file)
        return false;

    ::fread(&header, sizeof(header), 1, file.get());

    // LOG_INFO("path:{}, e_type:{:x}, e_version:{:x}, elf_version:{:x}", 
    //                 path, 
    //                 header.e_type, 
    //                 header.e_version, 
    //                 (unsigned char)header.e_ident[EI_VERSION]);

    elf_type = detail::ElfHeaderCheck<>::getType(header);
    return detail::ElfHeaderCheck<>::checkHeader(header);
}

}  // namespace utils





