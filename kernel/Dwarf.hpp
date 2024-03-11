/*
 * Copyright (c) YungRaj
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <Types.h>

#include <DwarfV5.hpp>

class Symbol;
class SymbolTable;

class Segment;
class Section;

#include "vector.hpp"

#include "MachO.hpp"
#include "Section.hpp"
#include "Segment.hpp"
#include "Symbol.hpp"
#include "SymbolTable.hpp"

#include "Log.hpp"

namespace Debug {
using namespace Debug;

class Dwarf;
class CompilationUnit;
class DIE;
class LineTable;
class AbbreviationTable;

struct AttrSpec {
    enum DW_AT name;
    enum DW_FORM form;
};

struct AttrAbbrev {
    enum DW_TAG tag;
    enum DW_CHILDREN children;

    struct AttrSpec attr_spec;

    UInt64 code;
};

struct Attribute {
    struct AttrAbbrev abbreviation;

    UInt64 value;
};

Dwarf* parseDebugSymbols(MachO* macho);
Dwarf* parseDebugSymbols(MachO* macho, const char* dSYM);

class DIE {
public:
    explicit DIE(Dwarf* dwarf, UInt64 code, char* name, enum DW_TAG tag,
                 enum DW_CHILDREN has_children);

    enum DW_TAG getTag() {
        return tag;
    }
    enum DW_CHILDREN getHasChildren() {
        return has_children;
    }

    UInt64 getCode() {
        return code;
    }

    Size getAttributesCount() {
        return abbreviationTable.size();
    }

    struct AttrAbbrev* getAttribute(enum DW_AT attr);
    struct AttrAbbrev* getAttribute(int index) {
        return abbreviationTable.at(index);
    }

    char* getName() {
        return name;
    }

    Offset getOffset() {
        return offset;
    }

    void setOffset(Offset offset) {
        this->offset = offset;
    }

    struct AttrAbbrev* getAbbreviation(int index) {
        return this->abbreviationTable.at(index);
    }
    struct AttrAbbrev* getAbbreviation(enum DW_AT attr);

    void addAbbreviation(struct AttrAbbrev* abbreviation) {
        this->abbreviationTable.push_back(abbreviation);
    }
    void removeAbbreviation(struct AttrAbbrev* abbreviation) {
        this->abbreviationTable.erase(
            std::remove(abbreviationTable.begin(), abbreviationTable.end(), abbreviation),
            abbreviationTable.end());
    }

private:
    Dwarf* dwarf;

    UInt64 code;

    std::vector<struct AttrAbbrev*> abbreviationTable;

    enum DW_TAG tag;

    enum DW_CHILDREN has_children;

    char* name;

    DIE* parent;

    UInt32 idx;
    UInt32 sibling_idx;
    UInt32 parent_idx;

    Offset offset;
};

class DwarfDIE {
public:
    explicit DwarfDIE(Dwarf* dwarf, CompilationUnit* unit, DIE* die, DwarfDIE* parent);

    Dwarf* getDwarf() {
        return dwarf;
    }

    DIE* getDebugInfoEntry() {
        return die;
    }

    enum DW_TAG getTag() {
        return die->getTag();
    }
    enum DW_CHILDREN hasChildren() {
        return die->getHasChildren();
    }

    DwarfDIE* getParent() {
        return parent;
    }

    std::vector<DwarfDIE*>& getChildren() {
        return children;
    }

    std::vector<struct Attribute*>& getAttributes() {
        return attributes;
    }

    void addChild(DwarfDIE* child) {
        this->children.push_back(child);
    }
    void removeChild(DwarfDIE* child) {
        this->children.erase(std::remove(children.begin(), children.end(), child), children.end());
    }

    void addAttribute(struct Attribute* attribute) {
        this->attributes.push_back(attribute);
    }
    void addAttributes(std::vector<struct Attribute*>& attrs) {
        for (int i = 0; i < attrs.size(); i++)
            this->attributes.push_back(attrs.at(i));
    }

    struct Attribute* getAttribute(enum DW_AT attr);
    struct Attribute* getAttribute(int index) {
        return this->attributes.at(index);
    }

    UInt64 getAttributeValue(enum DW_AT attr);

private:
    Dwarf* dwarf;

    CompilationUnit* compilationUnit;

    DIE* die;

    DwarfDIE* parent;

    std::vector<DwarfDIE*> children;
    std::vector<struct Attribute*> attributes;
};

#pragma pack(1)

struct CompileUnitHeader {
    UInt32 length;
    UInt16 format;
    UInt16 version;
    UInt16 abbr_offset;
    UInt8 addr_size;
};

#pragma options align = reset

class CompilationUnit {
public:
    explicit CompilationUnit(Dwarf* dwarf, struct CompileUnitHeader* hdr, DIE* die);

    std::vector<DwarfDIE*>* getDebugInfoEntries() {
        return &debugInfoEntries;
    }

    Dwarf* getDwarf() {
        return dwarf;
    }

    LineTable* getLineTable() {
        return lineTable;
    }

    char* getSourceFileName() {
        return source_file;
    }

    void addDebugInfoEntry(DwarfDIE* dwarfDIE) {
        this->debugInfoEntries.push_back(dwarfDIE);
    }

private:
    Dwarf* dwarf;

    DIE* die;

    struct CompileUnitHeader* header;

    std::vector<DwarfDIE*> debugInfoEntries;

    LineTable* lineTable;

    char* source_file;
    char* source_language;

    UInt16 dwarf_version;

    UInt8 unit_type;
    UInt64 dwo_id;

    UInt64 unit_type_signature;
    UInt64 unit_type_offset;

    xnu::Mach::VmAddress address_base = 0;
    xnu::Mach::VmAddress str_offsets_base = 0;
    xnu::Mach::VmAddress range_lists_base = 0;
};

#pragma pack(1)

struct LTPrologue {
    UInt32 total_length;
    UInt8 format;
    UInt8 version;
    UInt32 prologue_length;
    UInt8 min_inst_length;
    UInt8 max_ops_per_inst;
    UInt8 default_is_stmt;
    int8_t line_base;
    UInt8 line_range;
    UInt8 opcode_base;
};

struct LTStandardOpcodeLengths {
    DW_LNS copy;
    DW_LNS advance_pc;
    DW_LNS advance_line;
    DW_LNS set_file;
    DW_LNS set_column;
    DW_LNS negate_stmt;
    DW_LNS set_basic_block;
    DW_LNS const_add_pc;
    DW_LNS fixed_advance_pc;
    DW_LNS set_prologue_end;
    DW_LNS set_epilogue_begin;
    DW_LNS set_isa;
};

struct LTSourceFileMetadata {
    UInt8 dir_index;
    UInt8 mod_time;
    UInt8 length;
};

#pragma options align = reset

struct LTSourceFile {
    char* source_file;

    struct LTSourceFileMetadata metadata;
};

struct LTStateMachine {
    UInt64 address;

    UInt8 isa;
    int64_t line;
    UInt64 column;
    UInt16 file;
    UInt32 discriminator;

    UInt8 statement : 1, basic_block : 1, end_sequence : 1, prologue_end : 1, epilogue_begin : 1;
};

struct LTSourceLine {
    struct LTSourceFile* source_file;

    struct LTStateMachine state;
};

struct Sequence {
    UInt64 LowPC;
    UInt64 HighPC;

    Segment* segment;
    Section* section;

    std::vector<LTSourceLine*> sourceLines;
};

static LTStateMachine gInitialState = {.address = 0,
                                       .isa = 0,
                                       .line = 1,
                                       .column = 1,
                                       .file = 1,
                                       .discriminator = 1,
                                       .statement = 1,
                                       .basic_block = 0,
                                       .end_sequence = 0,
                                       .prologue_end = 0,
                                       .epilogue_begin = 0};

class LineTable {
public:
    explicit LineTable(MachO *binary, Dwarf* dwarf) : macho(binary), dwarf(dwarf) {}

    std::vector<struct LTSourceFile*>* getSourceFileNames() {
        return &files;
    }
    std::vector<char*>* getIncludeDirectories() {
        return &include_directories;
    }

    CompilationUnit* getCompilationUnit() {
        return compilationUnit;
    }

    LTSourceLine* getSourceLine(xnu::Mach::VmAddress pc);

    LTSourceFile* getSourceFile(int index) {
        return this->files.at(index);
    }

    void setCompilationUnit(CompilationUnit* cu) {
        this->compilationUnit = cu;
    }

    void setPrologue(struct LTPrologue* p) {
        memcpy(&prologue, p, sizeof(struct LTPrologue));
    }

    void setStandardOpcodeLengths(struct LTStandardOpcodeLengths* opcodes) {
        memcpy(&standardOpcodeLengths, opcodes, sizeof(struct LTStandardOpcodeLengths));
    }

    void addSequence(Sequence* sequence) {
        this->sources.push_back(sequence);
    }
    void addSourceFile(struct LTSourceFile* file) {
        this->files.push_back(file);
    }
    void addIncludeDirectory(char* directory) {
        this->include_directories.push_back(directory);
    }

private:
    MachO* macho;

    Dwarf* dwarf;

    struct LTPrologue prologue;

    struct LTStandardOpcodeLengths standardOpcodeLengths;

    CompilationUnit* compilationUnit;

    std::vector<Sequence*> sources;

    std::vector<char*> include_directories;
    std::vector<struct LTSourceFile*> files;
};

struct LocationTableEntry {
    DW_LLE kind;

    UInt32 offset;

    UInt64 value0;
    UInt64 value1;

    Segment* segment;

    std::vector<DW_OP> location_ops;
};

#pragma pack(1)

struct AddressRange {
    xnu::Mach::VmAddress start;
    xnu::Mach::VmAddress end;
};

struct AddressRangeHeader {
    UInt32 length;
    UInt16 version;
    UInt32 offset;
    UInt8 addr_size;
    UInt8 seg_size;
};

struct AddressRangeEntry {
    struct AddressRangeHeader header;

    std::vector<struct AddressRange*> ranges;
};

struct RangeEntry {
    UInt32 offset;

    UInt64 value0;
    UInt64 value1;
};

using RangeEntries = std::vector<struct RangeEntry*>;

#pragma options align = reset

class Dwarf {
public:
    explicit Dwarf(const char* debugSymbols);
    explicit Dwarf(MachO* macho, const char* debugSymbols);

    CompilationUnit getCompilationUnit(const char* source_file);

    DIE* getDebugInfoEntryByName(const char* name);
    DIE* getDebugInfoEntryByCode(UInt64 code);

    Segment* getDwarfSegment() {
        return dwarf;
    }

    Section* getDebugLine() {
        return __debug_line;
    }
    Section* getDebugLoc() {
        return __debug_loc;
    }
    Section* getDebugAranges() {
        return __debug_aranges;
    }
    Section* getDebugInfo() {
        return __debug_info;
    }
    Section* getDebugAbbrev() {
        return __debug_abbrev;
    }
    Section* getDebugStr() {
        return __debug_str;
    }

    Section* getAppleNames() {
        return __apple_names;
    }
    Section* getAppleNamespac() {
        return __apple_namespac;
    }
    Section* getAppleTypes() {
        return __apple_types;
    }
    Section* getAppleObjc() {
        return __apple_objc;
    }

    std::vector<CompilationUnit*>* getCompilationUnits() {
        return &compilationUnits;
    }
    std::vector<LineTable*>* getLineTables() {
        return &lineTables;
    }

    LineTable* getLineTable(const char* name);
    LineTable* getLineTable(CompilationUnit* unit);

    DIE* getUnit(const char* name);
    DIE* getFunction(const char* name);
    DIE* getType(const char* name);

    void populateDebugSymbols();

    void parseDebugAbbrev();
    void parseDebugInfo();
    void parseDebugLocations();
    void parseDebugLines();
    void parseDebugRanges();
    void parseDebugAddressRanges();

    const char* getSourceFile(xnu::Mach::VmAddress instruction);
    int64_t getSourceLineNumber(xnu::Mach::VmAddress instruction);

private:
    MachO* macho;
    MachO* machoWithDebug;

    std::vector<DIE*> dies;
    std::vector<CompilationUnit*> compilationUnits;

    std::vector<LineTable*> lineTables;

    std::vector<struct LocationTableEntry*> locationTable;

    std::vector<RangeEntries*> ranges;
    std::vector<struct AddressRangeEntry*> addressRanges;

    Segment* dwarf;

    Section* __debug_line;
    Section* __debug_loc;
    Section* __debug_aranges;
    Section* __debug_info;
    Section* __debug_ranges;
    Section* __debug_abbrev;
    Section* __debug_str;
    Section* __apple_names;
    Section* __apple_namespac;
    Section* __apple_types;
    Section* __apple_objc;
};

UInt64 GetStringSize(UInt8* p);

UInt64 ReadUleb128(UInt8* p, UInt8* end);
UInt64 ReadUleb128(UInt8* p, UInt8* end, UInt32* idx);

int64_t ReadSleb128(UInt8* p, UInt8* end);
int64_t ReadSleb128(UInt8* p, UInt8* end, UInt32* idx);
}; // namespace Debug
