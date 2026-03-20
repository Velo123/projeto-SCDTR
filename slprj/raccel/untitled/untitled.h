#ifndef untitled_h_
#define untitled_h_
#ifndef untitled_COMMON_INCLUDES_
#define untitled_COMMON_INCLUDES_
#include <stdlib.h>
#include "rtwtypes.h"
#include "sigstream_rtw.h"
#include "simtarget/slSimTgtSigstreamRTW.h"
#include "simtarget/slSimTgtSlioCoreRTW.h"
#include "simtarget/slSimTgtSlioClientsRTW.h"
#include "simtarget/slSimTgtSlioSdiRTW.h"
#include "simstruc.h"
#include "fixedpoint.h"
#include "raccel.h"
#include "slsv_diagnostic_codegen_c_api.h"
#include "rt_logging_simtarget.h"
#include "rt_nonfinite.h"
#include "math.h"
#include "dt_info.h"
#include "ext_work.h"
#endif
#include "untitled_types.h"
#include <stddef.h>
#include "rtw_modelmap_simtarget.h"
#include "rt_defines.h"
#include <string.h>
#include "rtGetInf.h"
#define MODEL_NAME untitled
#define NSAMPLE_TIMES (3) 
#define NINPUTS (0)       
#define NOUTPUTS (0)     
#define NBLOCKIO (8) 
#define NUM_ZC_EVENTS (0) 
#ifndef NCSTATES
#define NCSTATES (2)   
#elif NCSTATES != 2
#error Invalid specification of NCSTATES defined in compiler command
#endif
#ifndef rtmGetDataMapInfo
#define rtmGetDataMapInfo(rtm) (*rt_dataMapInfoPtr)
#endif
#ifndef rtmSetDataMapInfo
#define rtmSetDataMapInfo(rtm, val) (rt_dataMapInfoPtr = &val)
#endif
#ifndef IN_RACCEL_MAIN
#endif
typedef struct { real_T erkr3ou2mk ; real_T bgpyyuadau ; real_T gvcuw4a4mq ;
real_T eyvp5cxfdy ; real_T a4mk3skjh4 ; real_T d1msz2kjwf ; real_T dkqy2b0lq5
; real_T cukmkbaqu5 ; } B ; typedef struct { struct { void * LoggedData ; }
j5yz1t50bu ; int_T bh53ipsdxb ; } DW ; typedef struct { real_T lhtksapsjn ;
real_T fi0dpbq5wn ; } X ; typedef struct { real_T lhtksapsjn ; real_T
fi0dpbq5wn ; } XDot ; typedef struct { boolean_T lhtksapsjn ; boolean_T
fi0dpbq5wn ; } XDis ; typedef struct { real_T lhtksapsjn ; real_T fi0dpbq5wn
; } CStateAbsTol ; typedef struct { real_T lhtksapsjn ; real_T fi0dpbq5wn ; }
CXPtMin ; typedef struct { real_T lhtksapsjn ; real_T fi0dpbq5wn ; } CXPtMax
; typedef struct { real_T cpetedpfge ; } ZCV ; typedef struct {
rtwCAPI_ModelMappingInfo mmi ; } DataMapInfo ; struct P_ { real_T
Integrator_IC ; real_T TransferFcn_A ; real_T TransferFcn_C ; real_T
Step_Time ; real_T Step_Y0 ; real_T Step_YFinal ; real_T Gain_Gain ; real_T
Gain1_Gain ; real_T Gain2_Gain ; real_T Constant_Value ; } ; extern const
char_T * RT_MEMORY_ALLOCATION_ERROR ; extern B rtB ; extern X rtX ; extern DW
rtDW ; extern P rtP ; extern mxArray * mr_untitled_GetDWork ( ) ; extern void
mr_untitled_SetDWork ( const mxArray * ssDW ) ; extern mxArray *
mr_untitled_GetSimStateDisallowedBlocks ( ) ; extern const
rtwCAPI_ModelMappingStaticInfo * untitled_GetCAPIStaticMap ( void ) ; extern
SimStruct * const rtS ; extern DataMapInfo * rt_dataMapInfoPtr ; extern
rtwCAPI_ModelMappingInfo * rt_modelMapInfoPtr ; void MdlOutputs ( int_T tid )
; void MdlOutputsParameterSampleTime ( int_T tid ) ; void MdlUpdate ( int_T
tid ) ; void MdlTerminate ( void ) ; void MdlInitializeSizes ( void ) ; void
MdlInitializeSampleTimes ( void ) ; SimStruct * raccel_register_model ( ssExecutionInfo * executionInfo ) ;
#endif
