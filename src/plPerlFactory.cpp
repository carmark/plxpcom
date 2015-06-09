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
 * File: plPerlFactory.cpp
 * Created by: matt
 * Created on: Mon 04 Jun 2001 09:10:08 PM EDT
 * $Id: plPerlFactory.cpp,v 1.2 2005/03/31 08:28:59 poumeyrol Exp $
 *****************************************************************************/

#include "plPerlXPCOM_Base.h"

NS_IMPL_ISUPPORTS1_CI(plPerlFactory, nsIFactory);

plPerlFactory::plPerlFactory() {
  NS_INIT_ISUPPORTS();
}

plPerlFactory::plPerlFactory(SV *perlObject)
  : plPerlBase(perlObject, NS_GET_IID(nsIFactory)) {
  NS_INIT_ISUPPORTS();
}

NS_IMETHODIMP plPerlFactory::CreateInstance(nsISupports *aOuter, 
                                            const nsIID &iid,
                                            void **result) {
  if(!m_perlObject) { return NS_ERROR_FAILURE; }
  nsresult res;
  SV *cmsv = NULL;
  SV *retsv = NULL;
  nsIID isiid = NS_GET_IID(nsISupports);
  nsISupports *retval = nsnull;
  nsISupports *t_ret = nsnull;

  nsXPTCVariant inputs[2];
  nsXPTCVariant outputs[1];

  if(aOuter) {
    res = perlXPUtil::ConvertISupportsToSV(aOuter, &isiid, nsnull, &cmsv);
    if(NS_FAILED(res)) {
      cmsv = perlXPUtil::MakeUndef();
    }
  } else {
    cmsv = perlXPUtil::MakeUndef();
  }

  inputs[0].val.p = (void *)cmsv;
  inputs[0].ptr = inputs[0].val.p;
  inputs[0].type = nsXPTType::T_INTERFACE;
  inputs[0].flags = nsXPTCVariant::PTR_IS_DATA;

  inputs[1].val.p = (void *)&iid;
  inputs[1].ptr = inputs[1].val.p;
  inputs[1].type = nsXPTType::T_IID;
  inputs[1].flags = nsXPTCVariant::PTR_IS_DATA;

  outputs[0].val.p = nsnull;
  outputs[0].ptr = outputs[0].val.p;
  outputs[0].type = nsXPTType::T_VOID;
  
#ifdef XP_PERL_DEBUG
  warn("Calling plPerlFactory::createInstance()\n");
#endif
  res = plPerlBase::CallPerlMethod(m_perlObject, "createInstance",
                                   inputs, 2, outputs, 1);
  if(NS_FAILED(res)) {
    return res;
  }
  retsv = (SV*)outputs[0].val.p;
  if(retsv) {
#ifdef XP_PERL_DEBUG
    warn("Factory got an object, making an ISupports\n");
#endif
    VP_ConvertSVToISupports(retsv, &retval, &iid);
    if(retval) {
#ifdef XP_PERL_DEBUG
      warn("Got an object, calling QI() on it.\n");
#endif
      res = retval->QueryInterface(iid, (void **)&t_ret);
      NS_IF_RELEASE(retval);
      if(NS_FAILED(res)) {
#ifdef XP_PERL_DEBUG
        warn("QI() no working, bailing now.\n");
#endif
        SvREFCNT_dec(retsv);
        return res;
      }
      if(!t_ret) {
        return NS_ERROR_OUT_OF_MEMORY;
      }
      *result = t_ret;
#ifdef XP_PERL_DEBUG
      warn("Hot damn! It worked!\n");
#endif
      if(NS_FAILED(res)) {
        warn("Wierd, got a failure code, but got an object ok.\n");
      }
      //VP_GET_RETCODE(res);
      return res;
    }
    SvREFCNT_dec(retsv);
    //*result = (void *)retval;
  }
  VP_GET_RETCODE(res);
  return res;
}

NS_IMETHODIMP plPerlFactory::LockFactory(PRBool lock) {
  if(!m_perlObject) { return NS_ERROR_FAILURE; }

  nsresult res;
  nsXPTCVariant inputs[1];

  inputs[0].val.b = lock;
  inputs[0].type = nsXPTType::T_BOOL;

#ifdef XP_PERL_DEBUG
  warn("Calling plPerlFactory::lockFactory()\n");
#endif
  res = plPerlBase::CallPerlMethod(m_perlObject, "lockFactory",
                                   inputs, 1);

  if(NS_FAILED(res)) {
    return res;
  }
  VP_GET_RETCODE(res);
  return res;
}
