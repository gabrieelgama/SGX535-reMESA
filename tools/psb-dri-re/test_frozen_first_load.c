/* Only kernel boundaries are doubled; selected function bodies are extracted. */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define PCI_ANY_ID (~0U)
#define CONFIG_DRM_GMA600 1
#define CONFIG_DRM_GMA3600 1
struct device { int pm_references; };
struct pci_device_id {
    unsigned int vendor, device, subvendor, subdevice, class, class_mask;
    uintptr_t driver_data;
};
struct pci_driver;
struct pci_dev {
    unsigned int vendor, device, subsystem_vendor, subsystem_device, class;
    struct pci_driver *driver;
    struct device dev;
};
struct pci_driver {
    int (*probe)(struct pci_dev *, const struct pci_device_id *);
};
static const int psb_chip_ops, oaktrail_chip_ops, cdv_chip_ops;
static int probes, probe_result, warnings;
static struct pci_driver *expected_owner;
static int pm_runtime_get_sync(struct device *d) { return ++d->pm_references; }
static int pm_runtime_put_sync(struct device *d) { return --d->pm_references; }
#define pci_warn(dev, format, rc) (++warnings)

#include ACTUAL_SOURCE

static int controlled_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    ++probes;
    assert(dev->driver == expected_owner);
    assert(dev->dev.pm_references == 1);
    assert(id->driver_data == (uintptr_t)&psb_chip_ops);
    return probe_result;
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    int scenario = atoi(argv[1]);
    struct pci_dev dev = { .vendor = 0x8086, .device = 0x8108,
        .subsystem_vendor = 0x1028, .subsystem_device = 0x02b1, .class = 0x030000 };
    struct pci_driver derivative = { .probe = controlled_probe };
    struct pci_driver original = { .probe = controlled_probe };
    expected_owner = &derivative;
    switch (scenario) {
    case 0: {
        const struct pci_device_id *id = pci_match_id(pciidlist, &dev);
        assert(id && id->driver_data == (uintptr_t)&psb_chip_ops);
        dev.device = 0x8107;
        assert(!pci_match_id(pciidlist, &dev));
        assert(__pci_device_probe(&derivative, &dev) == -ENODEV);
        assert(probes == 0 && !dev.driver && dev.dev.pm_references == 0);
        break;
    }
    case 1:
        dev.driver = &original;
        assert(__pci_device_probe(&derivative, &dev) == 0);
        assert(probes == 0 && dev.driver == &original && dev.dev.pm_references == 0);
        break;
    case 2:
        assert(__pci_device_probe(&derivative, &dev) == 0);
        assert(probes == 1 && dev.driver == &derivative && dev.dev.pm_references == 1);
        break;
    case 3:
        probe_result = -ENOMEM;
        assert(__pci_device_probe(&derivative, &dev) == -ENOMEM);
        assert(probes == 1 && !dev.driver && dev.dev.pm_references == 0);
        break;
    case 4:
        module_blacklist = NULL;
        assert(!blacklisted("gma500_gfx"));
        module_blacklist = "gma500_gfx";
        /* Both preserved binary identities are tested by the Python test. */
        assert(blacklisted("gma500_gfx"));
        module_blacklist = "foo,gma500_gfx,bar";
        assert(blacklisted("gma500_gfx"));
        assert(!blacklisted("gma500_gfx_extra"));
        module_blacklist = "foo,gma500_gfx_extra,bar";
        assert(!blacklisted("gma500_gfx"));
        break;
    default:
        abort();
    }
    assert(warnings == 0);
    return 0;
}
