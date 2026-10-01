/* Review-only source. Do not install or run without a new operator authorization. */
#define _POSIX_C_SOURCE 200809L
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#if !defined(__i386__)
#error This helper is only for the retained i686 Mini 12 target.
#endif

_Static_assert(sizeof(uint32_t) == 4, "32-bit register load required");

#define PCI "/sys/bus/pci/devices/0000:00:02.0"
#define BAR0_START UINT64_C(0xd8100000)
#define BAR0_END UINT64_C(0xd817ffff)
#define BAR0_FLAGS UINT64_C(0x00040200)
#define BAR0_LENGTH UINT64_C(0x80000)
#define SGX_OFFSET UINT64_C(0x40000)
#define SGX_LENGTH UINT64_C(0x8000)
#define CORE_ID_OFFSET UINT64_C(0x10)
#define PAGE_BYTES 4096

extern uint32_t sgx535_load32_once(const volatile uint32_t *address);

static int refuse(const char *reason)
{
    fprintf(stderr, "REFUSE: %s\n", reason);
    return 1;
}

static int token_equals(const char *path, const char *expected)
{
    char text[128];
    ssize_t count;
    int fd = open(path, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) return 0;
    count = read(fd, text, sizeof(text) - 1);
    (void)close(fd);
    if (count <= 0 || count >= (ssize_t)sizeof(text) - 1) return 0;
    text[count] = '\0';
    if (text[count - 1] == '\n') text[count - 1] = '\0';
    return strcmp(text, expected) == 0;
}

static int driver_is_gma500(void)
{
    char text[128];
    ssize_t count = readlink(PCI "/driver", text, sizeof(text) - 1);
    if (count <= 0 || count >= (ssize_t)sizeof(text) - 1) return 0;
    text[count] = '\0';
    return strcmp(text, "../../../bus/pci/drivers/gma500") == 0;
}

static int bar0_matches(void)
{
    char line[128], extra;
    unsigned long long start, end, flags;
    FILE *stream = fopen(PCI "/resource", "r");
    if (!stream) return 0;
    if (!fgets(line, sizeof(line), stream)) {
        fclose(stream);
        return 0;
    }
    if (fclose(stream) != 0) return 0;
    if (!strchr(line, '\n')) return 0;
    if (sscanf(line, "%llx %llx %llx %c", &start, &end, &flags, &extra) != 3)
        return 0;
    return start == BAR0_START && end == BAR0_END && flags == BAR0_FLAGS;
}

int main(int argc, char **argv)
{
    struct stat resource_stat;
    void *page;
    const volatile uint32_t *register_address;
    uint32_t value;
    int fd;
    (void)argv;

    if (argc != 1) return refuse("arguments are forbidden");
    if (geteuid() != 0) return refuse("root execution required");
    if (!token_equals("/sys/class/dmi/id/sys_vendor", "Dell Inc.") ||
        /* Firmware supplies three ASCII spaces before product_name's newline. */
        !token_equals("/sys/class/dmi/id/product_name", "Inspiron 1210   ") ||
        !token_equals("/sys/class/dmi/id/board_name", "0X605H") ||
        !token_equals("/sys/class/dmi/id/bios_version", "A02"))
        return refuse("DMI identity changed");
    if (!token_equals(PCI "/vendor", "0x8086") ||
        !token_equals(PCI "/device", "0x8108") ||
        !token_equals(PCI "/subsystem_vendor", "0x1028") ||
        !token_equals(PCI "/subsystem_device", "0x02b1") ||
        !token_equals(PCI "/revision", "0x06"))
        return refuse("PCI identity changed");
    if (!driver_is_gma500() ||
        !token_equals("/sys/module/gma500_gfx/initstate", "live") ||
        !token_equals(PCI "/enable", "1") ||
        !token_equals(PCI "/power/control", "on") ||
        !token_equals(PCI "/power/runtime_status", "active"))
        return refuse("binding or OS-visible state changed");
    if (!bar0_matches()) return refuse("BAR0 changed");
    if (SGX_OFFSET + CORE_ID_OFFSET != UINT64_C(0x40010) ||
        BAR0_START + SGX_OFFSET + CORE_ID_OFFSET != UINT64_C(0xd8140010) ||
        SGX_OFFSET + SGX_LENGTH > BAR0_LENGTH ||
        CORE_ID_OFFSET + sizeof(uint32_t) > SGX_LENGTH ||
        CORE_ID_OFFSET % sizeof(uint32_t) != 0 ||
        sysconf(_SC_PAGESIZE) != PAGE_BYTES)
        return refuse("address or page derivation invalid");

    fd = open(PCI "/resource0", O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) return refuse("resource0 open failed");
    if (fstat(fd, &resource_stat) != 0 ||
        !S_ISREG(resource_stat.st_mode) ||
        resource_stat.st_size != (off_t)BAR0_LENGTH ||
        resource_stat.st_uid != 0) {
        close(fd);
        return refuse("resource0 identity or length changed");
    }
    page = mmap(NULL, PAGE_BYTES, PROT_READ, MAP_SHARED, fd, (off_t)SGX_OFFSET);
    if (page == MAP_FAILED) {
        close(fd);
        return refuse("resource0 one-page mapping failed");
    }
    if (close(fd) != 0) {
        munmap(page, PAGE_BYTES);
        return refuse("resource0 close failed");
    }

    register_address = (const volatile uint32_t *)((const char *)page + CORE_ID_OFFSET);
    value = sgx535_load32_once(register_address); /* The sole SGX MMIO load. */
    (void)munmap(page, PAGE_BYTES);
    if (printf("CORE_ID=0x%08" PRIx32 "\n", value) < 0 || fflush(stdout) == EOF)
        return refuse("result output failed after read");
    return 0;
}
