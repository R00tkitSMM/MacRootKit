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

#include <mach/mach_types.h>

#include <vector>

#include <Types.h>

#include "PAC.hpp"

class Segment;
class Section;

class MachO;

namespace mrk {
class UserMachO;
}

namespace ObjectiveC {
#ifdef __arm64e__

static const uintptr_t bigSignedMethodListFlag = 0x8000000000000000;

#else

static const uintptr_t bigSignedMethodListFlag = 0x0;

#endif

static const UInt32 smallMethodListFlag = 0x80000000;

static const UInt32 relativeMethodSelectorsAreDirectFlag = 0x40000000;

#define RO_META (1 << 0)
#define RO_ROOT (1 << 1)
#define RO_HAS_CXX_STRUCTORS (1 << 2)

#define RO_HIDDEN (1 << 4)
#define RO_EXCEPTION (1 << 5)
#define RO_HAS_SWIFT_INITIALIZER (1 << 6)
#define RO_IS_ARC (1 << 7)
#define RO_HAS_CXX_DTOR_ONLY (1 << 8)
#define RO_HAS_WEAK_WITHOUT_ARC (1 << 9)
#define RO_FORBIDS_ASSOCIATED_OBJECTS (1 << 10)

#define RO_FROM_BUNDLE (1 << 29)
#define RO_FUTURE (1 << 30)
#define RO_REALIZED (1 << 31)

#define RW_REALIZED (1 << 31)
#define RW_FUTURE (1 << 30)
#define RW_INITIALIZED (1 << 29)
#define RW_INITIALIZING (1 << 28)
#define RW_COPIED_RO (1 << 27)
#define RW_CONSTRUCTING (1 << 26)
#define RW_CONSTRUCTED (1 << 25)

#define RW_LOADED (1 << 23)
#if !SUPPORT_NONPOINTER_ISA
#define RW_INSTANCES_HAVE_ASSOCIATED_OBJECTS (1 << 22)
#endif

#define RW_HAS_INSTANCE_SPECIFIC_LAYOUT (1 << 21)
#define RW_FORBIDS_ASSOCIATED_OBJECTS (1 << 20)
#define RW_REALIZING (1 << 19)

#if CONFIG_USE_PREOPT_CACHES
#define RW_NOPREOPT_SELS (1 << 2)
#define RW_NOPREOPT_CACHE (1 << 1)
#endif

#define RW_META RO_META // (1<<0)

#define FAST_IS_SWIFT_LEGACY (1UL << 0)
#define FAST_IS_SWIFT_STABLE (1UL << 1)
#define FAST_HAS_DEFAULT_RR (1UL << 2)
#define FAST_DATA_MASK 0x00007ffffffffff8UL

#define FAST_FLAGS_MASK 0x0000000000000007UL
#define FAST_IS_RW_POINTER 0x8000000000000000UL

#define RW_HAS_CXX_CTOR (1 << 18)
#define RW_HAS_CXX_DTOR (1 << 17)
#define RW_HAS_DEFAULT_AWZ (1 << 16)

#define RW_REQUIRES_RAW_ISA (1 << 15)
#define RW_HAS_DEFAULT_RR (1 << 14)
#define RW_HAS_DEFAULT_CORE (1 << 13)

#define FAST_IS_SWIFT_LEGACY (1UL << 0)
#define FAST_IS_SWIFT_STABLE (1UL << 1)

class Ivar;
class Property;

class Method;

class Category;
class Protocol;

class ObjCClass;

class ObjCData;

struct _objc_ivar {
    UInt64 offset;
    UInt64 name;
    UInt64 type;
    UInt8 size;
};

enum _objc_method_type {
    _objc_method_invalid_type = 0,
    _objc_method_instance_type,
    _objc_method_class_type
};

struct _objc_method {
    UInt32 name;
    UInt32 type;
    UInt32 offset;
};

struct _objc_protocol {
    char* name;
    UInt64 offset;
    struct _objc_method* method;
    UInt32 methodCount;
};

struct objc_category {
    char* category_name;
    char* class_name;

    struct _objc_method* instance_methods;
    struct _objc_method* class_methods;
    struct _objc_protocol* protocols;
};

struct _objc_class {
    struct _objc_class* superCls;
    char* className;
    struct _objc_ivar* ivar;
    UInt32 ivarCount;
    struct _objc_method* method;
    UInt32 methodCount;
    struct _objc_protocol* protocol;
    UInt32 protocolCount;
};

struct _objc_module {
    char* impName;
    struct _objc_class* symbol;
};

struct _objc_module_raw {
    UInt32 version;
    UInt32 size;
    UInt32 name;
    UInt32 symtab;
};

enum _objc_2_class_type {
    _objc_2_class_invalid_type = 0,
    _objc_2_class_class_type,
    _objc_2_class_metaclass_type
};

#define kObjc2SelRef "__objc_selrefs"
#define kObjc2MsgRefs "__objc_msgrefs"
#define kObjc2ClassRefs "__objc_classrefs"
#define kObjc2SuperRefs "__objc_superrefs"
#define kObjc2ClassList "__objc_classlist"
#define kObjc2NlClsList "__objc_nlclslist"
#define kObjc2CatList "__objc_catlist"
#define kObjc2NlCatList "__objc_nlcatlist"
#define kObjc2ProtoList "__objc_protolist"
#define kObjc2ProtoRefs "__objc_protorefs"

struct _objc_2_class_method_info {
    UInt32 entrySize;
    UInt32 count;
};

struct _objc_2_class_protocol_info {
    UInt64 count;
};

struct _objc_2_class_ivar_info {
    UInt32 entrySize;
    UInt32 count;
};

struct _objc_2_class_property_info {
    UInt32 entrySize;
    UInt32 count;
};

struct _objc_2_class_method {
    UInt32 name;
    UInt32 type;
    Int32 imp;
};

struct _objc_2_method {
    UInt64 name;
    UInt64 type;
    UInt64 imp;
};

struct _objc_2_class_protocol {
    UInt64 isa;
    UInt64 name;

    UInt64 protocols;
    UInt64 instance_methods;
    UInt64 class_methods;
    UInt64 opt_instance_methods;
    UInt64 opt_class_methods;
    UInt64 instance_properties;

    UInt32 cb;
    UInt32 flags;
};

struct _objc_2_category {
    UInt64 category_name;
    UInt64 class_name;

    UInt64 instance_methods;
    UInt64 class_methods;
    UInt64 protocols;
    UInt64 properties;
};

struct _objc_2_class_ivar {
    UInt64 offset;
    UInt64 name;

    UInt64 type;
    UInt32 align;
    UInt32 size;
};

struct _objc_2_class_property {
    UInt64 name;

    UInt64 attributes;
};

struct _objc_2_class_data {
    UInt32 flags;
    UInt32 instanceStart;
    UInt32 instanceSize;
    UInt32 reserved;
    UInt64 iVarLayout;
    UInt64 name;

    UInt64 methods;
    UInt64 protocols;
    UInt64 ivars;

    UInt64 weakIVarLayout;
    UInt64 properties;
};

struct _objc_2_class {
    UInt64 isa;
    UInt64 superclass;
    UInt64 cache;
    UInt64 vtable;

    struct _objc_2_class_data* data;
};

struct _objc_2_class_rw_t {
    UInt32 flags;
    UInt16 witness;

    UInt64 ro_or_rw_ext;

    UInt64 firstSubclass;
    UInt64 nextSiblingClass;
};

struct _objc_2_class_rw_ext_t {
    UInt64 ro;

    UInt64 methods;
    UInt64 properties;
    UInt64 protocols;

    UInt64 demangledName;

    UInt32 version;
};
}; // namespace ObjectiveC

typedef void* id;

namespace ObjectiveC {
UInt64 getClass(const char* name);

const char* class_getName(UInt64 cls);
UInt64 class_getSuperClass(UInt64 cls);
UInt64 class_getSuperClass(UInt64 cls, UInt64 new_super);
bool class_isMetaClass(UInt64 cls);
UInt64 class_getInstanceVariable(UInt64 cls, const char* name);
UInt64 class_getClassVariable(UInt64 cls, const char* name);
bool class_addIvar(UInt64 cls, const char* name, Size size, UInt8 alignment, const char* types);
UInt64 class_getProperty(UInt64 cls, const char* name);
bool class_addMethod(UInt64 cls, char* name, UInt64 UInt64, const char* types);
UInt64 class_getInstanceMethod(UInt64 cls, char* name);
UInt64 class_getClassMethod(UInt64 cls, char* name);
UInt64 class_getMethodImplementation(UInt64 cls, char* name);
bool class_addProtocol(UInt64 cls, UInt64 protocol);
bool class_addProperty(UInt64 cls, const char* name, UInt64 attributes,
                       unsigned int attributeCount);

UInt64 object_getClass(id obj);
void object_setInstanceVariable(id obj, const char* name, void* value);
UInt64 object_getInstanceVariable(id obj, const char* name);
void object_setIvar(id obj, const char* name, id value);
UInt64 object_getIvar(id obj, UInt64 ivar);
const char* object_getClassName(id obj);
UInt64 object_getClass(id obj);
UInt64 object_setClass(id obj, UInt64 cls);

char* method_getName(UInt64 m);
UInt64 method_getImplementation(UInt64 m);
UInt64 method_setImplementation(UInt64 m, UInt64 UInt64);
void method_exchangeImplementations(UInt64 m1, UInt64 m2);

UInt64 object_getDataBits(id obj);
bool object_isObjectiveC(id obj);
bool object_isSwift(id obj);

class ObjC {
public:
    virtual ~ObjC() = default;

    ObjCData* getMetadata() {
        return metadata;
    }

protected:
    ObjCData* metadata;
};

ObjCData* parseObjectiveC(mrk::UserMachO* macho);

std::vector<ObjCClass*>* parseClassList(ObjCData* data);
std::vector<Category*>* parseCategoryList(ObjCData* data);
std::vector<Protocol*>* parseProtocolList(ObjCData* data);

enum MethodType {
    INSTANCE_METHOD = 0,
    CLASS_METHOD,
    OPT_INSTANCE_METHOD,
    OPT_CLASS_METHOD,
};

void parseMethodList(ObjCData* metadata, ObjC* object, std::vector<Method*>* methodList,
                     enum MethodType methtype, struct _objc_2_class_method_info* methodInfo);

void parsePropertyList(ObjCData* metadata, ObjC* object, std::vector<Property*>* propertyList,
                       struct _objc_2_class_property_info* propertyInfo);

UInt64 findSelectorsBase(mrk::UserMachO* macho);

class Protocol : public ObjC {
public:
    Protocol(ObjCData* data, struct _objc_2_class_protocol* prot);

    char* getName() {
        return name;
    }

    std::vector<Method*>& getInstanceMethods() {
        return instance_methods;
    }
    std::vector<Method*>& getClassMethods() {
        return class_methods;
    }

    std::vector<Method*>& getOptionalInstanceMethods() {
        return optional_instance_methods;
    }
    std::vector<Method*>& getOptionalClassMethods() {
        return optional_class_methods;
    }

    std::vector<Property*>& getInstanceProperties() {
        return instance_properties;
    }

    UInt64 getOffset() {
        return offset;
    }

private:
    struct _objc_2_class_protocol* protocol;

    char* name;

    UInt64 offset;

    std::vector<Method*> instance_methods;
    std::vector<Method*> class_methods;

    std::vector<Method*> optional_instance_methods;
    std::vector<Method*> optional_class_methods;

    std::vector<Property*> instance_properties;
};

class Category : public ObjC {
public:
    Category(ObjCData* data, struct _objc_2_category* cat);

    char* getName() {
        return name;
    }

    char* getClassName() {
        return class_name;
    }

    std::vector<Method*>& getInstanceMethods() {
        return instance_methods;
    }
    std::vector<Method*>& getClassMethods() {
        return class_methods;
    }

    std::vector<Property*>& getProperties() {
        return properties;
    }

private:
    struct _objc_2_category* category;

    char* name;
    char* class_name;

    std::vector<Method*> class_methods;
    std::vector<Method*> instance_methods;

    std::vector<Property*> properties;
};

class Ivar {
public:
    Ivar(ObjC* object, struct _objc_2_class_ivar* ivar);

    char* getName() {
        return name;
    }

    UInt64 getOffset() {
        return offset;
    }

    UInt64 getType() {
        return type;
    }

    Size getSize() {
        return size;
    }

private:
    ObjC* object;

    struct _objc_2_class_ivar* ivar;

    char* name;

    UInt64 offset;

    UInt64 type;

    Size size;
};

class Property {
public:
    Property(ObjC* object, struct _objc_2_class_property* property);

    char* getName() {
        return name;
    }

    char* getAttributes() {
        return attributes;
    }

private:
    ObjC* object;

    struct _objc_2_class_property* property;

    char* name;

    char* attributes;
};

class Method {
public:
    Method(ObjC* object, struct _objc_2_class_method* method);
    Method(ObjC* object, struct _objc_2_method* method);

    char* getName() {
        return name;
    }

    UInt64 getType() {
        return type;
    }

    UInt64 getImpl() {
        return impl;
    }

private:
    ObjC* object;

    struct _objc_2_class_method* method;

    char* name;

    UInt64 type;

    UInt64 impl;
};

class ObjCClass : public ObjC {
public:
    ObjCClass(ObjCData* metadata, struct _objc_2_class* c, bool metaclass);

    char* getName() {
        return name;
    }

    struct _objc_2_class* getClass() {
        return cls;
    }

    struct _objc_2_class_data* getData() {
        return data;
    }

    mrk::UserMachO* getMachO() {
        return macho;
    }

    ObjCClass* getSuperClass() {
        return super;
    }

    UInt64 getIsa() {
        return isa;
    }

    UInt64 getCache() {
        return cache;
    }

    UInt64 getVtable() {
        return vtable;
    }

    Method* getMethod(char* methodname);

    Protocol* getProtocol(char* protocolname);

    Ivar* getIvar(char* ivarname);

    Property* getProperty(char* propertyname);

    std::vector<Method*>& getMethods() {
        return methods;
    }

    std::vector<Protocol*>& getProtocols() {
        return protocols;
    }

    std::vector<Ivar*>& getIvars() {
        return ivars;
    }

    std::vector<Property*>& getProperties() {
        return properties;
    }

    bool isValid() {
        return (name && isa);
    }

    void parseMethods();

    void parseProtocols();

    void parseIvars();

    void parseProperties();

private:
    mrk::UserMachO* macho;

    bool metaclass;

    struct _objc_2_class* cls;

    struct _objc_2_class_data* data;

    char* name;

    ObjCClass* super;

    UInt64 isa;
    UInt64 superclass;
    UInt64 cache;
    UInt64 vtable;

    std::vector<Method*> methods;

    std::vector<Protocol*> protocols;

    std::vector<Ivar*> ivars;

    std::vector<Property*> properties;
};

class ObjCData {
public:
    ObjCData(mrk::UserMachO* macho) {
        this->macho = macho;

        this->parseObjC();
    }

    mrk::UserMachO* getMachO() {
        return macho;
    }

    Segment* getDataSegment() {
        return data;
    }
    Segment* getDataConstSegment() {
        return data_const;
    }

    Section* getClassList() {
        return classlist;
    }
    Section* getCategoryList() {
        return catlist;
    }
    Section* getProtocolList() {
        return protolist;
    }

    Section* getSelRefs() {
        return selrefs;
    }
    Section* getProtoRefs() {
        return protorefs;
    }
    Section* getClassRefs() {
        return classrefs;
    }
    Section* getSuperRefs() {
        return superrefs;
    }

    Section* getIvars() {
        return ivar;
    }
    Section* getObjcData() {
        return objc_data;
    }

    void parseObjC();

    ObjCClass* getClassByName(char* classname);
    ObjCClass* getClassByIsa(UInt64 isa);

    Protocol* getProtocol(char* protoname);

    Category* getCategory(char* catname);

    Method* getMethod(char* classname, char* methodname);

    Ivar* getIvar(char* classname, char* ivarname);

    Property* getProperty(char* classname, char* propertyname);

private:
    mrk::UserMachO* macho;

    std::vector<ObjCClass*> classes;
    std::vector<Category*> categories;
    std::vector<Protocol*> protocols;

    Segment* data;
    Segment* data_const;

    Section* classlist;
    Section* catlist;
    Section* protolist;

    Section* selrefs;
    Section* protorefs;
    Section* classrefs;
    Section* superrefs;

    Section* ivar;
    Section* objc_data;
};
}; // namespace ObjectiveC
