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

#include "SymbolTable.hpp"

#ifdef __USER__

#include <cxxabi.h>
#include <dlfcn.h>

#include <memory>

#endif

extern "C"
{
#ifdef __USER__

	char* cxx_demangle(char *mangled)
	{
		int status;

		char *ret = abi::__cxa_demangle(mangled, 0, 0, &status);  

		std::shared_ptr<char> retval;

		retval.reset( (char *)ret, [](char *mem) { if (mem) free((void*)mem); } );

		return static_cast<char*>(retval.get());
	}

	typedef char* (*_swift_demangle) (char *mangled, UInt32 length, UInt8 *output_buffer, UInt32 output_buffer_size, UInt32 flags);

	char* swift_demangle(char *mangled)
	{
		void *runtime_loader_default = dlopen(NULL, RTLD_NOW);

		if(runtime_loader_default)
		{
			void *sym = dlsym(runtime_loader_default, "swift_demangle");

			if(sym)
			{
				_swift_demangle f = reinterpret_cast<_swift_demangle>(sym);

				char *cString = f(mangled, strlen(mangled), NULL, 0, 0);
				
				if(cString)
				{
					dlclose(runtime_loader_default);

					return cString;
				}
			}

			dlclose(runtime_loader_default);
		}

		return NULL;
	}

#else

	char* cxx_demangle(char *mangled) { return NULL; }
	char* swift_demangle(char *mangled) { return NULL; }

#endif

}

Symbol* SymbolTable::getSymbolByName(char *symname)
{
	for(int32_t i = 0; i < symbolTable.size(); i++)
	{
		Symbol *symbol = symbolTable.at(i);

		if(strcmp(symbol->getName(), symname) == 0)
		{
			return symbol;
		}
	}

	return NULL;
}

Symbol* SymbolTable::getSymbolByAddress(xnu::Mach::VmAddress address)
{
	for(int32_t i = 0; i < symbolTable.size(); i++)
	{
		Symbol *symbol = symbolTable.at(i);

		if(symbol->getAddress() == address)
		{
			return symbol;
		}
	}

	return NULL;
}

Symbol* SymbolTable::getSymbolByOffset(Offset offset)
{
	for(int32_t i = 0; i < symbolTable.size(); i++)
	{
		Symbol *symbol = symbolTable.at(i);

		if(symbol->getOffset() == offset)
		{
			return symbol;
		}
	}

	return NULL;
}

void SymbolTable::replaceSymbol(Symbol *symbol)
{
	for(int i = ((int) symbolTable.size()) - 1; i >= 0; i--)
	{
		Symbol *sym = symbolTable.at(i);

		if(strcmp(sym->getName(), symbol->getName()) == 0)
		{
			symbolTable.erase(std::remove(symbolTable.begin(), symbolTable.end(), sym), symbolTable.end());

			i--;
		}
	}

	symbolTable.push_back(symbol);
}