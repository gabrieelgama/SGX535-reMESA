/* SPDX-License-Identifier: MIT */
/* Actual Mesa EGL admission probe. It never issues a draw or a clear. The
 * offline qualifier enforces a host ioctl-denial sandbox before exec/QEMU.
 * Successful software context creation is rejected, not SGX evidence. */
#define _GNU_SOURCE
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

struct gbm_device;

static int pci_word(int directory, const char *name, unsigned *value)
{
 char text[8];
 int fd = openat(directory, name, O_RDONLY | O_CLOEXEC);
 if (fd < 0) return errno;
 ssize_t size = read(fd, text, sizeof(text));
 int error = size < 0 ? errno : 0;
 close(fd);
 if (error) return error;
 if (size != 7 || text[0] != '0' || text[1] != 'x' || text[6] != '\n')
  return EPROTO;
 unsigned result = 0;
 for (unsigned i = 2; i < 6; ++i) {
  unsigned digit;
  if (text[i] >= '0' && text[i] <= '9') digit = (unsigned)(text[i] - '0');
  else if (text[i] >= 'a' && text[i] <= 'f') digit = (unsigned)(text[i] - 'a' + 10);
  else if (text[i] >= 'A' && text[i] <= 'F') digit = (unsigned)(text[i] - 'A' + 10);
  else return EPROTO;
  result = (result << 4) | digit;
 }
 *value = result;
 return 0;
}

/* Device admission is tied to this retained descriptor, not its path name.
 * This proves PCI identity only, not native ioctl or SGX execution admission. */
static int validate_device(int fd)
{
 struct stat st;
 if (fstat(fd, &st)) return errno;
 if (!S_ISCHR(st.st_mode) || major(st.st_rdev) != 226) return ENODEV;
 char path[96];
 int count = snprintf(path, sizeof(path), "/sys/dev/char/%u:%u/device",
                      major(st.st_rdev), minor(st.st_rdev));
 if (count < 0 || (size_t)count >= sizeof(path)) return EOVERFLOW;
 int directory = open(path, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
 if (directory < 0) return errno;
 unsigned vendor = 0, device = 0;
 int error = pci_word(directory, "vendor", &vendor);
 if (!error) error = pci_word(directory, "device", &device);
 close(directory);
 if (!error && (vendor != 0x8086 || device != 0x8108)) error = ENODEV;
 return error;
}

static void record(const char *stage, int success, EGLint error)
{
 printf("{\"stage\":\"%s\",\"success\":%s,\"egl_error\":%u}\n",
        stage, success ? "true" : "false", (unsigned)error);
 fflush(stdout);
}

static int hardware_renderer(const char *renderer)
{
 if (!renderer) return 0;
 static const char *const rejected[] = {"recording", "draw disabled", "no sgx",
  "llvmpipe", "softpipe", "swrast", "software", "resource", "simulated"};
 for (unsigned i = 0; i < sizeof(rejected) / sizeof(rejected[0]); ++i)
  if (strcasestr(renderer, rejected[i])) return 0;
 /* Mesa25.2.0 GetString(GL_RENDERER) returns pipe_screen.get_name directly.
  * This exact identity is necessary, not proof that any GPU work occurred. */
 return strcmp(renderer, "reMESA SGX535") == 0;
}

static int test_renderer(void)
{
 static const char *const rejected[] = {NULL, "", "SGX535", "reMESA",
  "reMESA GEM resources (DRAW DISABLED)", "reMESA recording backend (NO SGX)",
  "reMESA SGX535 recording", "reMESA SGX535 LLVMPipe", "reMESA SGX535 SOFTPIPE",
  "reMESA SGX535 Software", "remesa sgx535", "fake reMESA SGX535"};
 if (!hardware_renderer("reMESA SGX535")) return 1;
 for (unsigned i = 0; i < sizeof(rejected) / sizeof(rejected[0]); ++i)
  if (hardware_renderer(rejected[i])) return 1;
 puts("PASS: exact hardware renderer admission; 12 unsupported identities rejected; NO EGL CONTEXT OR GPU WORK");
 return 0;
}

int main(int argc, char **argv)
{
 if (argc == 2 && strcmp(argv[1], "--test-renderer-admission") == 0)
  return test_renderer();
 if ((argc != 2 && (argc != 6 || strcmp(argv[2], "--drm-device") ||
                               strcmp(argv[4], "--libgbm"))) ||
     (argc >= 2 && argv[1][0] != '/') ||
     (argc == 6 && (argv[3][0] != '/' || argv[5][0] != '/'))) {
  fprintf(stderr, "usage: %s /absolute/path/to/libEGL.so [--drm-device /dev/dri/cardN --libgbm /absolute/path/libgbm.so]\n", argv[0]);
  return 2;
 }
 /* Pin the requested driver and suppress generic Zink selection. Mesa may
  * still attempt software initialization after hardware failure; it must not
  * be accepted as SGX. The qualifier's private artifact directory contains
  * only the real gma500 build, and renderer identity is checked below. */
 if (setenv("LIBGL_ALWAYS_SOFTWARE", "false", 1) ||
     setenv("MESA_LOADER_DRIVER_OVERRIDE", "gma500", 1) ||
     setenv("GALLIUM_DRIVER", "sgx535", 1)) return 2;
 void *lib = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
 if (!lib) {
  record("dlopen_libEGL", 0, 0);
  fprintf(stderr, "%s\n", dlerror());
  return 1;
 }
#define LOAD(type, name) type name = (type)dlsym(lib, #name); \
 if (!name) { record("symbol_" #name, 0, 0); dlclose(lib); return 1; }
 LOAD(PFNEGLGETERRORPROC, eglGetError)
 LOAD(PFNEGLGETPLATFORMDISPLAYPROC, eglGetPlatformDisplay)
 LOAD(PFNEGLINITIALIZEPROC, eglInitialize)
 LOAD(PFNEGLBINDAPIPROC, eglBindAPI)
 LOAD(PFNEGLCHOOSECONFIGPROC, eglChooseConfig)
 LOAD(PFNEGLCREATECONTEXTPROC, eglCreateContext)
 LOAD(PFNEGLDESTROYCONTEXTPROC, eglDestroyContext)
 LOAD(PFNEGLCREATEPBUFFERSURFACEPROC, eglCreatePbufferSurface)
 LOAD(PFNEGLDESTROYSURFACEPROC, eglDestroySurface)
 LOAD(PFNEGLMAKECURRENTPROC, eglMakeCurrent)
 LOAD(PFNEGLTERMINATEPROC, eglTerminate)
 LOAD(PFNEGLGETPROCADDRESSPROC, eglGetProcAddress)
#undef LOAD
 int drm_fd = -1;
 void *gbm_lib = NULL;
 struct gbm_device *gbm = NULL;
 void (*destroy_gbm)(struct gbm_device *) = NULL;
 int result = 1;
 EGLDisplay display = EGL_NO_DISPLAY;
 if (argc == 6) {
  drm_fd = open(argv[3], O_RDWR | O_CLOEXEC);
  int device_error = drm_fd < 0 ? errno : validate_device(drm_fd);
  record("drm_device_identity", !device_error, 0);
  if (device_error) {
   fprintf(stderr, "DRM identity error: %d\n", device_error);
   goto done;
  }
  gbm_lib = dlopen(argv[5], RTLD_NOW | RTLD_LOCAL);
  record("dlopen_libgbm", gbm_lib != NULL, 0);
  if (!gbm_lib) { fprintf(stderr, "%s\n", dlerror()); goto done; }
  struct gbm_device *(*create_gbm)(int) =
   (struct gbm_device *(*)(int))dlsym(gbm_lib, "gbm_create_device");
  destroy_gbm = (void (*)(struct gbm_device *))dlsym(gbm_lib, "gbm_device_destroy");
  record("gbm_symbols", create_gbm && destroy_gbm, 0);
  if (!create_gbm || !destroy_gbm) goto done;
  gbm = create_gbm(drm_fd);
  record("gbm_create_device", gbm != NULL, 0);
  if (!gbm) goto done;
  display = eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR, gbm, NULL);
 } else {
  display = eglGetPlatformDisplay(EGL_PLATFORM_SURFACELESS_MESA,
                                  EGL_DEFAULT_DISPLAY, NULL);
 }
 EGLint error = eglGetError();
 record("eglGetPlatformDisplay", display != EGL_NO_DISPLAY, error);
 if (display == EGL_NO_DISPLAY) goto done;
 EGLint major = 0, minor = 0;
 EGLBoolean initialized = eglInitialize(display, &major, &minor);
 error = eglGetError();
 record("eglInitialize", initialized, error);
 if (!initialized) goto done;
 printf("EGL_VERSION=%d.%d\n", major, minor);
 EGLContext context = EGL_NO_CONTEXT;
 EGLSurface surface = EGL_NO_SURFACE;
 EGLBoolean bound = eglBindAPI(EGL_OPENGL_API);
 record("eglBindAPI", bound, eglGetError());
 if (!bound) goto cleanup;
 const EGLint attrs[] = {EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
  EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT, EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8,
  EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_NONE};
 EGLConfig config = NULL;
 EGLint count = 0;
 EGLBoolean chosen = eglChooseConfig(display, attrs, &config, 1, &count);
 error = eglGetError();
 record("eglChooseConfig", chosen && count == 1, error);
 if (!chosen || count != 1) goto cleanup;
 /* No forced version or profile: Mesa must admit its own truthful baseline. */
 const EGLint context_attrs[] = {EGL_NONE};
 context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attrs);
 record("eglCreateContext", context != EGL_NO_CONTEXT, eglGetError());
 if (context == EGL_NO_CONTEXT) goto cleanup;
 const EGLint surface_attrs[] = {EGL_WIDTH, 32, EGL_HEIGHT, 32, EGL_NONE};
 surface = eglCreatePbufferSurface(display, config, surface_attrs);
 record("eglCreatePbufferSurface", surface != EGL_NO_SURFACE, eglGetError());
 if (surface == EGL_NO_SURFACE) goto cleanup;
 EGLBoolean current = eglMakeCurrent(display, surface, surface, context);
 record("eglMakeCurrent", current, eglGetError());
 if (!current) goto cleanup;
 const unsigned char *(*get_string)(unsigned) =
  (const unsigned char *(*)(unsigned))eglGetProcAddress("glGetString");
 const char *renderer = get_string ? (const char *)get_string(0x1f01) : NULL;
 const char *version = get_string ? (const char *)get_string(0x1f02) : NULL;
 printf("GL_RENDERER=%s\nGL_VERSION=%s\n", renderer ? renderer : "NULL",
        version ? version : "NULL");
 if (!hardware_renderer(renderer)) {
  record("renderer_identity", 0, 0);
  goto cleanup;
 }
 record("renderer_identity", 1, 0);
 puts("CONTEXT_CREATED=YES; GPU_EXECUTION=UNTESTED; DRAW_SUBMISSIONS=0");
 result = 0;
cleanup:
 if (context != EGL_NO_CONTEXT) {
  EGLBoolean released = eglMakeCurrent(display, EGL_NO_SURFACE,
                                      EGL_NO_SURFACE, EGL_NO_CONTEXT);
  record("eglReleaseCurrent", released, eglGetError());
  if (!released) result = 1;
 }
 if (surface != EGL_NO_SURFACE) {
  EGLBoolean destroyed = eglDestroySurface(display, surface);
  record("eglDestroySurface", destroyed, eglGetError());
  if (!destroyed) result = 1;
 }
 if (context != EGL_NO_CONTEXT) {
  EGLBoolean destroyed = eglDestroyContext(display, context);
  record("eglDestroyContext", destroyed, eglGetError());
  if (!destroyed) result = 1;
 }
done:
 /* A valid EGLDisplay must be terminated even if eglInitialize failed. The
  * first API failure is preserved; termination is recorded independently.
  * This common exit is the sole terminate site and precedes unloading EGL. */
 if (display != EGL_NO_DISPLAY) {
  EGLBoolean terminated = eglTerminate(display);
  record("eglTerminate", terminated, eglGetError());
  if (!terminated) result = 1;
 }
 if (gbm) destroy_gbm(gbm);
 if (gbm_lib) dlclose(gbm_lib);
 if (drm_fd >= 0) close(drm_fd);
 dlclose(lib);
 return result;
}
