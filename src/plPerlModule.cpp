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
 * File: plPerlModule.cpp
 * Created by: matt
 * Created on: Mon 04 Jun 2001 07:41:59 PM EDT
 * $Id: plPerlModule.cpp,v 1.2 2005/03/31 08:29:00 poumeyrol Exp $
 *****************************************************************************/

#include "plPerlXPCOM_Base.h"

NS_IMPL_ISUPPORTS1_CI(plPerlModule, nsIModule)

plPerlModule::plPerlModule() {
  NS_INIT_ISUPPORTS();
}

plPerlModule::plPerlModule(SV *perlObject) 
  : plPerlBase(perlObject, NS_GET_IID(nsIModule)) {
  NS_INIT_ISUPPORTS();
}

NS_IMETHODIMP plPerlModule::GetClassObject(nsIComponentManager *aCompMgr,
                                           const nsCID &aClass,
                                           const nsIID &aIID,
                                           void **result) {
  if(!m_perlObject) {
    return NS_ERROR_FAILURE;
  }
  nsresult res;
  SV *cmsv = NULL;
  SV *retsv = NULL;
  nsIID cmiid = NS_GET_IID(nsIComponentManager);
  nsISupports *retval = nsnull;

  nsXPTCVariant inputs[3];
  nsXPTCVariant outputs[1];

  // Prep component manager arg
  res = perlXPUtil::ConvertISupportsToSV(aCompMgr, &cmiid, nsnull, &cmsv);
  if(NS_FAILED(res)) {
    warn("Warning: plPerlModule: No component manager to pass to perl.\n");
    cmsv = perlXPUtil::MakeUndef();
  }
  inputs[0].val.p = (void *)cmsv;
  inputs[0].ptr = inputs[0].val.p;
  inputs[0].type = nsXPTType::T_INTERFACE;
  inputs[0].flags = nsXPTCVariant::PTR_IS_DATA;
  
  // Prep nsCID arg
  inputs[1].val.p = (void *)&aClass;
  inputs[1].ptr = inputs[1].val.p;
  inputs[1].type = nsXPTType::T_IID;
  inputs[1].flags = nsXPTCVariant::PTR_IS_DATA;

  // Prep nsIID arg
  inputs[2].val.p = (void *)&aIID;
  inputs[2].ptr = inputs[2].val.p;
  inputs[2].type = nsXPTType::T_IID;
  inputs[2].flags = nsXPTCVariant::PTR_IS_DATA;

  // Prep return val. This will get put into result.
  outputs[0].val.p = nsnull;
  outputs[0].ptr = outputs[0].val.p;
  outputs[0].type = nsXPTType::T_VOID;

#ifdef XP_PERL_DEBUG
  warn("Calling getClassObject in plPerlModule\n");
#endif
  res = plPerlBase::CallPerlMethod(m_perlObject, "getClassObject",
                                   inputs, 3,
                                   outputs, 1);
#ifdef XP_PERL_DEBUG
  warn("PerlMethod call complete.\n");
#endif
  if(NS_FAILED(res)) {
    return res;
  }
  retsv = (SV*)outputs[0].val.p;
  if(retsv) {
#ifdef XP_PERL_DEBUG
    warn("Creating perl factory\n");
#endif
    retval = new plPerlFactory(retsv);
    //VP_ConvertSVToISupports(retsv, &retval, &aIID);
    if(retval) {
#ifdef XP_PERL_DEBUG
      warn("Factory made\n");
#endif
      NS_ADDREF(retval);
      *result = retval;
    }
    SvREFCNT_dec(retsv); // Because the new container takes ownership
#ifdef XP_PERL_DEBUG
    warn("Returning\n");
#endif
    return NS_OK;
    //*result = (void *)retval;
    //return NS_OK;
  } else {
    // If we got nothing in return the module should have returned undef 
    // and set the return code so grab it and return it.
#ifdef XP_PERL_DEBUG
    warn("Eep! No object\n");
#endif
    VP_GET_RETCODE(res);
    return res;
  }
  return NS_ERROR_FAILURE;
};

NS_IMETHODIMP plPerlModule::RegisterSelf(nsIComponentManager *aCompMgr,
                                         nsIFile *location,
                                         const char *registryLocation,
                                         const char *componentType) {
  if(!m_perlObject) {
    return NS_ERROR_FAILURE;
  }
  nsresult res = 0;
  SV *cmsv = NULL;
  nsXPTCVariant inputs[4];
  nsIID cmiid = NS_GET_IID(nsIComponentManager);
  nsIID fiid = NS_GET_IID(nsIFile);

  if(NS_FAILED(perlXPUtil::ConvertISupportsToSV(aCompMgr, &cmiid, nsnull, &cmsv))) {
    warn("plPerlModule Warning: No component manager given for RegisterSelf\n");
  }
  inputs[0].val.p = (void *)cmsv;
  inputs[0].ptr = inputs[0].val.p;
  inputs[0].type = nsXPTType::T_INTERFACE;
  inputs[0].flags = nsXPTCVariant::PTR_IS_DATA;
  
  if(NS_FAILED(perlXPUtil::ConvertISupportsToSV(location, &fiid, nsnull, &cmsv))) {
    warn("plPerlModule Warning: No location given for RegisterSelf\n");
  }
  inputs[1].val.p = (void *)cmsv;
  inputs[1].ptr = inputs[1].val.p;
  inputs[1].type = nsXPTType::T_INTERFACE;
  inputs[1].flags = nsXPTCVariant::PTR_IS_DATA;
  
  inputs[2].val.p = (void *)registryLocation;
  inputs[2].ptr = inputs[2].val.p;
  inputs[2].type = nsXPTType::T_CHAR_STR;

  inputs[3].val.p = (void *)componentType;
  inputs[3].ptr = inputs[3].val.p;
  inputs[3].type = nsXPTType::T_CHAR_STR;

#ifdef XP_PERL_DEBUG
  warn("Calling registerSelf in plPerlModule\n");
#endif
  res = plPerlBase::CallPerlMethod(m_perlObject, "registerSelf",
                                   inputs, 4);
  if(NS_FAILED(res)) {
    return res;
  }
  VP_GET_RETCODE(res);
  return res;
}

NS_IMETHODIMP plPerlModule::UnregisterSelf(nsIComponentManager *aCompMgr,
                                           nsIFile *location,
                                           const char *registryLocation) {
  if(!m_perlObject) { return NS_ERROR_FAILURE; }
  nsresult res = 0;
  SV *cmsv = NULL;
  nsXPTCVariant inputs[3];
  nsIID cmiid = NS_GET_IID(nsIComponentManager);
  nsIID fiid = NS_GET_IID(nsIFile);

  if(NS_FAILED(perlXPUtil::ConvertISupportsToSV(aCompMgr, &cmiid, nsnull, &cmsv))) {
    warn("plPerlModule Warning: No component manager given unregisterSelf\n");
  }
  inputs[0].val.p = (void *)cmsv;
  inputs[0].ptr = inputs[0].val.p;
  inputs[0].type = nsXPTType::T_INTERFACE;
  inputs[0].flags = nsXPTCVariant::PTR_IS_DATA;
  
  if(NS_FAILED(perlXPUtil::ConvertISupportsToSV(location, &fiid, nsnull, &cmsv))) {
    warn("plPerlModule Warning: No location given for unregisterSelf\n");
  }
  inputs[1].val.p = (void *)cmsv;
  inputs[1].ptr = inputs[1].val.p;
  inputs[1].type = nsXPTType::T_INTERFACE;
  inputs[1].flags = nsXPTCVariant::PTR_IS_DATA;
  
  inputs[2].val.p = (void *)registryLocation;
  inputs[2].ptr = inputs[2].val.p;
  inputs[2].type = nsXPTType::T_CHAR_STR;

#ifdef XP_PERL_DEBUG
  warn("Calling unregisterSelf in plPerlModule.\n");
#endif
  res = plPerlBase::CallPerlMethod(m_perlObject, "unregisterSelf",
                                   inputs, 3);
  if(NS_FAILED(res)) {
    return res;
  }
  VP_GET_RETCODE(res);
  return res;
}

NS_IMETHODIMP plPerlModule::CanUnload(nsIComponentManager *aCompMgr,
                                      PRBool *_retval) {
  if(!m_perlObject) { return NS_ERROR_FAILURE; }
  nsresult res = 0;
  SV *cmsv = NULL;
  nsXPTCVariant inputs[1];
  nsXPTCVariant outputs[1];
  
  nsIID cmiid = NS_GET_IID(nsIComponentManager);

  if(NS_FAILED(perlXPUtil::ConvertISupportsToSV(aCompMgr, &cmiid, nsnull, &cmsv))) {
    warn("plPerlModule Warning: No component manager given unregisterSelf\n");
  }
  inputs[0].val.p = (void *)cmsv;
  inputs[0].ptr = inputs[0].val.p;
  inputs[0].type = nsXPTType::T_INTERFACE;
  inputs[0].flags = nsXPTCVariant::PTR_IS_DATA;
  
  outputs[0].val.b = 0;
  outputs[0].ptr = &outputs[0].val.b;
  outputs[0].flags = nsXPTCVariant::PTR_IS_DATA;
  outputs[0].type = nsXPTType::T_BOOL;

#ifdef XP_PERL_DEBUG
  warn("Calling canUnload on plPerlModule\n");
#endif
  res = plPerlBase::CallPerlMethod(m_perlObject, "canUnload",
                                   inputs, 1, outputs, 1);
  if(NS_FAILED(res)) {
    return res;
  }
  VP_GET_RETCODE(res);
  return res;
}
