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
 * File: plPerlXPModule.cpp
 * Created by: matt
 * Created on: Sat Mar 17 01:46:22 2001
 * $Id: plPerlXPModule.cpp,v 1.2 2004/10/28 16:13:55 poumeyrol Exp $
 *****************************************************************************/

#include "nsIServiceManager.h"
#include "nsICategoryManager.h"
#include "nsIGenericFactory.h"
#include "plPerlXPCOM_Base.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(plPerlComponentLoader)
NS_DECL_CLASSINFO(plPerlComponentLoader)

NS_GENERIC_FACTORY_CONSTRUCTOR(plPerlModule)
NS_DECL_CLASSINFO(plPerlModule)

NS_GENERIC_FACTORY_CONSTRUCTOR(plPerlFactory)
NS_DECL_CLASSINFO(plPerlFactory)
 
NS_GENERIC_FACTORY_CONSTRUCTOR(plPerlComponentHelper)
NS_DECL_CLASSINFO(plPerlComponentHelper)

static NS_METHOD plComponentLoaderRegProc(nsIComponentManager *aCompMgr,
                                            nsIFile *aPath,
                                            const char *registryLocation,
                                            const char *componentType,
                                            const nsModuleComponentInfo *info) {
  nsresult res;
  nsCOMPtr<nsIComponentLoader> o = nsnull;
  nsCOMPtr<nsICategoryManager> cm = nsnull;

//  NS_DEFINE_CID(t_cmcid, NS_CATEGORYMANAGER_CID);
  NS_DEFINE_CID(t_cid, NS_PLCOMPONENTLOADER_CID);
  
  res = VP_NewPerlComponentLoader(getter_AddRefs(o));
  if(NS_FAILED(res)) {
    return res;
  }
  res = nsServiceManager::RegisterService(t_cid, o);
/*
  res = nsServiceManager::GetService(t_cmcid,
                                     NS_GET_IID(nsICategoryManager),
                                     (nsISupports **)getter_AddRefs(cm));
*/
  cm = do_GetService(NS_CATEGORYMANAGER_CONTRACTID, &res);
  
  if(NS_FAILED(res)) {
    return res;
  }
  return cm->AddCategoryEntry("component-loader",
                              perlComponentType,
                              NS_PLCOMPONENTLOADER_CONTRACTID,
                              PR_TRUE,
                              PR_TRUE,
                              0);
                                     
}

static NS_METHOD plComponentLoaderUnreg(nsIComponentManager *aCompMgr,
                                          nsIFile *aPath,
                                          const char *registryLocation,
                                          const nsModuleComponentInfo *info) {
  NS_DEFINE_CID(t_cid, NS_PLCOMPONENTLOADER_CID);
//  NS_DEFINE_CID(t_cmid, NS_CATEGORYMANAGER_CID);
  nsresult res;
  nsCOMPtr<nsICategoryManager> o = nsnull;
/*
  res = nsServiceManager::GetService(t_cmid,
                                     NS_GET_IID(nsICategoryManager),
                                     (nsISupports **)getter_AddRefs(o));
*/
  o = do_GetService(NS_CATEGORYMANAGER_CONTRACTID, &res);

  if(NS_SUCCEEDED(res)) {
    o->DeleteCategoryEntry("component-loader",
                           perlComponentType,
                           PR_TRUE);
  }
  return nsServiceManager::UnregisterService(t_cid);
}

static nsModuleComponentInfo components[] =
{
  { "Perl Component Loader",                            // Descriptive name
    NS_PLCOMPONENTLOADER_CID,                           // The class ID
    NS_PLCOMPONENTLOADER_CONTRACTID,                    // The contract ID
    plPerlComponentLoaderConstructor,                   // Factory constructor
    plComponentLoaderRegProc,                         // Registration proc
    plComponentLoaderUnreg,                           // Unregistration proc
    NULL,                                               // Factory destructor
    NS_CI_INTERFACE_GETTER_NAME(plPerlComponentLoader), // ??
    NULL,                                               // Language helper
    &NS_CLASSINFO_NAME(plPerlComponentLoader)           // Class info service
  },
  { "Perl Module",
    NS_PERLMODULE_CID,
    NS_PERLMODULE_CONTRACTID,
    plPerlModuleConstructor,
    NULL,
    NULL,
    NULL,
    NS_CI_INTERFACE_GETTER_NAME(plPerlModule),
    NULL,
    &NS_CLASSINFO_NAME(plPerlModule)
  },
  { "Perl Factory",
    NS_PERLFACTORY_CID,
    NS_PERLFACTORY_CONTRACTID,
    plPerlFactoryConstructor,
    NULL,
    NULL,
    NULL,
    NS_CI_INTERFACE_GETTER_NAME(plPerlFactory),
    NULL,
    &NS_CLASSINFO_NAME(plPerlFactory)
  },
  { "Perl Component Helper",
    PL_PERLCOMPONENTHELPER_CID,
    PL_PERLCOMPONENTHELPER_CONTRACTID,
    plPerlComponentHelperConstructor,
    NULL,
    NULL,
    NULL,
    NS_CI_INTERFACE_GETTER_NAME(plPerlComponentHelper),
    NULL,
    &NS_CLASSINFO_NAME(plPerlComponentHelper)
  }
};

NS_IMPL_NSGETMODULE(PerlXPCOM, components)
