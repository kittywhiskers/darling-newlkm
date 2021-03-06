/*
Copyright (c) 2014-2017, Wenqi Chen

Shanghai Mifu Infotech Co., Ltd
B112-113, IF Industrial Park, 508 Chunking Road, Shanghai 201103, China


All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS AS IS AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


*/

#include "duct.h"
#include "duct_pre_xnu.h"
#include "duct_kern_kalloc.h"
// #include "duct_machine_routines.h"

// #include <mach/mach_types.h>
// #include <ipc/ipc_init.h>
// #include <kern/locks.h>
// #include <kern/thread.h>
// #include <libkern/version.h>

extern void * duct_kalloc_canblock(vm_size_t * size, boolean_t canblock, vm_allocation_site_t * site) {
	if (canblock) {
		return vmalloc (*size);
	} else {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,8,0)
		return __vmalloc (size, GFP_ATOMIC | __GFP_HIGHMEM);
#else
		return __vmalloc (size, GFP_ATOMIC | __GFP_HIGHMEM, PAGE_KERNEL);
#endif
	}
};

// because we use Linux's vmalloc and it doesn't require any size argument for `vfree`,
// we implement `kfree_addr` and get `kfree` for free ;) (see what i did there?)

extern void kfree_addr(void* data) {
	vfree(data);
};

extern void duct_kfree (void * data, vm_size_t size)
{
	kfree_addr(data);
}
