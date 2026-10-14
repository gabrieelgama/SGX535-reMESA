/* SPDX-License-Identifier: MIT */
#ifndef REMESA_DRI_H
#define REMESA_DRI_H
#include <stdbool.h>
#include "frontend/drm_driver.h"
/* Actual Mesa 25.2 pipe-loader descriptor, selected by DRM name gma500.
 * OpenGL screen admission remains closed until frontend contracts exist. */
extern const struct drm_driver_descriptor gma500_driver_descriptor;
struct remesa_dri_identity { unsigned vendor,device; };
int remesa_dri_probe_fd(int fd,struct remesa_dri_identity *);
/* Per-call output, never a global last-error object: concurrent loader calls
 * cannot overwrite another call's attribution. No execution permission is
 * conferred by identity or transport validation. */
enum remesa_dri_stage {
 REMESA_DRI_RETAIN_FD,
 REMESA_DRI_PCI_IDENTITY,
 REMESA_DRI_DRM_TRANSPORT,
 REMESA_DRI_GALLIUM_SCREEN,
 REMESA_DRI_OPENGL_ADMISSION
};
enum remesa_dri_missing_contract {
 REMESA_DRI_MISSING_SHADER_PIPELINE = 1u << 0,
 REMESA_DRI_MISSING_TEXTURE_SAMPLING = 1u << 1,
 REMESA_DRI_MISSING_RASTER_RULES = 1u << 2,
 REMESA_DRI_MISSING_RENDER_STATES = 1u << 3,
 REMESA_DRI_MISSING_COLOR_PRESERVATION = 1u << 4
};
struct remesa_dri_report {
 enum remesa_dri_stage stage;
 int error;
 struct remesa_dri_identity identity;
 unsigned missing_contracts;
 bool identity_validated,transport_validated,screen_created;
};
struct pipe_screen *remesa_dri_create_screen(int,
 const struct pipe_screen_config *,struct remesa_dri_report *);
const char *remesa_dri_stage_name(enum remesa_dri_stage);
unsigned remesa_dri_missing_contracts(void);
const char *remesa_dri_unavailable_reason(void);
#endif
