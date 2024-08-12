#include "file_debug_info.hh"

#include <fmt/core.h>

#include <libdwarf/dwarf.h>
#include <libdwarf/libdwarf.h>

namespace Whiteboard {

namespace {

void process_dwarf_die(Dwarf_Die die) {
  //
  fmt::println("We have a DIE");
}

void record_die_and_siblings_e(Dwarf_Debug dbg, Dwarf_Die in_die, int is_info,
                               int in_level, Dwarf_Error *error) {
  int res = DW_DLV_OK;
  Dwarf_Die cur_die = in_die;
  Dwarf_Die child = 0;

  process_dwarf_die(in_die);

  /*   Loop on a list of siblings */
  for (;;) {
    Dwarf_Die sib_die = 0;

    /*  Depending on your goals, the in_level,
        and the DW_TAG of cur_die, you may want
        to skip the dwarf_child call. We descend
        the DWARF-standard way of depth-first. */
    res = dwarf_child(cur_die, &child, error);
    if (res == DW_DLV_ERROR) {
      printf("Error in dwarf_child , level %d \n", in_level);
      exit(EXIT_FAILURE);
    }
    if (res == DW_DLV_OK) {
      record_die_and_siblings_e(dbg, child, is_info, in_level + 1, error);
      /* No longer need 'child' die. */
      dwarf_dealloc(dbg, child, DW_DLA_DIE);
      child = 0;
    }
    /* res == DW_DLV_NO_ENTRY or DW_DLV_OK */
    res = dwarf_siblingof_c(cur_die, &sib_die, error);
    if (res == DW_DLV_ERROR) {
      exit(EXIT_FAILURE);
    }
    if (res == DW_DLV_NO_ENTRY) {
      /* Done at this level. */
      break;
    }
    /* res == DW_DLV_OK */
    if (cur_die != in_die) {
      dwarf_dealloc(dbg, cur_die, DW_DLA_DIE);
      cur_die = 0;
    }
    cur_die = sib_die;
    process_dwarf_die(sib_die);
  }
}

} // namespace

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
  if (res == DW_DLV_ERROR) {
    std::string error_msg =
        fmt::format("FileDebugInfo: error loading debug info from '{}' : {}",
                    path, dwarf_errmsg(error));
    /*  Necessary call even though dbg is null!
        This avoids a memory leak.  */
    dwarf_dealloc_error(dbg, error);
    throw std::runtime_error(error_msg);
  }
  if (res == DW_DLV_NO_ENTRY) {
    fmt::print("FileDebugInfo: no entry\n");
    return;
  }
  fmt::print("FileDebugInfo: The file we actually opened is {}\n",
             true_pathbuf);

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
      res = dwarf_next_cu_header_e(
          dbg, is_info, &cu_die, &cu_header_length, &version_stamp,
          &abbrev_offset, &address_size, &offset_size, &extension_size,
          &signature, &typeoffset, &next_cu_header, &header_cu_type, &error);

      if (res == DW_DLV_ERROR) {
        std::string error_msg =
            fmt::format("FileDebugInfo: error loading walking '{}' : {}", path,
                        dwarf_errmsg(error));
        /*  Necessary call even though dbg is null!
            This avoids a memory leak.  */
        dwarf_dealloc_error(dbg, error);
        dwarf_finish(dbg);
        throw std::runtime_error(error_msg);
      }

      //
      if (res == DW_DLV_NO_ENTRY) {
        if (is_info == 1) {
          /*  Done with .debug_info, now check for
              .debug_types. */
          fmt::println("done with debug_info, reading types");
          is_info = 0;
          continue;
        }
        /*  No more CUs to read! Never found */
        fmt::println("No more CUs to read");
        break;
      }

      // we have a DIE
      fmt::println("We have a DIE!");
      record_die_and_siblings_e(dbg, cu_die, is_info, 0, &error);
      dwarf_dealloc_die(cu_die);
    }
  }

  dwarf_finish(dbg);
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