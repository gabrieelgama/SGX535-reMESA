/* CPU-only behavioral tests. No device, ioctl or SGX operation is used. */
#include <stdio.h>
#include <string.h>
#include "frozen_capsule.h"

static int call_a, call_b, backend_a, backend_b, service_a, service_b, owner_a, owner_b;
static unsigned char image[4096];
static int failures, checks;
#define CHECK(x) do { checks++; if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); failures++; } } while (0)

static void start(struct sgx535_capsule *c)
{
    sgx535_capsule_call(c, &call_a);
    sgx535_capsule_admit(c, &call_a, &backend_a, &service_a, &owner_a, 2, 0, 0, 1);
    sgx535_capsule_service(c, &backend_a, &service_a, 1);
    sgx535_capsule_issued(c, &backend_a);
}

static void event(struct sgx535_capsule *c, unsigned ledger, unsigned phase)
{
    sgx535_capsule_sample(c, &backend_a, 1, 0x2000, 0, 1);
    sgx535_capsule_before(c, &backend_a, &service_a, 1, 0x2000, 0, 1);
    sgx535_capsule_after(c, &backend_a, &service_a, ledger, phase);
}

static void terminal(struct sgx535_capsule *c)
{
    event(c, 1, 7);
    event(c, 3, 7);
    event(c, 7, 8);
    sgx535_capsule_terminal(c, &backend_a, &service_a, 0, 9, 7);
    sgx535_capsule_color(c, &owner_a, 9, image, sizeof(image));
    sgx535_capsule_close(c, &call_a, 0);
}

static void good(struct sgx535_capsule *c)
{
    memset(c, 0, sizeof(*c));
    start(c);
    terminal(c);
}

int main(void)
{
    struct sgx535_capsule c;
    unsigned char raw[SGX535_CAPSULE_BYTES], old[SGX535_CAPSULE_BYTES];
    unsigned i;
    for (i = 0; i < sizeof(image); i++) image[i] = (unsigned char)(i * 13 + 5);

    memset(&c, 0, sizeof(c));
    CHECK(!sgx535_capsule_success(&c)); /* fresh is not success */
    good(&c);
    CHECK(sgx535_capsule_success(&c));
    CHECK(c.events == 7 && c.phase == 9 && c.result == 0 && c.color_present);
    CHECK(!memcmp(c.color, image, 4096));
    CHECK(!sgx535_capsule_encode(&c, raw, sizeof(raw)));
    CHECK(!memcmp(raw, "SGXCAPB1", 8));
    CHECK(!memcmp(raw + SGX535_CAPSULE_HEADER, image, 4096));
    CHECK(sgx535_capsule_encode(&c, raw, sizeof(raw) - 1) != 0);

    memcpy(old, raw, sizeof(raw));
    sgx535_capsule_call(&c, &call_b); /* second call cannot reuse */
    CHECK(!sgx535_capsule_success(&c));
    CHECK(!memcmp(c.color, image, 4096));
    CHECK(!sgx535_capsule_encode(&c, raw, sizeof(raw)));
    CHECK(!memcmp(old + 24, raw + 24, sizeof(raw) - 24)); /* immutable payload */

    memset(&c, 0, sizeof(c)); start(&c);
    sgx535_capsule_admit(&c, &call_a, &backend_b, &service_b, &owner_b, 2, 0, 0, 1);
    CHECK(c.invalid && !sgx535_capsule_success(&c));

    memset(&c, 0, sizeof(c));
    sgx535_capsule_call(&c, &call_a);
    sgx535_capsule_admit(&c, &call_a, &backend_a, &service_a, &owner_a, 2, 7, 0, 1);
    CHECK(c.invalid); /* stale ledger */

    memset(&c, 0, sizeof(c));
    sgx535_capsule_call(&c, &call_a);
    sgx535_capsule_admit(&c, &call_a, &backend_a, &service_a, &owner_a, 2, 0, 4, 1);
    CHECK(c.invalid); /* stale sequence */

    memset(&c, 0, sizeof(c));
    sgx535_capsule_call(&c, &call_a);
    sgx535_capsule_admit(&c, &call_a, &backend_a, &service_a, &owner_a, 2, 0, 0, 0);
    CHECK(c.invalid); /* pending source / wrong executable / admission failure */

    memset(&c, 0, sizeof(c)); start(&c);
    sgx535_capsule_sample(&c, &backend_b, 1, 0x2000, 0, 1);
    CHECK(c.invalid); /* cross-operation sample */

    memset(&c, 0, sizeof(c)); start(&c);
    sgx535_capsule_before(&c, &backend_a, &service_a, 1, 0x2000, 0, 1);
    CHECK(c.invalid); /* software claim without hardware receipt */

    memset(&c, 0, sizeof(c)); start(&c);
    sgx535_capsule_sample(&c, &backend_a, 1, 0x2000, 0, 1);
    sgx535_capsule_before(&c, &backend_a, &service_b, 1, 0x2000, 0, 1);
    CHECK(c.invalid); /* copied or different service */

    memset(&c, 0, sizeof(c)); start(&c);
    sgx535_capsule_sample(&c, &backend_a, 1, 0x2000, 0, 1);
    sgx535_capsule_before(&c, &backend_a, &service_a, 1, 0x4000, 0, 1);
    CHECK(c.invalid); /* substituted words */

    memset(&c, 0, sizeof(c)); start(&c); event(&c, 1, 7);
    sgx535_capsule_terminal(&c, &backend_a, &service_a, 0, 9, 7);
    CHECK(c.invalid); /* terminal result with unobserved ledger */

    memset(&c, 0, sizeof(c)); start(&c); event(&c, 7, 8);
    sgx535_capsule_terminal(&c, &backend_a, &service_b, 0, 9, 7);
    CHECK(c.invalid); /* wrong terminal source */

    memset(&c, 0, sizeof(c)); start(&c); event(&c, 7, 8);
    sgx535_capsule_terminal(&c, &backend_a, &service_a, 0, 9, 7);
    sgx535_capsule_color(&c, &owner_b, 9, image, sizeof(image));
    CHECK(c.invalid && !c.color_present); /* wrong color owner */

    memset(&c, 0, sizeof(c)); start(&c); event(&c, 7, 8);
    sgx535_capsule_terminal(&c, &backend_a, &service_a, 0, 9, 7);
    sgx535_capsule_color(&c, &owner_a, 8, image, sizeof(image));
    CHECK(c.invalid && !c.color_present); /* not retired */

    memset(&c, 0, sizeof(c)); start(&c); event(&c, 7, 8);
    sgx535_capsule_terminal(&c, &backend_a, &service_a, 0, 9, 7);
    sgx535_capsule_color(&c, &owner_a, 9, image, 4095);
    CHECK(c.invalid && !c.color_present); /* partial color */

    good(&c); memcpy(old, c.color, 4096);
    sgx535_capsule_color(&c, &owner_a, 9, image, sizeof(image));
    CHECK(c.invalid && !memcmp(c.color, old, 4096)); /* late writer */

    good(&c);
    sgx535_capsule_terminal(&c, &backend_a, &service_a, -7, 11, 0);
    CHECK(c.invalid && c.result == 0 && c.events == 7); /* duplicate terminal */

    memset(&c, 0, sizeof(c)); start(&c);
    sgx535_capsule_close(&c, &call_a, -14);
    CHECK(!sgx535_capsule_success(&c) && c.issued); /* ambiguous issuance */

    memset(&c, 0, sizeof(c)); start(&c); terminal(&c);
    c.syscall_result = -14;
    CHECK(!sgx535_capsule_success(&c)); /* failed copyout never response success */

    memset(&c, 0, sizeof(c)); start(&c); event(&c, 1, 7);
    sgx535_capsule_terminal(&c, &backend_a, &service_a, -1, 11, 1);
    sgx535_capsule_close(&c, &call_a, -5);
    CHECK(!c.invalid && c.terminal_present && c.events == 1 && !c.color_present);
    CHECK(!sgx535_capsule_success(&c)); /* attributable partial HOLD */

    good(&c); memcpy(old, c.color, 4096);
    sgx535_capsule_release(&c, &owner_a);
    CHECK(sgx535_capsule_success(&c) && !memcmp(c.color, old, 4096));
    memset(&c, 0, sizeof(c)); start(&c);
    sgx535_capsule_release(&c, &owner_a);
    CHECK(c.invalid); /* source lost before snapshot */

    memset(&c, 0, sizeof(c)); start(&c);
    sgx535_capsule_close(&c, &call_b, 0);
    CHECK(c.invalid); /* another process/call exit */

    memset(&c, 0, sizeof(c));
    sgx535_capsule_call(&c, &call_a);
    sgx535_capsule_admit(&c, &call_a, &backend_a, &service_a, &owner_a,
        SGX535_PHASE_HOST_IMAGE_FINAL, 0, 0, 0);
    sgx535_capsule_service(&c, &backend_a, &service_a, 1);
    sgx535_capsule_issued(&c, &backend_a);
    event(&c, 7, 8);
    sgx535_capsule_terminal(&c, &backend_a, &service_a, 0, 9, 7);
    sgx535_capsule_color(&c, &owner_a, 9, image, sizeof(image));
    sgx535_capsule_close(&c, &call_a, 0);
    CHECK(c.invalid && c.issued && c.terminal_present && c.color_present);
    CHECK(!sgx535_capsule_success(&c)); /* retain invalid partial facts, never success */

    memset(&c, 0, sizeof(c)); start(&c);
    sgx535_capsule_sample(&c, &backend_a, 1, 0x2000, 0, 1);
    sgx535_capsule_terminal(&c, &backend_a, &service_a, -8, 11, 1);
    CHECK(c.invalid && c.terminal_present && c.result == -8 && c.phase == 11);
    CHECK(c.events == 0); /* do not invent the missing acceptance observation */
    sgx535_capsule_close(&c, &call_a, -5);
    CHECK(!sgx535_capsule_success(&c));

    memset(&c, 0, sizeof(c)); start(&c);
    sgx535_capsule_terminal(&c, &backend_a, &service_a, 0, 9, 7);
    sgx535_capsule_color(&c, &owner_a, 9, image, sizeof(image));
    CHECK(c.invalid && c.events == 0 && c.color_present);
    CHECK(!memcmp(c.color, image, sizeof(image)));
    sgx535_capsule_close(&c, &call_a, 0);
    CHECK(!sgx535_capsule_success(&c)); /* retain unmatched retired pixels only */

    good(&c);
    CHECK(!sgx535_capsule_encode(&c, old, sizeof(old)));
    CHECK(!sgx535_capsule_encode(&c, raw, sizeof(raw)));
    CHECK(!memcmp(old, raw, sizeof(raw))); /* deterministic readonly materialization */
    printf("%d checks, %d failures; CPU SYNTHETIC ONLY\n", checks, failures);
    return failures != 0;
}
