#include "rtw_capi.h"
#ifdef HOST_CAPI_BUILD
#include "untitled_capi_host.h"
#define sizeof(...) ((size_t)(0xFFFF))
#undef rt_offsetof
#define rt_offsetof(s,el) ((uint16_T)(0xFFFF))
#define TARGET_CONST
#define TARGET_STRING(s) (s)
#ifndef SS_UINT64
#define SS_UINT64 17
#endif
#ifndef SS_INT64
#define SS_INT64 18
#endif
#else
#include "builtin_typeid_types.h"
#include "untitled.h"
#include "untitled_capi.h"
#include "untitled_private.h"
#ifdef LIGHT_WEIGHT_CAPI
#define TARGET_CONST
#define TARGET_STRING(s)               ((NULL))
#else
#define TARGET_CONST                   const
#define TARGET_STRING(s)               (s)
#endif
#endif
static const rtwCAPI_Signals rtBlockSignals [ ] = { { 0 , 0 , TARGET_STRING ( "untitled/Fcn" ) , TARGET_STRING ( "R" ) , 0 , 0 , 0 , 0 , 0 } , { 1 , 0 , TARGET_STRING ( "untitled/Gain1" ) , TARGET_STRING ( "" ) , 0 , 0 , 0 , 0 , 1 } , { 2 , 0 , TARGET_STRING ( "untitled/Gain2" ) , TARGET_STRING ( "" ) , 0 , 0 , 0 , 0 , 1 } , { 3 , 0 , TARGET_STRING ( "untitled/Integrator" ) , TARGET_STRING ( "" ) , 0 , 0 , 0 , 0 , 1 } , { 4 , 0 , TARGET_STRING ( "untitled/Divide" ) , TARGET_STRING ( "" ) , 0 , 0 , 0 , 0 , 1 } , { 5 , 0 , TARGET_STRING ( "untitled/Subtract" ) , TARGET_STRING ( "" ) , 0 , 0 , 0 , 0 , 1 } , { 6 , 0 , TARGET_STRING ( "untitled/Subtract1" ) , TARGET_STRING ( "" ) , 0 , 0 , 0 , 0 , 1 } , { 7 , 0 , TARGET_STRING ( "untitled/Transfer Fcn" ) , TARGET_STRING ( "" ) , 0 , 0 , 0 , 0 , 1 } , { 0 , 0 , ( NULL ) , ( NULL ) , 0 , 0 , 0 , 0 , 0 } } ; static const rtwCAPI_BlockParameters rtBlockParameters [ ] = { { 8 , TARGET_STRING ( "untitled/Constant" ) , TARGET_STRING ( "Value" ) , 0 , 0 , 0 } , { 9 , TARGET_STRING ( "untitled/Gain" ) , TARGET_STRING ( "Gain" ) , 0 , 0 , 0 } , { 10 , TARGET_STRING ( "untitled/Gain1" ) , TARGET_STRING ( "Gain" ) , 0 , 0 , 0 } , { 11 , TARGET_STRING ( "untitled/Gain2" ) , TARGET_STRING ( "Gain" ) , 0 , 0 , 0 } , { 12 , TARGET_STRING ( "untitled/Integrator" ) , TARGET_STRING ( "InitialCondition" ) , 0 , 0 , 0 } , { 13 , TARGET_STRING ( "untitled/Step" ) , TARGET_STRING ( "Time" ) , 0 , 0 , 0 } , { 14 , TARGET_STRING ( "untitled/Step" ) , TARGET_STRING ( "Before" ) , 0 , 0 , 0 } , { 15 , TARGET_STRING ( "untitled/Step" ) , TARGET_STRING ( "After" ) , 0 , 0 , 0 } , { 16 , TARGET_STRING ( "untitled/Transfer Fcn" ) , TARGET_STRING ( "A" ) , 0 , 0 , 0 } , { 17 , TARGET_STRING ( "untitled/Transfer Fcn" ) , TARGET_STRING ( "C" ) , 0 , 0 , 0 } , { 0 , ( NULL ) , ( NULL ) , 0 , 0 , 0 } } ; static int_T rt_LoggedStateIdxList [ ] = { - 1 } ; static const rtwCAPI_Signals rtRootInputs [ ] = { { 0 , 0 , ( NULL ) , ( NULL ) , 0 , 0 , 0 , 0 , 0 } } ; static const rtwCAPI_Signals rtRootOutputs [ ] = { { 0 , 0 , ( NULL ) , ( NULL ) , 0 , 0 , 0 , 0 , 0 } } ; static const rtwCAPI_ModelParameters rtModelParameters [ ] = { { 0 , ( NULL ) , 0 , 0 , 0 } } ;
#ifndef HOST_CAPI_BUILD
static void * rtDataAddrMap [ ] = { & rtB . a4mk3skjh4 , & rtB . d1msz2kjwf ,
& rtB . cukmkbaqu5 , & rtB . erkr3ou2mk , & rtB . eyvp5cxfdy , & rtB .
bgpyyuadau , & rtB . dkqy2b0lq5 , & rtB . gvcuw4a4mq , & rtP . Constant_Value
, & rtP . Gain_Gain , & rtP . Gain1_Gain , & rtP . Gain2_Gain , & rtP .
Integrator_IC , & rtP . Step_Time , & rtP . Step_Y0 , & rtP . Step_YFinal , &
rtP . TransferFcn_A , & rtP . TransferFcn_C , } ; static int32_T *
rtVarDimsAddrMap [ ] = { ( NULL ) } ;
#endif
static TARGET_CONST rtwCAPI_DataTypeMap rtDataTypeMap [ ] = { { "double" ,
"real_T" , 0 , 0 , sizeof ( real_T ) , ( uint8_T ) SS_DOUBLE , 0 , 0 , 0 } }
;
#ifdef HOST_CAPI_BUILD
#undef sizeof
#endif
static TARGET_CONST rtwCAPI_ElementMap rtElementMap [ ] = { { ( NULL ) , 0 ,
0 , 0 , 0 } , } ; static const rtwCAPI_DimensionMap rtDimensionMap [ ] = { {
rtwCAPI_SCALAR , 0 , 2 , 0 } } ; static const uint_T rtDimensionArray [ ] = {
1 , 1 } ; static const real_T rtcapiStoredFloats [ ] = { 0.0 , 1.0 } ; static
const rtwCAPI_FixPtMap rtFixPtMap [ ] = { { ( NULL ) , ( NULL ) ,
rtwCAPI_FIX_RESERVED , 0 , 0 , ( boolean_T ) 0 } , } ; static const
rtwCAPI_SampleTimeMap rtSampleTimeMap [ ] = { { ( const void * ) &
rtcapiStoredFloats [ 0 ] , ( const void * ) & rtcapiStoredFloats [ 1 ] , ( int8_T ) 1 , ( uint8_T ) 0 } , { ( const void * ) & rtcapiStoredFloats [ 0 ] , ( const void * ) & rtcapiStoredFloats [ 0 ] , ( int8_T ) 0 , ( uint8_T ) 0 } } ; static rtwCAPI_ModelMappingStaticInfo mmiStatic = { { rtBlockSignals , 8 , rtRootInputs , 0 , rtRootOutputs , 0 } , { rtBlockParameters , 10 , rtModelParameters , 0 } , { ( NULL ) , 0 } , { rtDataTypeMap , rtDimensionMap , rtFixPtMap , rtElementMap , rtSampleTimeMap , rtDimensionArray } , "float" , { 2406555788U , 2933295651U , 1403087645U , 496890032U } , ( NULL ) , 0 , ( boolean_T ) 0 , rt_LoggedStateIdxList } ; const rtwCAPI_ModelMappingStaticInfo * untitled_GetCAPIStaticMap ( void ) { return & mmiStatic ; }
#ifndef HOST_CAPI_BUILD
void untitled_InitializeDataMapInfo ( void ) { rtwCAPI_SetVersion ( ( *
rt_dataMapInfoPtr ) . mmi , 1 ) ; rtwCAPI_SetStaticMap ( ( *
rt_dataMapInfoPtr ) . mmi , & mmiStatic ) ; rtwCAPI_SetLoggingStaticMap ( ( *
rt_dataMapInfoPtr ) . mmi , ( NULL ) ) ; rtwCAPI_SetDataAddressMap ( ( *
rt_dataMapInfoPtr ) . mmi , rtDataAddrMap ) ; rtwCAPI_SetVarDimsAddressMap ( ( *
rt_dataMapInfoPtr ) . mmi , rtVarDimsAddrMap ) ;
rtwCAPI_SetInstanceLoggingInfo ( ( * rt_dataMapInfoPtr ) . mmi , ( NULL ) ) ;
rtwCAPI_SetChildMMIArray ( ( * rt_dataMapInfoPtr ) . mmi , ( NULL ) ) ;
rtwCAPI_SetChildMMIArrayLen ( ( * rt_dataMapInfoPtr ) . mmi , 0 ) ; }
#else
#ifdef __cplusplus
extern "C" {
#endif
void untitled_host_InitializeDataMapInfo ( untitled_host_DataMapInfo_T *
dataMap , const char * path ) { rtwCAPI_SetVersion ( dataMap -> mmi , 1 ) ;
rtwCAPI_SetStaticMap ( dataMap -> mmi , & mmiStatic ) ;
rtwCAPI_SetDataAddressMap ( dataMap -> mmi , ( NULL ) ) ;
rtwCAPI_SetVarDimsAddressMap ( dataMap -> mmi , ( NULL ) ) ; rtwCAPI_SetPath
( dataMap -> mmi , path ) ; rtwCAPI_SetFullPath ( dataMap -> mmi , ( NULL ) )
; rtwCAPI_SetChildMMIArray ( dataMap -> mmi , ( NULL ) ) ;
rtwCAPI_SetChildMMIArrayLen ( dataMap -> mmi , 0 ) ; }
#ifdef __cplusplus
}
#endif
#endif
