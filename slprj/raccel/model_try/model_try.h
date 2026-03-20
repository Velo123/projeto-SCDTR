#ifndef model_try_h_
#define model_try_h_
#ifndef model_try_COMMON_INCLUDES_
#define model_try_COMMON_INCLUDES_
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
#include "model_try_types.h"
#include <stddef.h>
#include "rtw_modelmap_simtarget.h"
#include "rt_defines.h"
#include <string.h>
#include "rtGetInf.h"
#define MODEL_NAME model_try
#define NSAMPLE_TIMES (3) 
#define NINPUTS (0)       
#define NOUTPUTS (0)     
#define NBLOCKIO (13) 
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
typedef struct { real_T gzs5ljzzcf ; real_T iyhc1d5uql ; real_T a134e1fwcs ;
real_T aermkud2je ; real_T afmigrzdnh ; real_T c1av40nqjq ; real_T gecrhmdlou
; real_T avorjxyj5i ; real_T opwes3yv33 ; real_T agh0sivjfe ; real_T
bbrkhev13p ; real_T n3uhymnxs2 ; real_T eo33mkuth1 ; } B ; typedef struct {
struct { void * LoggedData ; } dx4e4qby11 ; struct { void * LoggedData ; }
hqy1muysyt ; struct { void * LoggedData ; } cfmvyoq5ab ; struct { void *
LoggedData ; } cutmokzxpk ; struct { void * LoggedData ; } kqeuc0diwp ; int_T
gp52eidcv3 ; } DW ; typedef struct { real_T ik3kwpksuc ; real_T ehvaddwwy1 ;
} X ; typedef struct { real_T ik3kwpksuc ; real_T ehvaddwwy1 ; } XDot ;
typedef struct { boolean_T ik3kwpksuc ; boolean_T ehvaddwwy1 ; } XDis ;
typedef struct { real_T ik3kwpksuc ; real_T ehvaddwwy1 ; } CStateAbsTol ;
typedef struct { real_T ik3kwpksuc ; real_T ehvaddwwy1 ; } CXPtMin ; typedef
struct { real_T ik3kwpksuc ; real_T ehvaddwwy1 ; } CXPtMax ; typedef struct {
real_T m4k1guz4ye ; real_T n5c2ifrrnv ; } ZCV ; typedef struct {
rtwCAPI_ModelMappingInfo mmi ; } DataMapInfo ; struct P_ { real_T R ; real_T
Integrator_IC ; real_T Step_Time ; real_T Step_Y0 ; real_T Step_YFinal ;
real_T Gain_Gain ; real_T Integrator1_IC ; real_T Saturation_UpperSat ;
real_T Saturation_LowerSat ; real_T Gain2_Gain ; real_T Gain3_Gain ; real_T
Constant_Value ; } ; extern const char_T * RT_MEMORY_ALLOCATION_ERROR ;
extern B rtB ; extern X rtX ; extern DW rtDW ; extern P rtP ; extern mxArray
* mr_model_try_GetDWork ( ) ; extern void mr_model_try_SetDWork ( const
mxArray * ssDW ) ; extern mxArray * mr_model_try_GetSimStateDisallowedBlocks
( ) ; extern const rtwCAPI_ModelMappingStaticInfo *
model_try_GetCAPIStaticMap ( void ) ; extern SimStruct * const rtS ; extern
DataMapInfo * rt_dataMapInfoPtr ; extern rtwCAPI_ModelMappingInfo *
rt_modelMapInfoPtr ; void MdlOutputs ( int_T tid ) ; void
MdlOutputsParameterSampleTime ( int_T tid ) ; void MdlUpdate ( int_T tid ) ;
void MdlTerminate ( void ) ; void MdlInitializeSizes ( void ) ; void
MdlInitializeSampleTimes ( void ) ; SimStruct * raccel_register_model ( ssExecutionInfo * executionInfo ) ;
#endif
