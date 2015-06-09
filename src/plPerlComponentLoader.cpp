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

/*
 * File: plPerlComponentLoader.cpp
 * Created by: matt
 * Created on: Thu 31 May 2001 06:22:11 PM EDT
 * $Id: plPerlComponentLoader.cpp,v 1.5 2004/10/28 16:13:55 poumeyrol Exp $
 *
 * This file has been adapted from nsNativeComponentLoader.cpp
 */
 
//#define XP_PERL_DEBUG 1
 
#include "nsXPIDLString.h"
#include "nsReadableUtils.h"
#include "plPerlXPCOM_Base.h"
#include "nsILocalFile.h"
#include "nsIObserverService.h"
 
// XXX - This is a lamer fix. The API for the observer service apparently 
// froze last week. This is a kludge till I everyone is on branches with the 
// frozen API
#ifndef NS_OBSERVERSERVICE_CONTRACTID
#define NS_OBSERVERSERVICE_CONTRACTID "@mozilla.org/observer-service;1"
#endif
 
#include "prio.h"
#include "prlog.h"
#include "prprf.h"
#include "nsDebug.h"

#ifdef XP_PERL_DEBUG
#include <stdio.h> // This needs to go.
#endif

const char perlComponentType[] = "text/x-perl";

#ifdef HAVE_LOCAL_PERL
#ifdef pTHXo
EXTERN_C void xs_init (pTHXo);
#else
EXTERN_C void xs_init _((void));
#endif

PerlInterpreter *gPerlInterp;


void VP_ShutdownPerl(void) {
  if(gPerlInterp) {
    perl_destruct(gPerlInterp);
    perl_free(gPerlInterp);
    gPerlInterp = nsnull;
  }
}
#endif

static plPerlComponentLoader *g_plPerlComponentLoader = nsnull;
NS_COM nsresult 
VP_NewPerlComponentLoader(nsIComponentLoader **aComponentLoader) {
  return plPerlComponentLoader::GetLoader(aComponentLoader);
}

NS_IMPL_ISUPPORTS1_CI(plPerlComponentLoader, nsIComponentLoader) 

nsresult 
plPerlComponentLoader::GetLoader(nsIComponentLoader **aComponentLoader) {
  if(!aComponentLoader) {
    return NS_ERROR_NULL_POINTER;
  }
  if(!g_plPerlComponentLoader) {
    plPerlComponentLoader *it = new plPerlComponentLoader();
    if(!it) {
      return NS_ERROR_OUT_OF_MEMORY;
    }
    g_plPerlComponentLoader = it;
  }
  NS_ADDREF(g_plPerlComponentLoader);
  *aComponentLoader = g_plPerlComponentLoader;
  return NS_OK;
}

struct freePerlLibsClosure {
  nsIServiceManager *serviceMgr;
  PRInt32 when;
};

static PRBool PR_CALLBACK
nsPMod_Destroy(nsHashKey *aKey, void *aData, void *closure) {
  plPMFile *entry = NS_STATIC_CAST(plPMFile *, aData);
  delete entry;
  return PR_TRUE;
}


// Dummy right now. Currently, once a package is evaled into the interpreter's 
// namespace, there's no going back till the interpreter is shutdown, and that 
// ain't our job.
static nsresult PR_CALLBACK
nsFreePerlLib(plPMFile *dll, nsIServiceManager *serviceMgr, PRInt32 when) {
  return NS_OK;
}

static PRBool PR_CALLBACK
nsFreePerlLibEnum(nsHashKey *aKey, void *aData, void *closure) {
  plPMFile *dll = (plPMFile *)aData;
  struct freePerlLibsClosure *callData = (struct freePerlLibsClosure *)closure;
  nsFreePerlLib(dll,
                (callData ? callData->serviceMgr : NULL),
                0);
  return PR_TRUE;
}
  
plPerlComponentLoader::plPerlComponentLoader() :
/*  m_Registry(nsnull), */ m_CompMgr(nsnull), m_PmStore(nsnull), m_Modules(nsnull) {
  NS_INIT_ISUPPORTS();
#ifdef HAVE_LOCAL_PERL
#ifdef VIPER_MODULE_PATH
  static char *dummy_argv[] = { "", "-I"VIPER_MODULE_PATH, "-e", "0" };
#else
  static char *dummy_argv[] = { "", "-e", "0" };
#endif
  // Check to see if someone has set gPerlInterp. If not check to see if 
  // a perl interpreter has been created at all.
  if(!gPerlInterp) {
    gPerlInterp = PERL_GET_INTERP;
  }
  // If the Perl interpreter isn't running, start it.
  if(!gPerlInterp) {
    gPerlInterp = perl_alloc();
    perl_construct(gPerlInterp);
    if(perl_parse(gPerlInterp, xs_init, 3, dummy_argv, NULL) != 0) {
      NS_ABORT();
    }
    atexit(VP_ShutdownPerl);
  }
#endif
}

plPerlComponentLoader::~plPerlComponentLoader() {
//  m_Registry = nsnull;
  NS_IF_RELEASE(m_CompMgr);
  m_CompMgr = nsnull;

  delete m_PmStore;
  delete m_Modules;
}

NS_IMETHODIMP
plPerlComponentLoader::GetFactory(const nsIID& aCID,
                                  const char *aLocation,
                                  const char *aType,
                                  nsIFactory **_retval) {
  nsresult rv;
  if(!_retval) {
    return NS_ERROR_NULL_POINTER;
  }
#ifdef XP_PERL_DEBUG
  printf("plPerlComponentLoader::GetFactory(): Location: %s Type: %s\n", 
         aLocation, aType);
#endif
  plPMFile *dll;
  PRInt64 mod = LL_Zero(), size = LL_Zero();
#ifdef XP_PERL_DEBUG
  printf("GetFactory: Calling CreateModule()\n");
#endif
  rv = CreateModule(nsnull, aLocation, &mod, &size, &dll);
  if(NS_FAILED(rv)) return rv;

  if(!dll) return NS_ERROR_OUT_OF_MEMORY;

  nsIServiceManager *serviceMgr = NULL;
  rv = nsServiceManager::GetGlobalServiceManager(&serviceMgr);
  if(NS_FAILED(rv)) return rv;

  nsCOMPtr<nsIModule> module;
  rv = GetModule(m_CompMgr, dll, getter_AddRefs(module));
  if(NS_FAILED(rv)) return rv;

  return module->GetClassObject(m_CompMgr, aCID, NS_GET_IID(nsIFactory),
                                (void **)_retval);
}

NS_IMETHODIMP
plPerlComponentLoader::Init(nsIComponentManager *aCompMgr, nsISupports *aReg) {
//  nsresult rv;

#ifdef XP_PERL_DEBUG
  warn("In plPerlComponentLoader::Init()\n");
#endif
  m_CompMgr = aCompMgr;
  NS_ADDREF(m_CompMgr);
  
  if(!m_PmStore) {
    m_PmStore = new nsObjectHashtable(nsnull, nsnull, nsPMod_Destroy, nsnull,
                                      256, PR_TRUE);
    if(!m_PmStore) {
      return NS_ERROR_OUT_OF_MEMORY;
    }
  }
  if(!m_Modules) {
    m_Modules = new nsSupportsHashtable(256, PR_TRUE);
    if(!m_Modules) {
      return NS_ERROR_OUT_OF_MEMORY;
    }
  }

#if 0
  m_Registry = do_QueryInterface(aReg);
  if(!m_CompMgr || !m_Registry) {
    return NS_ERROR_INVALID_ARG;
  }

  rv = m_Registry->GetSubtree(nsIRegistry::Common, xpcomComponentsKeyName,
                              &m_XPCOMKey);
  if(NS_FAILED(rv)) return rv;

  nsCOMPtr<nsIEnumerator> dllEnum;
  rv = m_Registry->EnumerateSubtrees(m_XPCOMKey, getter_AddRefs(dllEnum));
  if(NS_FAILED(rv)) return rv;

  rv = dllEnum->First();
  for(; NS_SUCCEEDED(rv) && (dllEnum->IsDone() != NS_OK); 
      (rv = dllEnum->Next())) {
    nsCOMPtr<nsISupports> base;
    rv = dllEnum->CurrentItem(getter_AddRefs(base));
    if(NS_FAILED(rv)) continue;

    nsIID nodeIID = NS_IREGISTRYNODE_IID;
    nsCOMPtr<nsIRegistryNode> node;
    rv = base->QueryInterface(nodeIID, getter_AddRefs(node));
    if(NS_FAILED(rv)) continue;
    
    nsXPIDLCString library;
    rv = node->GetNameUTF8(getter_Copies(library));

    if(NS_FAILED(rv)) continue;

    char *uLibrary = nsnull;
    char *eLibrary = (char *)library.operator const char*();
    PRUint32 length = PL_strlen(eLibrary);
    rv = m_Registry->UnescapeKey((PRUint8 *)eLibrary, 1, &length, 
                                 (PRUint8 **)&uLibrary);
    if(NS_FAILED(rv)) continue;

    if(uLibrary == nsnull) uLibrary = eLibrary;
    
    nsRegistryKey libKey;
    rv = node->GetKey(&libKey);
    if(NS_SUCCEEDED(rv)) {
      plPMFile *dll = NULL;
      PRInt64 lastModTime;
      PRInt64 fileSize;
      GetRegistryModuleInfo(libKey, &lastModTime, &fileSize);
#ifdef XP_PERL_DEBUG
      printf("Init: Calling CreateModule()\n");
#endif
      rv = CreateModule(NULL, uLibrary, &lastModTime, &fileSize, &dll);
    }
    if(uLibrary && (uLibrary != eLibrary)) {
      nsMemory::Free(uLibrary);
    }
    
    if(NS_FAILED(rv)) continue;
    
  }
#endif
  return NS_OK;
}

NS_IMETHODIMP 
plPerlComponentLoader::AutoRegisterComponents(PRInt32 aWhen,
                                              nsIFile *aDirectory) {
  nsresult rv = NS_ERROR_FAILURE;
  PRBool isDir = PR_FALSE;
  nsCOMPtr<nsISimpleEnumerator> dirIterator;
  rv = aDirectory->GetDirectoryEntries(getter_AddRefs(dirIterator));
  if(NS_FAILED(rv)) return rv;

  nsCOMPtr<nsIFile> dirEntry;
  PRBool more = PR_FALSE;

  rv = dirIterator->HasMoreElements(&more);
  if(NS_FAILED(rv)) return rv;
  while(more) {
    rv = dirIterator->GetNext((nsISupports**)getter_AddRefs(dirEntry));
    if(NS_SUCCEEDED(rv)) {
      rv = dirEntry->IsDirectory(&isDir);
      if(NS_SUCCEEDED(rv)) {
        if(isDir) {
          rv = AutoRegisterComponents(aWhen, dirEntry);
        } else {
          PRBool registered;
          rv = AutoRegisterComponent(aWhen, dirEntry, &registered);
        }
      }
    }
    rv = dirIterator->HasMoreElements(&more);
    if(NS_FAILED(rv)) return rv;
  }
  return rv;
}

NS_IMETHODIMP plPerlComponentLoader::AutoRegisterComponent(PRInt32 aWhen,
                                                           nsIFile *component,
                                                           PRBool *registered) {
  nsresult rv;
  if(!registered) {
    return NS_ERROR_NULL_POINTER;
  }

  const char *ValidPMExtensions[] = {
    ".pm",
    ".pl",
    nsnull
  };

  *registered = PR_FALSE;

  PRBool validExtension = PR_FALSE;
  nsString leafName;
  
  rv = component->GetLeafName(leafName);
  NS_ENSURE_SUCCESS(rv,rv);
  
  PRInt32 extlen; 
  PRInt32 leaflen = leafName.Length();
  
  for(PRInt32 i = 0;ValidPMExtensions[i] != nsnull; i++) {
    extlen = PL_strlen(ValidPMExtensions[i]);
    if(leafName.Find(ValidPMExtensions[i], PR_TRUE, (leaflen-extlen)) != -1) {
#ifdef XP_PERL_DEBUG
      warn("plPerlComponentLoader: Interested in %s\n", 
           NS_ConvertUCS2toUTF8(leafName).get());
#endif
      validExtension = PR_TRUE;
      break;
    }
  }
  if(!validExtension) return NS_OK;

  nsXPIDLCString persistentDescriptor;
  nsCOMPtr<nsIComponentManagerObsolete> omgr = do_QueryInterface(m_CompMgr, &rv);
  rv = omgr->RegistryLocationForSpec(component,
                                     getter_Copies(persistentDescriptor));
  if(NS_FAILED(rv)) return rv;

  nsCStringKey key(persistentDescriptor);

  plPMFile *dll = nsnull;
  PRInt64 mod = LL_Zero(), size = LL_Zero();
#ifdef XP_PERL_DEBUG
  printf("AutoregisterComponent: Calling CreateModule()\n");
#endif
  rv = CreateModule(component, persistentDescriptor, &mod, &size, &dll);
  if(NS_FAILED(rv)) return rv;

  if(dll != NULL) {
    // Crap module?
    if(dll->GetStatus() != NS_OK) {
      warn("plPerlComponentLoader: Module '%s' not ok. Skipping...\n",
            dll->GetDisplayPath());
      return NS_ERROR_FAILURE;
    }
    // If it's unchanged, we don't care.
    if(!dll->HasChanged()) {
      warn("plPerlComponentLoader: Module '%s' has not changed. Skipping...\n",
            dll->GetDisplayPath());
      return NS_OK;
    }
    // Changed? Reregister it.
    //NS_WITH_SERVICE(nsIObserverService, observerService, 
    //                NS_OBSERVERSERVICE_CONTRACTID, &rv);
    nsCOMPtr<nsIObserverService> observerService = 
            do_GetService(NS_OBSERVERSERVICE_CONTRACTID, &rv);
    if(NS_SUCCEEDED(rv)) {
      nsIServiceManager *mgr; // Service manager never gets released.
      rv = nsServiceManager::GetGlobalServiceManager(&mgr);
      if(NS_SUCCEEDED(rv)) {
        NS_ConvertASCIItoUCS2 statusMsg("Registering Perl component ");
        nsString fileName(NS_LITERAL_STRING("(no name)"));
        nsCOMPtr<nsIFile> dllSpec;
        if(NS_SUCCEEDED(dll->GetDllSpec(getter_AddRefs(dllSpec))) && dllSpec) {
          dllSpec->GetLeafName(fileName);
        }
        statusMsg.Append(fileName);
        observerService->NotifyObservers(
             mgr,
             NS_XPCOM_AUTOREGISTRATION_OBSERVER_ID,
             statusMsg.get());
      }
      // Maybe to check and reload of symbol table here? That'll be a bitch.
    }
  } else {
#ifdef XP_PERL_DEBUG
    printf("Creating plPMFile for %s\n", persistentDescriptor.get());
#endif
    dll = new plPMFile(NS_ConvertASCIItoUCS2(persistentDescriptor));
    if(dll == NULL) return NS_ERROR_OUT_OF_MEMORY;
#ifdef XP_PERL_DEBUG
    printf("Putting onto module store in AutoregisterComponent.\n");
#endif
    m_PmStore->Put(&key, (void *)dll);
  }
  
  nsresult res = SelfRegisterModule(dll, persistentDescriptor, PR_FALSE);
  if(NS_FAILED(res)) {
    if(res == NS_ERROR_FACTORY_REGISTER_AGAIN) {
      m_DeferredComponents.AppendElement(dll);
      return NS_OK;
    } else {
      warn("plPerlComponentLoader: Autoregistration FAILED for '%s'.\n",
            dll->GetDisplayPath());
      return NS_ERROR_FACTORY_NOT_REGISTERED;
    }
  } else {
    warn("plPerlComponentLoader: Autoregistration Passed for '%s'.\n",
          dll->GetDisplayPath());
  }
  return NS_OK;
}

NS_IMETHODIMP 
plPerlComponentLoader::RegisterDeferredComponents(PRInt32 aWhen,
                                                  PRBool *aRegistered) {
#ifdef XP_PERL_DEBUG
  warn("nPCL: registering deferred (%d)\n", m_DeferredComponents.Count());
#endif
  *aRegistered = PR_FALSE;
  if(!m_DeferredComponents.Count()) return NS_OK;

  for(int i = m_DeferredComponents.Count() -1;i >= 0; i--) {
    plPMFile *dll = NS_STATIC_CAST(plPMFile *, m_DeferredComponents[i]);
    nsresult rv = SelfRegisterModule(dll,
                                     dll->GetRegistryLocation(),
                                     PR_TRUE);
    if(rv != NS_ERROR_FACTORY_REGISTER_AGAIN) {
      if(NS_SUCCEEDED(rv)) {
        *aRegistered = PR_TRUE;
      }
      m_DeferredComponents.RemoveElementAt(i);
    }
  }
  if(*aRegistered) {
    warn("nPCL: registered deferred, %d left\n",
          m_DeferredComponents.Count());
  } else {
    warn("nPCL: didn't register any components, %d left\n",
          m_DeferredComponents.Count());
  }
  return NS_OK;
}

NS_IMETHODIMP plPerlComponentLoader::OnRegister(const nsIID &aCID, 
                                                const char *aType,
                                                const char *aClassName,
                                                const char *aContractID,
                                                const char *aLocation,
                                                PRBool aReplace,
                                                PRBool aPersist) {
  return NS_OK;
}

NS_IMETHODIMP plPerlComponentLoader::UnloadAll(PRInt32 aWhen) {
#ifdef XP_PERL_DEBUG
  warn("plPerlComponentLoader: Unloading....\n");
#endif
  struct freePerlLibsClosure callData;
  callData.serviceMgr = NULL;
  callData.when = aWhen;
  if(m_PmStore) {
    m_PmStore->Enumerate(nsFreePerlLibEnum, &callData);
  }
  return NS_OK;
}

NS_IMETHODIMP
plPerlComponentLoader::AutoUnregisterComponent(PRInt32 aWhen,
                                               nsIFile *component,
                                               PRBool *unregistered) {
  nsresult rv = NS_ERROR_FAILURE;
  nsXPIDLCString persistentDescriptor;
  nsCOMPtr<nsIComponentManagerObsolete> omgr = do_QueryInterface(m_CompMgr, &rv);
  rv = omgr->RegistryLocationForSpec(component,
                                     getter_Copies(persistentDescriptor));
  if(NS_FAILED(rv)) return rv;

//  NS_WITH_SERVICE(nsIObserverService, observerService, 
//                  NS_OBSERVERSERVICE_CONTRACTID, &rv);
  nsCOMPtr<nsIObserverService> observerService = 
          do_GetService(NS_OBSERVERSERVICE_CONTRACTID, &rv);
  if(NS_SUCCEEDED(rv)) {
    nsIServiceManager *mgr;
    rv = nsServiceManager::GetGlobalServiceManager(&mgr);
    if(NS_SUCCEEDED(rv)) {
      observerService->NotifyObservers(
            mgr,
            NS_XPCOM_AUTOREGISTRATION_OBSERVER_ID,
            NS_ConvertASCIItoUCS2("Unregistering Perl component").get());
    }
  }

  plPMFile *dll = NULL;
  PRInt64 mod = LL_Zero(), size = LL_Zero();
#ifdef XP_PERL_DEBUG
  printf("AutoUnregisterComponent: Calling CreateModule()\n");
#endif
  rv = CreateModule(component, persistentDescriptor, &mod, &size, &dll);
  if(NS_FAILED(rv) || dll == NULL) return rv;

  rv = SelfUnregisterModule(dll);

  if(NS_SUCCEEDED(rv)) {
    RemoveRegistryModuleInfo(persistentDescriptor);
  }
  warn("plPerlComponentLoader: AutoUnregistration for %s %s\n",
       dll->GetDisplayPath(), (NS_FAILED(rv)?"FAILED":"succeeded"));
  return rv;
}

NS_IMETHODIMP 
plPerlComponentLoader::SelfUnregisterModule(plPMFile *dll) {
  nsIServiceManager *serviceMgr = NULL;
  nsresult res = nsServiceManager::GetGlobalServiceManager(&serviceMgr);
  if(NS_FAILED(res)) return res;

  nsCOMPtr<nsIModule> mobj;
  res = GetModule(m_CompMgr, dll, getter_AddRefs(mobj));
  if(NS_SUCCEEDED(res)) {
    nsCOMPtr<nsIFile> fs;
    res = dll->GetDllSpec(getter_AddRefs(fs));
    if(NS_FAILED(res)) return res;
    nsXPIDLCString registryName;
    nsCOMPtr<nsIComponentManagerObsolete> omgr = do_QueryInterface(m_CompMgr, &res);
    res = omgr->RegistryLocationForSpec(fs, getter_Copies(registryName));
    if(NS_FAILED(res)) return res;
    return mobj->UnregisterSelf(m_CompMgr, fs, registryName);
  }
  return res;
}

NS_IMETHODIMP
plPerlComponentLoader::GetModule(nsIComponentManager *mgr, plPMFile *dll,
                                 nsIModule **mod) {
// XXX - Todo
  // Get package name from file name.
  nsCOMPtr<nsIFile> spec;
  nsresult rv;
  nsString modFileName;
  
#ifdef XP_PERL_DEBUG
  printf( "plPerlComponentLoader::GetModule()\n");
#endif
  if(!dll) {
    return NS_ERROR_FAILURE;
  }
#ifdef XP_PERL_DEBUG
  printf("GetModule(): Fetching from %s\n", dll->GetDisplayPath());
#endif
  rv = dll->GetDllSpec(getter_AddRefs(spec));
  if(NS_FAILED(rv)) {
    return NS_ERROR_FAILURE;
  }
  
  spec->GetLeafName(modFileName);
  if(modFileName.IsEmpty()) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  nsCString moduleName;
  moduleName.AssignWithConversion(modFileName);
  // Crude extension chop
  moduleName.Trim("lmp", PR_FALSE, PR_TRUE);
  moduleName.Trim(".", PR_FALSE, PR_TRUE);
  //moduleName.ReplaceSubstring(".pm", "");
  //moduleName.ReplaceSubstring(".pl", "");

  char *tmodName = ToNewCString(moduleName);
  // moduleKey owns the memory in tmodName and will free it.
  nsCStringKey moduleKey(tmodName, 
                         moduleName.Length(),
                         nsCStringKey::OWN);

  // Check if we already have the module in our hash table. If so, send it
  // back to the caller
  nsISupports *theModule = (nsISupports *)m_Modules->Get(&moduleKey);
  if(theModule) {
    rv = theModule->QueryInterface(NS_GET_IID(nsIModule),
                                   (void **)mod);
    NS_RELEASE(theModule); // nsSupportsHashTable AddRefs the object
    return rv;
  }
// if not eval the file, get the SV object, make the plPerlModule and hash it
// Slurp in the file and eval it to trap any errors.
  nsCOMPtr<nsILocalFile> theFile;
  char *code = nsnull;
  PRUint32 codeLen = 0;
  char buf[1024];
  PRFileDesc *fd = nsnull;
  PRFileInfo fdinfo;
  
  rv = spec->QueryInterface(NS_GET_IID(nsILocalFile), 
                           (void **)getter_AddRefs(theFile));
  if(NS_FAILED(rv)) {
    return NS_ERROR_FAILURE;
  }
  if(NS_FAILED(theFile->OpenNSPRFileDesc(PR_RDONLY, 00700, &fd))) {
    return NS_ERROR_FAILURE;
  }
  memset(buf, 0, 1024);
  if(PR_GetOpenFileInfo(fd, &fdinfo) == PR_SUCCESS) {
    codeLen = fdinfo.size;
    code = NS_STATIC_CAST(char*, nsMemory::Alloc(codeLen + 1));
    memset(code, 0, codeLen+1);
    if(PR_Read(fd, code, codeLen) <= 0) {
      PR_Close(fd);
      nsMemory::Free(code);
#ifdef XP_PERL_DEBUG
      printf("plPerlComponentLoader: Error reading from file.\n");
#endif
      return NS_ERROR_FAILURE;
    }
  } else {
    PR_Close(fd);
#ifdef XP_PERL_DEBUG
    printf("plPerlComponentLoader: Error obtaining file info.\n");
#endif
    return NS_ERROR_FAILURE;
  }
#if 0
  while(PR_Read(fd, buf, 1023) > 0) {
    if(buf) {
      //cerr << "DEBUG PERL: Reading: " << endl << buf << endl;
      code = PR_sprintf_append(code, "%s", buf);
      if(!code) {
        PR_Close(fd);
        return NS_ERROR_OUT_OF_MEMORY;
      }
      nsCRT::memset(buf, 0, 1024);
    }
  }
#endif
  PR_Close(fd);
  // Eval the buffered code into the interpreter. Block of code and the 
  // previous block are the equivalent of this in perl:
  // undef $/;
  // open(IN, "script");
  // $code = <IN>;
  // close(IN);
  // eval "$code";
#ifdef XP_PERL_DEBUG
  printf("Evaling component code for %s\n", moduleName.get());
#endif
  dSP;
  ENTER;
  SAVETMPS;
  PUSHMARK(sp);

  SV* sv = sv_newmortal();
  sv_setpv(sv, (char *)code);
  PUTBACK;
  int result = perl_eval_sv(sv, G_DISCARD|G_KEEPERR);

  SPAGAIN;
  if(SvTRUE(ERRSV)) {
    warn("Eval Error: %s\n", SvPV(ERRSV, PL_na));
    SvREFCNT_dec(sv);
    FREETMPS;
    LEAVE;
    nsMemory::Free(code);
    return NS_ERROR_ILLEGAL_VALUE;
  }
  FREETMPS;
  LEAVE;

  nsMemory::Free(code);
  // The component script must contain a package of the same name as the file
  // and that package must have a sub named GetModule() which returns the 
  // module instance.
  moduleName.Append("::GetModule");
  nsIID cmiid = NS_GET_IID(nsIComponentManager);
  nsIID fsiid = NS_GET_IID(nsIFile);
  
  // Start the scope of the GetModule() call
  // Already declared dSP
  ENTER;
  SAVETMPS;
  PUSHMARK(sp);
  // The SV's returned by perlXPUtil::makeXPObject are mortal
  XPUSHs(perlXPUtil::makeXPObject(nsnull, &cmiid, mgr));
  XPUSHs(perlXPUtil::makeXPObject(nsnull, &fsiid, spec));
  
  PUTBACK;
#ifdef XP_PERL_DEBUG
  printf("Calling %s\n", moduleName.get());
#endif
  // Look for our function and do *not* create it.
  CV *getModuleCV = perl_get_cv((char *)moduleName.get(), 0);
  if(!getModuleCV) {
    warn("%s is not a valid perl XPCOM component. %s not found.\n",
          dll->GetDisplayPath(), moduleName.get());
    SPAGAIN;
    FREETMPS;
    LEAVE;
    return NS_ERROR_FAILURE;
  }
  // Set the eval flag so we don't kill ourselves if perl croaks.
  result = perl_call_sv((SV*)getModuleCV, G_SCALAR|G_EVAL);
  
  SPAGAIN;
  if(!result) {
    warn("No module returned from %s\n", moduleName.get());
    FREETMPS;
    LEAVE;
    return NS_ERROR_FAILURE;
  }

  SV *theSV = POPs; // This should be our blessed module object
  if(!SvROK(theSV)) {
    warn("return value of %s is not a blessed reference.\n", moduleName.get());
    FREETMPS;
    LEAVE;
    sv_2mortal(theSV);
    return NS_ERROR_FAILURE;
  }
  // Make sure this stays alive. We don't want perl to take care of it 
  // anymore.
  SvREFCNT_inc(theSV);
  plPerlModule *newModule = new plPerlModule(theSV);
  if(!newModule) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  
  nsIModule *modPtr;
  rv = newModule->QueryInterface(NS_GET_IID(nsIModule),
                                 (void **)&modPtr);
  if(NS_SUCCEEDED(rv)) {
    m_Modules->Put(&moduleKey, modPtr);
    *mod = modPtr;
  }
  return rv;
}

NS_IMETHODIMP 
plPerlComponentLoader::SelfRegisterModule(plPMFile *dll,
                                          const char *registryLocation,
                                          PRBool deferred) {
  //PR_ASSERT(deferred);

  nsIServiceManager *serviceMgr = NULL;
  nsresult res = nsServiceManager::GetGlobalServiceManager(&serviceMgr);
  if(NS_FAILED(res)) return res;

  nsCOMPtr<nsIModule> mobj;
  res = GetModule(m_CompMgr, dll, getter_AddRefs(mobj));
  if(NS_SUCCEEDED(res)) {
    nsCOMPtr<nsIFile> fs;
    // XXX - Using a different res var after reading warning about code 
    // generation bugs in the nsNativeComponentLoader (from which this code 
    // comes nearly verbatim) source. Taking the warning as a 'There Be 
    // Dragons Here' kinda thing and preferring to err on the side of 
    // paranoia.
    nsresult res2 = dll->GetDllSpec(getter_AddRefs(fs));
    if(NS_SUCCEEDED(res2)) {
      res = mobj->RegisterSelf(m_CompMgr, fs, registryLocation,
                               perlComponentType);
    } else {
      res = res2;
      warn("plPerlComponentLoader: dll->GetDllSpec() on %s FAILED.\n", 
            dll->GetDisplayPath());
    }
    mobj = NULL;
  }
  if(res != NS_ERROR_FACTORY_REGISTER_AGAIN) {
    dll->Sync();
    PRInt64 modTime, size;
    dll->GetLastModifiedTime(&modTime);
    dll->GetSize(&size);
    SetRegistryModuleInfo(registryLocation, &modTime, &size);
  }
  return res;
}

NS_IMETHODIMP 
plPerlComponentLoader::SetRegistryModuleInfo(const char *aLocation,
                                             PRInt64 *lastModifiedTime,
                                             PRInt64 *fileSize) {
#if 0
  nsresult rv;
  PRUint32 length = PL_strlen(aLocation);
  char *eLocation;
  rv = m_Registry->EscapeKey((PRUint8*)aLocation, 1, &length, 
                             (PRUint8**)&eLocation);
  if(NS_FAILED(rv)) return rv;
  
  if(eLocation == nsnull) {
    eLocation = (char *)aLocation;
  }

  nsRegistryKey key;
  rv = m_Registry->GetSubtreeRaw(m_XPCOMKey, eLocation, &key);
  if(NS_FAILED(rv)) return rv;

  rv = m_Registry->SetLongLong(key, lastModValueName, lastModifiedTime);
  if(NS_FAILED(rv)) return rv;

  rv = m_Registry->SetLongLong(key, fileSizeValueName, fileSize);
  if(aLocation != eLocation) {
    nsMemory::Free(eLocation);
  }
  return rv;
#else
  return NS_OK;
#endif  
}

NS_IMETHODIMP
plPerlComponentLoader::CreateModule(nsIFile *aSpec,
                                    const char *aLocation,
                                    PRInt64 *modifiedTime,
                                    PRInt64 *fileSize,
                                    plPMFile **aDll) {
  plPMFile *dll;
  nsCOMPtr<nsIFile> dllSpec;
  nsCOMPtr<nsIFile> spec;
  nsresult rv = NS_OK;
  
#ifdef XP_PERL_DEBUG
  printf("plPerlComponentLoader::CreateModule()\n");
  if(!aSpec) {
    printf("CreateModule() with no spec file\n");
  }
  if(!aLocation) {
    printf("CreateModule() with no location.\n");
  } else {
    printf("Creating module %s\n", aLocation);
  }
#endif
  nsCStringKey key(aLocation);
  dll = (plPMFile *)m_PmStore->Get(&key);
  if(dll) {
    *aDll = dll;
    return NS_OK;
  }
  
  if(!aSpec) {
      nsCOMPtr<nsIComponentManagerObsolete> omgr = 
        do_QueryInterface(m_CompMgr, &rv);
      rv = omgr->SpecForRegistryLocation(aLocation,
                                         getter_AddRefs(spec));
      if(NS_FAILED(rv)) return rv;
  } else {
    spec = aSpec;
  }

  if(!dll) {
    //PR_ASSERT(modifiedTime != NULL);
    //PR_ASSERT(fileSize != NULL);
    PRInt64 zit = LL_Zero();
    if(LL_EQ(*modifiedTime, zit) && LL_EQ(*fileSize,zit)) {
      rv = GetRegistryModuleInfo(aLocation, modifiedTime, fileSize);
    }
    dll = new plPMFile(spec, aLocation, modifiedTime, fileSize);
  }
  if(!dll) return NS_ERROR_OUT_OF_MEMORY;
  *aDll = dll;
  m_PmStore->Put(&key, dll);
  return NS_OK;
}

NS_IMETHODIMP 
plPerlComponentLoader::GetRegistryModuleInfo(const char *aLocation,
                                             PRInt64 *lastModifiedTime,
                                             PRInt64 *fileSize) {
#if 0
  nsresult rv;
  PRUint32 length = PL_strlen(aLocation);
  char *eLocation;
  rv = m_Registry->EscapeKey((PRUint8 *)aLocation, 1, &length, 
                             (PRUint8 **)&eLocation);
  if(rv != NS_OK) {
    return rv;
  }
  if(eLocation == nsnull) {
    eLocation = (char *)aLocation;
  }
  nsRegistryKey key;
  rv = m_Registry->GetSubtreeRaw(m_XPCOMKey, eLocation, &key);
  if(NS_FAILED(rv)) return rv;

  rv = GetRegistryModuleInfo(key, lastModifiedTime, fileSize);
  if(aLocation != eLocation) {
    nsMemory::Free(eLocation);
  }
  return rv;
#else
  return NS_OK;
#endif
}

NS_IMETHODIMP
plPerlComponentLoader::GetRegistryModuleInfo(nsRegistryKey key,
                                             PRInt64 *lastModifiedTime,
                                             PRInt64 *fileSize) {
#if 0
  nsresult rv;
  PRInt64 lastMod;
  nsresult rv = m_Registry->GetLongLong(key, lastModValueName, &lastMod);
  if(NS_FAILED(rv)) return rv;
  *lastModifiedTime = lastMod;

  PRInt64 fsize;
  rv = m_Registry->GetLongLong(key, fileSizeValueName, &fsize);
  if(NS_FAILED(rv)) return rv;
  *fileSize = fsize;
#endif
  return NS_OK;
}

NS_IMPL_ISUPPORTS1_CI(plPerlComponentHelper, plIPerlComponentHelper)

plPerlComponentHelper::plPerlComponentHelper() {
  NS_INIT_ISUPPORTS();
}

plPerlComponentHelper::~plPerlComponentHelper() {}
  
NS_IMETHODIMP
plPerlComponentHelper::WrapObject(void *object, 
                                  const nsIID& aIID,
                                  void **result) {
  if(!object || !result) {
    return NS_ERROR_NULL_POINTER;
  }
  return VP_ConvertSVToISupports((SV*)object, 
                                 (nsISupports **)result,
                                 &aIID);
}
