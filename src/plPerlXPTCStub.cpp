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
 * File: plPerlXPTCStub.cpp
 * Created by: matt
 * Created on: Mon 04 Jun 2001 10:08:30 PM EDT
 * $Id: plPerlXPTCStub.cpp,v 1.4 2005/03/31 08:29:00 poumeyrol Exp $
 *****************************************************************************/
#include "plPerlXPCOM_Base.h"

//#define XP_PERL_DEBUG

plPerlXPTCStub::plPerlXPTCStub() {
  NS_INIT_ISUPPORTS();
}

plPerlXPTCStub::plPerlXPTCStub(SV *perlObject, const nsIID& iid) 
  : plPerlBase(perlObject, iid) {
  NS_INIT_ISUPPORTS();
#ifdef XP_PERL_DEBUG
  warn("Creating a perl XPTCStub object.\n");
#endif
}

#ifdef XP_PERL_DEBUG
NS_IMETHODIMP_(nsrefcnt) plPerlXPTCStub::AddRef(void) {
  NS_PRECONDITION(PRInt16(mRefCnt) >= 0, "illegal refcnt");
  NS_ASSERT_OWNINGTHREAD(plPerlXPTCStub);
  ++mRefCnt;
  NS_LOG_ADDREF(this, mRefCnt, plPerlXPTCStub, sizeof(*this));
  return mRefCnt;
}

NS_IMETHODIMP_(nsrefcnt) plPerlXPTCStub::Release(void) {
  NS_PRECONDITION(0 != mRefCnt, "dup release");
  NS_ASSERT_OWNINGTHREAD(_class); 
  --mRefCnt;
  NS_LOG_RELEASE(this, mRefCnt, #_class);
  if (mRefCnt == 0) {
    mRefCnt = 1;
    NS_DELETEXPCOM(this);
    return 0;
  }
  return mRefCnt;
}
#else
NS_IMPL_ADDREF(plPerlXPTCStub);
NS_IMPL_RELEASE(plPerlXPTCStub);
#endif

NS_IMETHODIMP plPerlXPTCStub::QueryInterface(REFNSIID aIID, 
                                             void **aInstancePtr) {
  // If we have no perl object, check our native crap
  nsresult res;
  nsISupports *ret = nsnull;
  SV *retsv = NULL;
  nsXPTCVariant inputs[1];
  nsXPTCVariant outputs[1];
  
  if(m_perlObject) {
    inputs[0].val.p = (void *)&aIID;
    inputs[0].ptr = inputs[0].val.p;
    inputs[0].type = nsXPTType::T_IID;
    
    outputs[0].val.p = nsnull;
    outputs[0].ptr = outputs[0].val.p;
    outputs[0].type = nsXPTType::T_VOID;
    //outputs[0].SetValIsInterface();

#ifdef XP_PERL_DEBUG
    warn("Calling QueryInterface on perlstub object\n");
#endif
    res = plPerlBase::CallPerlMethod(m_perlObject, "QueryInterface",
                                     inputs, 1, outputs, 1);
    retsv = (SV*)outputs[0].val.p;
    
    if(NS_FAILED(res)) {
#ifdef XP_PERL_DEBUG
      warn("Perl QI() wasn't happy, trying native stuff\n");
#endif
      // Don't bail just yet, continue to the native QI check.
      *aInstancePtr = nsnull;
    } else {
      if(!retsv) {
#ifdef XP_PERL_DEBUG
        warn("No return SV but success code, got problems.\n");
#endif
        *aInstancePtr = nsnull;
        return NS_ERROR_FAILURE;
      }
      // If we succeeded and all is well, return now.
#ifdef XP_PERL_DEBUG
      warn("Getting new stub for QI()'ed result\n");
#endif
      res = VP_ConvertSVToISupports(retsv, &ret, &aIID);
      SvREFCNT_dec(retsv);
      if(!ret) {
        return NS_ERROR_OUT_OF_MEMORY;
      }
#ifdef XP_PERL_DEBUG
      warn("Ok, we got the object and we be gone.\n");
#endif
      *aInstancePtr = ret;
      return res;
    }
  }
  // Native check. We support the following interfaces natively
  // nsIInternalPerl, nsIWeakReference, nsISupports
  // nsCOMTypeInfo<nsIX>::GetIID()
  //
  nsISupports *foundIFace = nsnull;
#ifdef XP_PERL_DEBUG
  warn("Doing native QI() check\n");
#endif
//  if(aIID.Equals(nsCOMTypeInfo<nsIInternalPerl>::GetIID())) {
//    foundIFace = NS_STATIC_CAST(nsIInternalPerl*, this);
//  } else if(aIID.Equals(nsCOMTypeInfo<nsIWeakReference>::GetIID())) {
//    foundIFace = NS_STATIC_CAST(nsIWeakReference*, this);
//  } else 
  if(aIID.Equals(nsCOMTypeInfo<nsISupports>::GetIID())) {
    foundIFace = NS_STATIC_CAST(nsISupports*, this);
  } else {
    foundIFace = nsnull;
  }
  if(!foundIFace) {
    res = NS_ERROR_NO_INTERFACE;
  } else {
    NS_ADDREF(foundIFace);
    res = NS_OK;
  }
  *aInstancePtr = foundIFace;
  return res;
}

NS_IMETHODIMP plPerlXPTCStub::GetInterfaceInfo(nsIInterfaceInfo **info) {
  if(!info) {
    return NS_ERROR_FAILURE;
  }
  nsCOMPtr<nsIInterfaceInfoManager> iim = XPTI_GetInterfaceInfoManager();
  if(!iim) return NS_ERROR_FAILURE;

#ifdef XP_PERL_DEBUG
  warn("XXXXX: In plPerlXPTCStub::GetInterfaceInfo()\n");
#endif
  char *t = m_iid.ToString();
  nsMemory::Free(t);
  return iim->GetInfoForIID(&m_iid, info);
}

NS_IMETHODIMP plPerlXPTCStub::CallMethod(PRUint16 methodIndex,
                                         const nsXPTMethodInfo *info,
                                         nsXPTCMiniVariant *params) {
  if(!info) {
    return NS_ERROR_FAILURE;
  }
  nsresult res;
  const char *methodName = info->GetName();
  int intot = 0, outtot = 0, iotot = 0, x = 0, y = 0, z = 0;
  nsXPTCVariant inputs[32], outputs[32];
  nsCOMPtr<nsIInterfaceInfo> iface = nsnull;
  nsIID *inargIID = nsnull;
  SV *inargSV = NULL;
//  PRUint8 ifIs = 0;
  
#ifdef XP_PERL_DEBUG
  warn("Calling method %s on perl stub object.\n", methodName);
#endif
  
  iotot = (int)info->GetParamCount();
  for(x = 0;x < iotot;x++) {
    nsXPTParamInfo paramInfo = info->GetParam(x);
    if(paramInfo.IsIn() && !paramInfo.IsRetval()) {
      inputs[intot].Init(params[x], paramInfo.GetType(), 0);
      switch(paramInfo.GetType().TagPart()) {
        // Verify that the object being passed is the proper interface
        case nsXPTType::T_INTERFACE:
          if(!iface) {
            res = this->GetInterfaceInfo(getter_AddRefs(iface));
            if(NS_FAILED(res)) {
              return res;
            }
          }
          res = iface->GetIIDForParam(methodIndex, &paramInfo, &inargIID);
          if(NS_FAILED(res)) {
            warn("plPerlXPTCStub: Unable to determine IID for argument.\n");
            inputs[intot].val.p = (void *)&PL_sv_undef;
            inputs[intot].ptr = inputs[intot].val.p;
          } else {
            if(params[x].val.p) { 
              perlXPUtil::ConvertISupportsToSV((nsISupports *)params[x].val.p, 
                                      inargIID, 
                                      nsnull,
                                      &inargSV);
              inputs[intot].val.p = (void *)inargSV;
            } else {
              warn("Recieved null value for nsISupports object.\n");
              inputs[intot].val.p = (void *)&PL_sv_undef;
            }
            inputs[intot].ptr = inputs[intot].val.p;
          }
          break;
        case nsXPTType::T_INTERFACE_IS:
          // XXX - There never should be a T_INTERFACE_IS input should there?
#if 0
          ifIs = params[x].type.argnum;
          if(params[ifIs].type.TagPart() == nsXPTType::T_IID) {
            perlXPUtil::ConvertISupportsToSV((nsISupports *)params[x].val.p,
                                    (nsIID *)params[ifIs].val.p,
                                    nsnull,
                                    &inargSV);
            inputs[intot].val.p = (void *)inargSV;
            inputs[intot].ptr = inputs[intot].val.p;
          } else {
#endif
            warn("plPerlXPTCStub: Dependant arg is not an IID.\n");
            inputs[intot].val.p = (void *)&PL_sv_undef;
            inputs[intot].ptr = inputs[intot].val.p;
#if 0
          }
#endif
          break;
        case nsXPTType::T_ARRAY: {
          PRUint8 t_sz = paramInfo.type.argnum;
          if(!t_sz) {
            return NS_ERROR_ABORT;
          }
          PRUint16 t_dim = (PRUint16)params[t_sz].val.u16;
          if(!iface) {
            res = this->GetInterfaceInfo(getter_AddRefs(iface));
            if(NS_FAILED(res)) {
              return res;
            }
          }
          inputs[intot].val.p = (void*)perlXPUtil::convertArrayToSV(
                                                           params[x].val.p,
                                                           t_dim,
                                                           iface,
                                                           &paramInfo,
                                                           methodIndex,
                                                           &res);
          if(NS_FAILED(res)) {
            return res;
          }
          break;
        }
      }
      intot++;
    } else {
      outputs[outtot].Init(params[x], paramInfo.GetType(), 0);
      switch(paramInfo.GetType().TagPart()) {
        case nsXPTType::T_I8: 
          outputs[outtot].ptr = &outputs[outtot].val.i8; break;
        case nsXPTType::T_I16: 
          outputs[outtot].ptr = &outputs[outtot].val.i16; break;
        case nsXPTType::T_I32: 
          outputs[outtot].ptr = &outputs[outtot].val.i32; break;
        case nsXPTType::T_I64: 
          outputs[outtot].ptr = &outputs[outtot].val.i64; break;
        case nsXPTType::T_U8: 
          outputs[outtot].ptr = &outputs[outtot].val.u8; break;
        case nsXPTType::T_U16: 
          outputs[outtot].ptr = &outputs[outtot].val.u16; break;
        case nsXPTType::T_U32: 
          outputs[outtot].ptr = &outputs[outtot].val.u32; break;
        case nsXPTType::T_U64: 
          outputs[outtot].ptr = &outputs[outtot].val.u64; break;
        case nsXPTType::T_FLOAT: 
          outputs[outtot].ptr = &outputs[outtot].val.f; break;
        case nsXPTType::T_DOUBLE: 
          outputs[outtot].ptr = &outputs[outtot].val.d; break;
        case nsXPTType::T_BOOL: 
          outputs[outtot].ptr = &outputs[outtot].val.b; break;
        case nsXPTType::T_CHAR: 
          outputs[outtot].ptr = &outputs[outtot].val.c; break;
        case nsXPTType::T_WCHAR: 
          outputs[outtot].ptr = &outputs[outtot].val.wc; break;
        default:
          outputs[outtot].ptr = outputs[outtot].val.p; break;
      }
      outtot++;
    }
  }

  res = plPerlBase::CallPerlMethod(m_perlObject, methodName,
                                   (intot)? inputs: NULL,
                                   (intot)? intot: -1,
                                   (outtot)? outputs: NULL,
                                   (outtot)? outtot: -1);

  // If we got an error code, don't bother processing the results, they're 
  // probably bogus anyway.
  if(NS_FAILED(res)) {
#ifdef XP_PERL_DEBUG
      warn("CallPerlMethod %s failed.", methodName);
#endif
    return res;
  }
  for(x = 0;x < iotot;x++) {
    nsXPTParamInfo pI = info->GetParam(x);
    if(pI.IsOut()) {
      switch(pI.GetType().TagPart()) {
        case nsXPTType::T_I8: 
          *((PRInt8 *)params[x].val.p) = (PRInt8)outputs[y].val.i8; 
          break;
        case nsXPTType::T_I16: 
          *((PRInt16 *)params[x].val.p) = (PRInt16)outputs[y].val.i16;
          break;
        case nsXPTType::T_I32: 
          *((PRInt32 *)params[x].val.p) = (PRInt32)outputs[y].val.i32; 
          break;
        case nsXPTType::T_I64: 
          *((PRInt64 *)params[x].val.p) = (PRInt64)outputs[y].val.i64; 
          break;
        case nsXPTType::T_U8: 
          *((PRUint8 *)params[x].val.p) = (PRUint8)outputs[y].val.u8; 
          break;
        case nsXPTType::T_U16: 
          *((PRUint16 *)params[x].val.p) = (PRUint16)outputs[y].val.u16; 
          break;
        case nsXPTType::T_U32: 
          *((PRUint32 *)params[x].val.p) = (PRUint32)outputs[y].val.u32; 
          break;
        case nsXPTType::T_U64: 
          *((PRUint64 *)params[x].val.p) = (PRUint64)outputs[y].val.u64; 
          break;
        case nsXPTType::T_FLOAT: 
          *((float *)params[x].val.p) = (float)outputs[y].val.f; 
          break;
        case nsXPTType::T_DOUBLE: 
          *((double *)params[x].val.p) = (double)outputs[y].val.d; 
          break;
        case nsXPTType::T_BOOL: 
          *((PRBool *)params[x].val.p) = (PRBool)outputs[y].val.b; 
          break;
        case nsXPTType::T_CHAR: 
          *((char *)params[x].val.c) = (char)outputs[y].val.c; 
          break;
        case nsXPTType::T_WCHAR: 
          *((PRUnichar *)params[x].val.wc) = (PRUnichar)outputs[y].val.wc; 
          break;
        case nsXPTType::T_ARRAY: 
          if(!iface) {
            res = this->GetInterfaceInfo(getter_AddRefs(iface));
            if(NS_FAILED(res)) {
              return res;
            }
          }
          params[x].val.p = perlXPUtil::convertSVToArray((SV*)outputs[y].val.p,
                                                params[pI.type.argnum].val.u16,
                                                iface,
                                                &pI,
                                                methodIndex,
                                                &res);
          sv_2mortal((SV*)outputs[y].val.p);
          break;
        default:
          params[x].val.p = outputs[y].val.p; break;
      }
      y++;
    } else if(pI.IsIn()) {
      switch(pI.GetType().TagPart()) {
        // Just make sure we mortalize the SV's we created for these 
        // interface objects.
        // FIXME - We're not doing anything for T_INTERFACE_IS because the 
        // input processor is hardcoding undef since it can't deal with it 
        // yet.
        //case nsXPTType::T_INTERFACE_IS: // Fall through
        case nsXPTType::T_INTERFACE: 
          inargSV = (SV*)inputs[z].val.p;
          sv_2mortal(inargSV);
          break;
        case nsXPTType::T_ARRAY: {
          SV *element = nsnull;
          AV *t_arr = (AV*)SvRV((SV*)inputs[z].val.p);
          I32 t_x, t_y;

          t_y = av_len(t_arr);
          // perlXPUtil::ConvertArrayToSV increments the refcnt of each SV, so we 
          // deincrement it here.
          for(t_x = 0; t_x <= t_y; t_x++) {
            element = *av_fetch(t_arr, t_x, 0);
            SvREFCNT_dec(element);
          }
          sv_2mortal((SV*)t_arr);
          break;
        }
      }
      z++;
    }
  }
  return res;
}
