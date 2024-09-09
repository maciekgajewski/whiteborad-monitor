#include "file_debug_info.hh"

#include "logging.hh"

#include <boost/scope_exit.hpp>

#include <fmt/core.h>

namespace Whiteboard {

namespace {

template <typename... Args>
void throw_if_error(int res, Dwarf_Error &error, std::string_view action_fmt,
                    const Args &...args) {
  if (res == DW_DLV_ERROR) {
    std::string action =
        fmt::vformat(action_fmt, fmt::make_format_args(args...));
    std::string error_msg =
        fmt::format("DWARF error {} : {}", action, dwarf_errmsg(error));
    throw std::runtime_error(error_msg);
  }
}

constexpr bool debug_dump = false;

} // namespace

void FileDebugInfo::process_dwarf_cu(Dwarf_Die &cu_die, const char *die_name,
                                     Dwarf_Error &error) {
  // Dwarf_Line *linebuf = 0;
  // Dwarf_Signed linecount = 0;
  // Dwarf_Line *linebuf_actuals = 0;
  // Dwarf_Signed linecount_actuals = 0;
  Dwarf_Line_Context line_context = 0;
  Dwarf_Small table_count = 0;
  Dwarf_Unsigned lineversion = 0;

  int res = ::dwarf_srclines_b(cu_die, &lineversion, &table_count,
                               &line_context, &error);
  throw_if_error(res, error, "srclines");

  BOOST_SCOPE_EXIT(line_context) { ::dwarf_srclines_dealloc_b(line_context); }
  BOOST_SCOPE_EXIT_END;

  Logging::trace("FileDebugInfo: Dwarf CU {}, table_count={}", die_name,
                 table_count);

  if (table_count != 1)
    return; // TODO action needed?

  // iterate files
  Dwarf_Signed baseindex = 0;
  Dwarf_Signed file_count = 0;
  Dwarf_Signed endindex = 0;
  res = ::dwarf_srclines_files_indexes(line_context, &baseindex, &file_count,
                                       &endindex, &error);
  throw_if_error(res, error, "srclines_files_indexes");

  for (int i = baseindex; i < endindex; i++) {
    Dwarf_Unsigned dirindex = 0;
    Dwarf_Unsigned modtime = 0;
    Dwarf_Unsigned flength = 0;
    Dwarf_Form_Data16 *md5data = 0;
    const char *name = 0;

    res = ::dwarf_srclines_files_data_b(line_context, i, &name, &dirindex,
                                        &modtime, &flength, &md5data, &error);
    throw_if_error(res, error, "srclines_files_data_b");

    Logging::trace("FileDebugInfo: Dwarf files data: i={}, dirindex={}, "
                   "modtime={}, flength={}, name={}",
                   i, dirindex, modtime, flength, name);
  }

  // iterate dirs
  Dwarf_Signed dw_count = 0;
  res = ::dwarf_srclines_include_dir_count(line_context, &dw_count, &error);
  throw_if_error(res, error, "srclines_include_dir_count");

  for (int i = 0; i < dw_count; ++i) {
    const char *dname = nullptr;
    res = ::dwarf_srclines_include_dir_data(line_context, i, &dname, &error);
    throw_if_error(res, error, "srclines_include_dir_data");

    Logging::trace("FileDebugInfo: Dwarf dir i={} name={}", i, dname);
  }

  // iterate lines
  Dwarf_Line *linebuf = 0;
  Dwarf_Signed linecount = 0;
  res = ::dwarf_srclines_from_linecontext(line_context, &linebuf, &linecount,
                                          &error);

  for (int i = 0; i < linecount; ++i) {
    Dwarf_Unsigned linenum = 0;
    res = ::dwarf_lineno(linebuf[i], &linenum, &error);
    throw_if_error(res, error, "lineno");

    Dwarf_Unsigned filenum = 0;
    res = ::dwarf_line_srcfileno(linebuf[i], &filenum, &error);
    throw_if_error(res, error, "line_srcfileno");

    Dwarf_Addr addr = 0;
    res = ::dwarf_lineaddr(linebuf[i], &addr, &error);
    throw_if_error(res, error, "lineaddr");

    Dwarf_Bool prologue_end = 0;
    Dwarf_Bool epilogue_end = 0;
    Dwarf_Unsigned isa = 0;
    Dwarf_Unsigned discriminator = 0;

    res = ::dwarf_prologue_end_etc(linebuf[i], &prologue_end, &epilogue_end,
                                   &isa, &discriminator, &error);
    throw_if_error(res, error, "prologue_end_etc");

    Dwarf_Bool end_sequence = 0;
    res = ::dwarf_lineendsequence(linebuf[i], &end_sequence, &error);
    throw_if_error(res, error, "lineendsequence");

    Dwarf_Bool begin_statement = 0;
    res = ::dwarf_linebeginstatement(linebuf[i], &begin_statement, &error);
    throw_if_error(res, error, "linebeginstatement");

    Logging::trace(
        "FileDebugInfo: Dwarf line i={}, prologue_end={}, "
        "epilogue_end={}, isa={}, discriminator={}, end_sequence={}, "
        "linenum={}, filenum={}, addr=0x{:<8x}",
        i, prologue_end, epilogue_end, isa, discriminator, end_sequence,
        linenum, filenum, addr);
  }
}

void FileDebugInfo::process_dwarf_die(Dwarf_Die &die, Dwarf_Error &error,
                                      int in_level) {

  // tag
  Dwarf_Half tag = 0;
  int res = ::dwarf_tag(die, &tag, &error);
  throw_if_error(res, error, "reading tag");

  // die name
  char *die_name_ptr = nullptr;
  res = ::dwarf_diename(die, &die_name_ptr, &error);
  throw_if_error(res, error, "reading die name");
  if (res == DW_DLV_NO_ENTRY)
    return;

  // addr
  Dwarf_Addr low_pc = 0;
  res = ::dwarf_lowpc(die, &low_pc, &error);
  throw_if_error(res, error, "reading die low_pc");

  // record function
  // TODO only top level. A proper walker is needed, that builds a name
  // recursively
  if (tag == DW_TAG_subprogram) {
    if (die_name_ptr && in_level == 1 && low_pc) {
      auto [_, success] = _functions.try_emplace(die_name_ptr, low_pc);
      if (!success) {
        throw std::runtime_error(
            fmt::format("Duplicate function name: {}", die_name_ptr));
      }
    }
  }

  // record compilation unit
  if (tag == DW_TAG_compile_unit) {
    if (die_name_ptr) {
      process_dwarf_cu(die, die_name_ptr, error);
    }
  }

  // debug dump
  if (debug_dump) {

    // tag name
    const char *die_name = die_name_ptr ? die_name_ptr : "(unnamed)";
    const char *tag_name = nullptr;
    res = ::dwarf_get_TAG_name(tag, &tag_name);
    throw_if_error(res, error, "reading tag name");

    Logging::trace(
        "FileDebugInfo: Dwarf DIE {:>{}} tag={}, name={}, addr={}, level={}",
        "", in_level, tag_name, die_name, (void *)(low_pc), in_level);

    // attr list
    {
      Dwarf_Signed atcount;
      Dwarf_Attribute *atlist;
      res = ::dwarf_attrlist(die, &atlist, &atcount, &error);
      throw_if_error(res, error, "attrrlist");

      for (int i = 0; i < atcount; ++i) {
        Dwarf_Half attrnum = 0;
        const char *attrname = 0;

        /*  use atlist[i], likely calling
            libdwarf functions and likely
            returning DW_DLV_ERROR if
            what you call gets DW_DLV_ERROR */
        res = ::dwarf_whatattr(atlist[i], &attrnum, &error);
        throw_if_error(res, error, "whatattr");
        ::dwarf_get_AT_name(attrnum, &attrname);
        fmt::println("{:>{}}  * #{} {} : {}", "", in_level, i, attrnum,
                     attrname);
        dwarf_dealloc_attribute(atlist[i]);
        atlist[i] = 0;
      }
      // dwarf_dealloc(dbg, atlist, DW_DLA_LIST);
    }
  } // dd
}

void FileDebugInfo::walk_dwarf_die(Dwarf_Debug dbg, Dwarf_Die in_die,
                                   int is_info, int in_level,
                                   Dwarf_Error &error) {
  int res = DW_DLV_OK;
  Dwarf_Die cur_die = in_die;
  Dwarf_Die child = 0;

  process_dwarf_die(in_die, error, in_level);

  /*   Loop on a list of siblings */
  for (;;) {
    Dwarf_Die sib_die = 0;

    /*  Depending on your goals, the in_level,
        and the DW_TAG of cur_die, you may want
        to skip the dwarf_child call. We descend
        the DWARF-standard way of depth-first. */
    res = ::dwarf_child(cur_die, &child, &error);
    throw_if_error(res, error, "dwarf_child");

    if (res == DW_DLV_OK) {
      walk_dwarf_die(dbg, child, is_info, in_level + 1, error);
      /* No longer need 'child' die. */
      ::dwarf_dealloc(dbg, child, DW_DLA_DIE);
      child = 0;
    }
    /* res == DW_DLV_NO_ENTRY or DW_DLV_OK */
    res = dwarf_siblingof_c(cur_die, &sib_die, &error);
    throw_if_error(res, error, "dwarf_siblingof_c");

    if (res == DW_DLV_NO_ENTRY) {
      /* Done at this level. */
      break;
    }
    /* res == DW_DLV_OK */
    if (cur_die != in_die) {
      ::dwarf_dealloc(dbg, cur_die, DW_DLA_DIE);
      cur_die = 0;
    }
    cur_die = sib_die;
    process_dwarf_die(sib_die, error, in_level);
  }
}

FileDebugInfo::FileDebugInfo(const std::string &path) {

  // TODO use libdwarf to populate

  static char true_pathbuf[FILENAME_MAX];
  unsigned tpathlen = FILENAME_MAX;
  Dwarf_Handler errhand = 0;
  Dwarf_Ptr errarg = 0;
  Dwarf_Error error = 0;
  Dwarf_Debug dbg = 0;
  int res = 0;

  res = dwarf_init_path(path.c_str(), true_pathbuf, tpathlen,
                        DW_GROUPNUMBER_ANY, errhand, errarg, &dbg, &error);

  BOOST_SCOPE_EXIT(dbg, error) {
    ::dwarf_dealloc_error(dbg, error);
    ::dwarf_finish(dbg);
  }
  BOOST_SCOPE_EXIT_END

  throw_if_error(res, error, "loading debug info from '{}'", path);
  if (res == DW_DLV_NO_ENTRY) {
    return;
  }
  // fmt::print("FileDebugInfo: The file we actually opened is {}\n",
  //            true_pathbuf);

  // walk the tree
  {
    Dwarf_Unsigned abbrev_offset = 0;
    Dwarf_Half address_size = 0;
    Dwarf_Half version_stamp = 0;
    Dwarf_Half offset_size = 0;
    Dwarf_Half extension_size = 0;
    Dwarf_Sig8 signature;
    Dwarf_Unsigned typeoffset = 0;
    Dwarf_Unsigned next_cu_header = 0;
    Dwarf_Half header_cu_type = 0;
    Dwarf_Bool is_info = 1;
    int res = 0;

    while (true) {
      Dwarf_Die cu_die = 0;
      Dwarf_Unsigned cu_header_length = 0;

      memset(&signature, 0, sizeof(signature));
      res = ::dwarf_next_cu_header_e(
          dbg, is_info, &cu_die, &cu_header_length, &version_stamp,
          &abbrev_offset, &address_size, &offset_size, &extension_size,
          &signature, &typeoffset, &next_cu_header, &header_cu_type, &error);

      throw_if_error(res, error, "walking '{}'", path);

      if (res == DW_DLV_NO_ENTRY) {
        if (is_info == 1) {
          /*  Done with .debug_info, now check for
              .debug_types. */
          is_info = 0;
          continue;
        }
        break;
      }

      // we have a DIE
      walk_dwarf_die(dbg, cu_die, is_info, 0, error);
      ::dwarf_dealloc_die(cu_die);
    }
  }
}

FileDebugInfo::~FileDebugInfo() = default;

offset_t FileDebugInfo::findFunction(const std::string &fname) const {
  auto it = _functions.find(fname);
  if (it == _functions.end()) {
    throw std::runtime_error(fmt::format("Function '{}' not found", fname));
  }
  return it->second;
}

} // namespace Whiteboard