#ifndef model_try_cap_host_h__
#define model_try_cap_host_h__
#ifdef HOST_CAPI_BUILD
#include "rtw_capi.h"
#include "rtw_modelmap_simtarget.h"
typedef struct { rtwCAPI_ModelMappingInfo mmi ; }
model_try_host_DataMapInfo_T ;
#ifdef __cplusplus
extern "C" {
#endif
void model_try_host_InitializeDataMapInfo ( model_try_host_DataMapInfo_T *
dataMap , const char * path ) ;
#ifdef __cplusplus
}
#endif
#endif
#endif
