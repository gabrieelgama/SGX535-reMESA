/* SPDX-License-Identifier: MIT */
#ifndef REMESA_DRI_H
#define REMESA_DRI_H
#include "frontend/drm_driver.h"
/* Actual Mesa 25.2 pipe-loader descriptor, selected by DRM name gma500.
 * OpenGL screen admission remains closed until frontend contracts exist. */
extern const struct drm_driver_descriptor gma500_driver_descriptor;
struct remesa_dri_identity { unsigned vendor,device; };
int remesa_dri_probe_fd(int fd,struct remesa_dri_identity *);
const char *remesa_dri_unavailable_reason(void);
#endif
