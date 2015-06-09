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
 * File: plPerlBase.cpp
 * Created by: matt
 * Created on: Wed 06 Jun 2001 02:35:30 PM EDT
 * $Id: plPerlBase.cpp,v 1.4 2005/03/31 08:28:59 poumeyrol Exp $
 *****************************************************************************/

#include "plPerlXPCOM_Base.h"

//#define XP_PERL_DEBUG 1

plPerlBase::plPerlBase() {
  m_perlObject = NULL;
}

plPerlBase::plPerlBase(SV *perlObject, const nsIID& iid) {
  m_perlObject = perlObject;
  SvREFCNT_inc(m_perlObject);
#ifdef XP_PERL_DEBUG
  if(SvROK(m_perlObject)) {
    warn("Creating an plPerlBase for SV object of package: %s\n",
          HvNAME(SvSTASH((SV*)SvRV(m_perlObject))));
  }
  warn("new plPerlBase object's refcnt is %d\n", SvREFCNT(m_perlObject));
#endif
  m_iid = iid;
}

plPerlBase::~plPerlBase() {
#ifdef XP_PERL_DEBUG
  warn("In plPerlBase destructor.\n");
#endif
  if(m_perlObject) {
#ifdef XP_PERL_DEBUG
    warn("DTor of object '%s', refcnt %d.\n",
         HvNAME(SvSTASH((SV*)SvRV(m_perlObject))), SvREFCNT(m_perlObject));
#endif
    SvREFCNT_dec(m_perlObject);
  }
}

nsresult plPerlBase::CallPerlMethod(SV *perlObject,
                                    const char *methodName,
                                    const nsXPTCVariant *d,
                                    int paramIndex,
                                    nsXPTCVariant *o,
                                    int outputIndex) {
  if(!perlObject || !methodName) {
    return NS_ERROR_FAILURE;
  }
  if(perlObject == &PL_sv_undef) {
    warn("Attempt to invoke method '%s' on an undefined scalar.\n",methodName);
    return NS_ERROR_ABORT;
  }
  nsresult res = NS_OK;
  int x = 0, retval = 0;
  SV *argSV = NULL;
  I32 ax = 0;
  
#ifdef XP_PERL_DEBUG
  warn("plPerlBase::CallPerlMethod() for %s\n", methodName);
  warn("Object has refcnt of %d\n", SvREFCNT(perlObject));
  if(SvROK(perlObject)) {
    warn("Object's package name is %s\n",
          HvNAME(SvSTASH((SV*)SvRV(perlObject))));
  }
#endif

  dSP;
  ENTER;
  SAVETMPS;
  PUSHMARK(SP);

  XPUSHs(perlObject); // The object itself always goes first
  if(paramIndex > 0 && d) {
#ifdef XP_PERL_DEBUG
    warn("Processing %d input args\n", paramIndex);
#endif
    for(x = 0;x < paramIndex;x++) {
      switch(d[x].type.TagPart()) {
        case nsXPTType::T_U8:
          res = perlXPUtil::ConvertUint8ToSV(d[x].val.u8, &argSV);
          break;
        case nsXPTType::T_U16:
          res = perlXPUtil::ConvertUint16ToSV(d[x].val.u16, &argSV);
          break;
        case nsXPTType::T_U32:
          res = perlXPUtil::ConvertUint32ToSV(d[x].val.u32, &argSV);
          break;
        case nsXPTType::T_U64:
          res = perlXPUtil::ConvertUint64ToSV(d[x].val.u64, &argSV);
          break;
        case nsXPTType::T_I8:
          res = perlXPUtil::ConvertInt8ToSV(d[x].val.i8, &argSV);
          break;
        case nsXPTType::T_I16:
          res = perlXPUtil::ConvertInt16ToSV(d[x].val.i16, &argSV);
          break;
        case nsXPTType::T_I32:
          res = perlXPUtil::ConvertInt32ToSV(d[x].val.i32, &argSV);
          break;
        case nsXPTType::T_I64:
          res = perlXPUtil::ConvertInt64ToSV(d[x].val.i64, &argSV);
          break;
        case nsXPTType::T_FLOAT:
          res = perlXPUtil::ConvertFloatToSV(d[x].val.f, &argSV);
          break;
        case nsXPTType::T_DOUBLE:
          res = perlXPUtil::ConvertDoubleToSV(d[x].val.d, &argSV);
          break;
        case nsXPTType::T_BOOL:
          res = perlXPUtil::ConvertBoolToSV(d[x].val.b, &argSV);
          break;
        case nsXPTType::T_CHAR:
          res = perlXPUtil::ConvertCharToSV(d[x].val.c, &argSV);
          break;
        case nsXPTType::T_WCHAR:
          res = perlXPUtil::ConvertWCharToSV(d[x].val.wc, &argSV);
          break;
        case nsXPTType::T_IID:
          res = perlXPUtil::ConvertIDToSV(*(nsID *)d[x].val.p, &argSV);
          break;
        case nsXPTType::T_CHAR_STR: // Fall through
        case nsXPTType::T_PSTRING_SIZE_IS:
          res = perlXPUtil::ConvertStringToSV((const char *)d[x].val.p,PR_FALSE,&argSV);
          break;
        case nsXPTType::T_WCHAR_STR: // Fall through
        case nsXPTType::T_PWSTRING_SIZE_IS:
          res = perlXPUtil::ConvertWStringToSV((const PRUnichar *)d[x].val.p, &argSV);
          break;
        case nsXPTType::T_DOMSTRING:
        case nsXPTType::T_ASTRING:
          res = perlXPUtil::ConvertAStringToSV((nsAString*)d[x].val.p, &argSV);
          break;
        case nsXPTType::T_CSTRING:
          res = perlXPUtil::ConvertACStringToSV((nsACString*)d[x].val.p, PR_FALSE,
                  &argSV);
          break;
        case nsXPTType::T_UTF8STRING:
          res = perlXPUtil::ConvertACStringToSV((nsACString*)d[x].val.p, PR_TRUE, 
                  &argSV);
          break;
        case nsXPTType::T_INTERFACE_IS: // Fall through
        case nsXPTType::T_INTERFACE:
        case nsXPTType::T_ARRAY:
          // XXX Right now, the caller is responsible for getting the 
          // ISupports object into an SV.
          argSV = (SV*)d[x].val.p;
          break;
        default:
          warn("unhandled type %u\n", d[x].type.TagPart());
          res = NS_ERROR_ILLEGAL_VALUE;
          argSV = sv_newmortal();
          sv_setsv(argSV, &PL_sv_undef);
          break;
      }
      if(NS_FAILED(res)) {
        warn("CallPerlMethod: Error processing input arg %d. Sending undef.\n", 
             x);
      }
      XPUSHs(argSV);
    }
  }
  PUTBACK;
#ifdef XP_PERL_DEBUG
  warn("Calling method\n");
#endif

  retval = call_method(methodName, G_ARRAY|G_EVAL);
  
  SPAGAIN;
  
  // Even if we get a return, only process it if we have somewhere to put it.
  if(retval > 0 && o && outputIndex > 0) {
#ifdef XP_PERL_DEBUG
    warn("Processing %d output args\n", outputIndex);
#endif
    // We're processing return values XS style (e.g. ST(x) instead of POPs)
    SP -= retval;
    ax = (SP - PL_stack_base) + 1;
    for(x = 0;x < outputIndex;x++) {
      switch(o[x].type.TagPart()) {
        case nsXPTType::T_U8:
          perlXPUtil::ConvertSVToUint8(ST(x), (PRUint8 *)o[x].ptr);
          break;
        case nsXPTType::T_U16:
          perlXPUtil::ConvertSVToUint16(ST(x), (PRUint16 *)o[x].ptr);
          break;
        case nsXPTType::T_U32:
          perlXPUtil::ConvertSVToUint32(ST(x), (PRUint32 *)o[x].ptr);
          break;
        case nsXPTType::T_U64:
          perlXPUtil::ConvertSVToUint64(ST(x), (PRUint64 *)o[x].ptr);
          break;
        case nsXPTType::T_I8:
          perlXPUtil::ConvertSVToInt8(ST(x), (PRInt8 *)o[x].ptr);
          break;
        case nsXPTType::T_I16:
          perlXPUtil::ConvertSVToInt16(ST(x), (PRInt16 *)o[x].ptr);
          break;
        case nsXPTType::T_I32:
          perlXPUtil::ConvertSVToInt32(ST(x), (PRInt32 *)o[x].ptr);
          break;
        case nsXPTType::T_I64:
          perlXPUtil::ConvertSVToInt64(ST(x), (PRInt64 *)o[x].ptr);
          break;
        case nsXPTType::T_FLOAT:
          perlXPUtil::ConvertSVToFloat(ST(x), (float *)o[x].ptr);
          break;
        case nsXPTType::T_DOUBLE:
          perlXPUtil::ConvertSVToDouble(ST(x), (double *)o[x].ptr);
          break;
        case nsXPTType::T_BOOL:
          perlXPUtil::ConvertSVToBool(ST(x), (PRBool *)o[x].ptr);
          break;
        case nsXPTType::T_CHAR:
          perlXPUtil::ConvertSVToChar(ST(x), (char *)o[x].ptr);
          break;
        case nsXPTType::T_IID:
          perlXPUtil::ConvertSVToID(ST(x), (nsID *)o[x].ptr);
          break;
        case nsXPTType::T_DOMSTRING:
        case nsXPTType::T_ASTRING:
          perlXPUtil::ConvertSVToAString(ST(x), *(nsAString*)o[x].ptr);
          break;
        case nsXPTType::T_CSTRING:
          perlXPUtil::ConvertSVToACString(ST(x), PR_FALSE, *(nsACString*)o[x].ptr);
          break;
        case nsXPTType::T_UTF8STRING:
          perlXPUtil::ConvertSVToACString(ST(x), PR_TRUE, *(nsACString*)o[x].ptr);
          break;
        case nsXPTType::T_PSTRING_SIZE_IS: // Fall through
        case nsXPTType::T_CHAR_STR:
          perlXPUtil::ConvertSVToString(ST(x), PR_FALSE, (char **)o[x].ptr);
          break;
        case nsXPTType::T_PWSTRING_SIZE_IS: // Fall through
        case nsXPTType::T_WCHAR_STR:
          perlXPUtil::ConvertSVToWString(ST(x), (PRUnichar **)o[x].ptr);
          break;
        case nsXPTType::T_INTERFACE_IS: // Fall through
        case nsXPTType::T_INTERFACE:
          VP_ConvertSVToISupports(ST(x), (nsISupports **)o[x].ptr);
          break;
        case nsXPTType::T_ARRAY:
          o[x].val.p = ST(x);
          break;
        case nsXPTType::T_VOID:
          // XXX - This should be removed.
          // When we get a T_VOID we just increment the SV to keep it from 
          // going out of scope. This is because a T_VOID should be only 
          // an internal handler, since the perl handlers deal with the 
          // void ptr aspect of QueryInterface and generally a void ptr 
          // shouldn't be scriptable.
          {
#ifdef XP_PERL_DEBUG
            warn("In type void output handler.\n");
#endif
            SV *tsv = ST(x);
            if(SvROK(tsv)) {
#ifdef XP_PERL_DEBUG
              warn("Returning an SV object of package: %s\n",
                    HvNAME(SvSTASH((SV*)SvRV(tsv))));
#endif
            }
            SvREFCNT_inc(tsv);
            o[x].val.p = (void *)tsv;
          }
          break;
        default:
            warn("Unhandled type (%u)\n", o[x].type.TagPart());
          break;
      }
    }
#ifndef XP_PERL_DEBUG
  }
#else
  } else {
    warn("Not processing any return args. Ret count is %d\n", retval);
  }
#endif

  FREETMPS;
  LEAVE;
  VP_GET_RETCODE(res);
  return res;
}
