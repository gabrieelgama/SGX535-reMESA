/* SPDX-License-Identifier: MIT */
#ifndef REMESA_RETAINED_TRANSPORT_H
#define REMESA_RETAINED_TRANSPORT_H
#include <stdbool.h>
#include "remesa/retained_uapi.h"
/* Laboratory transport ONLY, no discovery, allocation, automatic retry or
 * ordinary Gallium dispatch replacement. Recorder must preserve complete raw
 * request/response. Attempted means ioctl was entered, not SGX admission.
 * A successful response is still not physical completion authentication:
 * preserved observer/lifecycle/GEM records must independently corroborate it.
 */
typedef int (*remesa_retained_recorder)(void *,const struct remesa_retained_request *,
 const struct remesa_retained_request *,int,int);
int remesa_retained_submit_once(int,struct remesa_retained_request *,bool *,
 remesa_retained_recorder,void *);
#endif
