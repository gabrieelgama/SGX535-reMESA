/* SPDX-License-Identifier: MIT */
#ifndef REMESA_LOADER_H
#define REMESA_LOADER_H
struct pipe_screen;
/* Loader fd intake for isolated Gallium clients. Borrows fd, owns a CLOEXEC
 * duplicate. This diagnostic screen is not OpenGL-capable and is not registered
 * in Mesa's DRI loader. No draw or memory ioctl is issued during construction. */
struct pipe_screen *remesa_loader_diagnostic_screen(int drm_fd, int *error);
#endif
