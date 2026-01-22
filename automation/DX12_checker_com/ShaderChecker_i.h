

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* at Tue Jan 19 08:44:07 2038
 */
/* Compiler settings for C:\Users\samar\Documents\ser_py_experiment\DX12_checker_com\ShaderChecker.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0628 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __ShaderChecker_i_h__
#define __ShaderChecker_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if defined(_CONTROL_FLOW_GUARD_XFG)
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

#ifndef __IShaderChecker_FWD_DEFINED__
#define __IShaderChecker_FWD_DEFINED__
typedef interface IShaderChecker IShaderChecker;

#endif 	/* __IShaderChecker_FWD_DEFINED__ */


#ifndef __CShaderChecker_FWD_DEFINED__
#define __CShaderChecker_FWD_DEFINED__

#ifdef __cplusplus
typedef class CShaderChecker CShaderChecker;
#else
typedef struct CShaderChecker CShaderChecker;
#endif /* __cplusplus */

#endif 	/* __CShaderChecker_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IShaderChecker_INTERFACE_DEFINED__
#define __IShaderChecker_INTERFACE_DEFINED__

/* interface IShaderChecker */
/* [oleautomation][dual][unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IShaderChecker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AAC09469-23D7-4B8E-B454-776CA3B19AD2")
    IShaderChecker : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_IsSupported( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_HighestShaderModel( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_AdapterName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_FeatureLevel( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_LastError( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_ExperimentalFeaturesEnabled( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IShaderCheckerVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShaderChecker * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShaderChecker * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShaderChecker * This);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfoCount)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IShaderChecker * This,
            /* [out] */ UINT *pctinfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetTypeInfo)
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IShaderChecker * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        DECLSPEC_XFGVIRT(IDispatch, GetIDsOfNames)
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IShaderChecker * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        DECLSPEC_XFGVIRT(IDispatch, Invoke)
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IShaderChecker * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        DECLSPEC_XFGVIRT(IShaderChecker, get_IsSupported)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_IsSupported )( 
            IShaderChecker * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        DECLSPEC_XFGVIRT(IShaderChecker, get_HighestShaderModel)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_HighestShaderModel )( 
            IShaderChecker * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IShaderChecker, get_AdapterName)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_AdapterName )( 
            IShaderChecker * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IShaderChecker, get_FeatureLevel)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_FeatureLevel )( 
            IShaderChecker * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IShaderChecker, get_LastError)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_LastError )( 
            IShaderChecker * This,
            /* [retval][out] */ BSTR *pVal);
        
        DECLSPEC_XFGVIRT(IShaderChecker, get_ExperimentalFeaturesEnabled)
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_ExperimentalFeaturesEnabled )( 
            IShaderChecker * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        END_INTERFACE
    } IShaderCheckerVtbl;

    interface IShaderChecker
    {
        CONST_VTBL struct IShaderCheckerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShaderChecker_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IShaderChecker_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IShaderChecker_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IShaderChecker_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IShaderChecker_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IShaderChecker_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IShaderChecker_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IShaderChecker_get_IsSupported(This,pVal)	\
    ( (This)->lpVtbl -> get_IsSupported(This,pVal) ) 

#define IShaderChecker_get_HighestShaderModel(This,pVal)	\
    ( (This)->lpVtbl -> get_HighestShaderModel(This,pVal) ) 

#define IShaderChecker_get_AdapterName(This,pVal)	\
    ( (This)->lpVtbl -> get_AdapterName(This,pVal) ) 

#define IShaderChecker_get_FeatureLevel(This,pVal)	\
    ( (This)->lpVtbl -> get_FeatureLevel(This,pVal) ) 

#define IShaderChecker_get_LastError(This,pVal)	\
    ( (This)->lpVtbl -> get_LastError(This,pVal) ) 

#define IShaderChecker_get_ExperimentalFeaturesEnabled(This,pVal)	\
    ( (This)->lpVtbl -> get_ExperimentalFeaturesEnabled(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IShaderChecker_INTERFACE_DEFINED__ */



#ifndef __ShaderCheckerTypeLib_LIBRARY_DEFINED__
#define __ShaderCheckerTypeLib_LIBRARY_DEFINED__

/* library ShaderCheckerTypeLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_ShaderCheckerTypeLib;

EXTERN_C const CLSID CLSID_CShaderChecker;

#ifdef __cplusplus

class DECLSPEC_UUID("54F71999-647C-4B98-BFF9-84F28A9A25E5")
CShaderChecker;
#endif
#endif /* __ShaderCheckerTypeLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


