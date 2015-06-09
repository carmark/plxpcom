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
 * File: perlXPUtil.h
 * Created by: matt
 * Created on: Wed 30 May 2001 03:14:23 PM EDT
 * $Id: perlXPUtil.h,v 1.1 2005/03/31 08:13:29 poumeyrol Exp $
 *****************************************************************************/
#ifndef PERLXPUTIL_H_
#define PERLXPUTIL_H_

#include "nsString.h"
#include "nsReadableUtils.h"
#include "nsCRT.h"
#include "nsCOMPtr.h"
#include "nsIServiceManager.h"
//#include "nsXPIDLString.h"
#include "nsISupports.h"
#include "nsIInterfaceInfo.h"
#include "nsIInterfaceInfoManager.h"
#include "nsMemory.h"
#include "xptinfo.h"
#include "xptcall.h"

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#define VP_RETURN_UNDEF \
  ST(0) = sv_newmortal(); \
  sv_setsv(ST(0), &PL_sv_undef); \
  XSRETURN(1);

#define VP_SET_RETCODE(ret_) \
{ \
  SV *_retsv_ = perl_get_sv("Components::returnCode", 1); \
  if(_retsv_) { \
    sv_setuv(_retsv_, (UV)ret_); \
  } \
}

#define VP_GET_RETCODE(ret_) \
{ \
  SV *_retsv_ = perl_get_sv("Components::returnCode", 1); \
  if(_retsv_) { \
    ret_ = (nsresult)SvUV(_retsv_); \
  } \
}

#define MAX_XPTC_ARG_COUNT 16

// Uncomment to turn on verbose bitching
//#define XP_PERL_DEBUG 1

// This is for making perl's typemapping happy, even though the typemaps are 
// never used...
typedef nsISupports rawISupports;

class perlXPUtil {
  public:
    static nsresult fetchXPObject(SV *theSV,
                                  nsCID **theCID,
                                  nsIID **theIID,
                                  nsISupports **theObject);

    static SV* makeXPObject(nsCID *cid, nsIID *iid, nsISupports *obj);
    
    static PRBool lookupConstant(nsIInterfaceInfo *iFace,
                                 const char *name,
                                 const nsXPTConstant **idx);

    static SV* constantToSV(const nsXPTConstant *theConstant, PRBool *result);

    static PRUint32 getSizeOfType(nsXPTType& theType);

    static void *convertSVToArray(SV *avRef,
                                  PRUint16 dimension,
                                  nsIInterfaceInfo *iface,
                                  nsXPTParamInfo *theParam,
                                  PRUint16 methodIndex,
                                  nsresult *res);

    static SV *convertArrayToSV(void *arrPtr,
                                PRUint16 dimension,
                                nsIInterfaceInfo *iface,
                                nsXPTParamInfo *theParam,
                                PRUint16 methodIndex,
                                nsresult *res);

    static nsresult fetchIIDFromSV(SV *theIID, nsIID& anIID);

    static nsresult GetInterfaceInfo(nsIInterfaceInfoManager *iim, 
                                 const nsIID& iid,
                                 nsIInterfaceInfo **iface);

    /**
      * Iterates through an array of interface ids to find one that supports 
      * the requested method.
      */
    static nsresult FindMethod(nsIInterfaceInfoManager *iim,
                           PRUint32 count,
                           nsIID **array,
                           const char *meth,
                           PRUint16 inputCount,
                           PRUint16 *methIndex,
                           const nsXPTMethodInfo **info,
                           nsIInterfaceInfo **retIFace);

    /**
      * Utility function that QI's an object for support for nsIClassInfo 
      * and if the object supports it fetches the supported interfaces via 
      * nsIClassInfo::GetInterfaces
      */
    static nsresult GetInterfaceList(nsISupports *object,
                                 PRUint32 *count,
                                 nsIID * **array);

    static nsresult GetMethodInfo(nsIInterfaceInfo *iface, 
                              const char *methName,
                              PRUint16 inputCount,
                              PRUint16 *methIndex,
                              const nsXPTMethodInfo **info);

    static void *ConvertSVToArray(SV *avRef, 
                          PRUint16 dimension,
                          nsIInterfaceInfo *iface,
                          nsXPTParamInfo *theParam,
                          PRUint16 methodIndex,
                          nsresult *res);

    static SV *ConvertArrayToSV(void *arrPtr,
                        PRUint16 dimension,
                        nsIInterfaceInfo *iface,
                        nsXPTParamInfo *theParam,
                        PRUint16 methodIndex,
                        nsresult *res);

    // Native to SV converters
    static nsresult ConvertInt8ToSV(PRInt8 in, SV **out) {
      SV *ret = sv_newmortal();
      *out = NULL;
      sv_setiv(ret, (IV)in);
      if(!ret) {
        return NS_ERROR_FAILURE;
      }
      *out = ret;
      return NS_OK;
    }

    static nsresult ConvertInt16ToSV(PRInt16 in, SV **out) {
      SV *ret = sv_newmortal();
      *out = NULL;
      sv_setiv(ret, (IV)in);
      if(!ret) {
        return NS_ERROR_FAILURE;
      }
      *out = ret;
      return NS_OK;
    }

    static nsresult ConvertInt32ToSV(PRInt32 in, SV **out) {
      SV *ret = sv_newmortal();
      *out = NULL;
      sv_setiv(ret, (IV)in);
      if(!ret) {
        return NS_ERROR_FAILURE;
      }
      *out = ret;
      return NS_OK;
    }

    static nsresult ConvertInt64ToSV(PRInt64 in, SV **out) {
      SV *ret = sv_newmortal();
      *out = NULL;
      sv_setiv(ret, (IV)in);
      if(!ret) {
        return NS_ERROR_FAILURE;
      }
      *out = ret;
      return NS_OK;
    }

    static nsresult ConvertUint8ToSV(PRUint8 in, SV **out) {
      SV *ret = sv_newmortal();
      *out = NULL;
      sv_setuv(ret, (UV)in);
      if(!ret) {
        return NS_ERROR_FAILURE;
      }
      *out = ret;
      return NS_OK;
    }

    static nsresult ConvertUint16ToSV(PRUint16 in, SV **out) {
      SV *ret = sv_newmortal();
      *out = NULL;
      sv_setuv(ret, (UV)in);
      if(!ret) {
        return NS_ERROR_FAILURE;
      }
      *out = ret;
      return NS_OK;
    }

    static nsresult ConvertUint32ToSV(PRUint32 in, SV **out) {
      SV *ret = sv_newmortal();
      *out = NULL;
      sv_setuv(ret, (UV)in);
      if(!ret) {
        return NS_ERROR_FAILURE;
      }
      *out = ret;
      return NS_OK;
    }

    static nsresult ConvertUint64ToSV(PRUint64 in, SV **out) {
      *out = NULL;
      SV *ret = sv_newmortal();
      sv_setuv(ret, (UV)in);
      if(!ret) {
        return NS_ERROR_FAILURE;
      }
      *out = ret;
      return NS_OK;
    }

    static nsresult ConvertFloatToSV(float in, SV **out) {
      SV *ret = sv_newmortal();
      *out = NULL;
      sv_setnv(ret, (double)in);
      if(!ret) {
        return NS_ERROR_FAILURE;
      }
      *out = ret;
      return NS_OK;
    }

    static nsresult ConvertDoubleToSV(double in, SV **out) {
      SV *ret = sv_newmortal();
      *out = NULL;
      sv_setnv(ret, (double)in);
      if(!ret) {
        return NS_ERROR_FAILURE;
      }
      *out = ret;
      return NS_OK;
    }

    static nsresult ConvertBoolToSV(PRBool in, SV **out) {
      SV *ret = sv_newmortal();
      *out = NULL;
      ret = boolSV(in);
      if(!ret) {
        return NS_ERROR_FAILURE;
      }
      *out = ret;
      return NS_OK;
    }

    static nsresult ConvertCharToSV(char in, SV **out) {
      SV *ret = sv_newmortal();
      *out = NULL;
      sv_setpvn(ret, (char *)&in, 1);
      if(!ret) {
        return NS_ERROR_FAILURE;
      }
      *out = ret;
      return NS_OK;
    }

    static nsresult ConvertWCharToSV(PRUnichar in, SV **out) {
      SV *ret = sv_newmortal();
      *out = NULL;
      sv_setiv(ret, (IV)in);
      if(!ret) {
        return NS_ERROR_FAILURE;
      }
      *out = ret;
      return NS_OK;
    }

    static nsresult ConvertIDToSV(nsID& in, SV **out) {
      SV *ret = sv_newmortal();
      *out = NULL;
      nsID *id = new nsID;
      if(!id) return NS_ERROR_OUT_OF_MEMORY;
      *id = in;
      sv_setref_pv(ret, "nsID", id);
      if(!ret) return NS_ERROR_FAILURE;
      *out = ret;
      return NS_OK;
    }

    // char * to SV (UTF8 or ISO)
    static nsresult ConvertStringToSV(const char *in, PRBool bUTF8, 
                                         SV **out) {
      SV *ret = sv_newmortal();
      if(!ret) return NS_ERROR_FAILURE;
      *out = NULL;
      sv_setpv(ret, in);
      if(bUTF8)
          SvUTF8_on(ret);
      *out = ret;
      return NS_OK;
    }

    // nsACString* to SV (UTF8 or ISO)
    static nsresult ConvertACStringToSV( nsACString* str, 
                                            PRBool bUTF8, SV **out) {
      return ConvertStringToSV(nsCString(*str).get(), PR_FALSE, out);
    }

    // PRUnichar* to SV (UTF8)
    static nsresult ConvertWStringToSV(const PRUnichar *in, SV **out) {
      return ConvertStringToSV(NS_ConvertUCS2toUTF8(in).get(), PR_TRUE, out);
    }

    // AString to SV (UTF8)
    static nsresult ConvertAStringToSV(nsAString* str, SV **out) {
      return ConvertWStringToSV(nsString(*str).get(), out);
    }

    static nsresult
    ConvertISupportsToSV(nsISupports *in, nsIID *iid, nsCID *cid, SV **out) {
      *out = perlXPUtil::makeXPObject(cid, iid, in);
      if(!*out) return NS_ERROR_FAILURE;
      return NS_OK;
    }

    // SV To Native converters
    static nsresult ConvertSVToInt8(SV *in, PRInt8 *out) {
      *out = (PRInt8)SvIV(in);
      return NS_OK;
    }

   static  nsresult ConvertSVToInt16(SV *in, PRInt16 *out) {
      *out = (PRInt16)SvIV(in);
      return NS_OK;
    }

    static nsresult ConvertSVToInt32(SV *in, PRInt32 *out) {
      *out = (PRInt32)SvIV(in);
      return NS_OK;
    }

    static nsresult ConvertSVToInt64(SV *in, PRInt64 *out) {
      *out = (PRInt64)SvIV(in);
      return NS_OK;
    }

    static nsresult ConvertSVToUint8(SV *in, PRUint8 *out) {
      *out = (PRUint8)SvIV(in);
      return NS_OK;
    }

    static nsresult ConvertSVToUint16(SV *in, PRUint16 *out) {
      *out = (PRUint16)SvIV(in);
      return NS_OK;
    }

    static nsresult ConvertSVToUint32(SV *in, PRUint32 *out) {
      *out = (PRUint32)SvIV(in);
      return NS_OK;
    }

    static nsresult ConvertSVToUint64(SV *in, PRUint64 *out) {
      *out = (PRUint64)SvIV(in);
      return NS_OK;
    }

    static nsresult ConvertSVToFloat(SV *in, float *out) {
      *out = (float)SvNV(in);
      return NS_OK;
    }

    static nsresult ConvertSVToDouble(SV *in, double *out) {
      *out = (double)SvNV(in);
      return NS_OK;
    }

    static nsresult ConvertSVToBool(SV *in, PRBool *out) {
      *out = (PRBool)SvIV(in);
      return NS_OK;
    }

    static nsresult ConvertSVToChar(SV *in, char *out) {
      *out = (char)*SvPV(in,PL_na);
      return NS_OK;
    }

    static nsresult ConvertSVToWChar(SV *in, PRUnichar *out) {
      *out = (PRUnichar)SvIV(in);
      return NS_OK;
    }

    static nsresult ConvertSVToID(SV *in, nsID *out) {
      if(SvROK(in)) { // Better checking needed
        nsID *id = (nsID *)SvIV((SV*)SvRV(in));
        *out = *id;
        return NS_OK;
      }
      return NS_ERROR_FAILURE;
    }

    static nsresult ConvertSVToString(SV *in, PRBool bUTF8, char **out) {
      if(SvUTF8(in) && !bUTF8)
      {
          SV* conv = sv_newmortal();
          sv_setsv(conv, in);
          sv_utf8_downgrade(conv, TRUE);
          in = conv;
      }
      if(!SvUTF8(in) && bUTF8)
      {
          SV* conv = sv_newmortal();
          sv_setsv(conv, in);
          sv_utf8_upgrade(conv);
          in = conv;
      }
      PRUint32 len;
      char *s = SvPV(in, len);
      char *ret = (char*) nsMemory::Clone(s, len);
      if(!ret) { return NS_ERROR_OUT_OF_MEMORY; }
      *out = ret;
      return NS_OK;
    }

    static nsresult ConvertSVToACString(SV* in, PRBool bUTF8, 
            nsACString& out)
    {
        char* pcTmp;
        nsresult res = ConvertSVToString(in, bUTF8, &pcTmp);
        if(NS_FAILED(res))
        {
            nsMemory::Free(pcTmp);
            return res;
        }
        out.Assign(pcTmp);
        return NS_OK;
    }

    static nsresult ConvertSVToWString(SV *in, PRUnichar **out) {
      if(SvUTF8(in))
        *out = ToNewUnicode(NS_ConvertUTF8toUCS2(SvPV_nolen(in)));
      else
        *out = ToNewUnicode(NS_ConvertASCIItoUCS2(SvPV_nolen(in)));
      if(!*out) {
        return NS_ERROR_OUT_OF_MEMORY;
      }
      return NS_OK;
    }

    static nsresult ConvertSVToAString(SV* in, nsAString& out)
    {
      if(SvUTF8(in))
        out.Assign(ToNewUnicode(NS_ConvertUTF8toUCS2(SvPV_nolen(in))));
      else
        out.Assign(ToNewUnicode(NS_ConvertASCIItoUCS2(SvPV_nolen(in))));
      return NS_OK;
    }

    static SV *MakeUndef() {
      SV *ret = sv_newmortal();
      sv_setsv(ret, &PL_sv_undef);
      return ret;
    }

};
#endif


