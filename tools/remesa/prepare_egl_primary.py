#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Prepare a new pinned Mesa25.2 primary-only Poulsbo EGL discovery overlay.
Discovery does not confer OpenGL capability, rendering permission or scanout.
Never modifies its source tree and never opens a physical device while staging.
"""
import argparse,hashlib,json
from pathlib import Path
PIN={
'src/egl/drivers/dri2/platform_drm.c':'01c436572941585e5bc32772753969ffa41898b437f2dbe8dc544158d5960809',
'src/egl/drivers/dri2/egl_dri2.c':'f962890a3690a12d41f065d58bab81a302ec8f620cd70b04683884523176d667',
'src/egl/main/egldevice.c':'3aba4439404c82a6e1e45f4989f65fb259151a25ebd9974954b6d748c83697ea',
'src/egl/meson.build':'972b21d44e156954b40d9d34c558951f516fa4823e42e608530a0ee105f2b489'}
HEADER=r'''/* SPDX-License-Identifier: MIT */
/* Independently implemented discovery policy. No GPU commands or permission. */
#ifndef REMESA_EGL_PRIMARY_DEVICE_H
#define REMESA_EGL_PRIMARY_DEVICE_H
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <xf86drm.h>
static inline bool
remesa_egl_primary_metadata(drmDevicePtr device)
{
   return device && device->bustype == DRM_BUS_PCI && device->deviceinfo.pci &&
          device->deviceinfo.pci->vendor_id == 0x8086 &&
          device->deviceinfo.pci->device_id == 0x8108 &&
          (device->available_nodes & (1 << DRM_NODE_PRIMARY)) &&
          !(device->available_nodes & (1 << DRM_NODE_RENDER)) &&
          device->nodes && device->nodes[DRM_NODE_PRIMARY];
}
static inline bool
remesa_egl_primary_fd(int borrowed_fd)
{
   bool accepted = false;
   struct stat st;
   drmDevicePtr device = NULL;
   drmVersionPtr version = NULL;
   int fd = fcntl(borrowed_fd, F_DUPFD_CLOEXEC, 0);
   if (fd < 0)
      return false;
   int flags = fcntl(fd, F_GETFL);
   if (flags < 0 || (flags & O_ACCMODE) != O_RDWR ||
       fstat(fd, &st) || !S_ISCHR(st.st_mode) || major(st.st_rdev) != 226 ||
       drmGetNodeTypeFromFd(fd) != DRM_NODE_PRIMARY ||
       drmGetDevice2(fd, 0, &device) || !remesa_egl_primary_metadata(device))
      goto out;
   version = drmGetVersion(fd);
   accepted = version && version->name && version->name_len == 6 &&
              !memcmp(version->name, "gma500", 6);
out:
   if (version)
      drmFreeVersion(version);
   if (device)
      drmFreeDevice(&device);
   close(fd);
   return accepted;
}
static inline bool
remesa_egl_primary_device(drmDevicePtr device)
{
   if (!remesa_egl_primary_metadata(device))
      return false;
   int fd = open(device->nodes[DRM_NODE_PRIMARY], O_RDWR | O_CLOEXEC);
   if (fd < 0)
      return false;
   drmDevicePtr actual = NULL;
   bool accepted = remesa_egl_primary_fd(fd) &&
                   !drmGetDevice2(fd, 0, &actual) &&
                   drmDevicesEqual(device, actual);
   if (actual)
      drmFreeDevice(&actual);
   close(fd);
   return accepted;
}
#endif
'''
def render(mesa):
    """Return independently pinned overlay texts without writing any files."""
    texts={}
    for rel,h in PIN.items():
        b=(mesa/rel).read_bytes()
        if hashlib.sha256(b).hexdigest()!=h:raise ValueError('Pinned Mesa mismatch: '+rel)
        texts[rel]=b.decode()
    def replace(rel,old,new):
        if texts[rel].count(old)!=1:raise ValueError('Nonunique anchor: '+rel+' '+old[:60])
        texts[rel]=texts[rel].replace(old,new)
    include='\n#ifdef REMESA_PRIMARY_GMA500\n#include "drivers/dri2/remesa_primary_device.h"\n#endif\n'
    for rel in ['src/egl/drivers/dri2/platform_drm.c','src/egl/drivers/dri2/egl_dri2.c']:
        anchor = '#include "egldevice.h"' if rel.endswith('platform_drm.c') else '#include "egl_dri2.h"'
        replace(rel,anchor,anchor+include)
    rel='src/egl/drivers/dri2/platform_drm.c'
    replace(rel,'   if (loader_is_device_render_capable(fd_display_gpu))',
'''   if (loader_is_device_render_capable(fd_display_gpu)
#ifdef REMESA_PRIMARY_GMA500
       || remesa_egl_primary_fd(fd_display_gpu)
#endif
      )''')
    rel='src/egl/drivers/dri2/egl_dri2.c'
    replace(rel,'   } else if (loader_is_device_render_capable(dri2_dpy->fd_render_gpu)) {',
'''   } else if (loader_is_device_render_capable(dri2_dpy->fd_render_gpu)
#ifdef REMESA_PRIMARY_GMA500
              || remesa_egl_primary_fd(dri2_dpy->fd_render_gpu)
#endif
             ) {''')
    rel='src/egl/main/egldevice.c'
    replace(rel,'#include <xf86drm.h>','#include <xf86drm.h>'+include)
    replace(rel,'   assert(device->available_nodes & ((1 << DRM_NODE_RENDER)));',
'''   bool has_render_node = device->available_nodes & (1 << DRM_NODE_RENDER);
#ifdef REMESA_PRIMARY_GMA500
   if (!has_render_node && !remesa_egl_primary_device(device))
      return -1;
#else
   assert(has_render_node);
#endif''')
    replace(rel,'   const char *node = device->nodes[DRM_NODE_RENDER];',
'''   const char *node = device->nodes[has_render_node ? DRM_NODE_RENDER : DRM_NODE_PRIMARY];''')
    replace(rel,'   if (_eglDriver.QueryDeviceInfo)', '   if (has_render_node && _eglDriver.QueryDeviceInfo)')
    replace(rel,'   else\n      dev->extensions = "EGL_EXT_device_drm EGL_EXT_device_drm_render_node";',
'''   else
      dev->extensions = has_render_node ? "EGL_EXT_device_drm EGL_EXT_device_drm_render_node" : "EGL_EXT_device_drm";''')
    replace(rel,'   dev->EXT_device_drm_render_node = EGL_TRUE;',
                '   dev->EXT_device_drm_render_node = has_render_node;')
    replace(rel,'      if (!(devices[i]->available_nodes & (1 << DRM_NODE_RENDER))) {',
'''      if (!(devices[i]->available_nodes & (1 << DRM_NODE_RENDER))
#ifdef REMESA_PRIMARY_GMA500
          && !remesa_egl_primary_device(devices[i])
#endif
         ) {''')
    replace('src/egl/meson.build','c_args_for_egl = [asan_c_args]',
'''c_args_for_egl = [asan_c_args]
if with_gallium_sgx535
  c_args_for_egl += '-DREMESA_PRIMARY_GMA500'
endif''')
    # _eglFindDevice matches real libdrm devices in the refreshed list. Primary
    # entries pass the same strict helper; never synthesize a render node or fd.
    texts['src/egl/drivers/dri2/remesa_primary_device.h']=HEADER
    return texts

def prepare(mesa,output):
    if output.exists():raise ValueError('Output exists: preserve original overlays')
    texts=render(mesa)
    output.mkdir(parents=True)
    for rel,body in texts.items():
        p=output/rel;p.parent.mkdir(parents=True,exist_ok=True);p.write_text(body)
    manifest={'mesa':'25.2.0','origin':'https://archive.mesa3d.org/mesa-25.2.0.tar.xz',
              'scope':'Primary-only strict Poulsbo discovery; not OpenGL admission or scanout',
              'originals':PIN,'files':{r:hashlib.sha256((output/r).read_bytes()).hexdigest() for r in texts}}
    (output/'integration.json').write_text(json.dumps(manifest,indent=2)+'\n')
    return manifest
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--mesa',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    print(json.dumps(prepare(a.mesa,a.output),indent=2))
