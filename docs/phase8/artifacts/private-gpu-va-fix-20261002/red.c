#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
typedef uint64_t resource_size_t;
#define IORESOURCE_MEM 0x200
#define IORESOURCE_UNSET 0x20000000
#define BIOS_ROM_BASE 0xffe00000ULL
#define BIOS_ROM_END 0xffffffffULL
#define ALIGN(x,a) (((x)+(a)-1)&~((a)-1))
#define EBUSY 16
struct resource {resource_size_t start,end; unsigned long flags;struct resource *child,*sibling;};
struct resource_constraint {resource_size_t min,max,align;resource_size_t (*alignf)(void *,const struct resource *,resource_size_t,resource_size_t);void *alignf_data;};
struct e820_entry {uint64_t addr,size;};
struct e820_table {int nr_entries;struct e820_entry entries[16];};
static struct e820_table captured_e820, *e820_table=&captured_e820;
static int resource_contains(struct resource *a,struct resource *b){return (a->flags&0x1f00)==(b->flags&0x1f00)&&!(a->flags&IORESOURCE_UNSET)&&!(b->flags&IORESOURCE_UNSET)&&a->start<=b->start&&a->end>=b->end;}
// SPDX-License-Identifier: GPL-2.0

static void e820_resource_clip(struct resource *res, resource_size_t start,
			  resource_size_t end)
{
	resource_size_t low = 0, high = 0;

	if (res->end < start || res->start > end)
		return;		/* no conflict */

	if (res->start < start)
		low = start - res->start;

	if (res->end > end)
		high = res->end - end;

	/* Keep the area above or below the conflict, whichever is larger */
	if (low > high)
		res->end = start - 1;
	else
		res->start = end + 1;
}

static void remove_e820_regions(struct resource *avail)
{
	int i;
	struct e820_entry *entry;

	for (i = 0; i < e820_table->nr_entries; i++) {
		entry = &e820_table->entries[i];

		e820_resource_clip(avail, entry->addr,
			      entry->addr + entry->size - 1);
	}
}

void arch_remove_reservations(struct resource *avail)
{
	/*
	 * Trim out BIOS area (high 2MB) and E820 regions. We do not remove
	 * the low 1MB unconditionally, as this area is needed for some ISA
	 * cards requiring a memory range, e.g. the i82365 PCMCIA controller.
	 */
	if (avail->flags & IORESOURCE_MEM) {
		e820_resource_clip(avail, BIOS_ROM_BASE, BIOS_ROM_END);

		remove_e820_regions(avail);
	}
}
static resource_size_t simple_align_resource(void *data,
					     const struct resource *avail,
					     resource_size_t size,
					     resource_size_t align)
{
	return avail->start;
}
static void resource_clip(struct resource *res, resource_size_t min,
			  resource_size_t max)
{
	if (res->start < min)
		res->start = min;
	if (res->end > max)
		res->end = max;
}
static int __find_resource(struct resource *root, struct resource *old,
			 struct resource *new,
			 resource_size_t  size,
			 struct resource_constraint *constraint)
{
	struct resource *this = root->child;
	struct resource tmp = *new, avail, alloc;

	tmp.start = root->start;
	/*
	 * Skip past an allocated resource that starts at 0, since the assignment
	 * of this->start - 1 to tmp->end below would cause an underflow.
	 */
	if (this && this->start == root->start) {
		tmp.start = (this == old) ? old->start : this->end + 1;
		this = this->sibling;
	}
	for(;;) {
		if (this)
			tmp.end = (this == old) ?  this->end : this->start - 1;
		else
			tmp.end = root->end;

		if (tmp.end < tmp.start)
			goto next;

		resource_clip(&tmp, constraint->min, constraint->max);
		arch_remove_reservations(&tmp);

		/* Check for overflow after ALIGN() */
		avail.start = ALIGN(tmp.start, constraint->align);
		avail.end = tmp.end;
		avail.flags = new->flags & ~IORESOURCE_UNSET;
		if (avail.start >= tmp.start) {
			alloc.flags = avail.flags;
			alloc.start = constraint->alignf(constraint->alignf_data, &avail,
					size, constraint->align);
			alloc.end = alloc.start + size - 1;
			if (alloc.start <= alloc.end &&
			    resource_contains(&avail, &alloc)) {
				new->start = alloc.start;
				new->end = alloc.end;
				return 0;
			}
		}

next:		if (!this || this->end == root->end)
			break;

		if (this != old)
			tmp.start = this->end + 1;
		this = this->sibling;
	}
	return -EBUSY;
}
int main(void){
 struct resource excluded={.start=0xd0000000,.end=0xd7ffffff,.flags=IORESOURCE_MEM};
 struct resource root={.start=0x20000000,.end=0xdfffffff,.flags=IORESOURCE_MEM,.child=&excluded};
 struct resource bo={.flags=IORESOURCE_MEM};
 struct resource_constraint c={.min=0x20000000,.max=0x2fffffff,.align=0x1000,.alignf=simple_align_resource};
 captured_e820.nr_entries=1;captured_e820.entries[0].addr=0x100000;captured_e820.entries[0].size=0x3f6b0000-0x100000;
 int rc=__find_resource(&root,NULL,&bo,0x20000,&c);
 printf("PDS rc=%d start=0x%llx end=0x%llx\n",rc,(unsigned long long)bo.start,(unsigned long long)bo.end);fflush(stdout);
 if(rc!=0||bo.start!=0x20000000||bo.end!=0x2001ffff)return 1;
 /* Existing GTT interval is still occupied in the private tree. */
 c.min=0xd0000000;c.max=0xd7ffffff;
 rc=__find_resource(&root,NULL,&bo,0x1000,&c);printf("GTT overlap rc=%d\n",rc);assert(rc==-EBUSY);
 /* A genuinely occupied PDS domain cannot be allocated either. */
 struct resource occupied={.start=0x20000000,.end=0x2fffffff,.flags=IORESOURCE_MEM,.sibling=&excluded};root.child=&occupied;
 c.min=0x20000000;c.max=0x2fffffff;
 rc=__find_resource(&root,NULL,&bo,0x20000,&c);printf("Occupied PDS rc=%d\n",rc);assert(rc==-EBUSY);
 /* Exact existing alignment is still applied. */
 root.start=0x20000001;root.child=&excluded;
 rc=__find_resource(&root,NULL,&bo,0x8000,&c);assert(rc==0&&bo.start==0x20001000);
 puts("allocation/alignment/GTT/overlap guards PASS");return 0;
}
