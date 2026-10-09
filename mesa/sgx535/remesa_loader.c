/* SPDX-License-Identifier: MIT */
#include "remesa_loader.h"
#include "remesa_linux.h"
#include "remesa_screen.h"
#include <errno.h>
static int close_device(struct remesa_device *device)
{ remesa_linux_close(device);return 0; }
struct pipe_screen *remesa_loader_diagnostic_screen(int fd,int *error)
{
 struct remesa_device device={0};struct pipe_screen *screen;int e;
 if(!error) return NULL;
 *error=0;e=remesa_linux_open_fd(&device,fd);
 if(e) { *error=e;return NULL; }
 screen=remesa_screen_create_owned(&device,close_device);
 if(!screen) { remesa_linux_close(&device);*error=ENOMEM; }
 return screen;
}
