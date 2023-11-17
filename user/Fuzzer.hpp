#ifndef __FUZZER_HPP_
#define __FUZZER_HPP_

#include <type_traits>

extern "C"
{
	#include <stdio.h>
	#include <string.h>
};

#include <vector>

#include "Kernel.hpp"

#include "BinaryFormat.hpp"

#include "MachO.hpp"
#include "KernelMachO.hpp"
#include "KextMachO.hpp"

#include "PortableExecutable.hpp"
#include "TerseExecutable.hpp"

#include "ELF.hpp"

template <typename T>
concept ClassType = std::is_class_v<T>;

template<typename T>
concept VoidPointerType = std::is_same_v<std::remove_pointer_t<T>, void>;

template <typename T>
concept PointerToClassType = (std::is_pointer_v<T> && std::is_class_v<std::remove_pointer_t<T>>) || VoidPointerType<T>;

template <typename T>
concept FundamentalType = std::is_fundamental_v<T>;

template <typename T>
concept ScalarType = std::is_scalar_v<T>;

template <typename T>
concept PodType = std::is_pod_v<T>;

template<typename T>
concept IntegralType = std::is_integral_v<T>;

template<typename T>
concept PointerType = std::is_pointer_v<T>;

template<typename T>
concept IntegralOrPointerType = IntegralType<T> || PointerType<T>;

template<typename T>
concept CastableType = FundamentalType<T> || PodType<T> || IntegralType<T> || PointerType<T>;

namespace Virtualization
{
	class Hypervisor;
};

namespace Fuzzer
{
	class Loader;
	class Module;

	enum LanguageType
	{
		LANG_TYPE_C,
		LANG_TYPE_CXX,
		LANG_TYPE_OBJC,
		LANG_TYPE_SWIFT,
		LANG_TYPE_RUST
	};

	template<LanguageType LangType>
	concept ManglableLang = LangType == LANG_TYPE_CXX || LangType == LANG_TYPE_SWIFT;

	template <typename T, typename Sym>
	concept HasCompatibleSymbol = std::is_same_v<Sym, decltype(std::declval<T>()->getSymbol(nullptr))>;

	template <typename T, typename Seg>
	concept HasCompatibleSegment = std::is_same_v<Seg, decltype(std::declval<T>()->getSegment(nullptr))>;

	template <typename T, typename Sect>
	concept HasCompatibleSection = std::is_same_v<Sect, decltype(std::declval<T>()->getSection(nullptr))>;

	template <typename T, typename Sym, typename Seg>
	concept MappableBinary = std::is_base_of_v<Binary::BinaryFormat, std::remove_pointer_t<T>> && HasCompatibleSymbol<T, Sym> && (HasCompatibleSegment<T, Seg> || HasCompatibleSection<T, Seg>);

	template<typename T, typename Sym, typename Sect>
	struct LinkerMap
	{
		static_assert(MappableBinary<T, Sym, Sect>);

		public:
			explicit LinkerMap(T binary, char *map)
					: binary(binary), mapFilePath(map) { }

			T getBinary() const { return binary; }

			char* getMapFilePath() const { return mapFilePath; }

			char* getMapFile() const { return mapFile; }

			std::vector<Sym>& getSymbols() { return symbols; }
			std::vector<Sect>& getSections() { return sections; }

			size_t getSymbolCount() { return symbols.size(); }

			size_t getSectionCount() { return sections.size(); }

			void read();

		private:
			T binary;

			std::vector<Sym> symbols;
			std::vector<Sect> sections;

			char *mapFilePath;
			char *mapFile;
	};

	struct RawBinary : public Binary::BinaryFormat
	{

		struct SectionRaw
		{
			public:
				explicit SectionRaw(char *name, mach_vm_address_t address, size_t size, int prot, int idx) : name(name), address(address), size(size), prot(prot), idx(idx) { }

				char* getName() const { return name; }

				template<typename T>
				T operator[](uint64_t index) const { return reinterpret_cast<T>((uint8_t*) address + index); }

				mach_vm_address_t getAddress() const { return address; }

				template<typename T>
				T getAddressAs() const
				{
					return reinterpret_cast<T>(address);
				}

				size_t getSize() const { return size; }

				int getProt() const { return prot; }

				int getIndex() const { return idx; }

			private:
				char *name;

				mach_vm_address_t address;

				size_t size;

				int prot;
				int idx;

		};

		struct SymbolRaw
		{
			public:
				explicit SymbolRaw(char *name, mach_vm_address_t address, int type) : name(name), address(address), type(type) { }

				char* getName() const { return name; }

				template<LanguageType LangType> requires ManglableLang<LangType>
				char* getDemangledName()
				{
					if constexpr (LangType == LANG_TYPE_SWIFT)
					{
						char *_swift_demangle = swift_demangle(getName());

						if(_swift_demangle)
							return _swift_demangle;
					}

					if constexpr (LangType == LANG_TYPE_CXX)
					{
						char *_cxx_demangle = cxx_demangle(getName());

						if(_cxx_demangle)
							return _cxx_demangle;
					}

					char *empty = new char[1];

					*empty = '\0';

					return empty;
				}

				mach_vm_address_t getAddress() const { return address; }

				int getType() const { return type; }

				bool isNamed() { return !name || strcmp(name, "") == 0; }

				bool isUndefined() const { return false; }

				bool isExternal() const { return false; }

			private:
				char *name;

				mach_vm_address_t address;

				int type;
		};

		public:

			explicit RawBinary(char *path, char *mapFile) : path(path), mapFile(mapFile) { }

			template<typename T>
			T operator[](uint64_t index) const { return reinterpret_cast<T>((uint8_t*) base + index); }

			mach_vm_address_t getBase() const { return base; }

			template<typename T>
			T getBaseAs() const
			{
				return reinterpret_cast<T>(base);
			}

			char* getMapFile() const { return mapFile; }

			std::vector<SymbolRaw*>& getAllSymbols() { return linkerMap->getSymbols(); }

			SymbolRaw* getSymbol(const char *name)
			{
				std::vector<SymbolRaw*> &symbols = linkerMap->getSymbols();

				for(int i = 0; i < symbols.size(); i++)
				{
					SymbolRaw *sym = symbols.at(i);

					if(strcmp(name, sym->getName()) == 0)
						return sym;
				}

				return NULL;
			}

			SectionRaw* getSection(const char *name)
			{
				std::vector<SectionRaw*> &sections = linkerMap->getSections();

				for(int i = 0; i < sections.size(); i++)
				{
					SectionRaw *sect = sections.at(i);

					if(strcmp(name, sect->getName()) == 0)
						return sect;
				}

				return NULL;
			}

			void populateSymbols();
			void populateSections();

		private:
			mach_vm_address_t base;

			char *path;

			char *mapFile;

			LinkerMap<RawBinary*, SymbolRaw*, SectionRaw*> *linkerMap;
	};

	template<typename T>
	concept BinaryFmt = std::is_same_v<T, Binary::BinaryFormat*>;

	template<typename T>
	concept MachOFormat = std::is_base_of_v<MachO, std::remove_pointer_t<T>> || std::is_same_v<T, MachO*>;

	template<typename T>
	concept ELFFormat = std::is_same_v<T, ELF*>;

	template<typename T>
	concept PEFormat = std::is_same_v<T, PE*>;

	template<typename T>
	concept TEFormat = std::is_same_v<T, TE*>;

	template<typename T>
	concept RawBinaryFormat = std::is_same_v<T, RawBinary*>;

	template<typename T>
	concept AnyBinaryFormat = BinaryFmt<T> || MachOFormat<T> || ELFFormat<T> || PEFormat<T> || TEFormat<T> || RawBinaryFormat<T> || VoidPointerType<T>;

	static_assert(MachOFormat<xnu::KernelMachO*>, "KernelMachO does not satisfy MachOFormat constraint");
	static_assert(MachOFormat<xnu::KextMachO*>, "KextMachO does not satisfy MachOFormat constraint");

	static_assert(RawBinaryFormat<RawBinary*>);

	using SymbolRaw = RawBinary::SymbolRaw;
	using SectionRaw = RawBinary::SectionRaw;

	template <typename T>
	using GetSymbolReturnType = decltype(std::declval<T>()->getSymbol(nullptr));

	template <typename T>
	using GetSegmentReturnType = decltype(std::declval<T>()->getSegment(nullptr));

	template <typename T>
	using GetSectionReturnType = decltype(std::declval<T>()->getSection(nullptr));

	static_assert(std::is_same_v<GetSymbolReturnType<MachO*>, Symbol*>);
	static_assert(std::is_same_v<GetSymbolReturnType<RawBinary*>, SymbolRaw*>);

	struct FuzzBinary
	{
		const char *path;

		void *base;
		void *originalBase;

		size_t size;

		template <PointerToClassType T> requires AnyBinaryFormat<T>
		union Bin
		{
			/* We know the binary is a any of the below BinaryFormats */
			T bin;

			/* Support BinaryFormat */
			Binary::BinaryFormat *binFmt;

			/* Support MachO */
			MachO *macho;

			/* Support Linux Binary fuzzing on ELFs */
			ELF *elf;

			/* Support EFI fuzzing on PE32/TE */
			PE *portableExecutable;

			TE *terseExecutable;

			/* Support Raw Binary */
			RawBinary *raw;

			/* Support void* to use type punning */
			void* binPtr;

			/* This union constrains the Binary Format types */
			/* The following binary formats are supported but may not be implemented */

			Bin() : binPtr(nullptr) {}

			Bin(T ptr) : bin(ptr) {}

			template <typename U> requires AnyBinaryFormat<U>
			Bin(const Bin<U>& other)
			{
				static_assert(std::is_convertible_v<U, T>, "Incompatible BinaryFormat types for type punning");
				
				binPtr = static_cast<T>(static_cast<U>(other.binPtr));
			}
		};

		template<PointerToClassType T> requires AnyBinaryFormat<T>
		static constexpr Bin<T> MakeBinary(T ptr)
		{
		    return Bin<T>(ptr);
		}

		using AnyBinary = Bin<void*>;

		AnyBinary binary;

		template<PointerToClassType B> requires AnyBinaryFormat<B>
		B getBinary() { return reinterpret_cast<B>(binary.binFmt); }

		template<typename T> requires CastableType<T>
		T operator[](uint64_t index) const { return reinterpret_cast<T>((uint8_t*) base + index); }
		
		const char* getPath() const { return path; }

		template<typename T> requires IntegralOrPointerType<T>
		T getBase() const
		{
			return reinterpret_cast<T>(base);
		}

		template<typename T> requires IntegralOrPointerType<T>
		T getOriginalBase() const
		{
			return reinterpret_cast<T>(base);
		}

		template<typename Sym, typename Binary>
		Sym getSymbol(char *symbolname) requires requires (Sym sym, Binary bin)
		{
			sym->getName();
			sym->getAddress();
			std::is_same_v<GetSymbolReturnType<Binary>, Sym>;
		}
		{
			static_assert(AnyBinaryFormat<Binary>,
		                  "Unsupported type for FuzzBinary:getSymbol()");

		    if constexpr (std::is_base_of_v<MachO, std::remove_pointer_t<Binary>>)
		    {
		        return dynamic_cast<Sym>(binary.macho->getSymbol(symbolname));
		    }

		    if constexpr (std::is_same_v<Binary, MachO*>)
		    {
		    	return binary.macho->getSymbol(symbolname);
		    }

		    if constexpr (std::is_same_v<Binary, RawBinary*>)
		    {
		    	return binary.raw->getSymbol(symbolname);
		    }

		    return NULL;
		}

		template<typename Seg, typename Binary>
		Seg getSegment(char *segname) requires requires (Seg seg, Binary bin)
		{
			seg->getName();
			seg->getAddress();
			std::is_same_v<GetSegmentReturnType<Binary>, Seg> || std::is_same_v<GetSectionReturnType<Binary>, Seg>;
		}
		{
			static_assert(AnyBinaryFormat<Binary>,
		                  "Unsupported type for FuzzBinary:getSegment()");

		    if constexpr (std::is_base_of_v<MachO, std::remove_pointer_t<Binary>>)
		    {
		        return dynamic_cast<Seg>(binary.macho->getSegment(segname));
		    }

		    if constexpr (std::is_same_v<Binary, MachO*>)
		    {
		        return binary.macho->getSegment(segname);
		    }

		    if constexpr (std::is_same_v<Binary, RawBinary*>)
		    {
		        return binary.raw->getSection(segname);
		    }

		    return NULL;
		}
	};

	static_assert(std::is_same_v<GetSegmentReturnType<MachO*>, Segment*>);
	static_assert(std::is_same_v<GetSectionReturnType<RawBinary*>, SectionRaw*>);

	static_assert(AnyBinaryFormat<decltype(FuzzBinary::binary.macho)> &&
				  AnyBinaryFormat<decltype(FuzzBinary::binary.elf)>&&
				  AnyBinaryFormat<decltype(FuzzBinary::binary.portableExecutable)> &&
				  AnyBinaryFormat<decltype(FuzzBinary::binary.terseExecutable)> &&
				  AnyBinaryFormat<decltype(FuzzBinary::binary.raw)>,
				  "All types in the union must satisfy the BinaryFormat concept");

	template <typename T>
	concept FuzzableType = ClassType<T> || FundamentalType<T> || PodType<T> || IntegralOrPointerType<T>;

	class Harness
	{
		public:
			explicit Harness(xnu::Kernel *kernel);

			explicit Harness(const char *binary);
			explicit Harness(const char *binary, const char *mapFile);

			~Harness();

			struct FuzzBinary* getFuzzBinary() const { return fuzzBinary; }

			template<typename Sym>
			std::vector<Sym>* getSymbols() requires requires (Sym sym)
			{
				sym->getName();
				sym->getAddress();
			};

			template<typename T> requires AnyBinaryFormat<T> && PointerToClassType<T>
			T getBinary()
			{
			    static_assert(AnyBinaryFormat<T>,
			                  "Unsupported type for Harness::getBinary()");

			    if constexpr (std::is_base_of_v<MachO, std::remove_pointer_t<T>>)
			    {
			        return dynamic_cast<T>(this->fuzzBinary->binary.macho);
			    }

			    if constexpr (std::is_same_v<T, MachO*>)
			    {
			    	return this->fuzzBinary->binary.macho;
			    }

			    if constexpr (std::is_same_v<T, RawBinary*>)
			    {
			        return this->fuzzBinary->binary.raw;
			    }

			    return NULL;
			}

			Fuzzer::Loader* getLoader() const { return loader; }

			char* getMapFile() const { return mapFile; }

			template <int CpuType>
			char* getMachOFromFatHeader(char *file_data);

			template<typename Binary, typename Seg> requires AnyBinaryFormat<Binary>
			bool mapSegments(char *file_data, char *mapFile);

			template<typename Binary, typename Seg> requires AnyBinaryFormat<Binary>
			bool unmapSegments();

			template<typename Binary> requires AnyBinaryFormat<Binary>
			void getMappingInfo(char *file_data, size_t *size, mach_vm_address_t *load_addr);

			void updateSegmentLoadCommandsForNewLoadAddress(char *file_data, mach_vm_address_t newLoadAddress, mach_vm_address_t oldLoadAddress);
			void updateSymbolTableForMappedMachO(char *file_data, mach_vm_address_t newLoadAddress, mach_vm_address_t oldLoadAddress);

			template<typename Binary = RawBinary> requires (AnyBinaryFormat<Binary> && !MachOFormat<Binary>)
			void loadBinary(const char *path, const char *mapFile);

			void loadMachO(const char *path);

			void startKernel();

			void callFunctionInKernel(const char *funcname);
			void callFunctionInKernelUsingHypervisor(const char *funcname);

			void getEntryPointFromKC(mach_vm_address_t kc, mach_vm_address_t *entryPoint);

			void getKernelFromKC(mach_vm_address_t kc, mach_vm_address_t *loadAddress, off_t *loadOffset);

			void loadKernel(const char *path, off_t slide);
			void loadKernelExtension(const char *path);
			
			bool loadKernelCache(const char *kernelPath, mach_vm_address_t *kernelCache, size_t *kernelCacheSize, off_t *loadOffset, mach_vm_address_t *loadAddress);

			void addDebugSymbolsFromKernel(const char *debugSymbols);

			template<typename Binary = RawBinary> requires AnyBinaryFormat<Binary>
			void populateSymbolsFromMapFile(const char *mapFile);

			template <typename T>
			void mutate(T data) requires FuzzableType<T>;

			template<typename Func, typename... Args, typename Binary, typename Sym> requires requires (Binary bin, Sym sym)
			{
				std::is_invocable_v<Func, Args...>;

				sym->getName();
				sym->getAddress();
				std::is_same_v<GetSymbolReturnType<Binary>, Sym>;

			} std::invoke_result_t<Func, Args...> execute(const char *name, Func func, Args... args);

		private:
			Virtualization::Hypervisor *hypervisor;

			xnu::Kernel *kernel;

			xnu::KDKInfo *kdkInfo;

			struct FuzzBinary *fuzzBinary;

			Fuzzer::Loader *loader;

			char *mapFile;
	};
};

#endif