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

#include <stddef.h>
#include <stdint.h>

#include <mach/mach_types.h>
#include <mach/vm_types.h>

#include "Hook.hpp"

#include "Arch.hpp"

namespace xnu
{
	class Kernel;
	class Task;
};

using namespace xnu;

namespace mrk
{
	class Payload
	{
		static constexpr UInt32 expectedSize = Arch::getPageSize<Arch::getCurrentArchitecture()>();

		public:
			Payload(xnu::Task *task, Hook *hook, xnu::Mach::VmProtection prot);

			~Payload();

			Hook* getHook() { return hook; }

			xnu::Mach::VmAddress getAddress() { return address; }

			void setCurrentOffset(Offset offset);

			Offset getCurrentOffset() { return current_offset; }

			Size getSize() { return size; }

			xnu::Mach::VmProtection getProt() { return prot; }

			Task* getTask() { return task; }

			bool readBytes(UInt8 *bytes, Size size);
			bool readBytes(Offset offset, UInt8 *bytes, Size size);

			bool writeBytes(UInt8 *bytes, Size size);
			bool writeBytes(Offset offset, UInt8 *bytes, Size size);

			void setWritable();
			void setExecutable();

			bool prepare();

			bool commit();
		
		private:
			xnu::Task *task;

			xnu::Mach::VmAddress address;

			Offset current_offset;

			Hook *hook;

			bool kernelPayload = false;

			Size size;

			xnu::Mach::VmProtection prot;
	};
};
