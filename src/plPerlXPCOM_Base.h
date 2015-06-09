/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Perl XPCOM language bindings.
 *
 * The Initial Developer of the Original Code is
 * Jumpline.com, Inc. Portions created by Jumpline.com, Inc. are
 * Copyright (C) 2001 Jumpline.com, Inc. All Rights Reserved.
 *
 * Contributor(s):
 * Matt Kennedy <matt@jumpline.com> (original author)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

/*****************************************************************************
 * File: plPerlXPCOM_Base.h
 * Created by: matt
 * Created on: Fri 18 May 2001 04:06:50 PM EDT
 * $Id: plPerlXPCOM_Base.h,v 1.5 2005/03/31 08:29:00 poumeyrol Exp $
 *****************************************************************************/
#ifndef VPPERLXPCOM_BASE_H_
#define VPPERLXPCOM_BASE_H_

#include "nsISupports.h"
//#include "nsIWeakReference.h"
#include "nsIInterfaceInfoManager.h"
#include "nsIInterfaceInfo.h"
#include "nsIComponentLoader.h"
#include "plIPerlComponentHelper.h"

// MOZILLA_VERSION is not defined in 1.2.1
#include "../../../mozilla-config.h"
#ifdef MOZILLA_VERSION
#include "xpcom-obsolete/nsIRegistry.h"
#else
#include "nsIRegistry.h"
#endif

#include "nsIModule.h"
#include "nsIFactory.h"
#include "nsIFile.h"
#include "nsVoidArray.h"
#include "nsCOMPtr.h"
#include "nsHashtable.h"
#include "plPMFile.h"
#include "xptcall.h"
#include "../perlXPUtil.h"
//#include "EXTERN.h"
//#include "perl.h"

extern const char perlComponentType[];

#define NS_IINTERNALPERL_IID_STR "4c4dc1d7-0d8f-4414-b17e-885ef3f3d0c3"
#define NS_IINTERNALPERL_IID \
  { 0x4c4dc1d7, 0x0d8f, 0x4414, \
    { 0xb1, 0x7e, 0x88, 0x5e, 0xf3, 0xf3, 0xd0, 0xc3 }}

class nsIInternalPerl : public nsISupports {
  public:
    NS_DEFINE_STATIC_IID_ACCESSOR(NS_IINTERNALPERL_IID)
};

/**
 * The base class for Perl XPCOM components. This class provides the container 
 * for the perl object and a call mechanism to invoke methods on that object.
 */
class plPerlBase {// : public nsIInternalPerl { //, public nsIWeakReference {
  public:
    //NS_DECL_ISUPPORTS
    //NS_DECL_NSISUPPORTSWEAKREFERENCE

    /**
     * Default constructor.
     */
    plPerlBase();

    /**
     * Constructor with a blessed perl object and an interface ID that it 
     * implements. The perl object isn't copied, its reference count is 
     * merely incremented.
     */
    plPerlBase(SV *perlObject, const nsIID& iid);

    /**
     * Destructor. If the object is holding a reference to a blessed perl 
     * object it releases it here.
     */
    virtual ~plPerlBase();


    /**
     * Call an XPCOM method on a perl obkect. This function takes a perl 
     * object, a method to invoke by name, and a list of all input and 
     * output arguments.
     *
     * @param perlObject The blessed perl object to use in the method call.
     * @param methodName The method to call.
     * @param d A pointer to an array of input arguments to pass to the 
     *          method.
     * @param paramIndex A count of the number of input arguments passed.
     * @param o A pointer to an array of output arguments for the method call
     *          to use to store return values.
     * @param outputIndex A count of the number of output arguments given.
     */
    static nsresult CallPerlMethod(SV *perlObject, 
                                   const char *methodName,
                                   const nsXPTCVariant *d = NULL,
                                   int paramIndex = -1,
                                   nsXPTCVariant *o = NULL,
                                   int outputIndex = -1);
    SV *m_perlObject;
    nsIID m_iid;
};

/**
 * The stub XPCOM representation class for a Perl XPCOM object. This class 
 * stores a perl object which implements one or more XPCOM interfaces and 
 * can masquerade as any XPCOM object. Any perl XPCOM object which is not 
 * an implementation of nsIModule or nsIFactory will be an instance of this 
 * class.
 */
class plPerlXPTCStub : public plPerlBase, public nsXPTCStubBase {
  public:
    NS_DECL_ISUPPORTS

    /**
     * Fetches the interface info for the XPCOM interface the stub is 
     * currently masquerading as.
     */
    NS_IMETHOD GetInterfaceInfo(nsIInterfaceInfo **info);

    /**
     * This is the handler method that the stubs inherited from nsXPTCStubBase 
     * funnel down to in order to invoke perl methods on the underlying 
     * blessed object.
     *
     * @param methodIndex The method to call specified by its numeric index.
     * @param info The method info for the requested method to call.
     * @param params A pointer to an array of input and output parameters 
     *        given to the method call.
     */
    NS_IMETHOD CallMethod(PRUint16 methodIndex,
                          const nsXPTMethodInfo *info,
                          nsXPTCMiniVariant *params);

    plPerlXPTCStub();
    plPerlXPTCStub(SV *perlObject, const nsIID& iid);
    //virtual ~plPerlXPTCStub();
};

#define NS_PERLMODULE_CID \
{ 0x3eb7e7b7, 0xe25e, 0x4b01, \
	{ 0x9e, 0x65, 0x98, 0x89, 0x30, 0xcf, 0x86, 0x89 }}

#define NS_PERLMODULE_CONTRACTID "@jumpline.com/perl-module;1"

/**
 * This class is a dedicated wrapper for perl objects implementing the 
 * nsIModule interface.
 */
class plPerlModule : public plPerlBase, public nsIModule {
  public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSIMODULE

    plPerlModule(SV *perlObject);
    //virtual ~plPerlModule() {}
    plPerlModule();
};

#define NS_PERLFACTORY_CID \
{ 0x7628e93d, 0xa251, 0x4fc0, \
	{ 0xbd, 0xa0, 0x4d, 0xa0, 0x11, 0xba, 0xc5, 0x63 }}

#define NS_PERLFACTORY_CONTRACTID "@jumpline.com/perl-factory;1"

/**
 * This class is a dedicated wrapper for perl objects which implement the 
 * nsIFactory interface.
 */
class plPerlFactory : public plPerlBase, public nsIFactory {
  public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSIFACTORY

    plPerlFactory(SV *perlObject);
    //virtual ~plPerlFactory();
    plPerlFactory();
};

#define NS_PLCOMPONENTLOADER_CID \
{ 0x21031a23, 0xae72, 0x4d18, \
	{ 0x8e, 0x55, 0x03, 0x94, 0x76, 0xaf, 0x6b, 0x7c }}

#define NS_PLCOMPONENTLOADER_CONTRACTID "@jumpline.com/perl-component-loader;1"

/**
 * This class is the perl component loader utilized by the nsIComponentManager 
 * for handling all requests to register and access perl components. The
 * perl component loader serves as a protective barrier to the application 
 * utilizing XPCOM in the event of encountering a module with syntax errors 
 * at runtime.
 *
 * Perl XPCOM component modules are regular perl modules, however they are not 
 * loaded with Perl's <code>use</code> pragma, nor do they reside in perl's 
 * perl's include path. The perl component loader class handles detecting 
 * modules in XPCOM's component directory and loads the module into the perl 
 * interpreter with the equivalent of a perl <code>eval</code> statement. As
 * a result, any syntax errors are caught at runtime without killing the 
 * interpreter, so an application using XPCOM will suffer no ill effects (that 
 * is of course as long the application writer does proper error handling when
 * instantiating her objects).
 */
class plPerlComponentLoader : public nsIComponentLoader {
  public:
    NS_DECL_ISUPPORTS
    NS_DECL_NSICOMPONENTLOADER

    plPerlComponentLoader();
    virtual ~plPerlComponentLoader();

    static nsresult GetLoader(nsIComponentLoader **aComponentLoader);
  protected:
//    nsCOMPtr<nsIRegistry>   m_Registry;
    nsIComponentManager    *m_CompMgr;
    nsObjectHashtable      *m_PmStore;
    nsSupportsHashtable    *m_Modules; // Stores out module instances since 
                                       // we don't use the native DLL aspects 
                                       // of plPMFile.
    nsRegistryKey           m_XPCOMKey;
    nsVoidArray             m_DeferredComponents;
  private:
    /**
     * Creates a new plPMFile object for a perl XPCOM module.
     *
     * @param aSpec The file spec for the module.
     * @param aLocation The location of the module as given by 
     *                  nsIComponentLoader.
     * @param modifiedTime The module's last modified timestamp.
     * @param fileSize The size of the module file in bytes.
     * @return aDll A representation of the module as an plPMFile.
     */
    nsresult CreateModule(nsIFile *aSpec, const char *aLocation,
                      PRInt64 *modifiedTime, PRInt64 *fileSize, plPMFile **aDll);
    nsresult SelfRegisterModule(plPMFile *dll, const char *registryLocation, 
                                PRBool deferred);
    nsresult GetRegistryModuleInfo(const char *aLocation,
                                   PRInt64 *lastModifiedTime,
                                   PRInt64 *fileSize);
    nsresult GetRegistryModuleInfo(nsRegistryKey key,
                                   PRInt64 *lastModifiedTime,
                                   PRInt64 *fileSize);
    nsresult SelfUnregisterModule(plPMFile *dll);
    nsresult RemoveRegistryModuleInfo(const char *aLocation);
    nsresult GetModule(nsIComponentManager *mgr, plPMFile *dll, nsIModule **mod);
    nsresult SetRegistryModuleInfo(const char *aLocation, 
                                   PRInt64 *lastModifiedTime,
                                   PRInt64 *fileSize);
};

class plPerlComponentHelper : public plIPerlComponentHelper {
  public:
    NS_DECL_ISUPPORTS
    NS_DECL_PLIPERLCOMPONENTHELPER

    plPerlComponentHelper();
    virtual ~plPerlComponentHelper();
};

NS_COM nsresult 
VP_NewPerlComponentLoader(nsIComponentLoader **aComponentLoader);

inline nsresult
VP_ConvertSVToISupports(SV *in, nsISupports **out, const nsIID *iid = nsnull) {
  if(!in || !out) return NS_ERROR_FAILURE;
  nsIID *tIID = nsnull;
  nsIID riid;
  if(!iid) {
    nsresult res = perlXPUtil::fetchXPObject(in, nsnull, &tIID, nsnull);
    if(NS_FAILED(res)) {
      riid = NS_GET_IID(nsISupports);
    } else {
      riid = *tIID;
      delete tIID;
    }
  } else {
    riid = *iid;
  }
  if(riid.Equals(NS_GET_IID(nsIFactory))) {
    plPerlFactory *fact = new plPerlFactory(in);
    NS_ADDREF(fact);
    *out = fact;
  } else if(riid.Equals(NS_GET_IID(nsIModule))) {
    plPerlModule *mod = new plPerlModule(in);
    NS_ADDREF(mod);
    *out = mod;
  } else {
    plPerlXPTCStub *ret = new plPerlXPTCStub(in, riid);
    NS_ADDREF(ret);
    *out = ret;
  }
  return NS_OK;
}
#endif
