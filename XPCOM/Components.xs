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

#include "nsIServiceManager.h"
#include "nsILocalFile.h"
#include "../perlXPUtil.h"
#include "plIPerlComponentHelper.h"
#include "prprf.h"
//#include "nsXPIDLString.h"
#include <stdio.h>
#include <stdlib.h>
#include "string.h"

char *mozdir;

void XPCOMShutdown(void) {
#ifdef XP_PERL_DEBUG
  warn("Shutting down XPCOM.\n");
#endif
  NS_ShutdownXPCOM(nsnull);
  PR_smprintf_free(mozdir);
}

MODULE = XPCOM		PACKAGE = Components		

BOOT:
    nsresult res;
    nsCOMPtr<nsILocalFile> xpDir = nsnull;
    char *mozenv = nsnull;
    mozdir = nsnull;
    // If MOZILLA_FIVE_HOME is predefined you can use this module for 
    // standalone perl scripts. Great for those of you who've been dying 
    // to XUL-ify the CPAN shell!! ;)
    mozenv = getenv("MOZILLA_FIVE_HOME");
    if(!mozenv) {
      warn("Warning: MOZILLA_FIVE_HOME is not in environment.\n");
    }
    mozdir = PR_smprintf("MOZILLA_FIVE_HOME=%s", mozenv);
    res = NS_NewLocalFile(NS_ConvertASCIItoUCS2(mozenv), 
                          PR_FALSE, 
                          getter_AddRefs(xpDir));
    putenv(mozdir);
    // Unless you're running a standalone perl script, this will most likely 
    // initialize XPCOM twice. This will cause an NS_ASSERTION in debug builds 
    // however the library can handle it. I have been unable to find any way 
    // to programmatically determine if XPCOM is initialized, so I resorted 
    // to the brute force approach.
    res = NS_InitXPCOM2(nsnull, xpDir, nsnull);
    // If we failed, chances are good that init was already called.
    if(NS_SUCCEEDED(res)) {
      atexit(XPCOMShutdown);
    }
#ifdef XP_PERL_DEBUG
      warn("Initialized XPCOM.\n");
#endif

void
Classes(c)
  INPUT:
    char *c
  INIT:
    nsCID cid;
    nsISupports *obj = nsnull;
    nsresult res;
  CODE:
    res = nsComponentManager::ContractIDToClassID(c, &cid);
    if(NS_FAILED(res)) {
      warn("Unrecognized contract ID: %s\n", c);
      VP_SET_RETCODE(res);
      VP_RETURN_UNDEF;
    }
#ifdef XP_PERL_DEBUG
    char *tmpc = cid.ToString();
    warn("Got CID of %s for %s\n", tmpc, c);
    nsMemory::Free(tmpc);
    warn("Creating instance of class.\n");
#endif
    res = nsComponentManager::CreateInstance(cid, 
                                             nsnull, 
                                             NS_GET_IID(nsISupports), 
                                             (void **)&obj);
    VP_SET_RETCODE(res);
    if(NS_FAILED(res)) {
#ifdef XP_PERL_DEBUG
      warn("Error creating instance.\n");
#endif
      VP_RETURN_UNDEF;
    }
    ST(0) = perlXPUtil::makeXPObject(&cid, nsnull, obj);
    NS_RELEASE(obj);
    XSRETURN(1);

void
ClassesByID(c)
  INPUT:
    char *c
  INIT:
    nsCID cid;
    nsresult res = 0;
    nsISupports *obj = nsnull;
  CODE:
    VP_SET_RETCODE(NS_ERROR_NOT_IMPLEMENTED);
    VP_RETURN_UNDEF; // Disabled right now. Nothing uses it
    cid.Parse(c);
    res = nsComponentManager::CreateInstance(cid,
                                             nsnull,
                                             NS_GET_IID(nsISupports),
                                             (void **)&obj);
    if(NS_FAILED(res)) {
      VP_RETURN_UNDEF;
    }
    ST(0) = sv_newmortal();
    XSRETURN(1);

int
isSuccessCode(r)
  INPUT:
    PRUint32 r
  CODE:
    if(NS_SUCCEEDED(r)) {
      RETVAL = 1;
    } else {
      RETVAL = 0;
    }
  OUTPUT:
    RETVAL

void
Interfaces(i)
  INPUT:
    char *i
  INIT:
    nsIID *iid = nsnull;
    nsIID *riid = nsnull;
    nsIInterfaceInfoManager *iim = XPTI_GetInterfaceInfoManager();
  CODE:
    if(NS_FAILED(iim->GetIIDForName(i, &iid))) {
      NS_RELEASE(iim);
      VP_RETURN_UNDEF;
    }
    riid = new nsIID;
    *riid = *iid;
    ST(0) = sv_newmortal();
    sv_setref_pv(ST(0), "nsID", (void *)riid);
    NS_RELEASE(iim);
    XSRETURN(1);
    
void 
GetService(c,i)
  INIT:
    const char *c = (const char *)SvPV(ST(0),PL_na);
    nsCID cid;
    nsIID iid;
    nsISupports *newService = nsnull;
    nsresult res = 0;
  CODE:
    res = nsComponentManager::ContractIDToClassID(c, &cid);
    if(NS_FAILED(res)) {
        printf("ContractIDToClassID failed for cid %s, res=0x%x\n", c, res);
        VP_SET_RETCODE(res);
      VP_RETURN_UNDEF;
    }
    // XXX - The XPCOM module family supports a pair of tied hashes to 
    // access the classes and interfaces available through XPCOM. A somewhat 
    // bizarre effect of this is that the magic associated with the SV through 
    // the tie is not invoked if the tied hash is used in an argument. As a 
    // result, we need to check if the argument has get magic associated with 
    // it and invoke it to get to the value we want. For example, without the 
    // next if statement, the following code will not work in perl:
    //
    // $someObject->QueryInterface($Components::interfaces{'someIFace'});
    //
    // But this does:
    //
    // my $iid = $Components::interfaces{'someIFace'};
    // $someObject->QueryInterface($iid);
    //
    // Fucked up don't you think? I imagine there's good reason for it, but 
    // it didn't exactly obey the Principle of Least Astonishment.
    // 
    // FIXME - This code can be handled by fetchIIDFromSV() now since it 
    // occurs often enough.
    res = perlXPUtil::fetchIIDFromSV(ST(1), iid);
    if(NS_FAILED(res)) {
      VP_SET_RETCODE(res);
      VP_RETURN_UNDEF;
    }
    res = nsServiceManager::GetService(cid, iid, (nsISupports **)&newService);
    if(NS_FAILED(res)) {
      VP_SET_RETCODE(res);
      VP_RETURN_UNDEF;
    }
    ST(0) = perlXPUtil::makeXPObject(&cid, &iid, newService);
    NS_RELEASE(newService);
    VP_SET_RETCODE(res);
    XSRETURN(1);
    
void
Constructor(c,iid)
  INIT:
    const char *c = (const char *)SvPV(ST(0),PL_na);
    nsCID cid;
    nsIID iid;
    nsresult res = 0;
    nsISupports *obj;
  CODE:
    res = nsComponentManager::ContractIDToClassID(c, &cid);
    if(NS_FAILED(res)) {
      warn("Unrecognized contract ID: %s\n", c);
      VP_SET_RETCODE(res);
      VP_RETURN_UNDEF;
    }
    if(NS_FAILED(perlXPUtil::fetchIIDFromSV(ST(1), iid))) {
      warn("Argument 2 of Components::Constructor is not an interface ID.\n");
      VP_RETURN_UNDEF;
    }
    res = nsComponentManager::CreateInstance(cid, nsnull, iid, (void **)&obj);
    if(NS_FAILED(res)) {
      warn("Unable to create object with class id %s\n", c);
      VP_RETURN_UNDEF;
    }
    ST(0) = perlXPUtil::makeXPObject(&cid, &iid, obj);
    NS_RELEASE(obj);
    XSRETURN(1);

void
Manager()
  INIT:
    nsIComponentManager *compMgr = nsnull;
    nsresult res = NS_OK;
    nsIID cmiid = NS_GET_IID(nsIComponentManager);
  CODE:
    // XXX - This isn't Add Ref'ed.
    res = NS_GetGlobalComponentManager(&compMgr);
    if(NS_FAILED(res)) {
      VP_SET_RETCODE(res);
      VP_RETURN_UNDEF;
    }
    ST(0) = perlXPUtil::makeXPObject(nsnull, &cmiid, compMgr);
    VP_SET_RETCODE(NS_OK);
    XSRETURN(1);
  
void
WrapObject(obj,iid)
  INIT:
    SV *obj = nsnull;
    nsIID iid;
    nsresult res = NS_OK;
    nsISupports *retobj = nsnull;
    nsCOMPtr<plIPerlComponentHelper> helper = nsnull;
  CODE:
    obj = ST(0);
    if(NS_FAILED(perlXPUtil::fetchIIDFromSV(ST(1), iid))) {
      warn("Argument 2 of Components::WrapObject is not an interface ID.\n");
      VP_SET_RETCODE(NS_ERROR_ABORT);
      VP_RETURN_UNDEF;
    }
    helper = do_CreateInstance(PL_PERLCOMPONENTHELPER_CONTRACTID, &res);
    if(NS_FAILED(res)) {
      warn("Unable to obtain perl component helper.\n");
      VP_SET_RETCODE(res);
      VP_RETURN_UNDEF;
    }
    res = helper->WrapObject(obj, iid, (void**)&retobj);
    if(NS_FAILED(res)) {
      warn("Unable to wrap object.\n");
      VP_SET_RETCODE(res);
      VP_RETURN_UNDEF;
    }
    ST(0) = perlXPUtil::makeXPObject(nsnull, &iid, retobj);
    NS_RELEASE(retobj);
    VP_SET_RETCODE(NS_OK);
    XSRETURN(1);
    
nsID *
ID(...)
  INIT:
    const char *ids = nsnull;
  CODE:
    RETVAL = new nsID;
    if(items == 1) {
      ids = (const char *)SvPV(ST(0), PL_na);
      RETVAL->Parse(ids);
    }
  OUTPUT:
    RETVAL
    
MODULE = XPCOM    PACKAGE = rawISupports

void
rawISupports::DESTROY()
  INIT:
    nsISupports *t = (nsISupports *)SvIV((SV*)SvRV(ST(0)));
  CODE:
    NS_IF_RELEASE(t);

  
MODULE = XPCOM    PACKAGE = nsID

void
nsID::DESTROY()
  CODE:
    delete THIS;

int
nsID::equals(...)
  INIT:
    nsIID iid;
    nsresult res = NS_OK;
  CODE:
    res = perlXPUtil::fetchIIDFromSV(ST(1), iid);
    if(NS_FAILED(res)) {
      VP_SET_RETCODE(res);
      VP_RETURN_UNDEF;
    }
    RETVAL = (THIS->Equals(iid))?1:0;
  OUTPUT:
    RETVAL

int
nsID::parse(c)
  const char *c
  CODE:
    RETVAL = (THIS->Parse(c))?1:0;
  OUTPUT:
    RETVAL
    
void
nsID::toString()
  INIT:
    char *buf = nsnull;
  CODE:
    buf = THIS->ToString();
    ST(0) = sv_newmortal();
    sv_setpv(ST(0), buf);
    nsMemory::Free(buf);
    XSRETURN(1);
    
void
nsID::LookupIFaceConstant(symbol)
  const char *symbol
  INIT:
    nsIInterfaceInfoManager *iim = XPTI_GetInterfaceInfoManager();
    nsIInterfaceInfo *theIFace = nsnull;
    const nsXPTConstant *constPtr = nsnull;
    PRBool result = PR_FALSE;
  CODE:
    if(NS_FAILED(iim->GetInfoForIID(THIS, & theIFace))) {
      NS_IF_RELEASE(iim);
      VP_RETURN_UNDEF;
    }
    if(perlXPUtil::lookupConstant(theIFace, symbol, &constPtr)) {
      ST(0) = perlXPUtil::constantToSV(constPtr, &result);
      NS_IF_RELEASE(iim);
      NS_IF_RELEASE(theIFace);
      if(result) {
        XSRETURN(1);
      } else {
        VP_RETURN_UNDEF;
      }
    }
    VP_RETURN_UNDEF;
  
MODULE = XPCOM    PACKAGE = Components::results

PRUint32
NS_OK()
  CODE:
    RETVAL = NS_OK;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_BASE()
  CODE:
    RETVAL = NS_ERROR_BASE;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_NOT_INITIALIZED()
  CODE:
    RETVAL = NS_ERROR_NOT_INITIALIZED;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_ALREADY_INITIALIZED()
  CODE:
    RETVAL = NS_ERROR_ALREADY_INITIALIZED;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_NOT_IMPLEMENTED()
  CODE:
    RETVAL = NS_ERROR_NOT_IMPLEMENTED;
  OUTPUT:
    RETVAL

PRUint32
NS_NOINTERFACE()
  CODE:
    RETVAL = NS_NOINTERFACE;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_NO_INTERFACE()
  CODE:
    RETVAL = NS_ERROR_NO_INTERFACE;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_INVALID_POINTER()
  CODE:
    RETVAL = NS_ERROR_INVALID_POINTER;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_NULL_POINTER()
  CODE:
    RETVAL = NS_ERROR_NULL_POINTER;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_ABORT()
  CODE:
    RETVAL = NS_ERROR_ABORT;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_FAILURE()
  CODE:
    RETVAL = NS_ERROR_FAILURE;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_UNEXPECTED()
  CODE:
    RETVAL = NS_ERROR_UNEXPECTED;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_OUT_OF_MEMORY()
  CODE:
    RETVAL = NS_ERROR_OUT_OF_MEMORY;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_ILLEGAL_VALUE()
  CODE:
    RETVAL = NS_ERROR_ILLEGAL_VALUE;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_INVALID_ARG()
  CODE:
    RETVAL = NS_ERROR_INVALID_ARG;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_NO_AGGREGATION()
  CODE:
    RETVAL = NS_ERROR_NO_AGGREGATION;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_NOT_AVAILABLE()
  CODE:
    RETVAL = NS_ERROR_NOT_AVAILABLE;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_FACTORY_NOT_REGISTERED()
  CODE:
    RETVAL = NS_ERROR_FACTORY_NOT_REGISTERED;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_FACTORY_REGISTER_AGAIN()
  CODE:
    RETVAL = NS_ERROR_FACTORY_REGISTER_AGAIN;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_FACTORY_NOT_LOADED()
  CODE:
    RETVAL = NS_ERROR_FACTORY_NOT_LOADED;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_FACTORY_NO_SIGNATURE_SUPPORT()
  CODE:
    RETVAL = NS_ERROR_FACTORY_NO_SIGNATURE_SUPPORT;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_FACTORY_EXISTS()
  CODE:
    RETVAL = NS_ERROR_FACTORY_EXISTS;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_PROXY_INVALID_IN_PARAMETER()
  CODE:
    RETVAL = NS_ERROR_PROXY_INVALID_IN_PARAMETER;
  OUTPUT:
    RETVAL

PRUint32
NS_ERROR_PROXY_INVALID_OUT_PARAMETER()
  CODE:
    RETVAL = NS_ERROR_PROXY_INVALID_OUT_PARAMETER;
  OUTPUT:
    RETVAL

INCLUDE: nsISupports.xs
