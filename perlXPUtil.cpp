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
 * File: perlXPUtil.cpp
 * Created by: matt
 * Created on: Wed 30 May 2001 03:08:47 PM EDT
 * $Id: perlXPUtil.cpp,v 1.1 2005/03/31 08:13:29 poumeyrol Exp $
 *****************************************************************************/

#include "nsString.h"
#include "nsCRT.h"
#include "perlXPUtil.h"

nsresult perlXPUtil::fetchXPObject(SV *theSV,
                                   nsCID **theCID,
                                   nsIID **theIID,
                                   nsISupports **theObject) {
#ifdef XP_PERL_DEBUG
  warn("In perlXPUtil::fetchXPObject()\n");
#endif
  if(!theSV) {
    return NS_ERROR_FAILURE;
  }
  if(!SvROK(theSV)) {
    return NS_ERROR_FAILURE;
  }
  if(SvTYPE(SvRV(theSV)) != SVt_PVHV) {
    return NS_ERROR_FAILURE;
  }
#ifdef XP_PERL_DEBUG
  warn("Fetching data from XPCOM object with refcnt of %d\n",
       SvREFCNT(theSV));
#endif
  // First, make sure we have a hash reference to an nsISupports object
//  if(sv_isobject(theSV) && sv_derived_from(theSV, "XPCOM::nsISupports")) {
//    warn("Error: SV is not an XPCOM::nsISupports object\n");
//    return NS_ERROR_FAILURE;
//  }
//  if(SvTYPE(SvRV(theSV)) != SVt_PVHV) {
//    return NS_ERROR_FAILURE;
//  }
  HV *theHV;
  SV *item;
  SV **hvres;
  nsCID *oCID = nsnull, *retCID = nsnull;
  nsIID *oIID = nsnull, *retIID = nsnull;
  nsISupports *oObject;
  
  theHV = (HV*)SvRV(theSV);
  // If the caller wants a CID check for it in the hash. If it exists, we 
  // make a copy which the caller must free, otherwise we set the return 
  // pointer to null.
  if(theCID) {
    hvres = hv_fetch(theHV,"__cid",5,0);
    if(hvres) {
      item = *hvres;
      if(sv_isobject(item) && sv_derived_from(item, "nsID")) {
        oCID = (nsCID *)SvIV((SV*)SvRV(item));
        if(oCID) {
#ifdef XP_PERL_DEBUG
          char *cst = oCID->ToString();
          warn("fetchXPObject: CID is %s\n", cst);
          nsMemory::Free(cst);
#endif
          retCID = new nsCID;
          *retCID = *oCID;
          *theCID = retCID;
        } else {
          *theCID = nsnull;
        }
      } else {
        *theCID = nsnull;
      }
      //item = sv_2mortal(item); // Mortalize the return so we don't leak.
    } else {
      *theCID = nsnull;
    }
  }
  // Same goes for the IID.
  if(theIID) {
    hvres = hv_fetch(theHV,"__iid",5,0);
    if(hvres) {
      item = *hvres;
      if(sv_isobject(item) && sv_derived_from(item, "nsID")) {
        oIID = (nsIID *)SvIV((SV*)SvRV(item));
        if(oIID) {
#ifdef XP_PERL_DEBUG
          char *ist = oIID->ToString();
          warn("fetchXPObject: IID is %s\n", ist);
          nsMemory::Free(ist);
#endif
          retIID = new nsIID;
          *retIID = *oIID;
          *theIID = retIID;
        } else {
          *theIID = nsnull;
        }
      } else {
        *theIID = nsnull;
      }
      //item = sv_2mortal(item); // Mortalize the return so we don't leak.
    } else {
      *theIID = nsnull;
    }
  }
  // Similar for the actual object, however we just increment the 
  // reference count on the object.
  if(theObject) {
    hvres = hv_fetch(theHV, "__xpobject", 10, 0);
    if(hvres) {
      item = *hvres;
      if(sv_isobject(item) && sv_derived_from(item, "rawISupports")) {
        oObject = (nsISupports *)SvIV((SV*)SvRV(item));
#ifdef XP_PERL_DEBUG
        warn("fetchXPObject: Extracted nsISupports pointer.\n");
#endif
        NS_ADDREF(oObject);
        *theObject = oObject;
      } else {
        *theObject = nsnull;
      }
      //item = sv_2mortal(item);
    } else {
      *theObject = nsnull;
    }
  }
  return NS_OK;
}

/** This function takes a CID, an IID, and an nsISupports object and 
  * returns a blessed object encapsulating them. The returned SV is not 
  * mortal, so the caller should address that in its code. *ALL* arguments 
  * can take nulls. All ID pointers have their values copied. All nsISupports 
  * objects will have their reference count incremented.
  *
  * @param cid An optional CID pointer.
  * @param iid An optional IID pointer.
  * @param obj An optional nsISupports pointer.
  */
SV *perlXPUtil::makeXPObject(nsCID *cid, nsIID *iid, nsISupports *obj) {
    HV *retObj, *stash;
    SV *anSV, *retval;
    nsIID *newiid;
    nsCID *newcid;
    
#ifdef XP_PERL_DEBUG
    warn("in perlXPUtil::makeXPObject\n");
#endif
    retObj = newHV();
    if(cid) {
#ifdef XP_PERL_DEBUG
      warn("Has a CID\n");
#endif
      anSV = sv_newmortal();
      newcid = new nsCID;
      *newcid = *cid;
      sv_setref_pv(anSV, "nsID", (void *)newcid);
      SvREFCNT_inc(anSV);
      hv_store(retObj, "__cid", 5, anSV, 0);
    }
    if(iid) {
#ifdef XP_PERL_DEBUG
      warn("Has an IID\n");
#endif
      anSV = sv_newmortal();
      newiid = new nsIID;
      *newiid = *iid;
      sv_setref_pv(anSV, "nsID", (void *)newiid);
      SvREFCNT_inc(anSV);
      hv_store(retObj, "__iid", 5, anSV, 0);
    }
    if(obj) {
#ifdef XP_PERL_DEBUG
      warn("Has an object\n");
#endif
      anSV = sv_newmortal();
      NS_ADDREF(obj);
      // rawISupports is a dummy container. It only has a DESTROY method, 
      // which will drop the reference count on the nsISupports object it 
      // holds.
      sv_setref_pv(anSV, "rawISupports", (void *)obj);
      SvREFCNT_inc(anSV);
      hv_store(retObj, "__xpobject", 10, anSV, 0);
    }
    // Promote our hash to an RV and bless it into the nsISupports package
    stash = gv_stashpv("XPCOM::nsISupports", TRUE);
    retval = sv_2mortal((SV *)sv_bless((SV*)newRV_noinc((SV*)retObj), stash));
#ifdef XP_PERL_DEBUG
    warn("Created new XPCOM object with refcnt of %d\n",
          SvREFCNT(retval));
    warn("The Blessed HV in this object has a refcnt of %d\n", 
          SvREFCNT(retObj));
#endif
    return retval;
}

/**
 * Takes the given interface and a name for a potential constant and looks it 
 * up. If the constant is found a pointer to the nsXPTConstant representing 
 * the constant is returned through idx and the function will return PR_TRUE.
 * As with most nsIInterfaceInfo, the pointer to the nsXPTConstant is *not* 
 * dynamically allocated.
 */
PRBool perlXPUtil::lookupConstant(nsIInterfaceInfo *iFace,
                                  const char *name,
                                  const nsXPTConstant **idx) {
  PRUint16 cTot = 0, x;
  const nsXPTConstant *constData;
  
  iFace->GetConstantCount(&cTot);
  for(x = 0;x < cTot;x++) {
    if(NS_SUCCEEDED(iFace->GetConstant(x, &constData))) {
      if(nsCRT::strncmp(name, constData->GetName(), nsCRT::strlen(name))==0) {
        *idx = constData;
        return PR_TRUE;
      }
    }
  }
  return PR_FALSE;
}

/**
 * Takes a pointer to an nsXPTConstant and returns a mortal SV suitable for 
 * placement on the return stack. Success or failure is logged into the 
 * result arg.
 */
SV *perlXPUtil::constantToSV(const nsXPTConstant *theConstant, PRBool *result) {
  SV *mySV = sv_newmortal();
  const nsXPTCMiniVariant *constData = theConstant->GetValue();
  switch(theConstant->GetType().TagPart()) {
    case nsXPTType::T_I8:
      sv_setiv(mySV, (IV)constData->val.i8);
      break;
    case nsXPTType::T_I16:
      sv_setiv(mySV, (IV)constData->val.i16);
      break;
    case nsXPTType::T_I32:
      sv_setiv(mySV, (IV)constData->val.i32);
      break;
    case nsXPTType::T_I64:
      sv_setiv(mySV, (IV)constData->val.i64);
      break;
    case nsXPTType::T_U8:
      sv_setuv(mySV, (UV)constData->val.u8);
      break;
    case nsXPTType::T_U16:
      sv_setuv(mySV, (UV)constData->val.u16);
      break;
    case nsXPTType::T_U32:
      sv_setuv(mySV, (UV)constData->val.u32);
      break;
    case nsXPTType::T_U64:
      sv_setuv(mySV, (UV)constData->val.u64);
      break;
    case nsXPTType::T_FLOAT:
      sv_setnv(mySV, (double)constData->val.f);
      break;
    case nsXPTType::T_DOUBLE:
      sv_setnv(mySV, (double)constData->val.d);
      break;
    case nsXPTType::T_BOOL:
      mySV = boolSV(constData->val.b);
      break;
    case nsXPTType::T_CHAR:
      sv_setpvn(mySV, (char *)&constData->val.c, 1);
      break;
    case nsXPTType::T_WCHAR:
      sv_setiv(mySV, (IV)constData->val.wc);
      break;
    case nsXPTType::T_IID: {
      nsIID *rtIID = (nsIID *)constData->val.p;
      char *rciid = rtIID->ToString();
      sv_setpv(mySV, rciid);
      nsMemory::Free(rciid);
      nsMemory::Free(rtIID);
      break;
    }
    case nsXPTType::T_CHAR_STR: 
      sv_setpv(mySV, (char *)constData->val.p);
      nsMemory::Free(constData->val.p);
      break;
    // XXX - I am explicitly converting any Unicode to a C string because 
    // while perl claims Unicode support, it doesn't behave like it. At least, 
    // not in all versions.
    case nsXPTType::T_WCHAR_STR: {
      sv_setpv(mySV, NS_ConvertUCS2toUTF8((PRUnichar *)constData->val.p).get());
      //nsString toc((PRUnichar *)constData->val.p);
      //char *dc = toc.ToNewCString();
      //sv_setpv(mySV, (char *)dc);
      //nsMemory::Free(dc);
      nsMemory::Free(constData->val.p);
      break;
    }
    
  }
  // Any error returns will be dealt with in the cases of the switch 
  // statement, so if we got here we're ok.
  *result = PR_TRUE;
  return mySV;
}

PRUint32 perlXPUtil::getSizeOfType(nsXPTType& theType) {
  switch(theType.TagPart()) {
    case nsXPTType::T_I8:
      return sizeof(PRInt8);
      break;
    case nsXPTType::T_I16:
      return sizeof(PRInt16);
      break;
    case nsXPTType::T_I32:
      return sizeof(PRInt32);
      break;
    case nsXPTType::T_I64:
      return sizeof(PRInt64);
      break;
    case nsXPTType::T_U8:
      return sizeof(PRUint8);
      break;
    case nsXPTType::T_U16:
      return sizeof(PRUint16);
      break;
    case nsXPTType::T_U32:
      return sizeof(PRUint32);
      break;
    case nsXPTType::T_U64:
      return sizeof(PRUint64);
      break;
    case nsXPTType::T_FLOAT:
      return sizeof(float);
      break;
    case nsXPTType::T_DOUBLE:
      return sizeof(double);
      break;
    case nsXPTType::T_BOOL:
      return sizeof(PRBool);
      break;
    case nsXPTType::T_CHAR:
      return sizeof(char);
      break;
    case nsXPTType::T_WCHAR:
      return sizeof(PRUnichar);
      break;
    case nsXPTType::T_IID:
      return sizeof(nsID);
      break;
    case nsXPTType::T_DOMSTRING:
      return sizeof(nsString);
      break;
    case nsXPTType::T_CHAR_STR:
      return sizeof(char*);
      break;
    case nsXPTType::T_WCHAR_STR:
      return sizeof(PRUnichar *);
      break;
    case nsXPTType::T_INTERFACE:
      return sizeof(nsISupports*);
      break;
  }
  return 0;
}

void *perlXPUtil::convertSVToArray(SV *avRef,
                                   PRUint16 dimension,
                                   nsIInterfaceInfo *iface,
                                   nsXPTParamInfo *theParam,
                                   PRUint16 methodIndex,
                                   nsresult *res) {
  if(!dimension) { // A 0 element array just goes back null
    *res = NS_OK;
    return nsnull;
  }
  if(!avRef || !iface || !theParam) {
    if(res) {
      *res = NS_ERROR_NULL_POINTER;
    }
    return nsnull;
  }
  if(SvROK(avRef)) {
    if(SvTYPE(SvRV((SV*)avRef)) != SVt_PVAV) {
      *res = NS_ERROR_ABORT;
      return nsnull;
    }
  } else {
    *res = NS_ERROR_ABORT;
    return nsnull;
  }
  nsXPTType elementType;
  void *result = nsnull;
  nsresult rv = NS_OK;
  PRUint16 x;
  SV *element = nsnull;
  AV *theArray = (AV*)SvRV(avRef);

  // XXX - What's up with the dimension field? Does it refer to the array 
  // dimension as in a single or multidimensional array (even though 
  // multidimensional arrays are not currently supported according to the 
  // ArrayTypeDescriptor docs), or the number of elements in the array?
  //
  // Well, the number of elements in the array doesn't work as a valid arg 
  // in the dimension field, so I'm confused...
  rv = iface->GetTypeForParam(methodIndex, theParam, 1, &elementType);
  if(NS_FAILED(rv)) {
    *res = rv;
    return nsnull;
  }

  PRUint32 elementSize = perlXPUtil::getSizeOfType(elementType);
  if(!elementSize) {
    warn("Failed to get size of type.\n");
    *res = NS_ERROR_FAILURE;
    return nsnull;
  }
  result = nsMemory::Alloc(elementSize * dimension);

  for(x = 0;x < dimension;x++) {
    element = *av_fetch(theArray, x, 0);
    switch(elementType.TagPart()) {
      case nsXPTType::T_I8: {
        PRInt8 *t_i8 = (PRInt8 *)result;
        perlXPUtil::ConvertSVToInt8(element, (t_i8+x));
        break;
      }
      case nsXPTType::T_I16: {
        PRInt16 *t_i16 = (PRInt16 *)result;
        perlXPUtil::ConvertSVToInt16(element, (t_i16+x));
        break;
      }
      case nsXPTType::T_I32: {
        PRInt32 *t_i32 = (PRInt32 *)result;
        perlXPUtil::ConvertSVToInt32(element, (t_i32+x));
        break;
      }
      case nsXPTType::T_I64: {
        PRInt64 *t_i64 = (PRInt64 *)result;
        perlXPUtil::ConvertSVToInt64(element, (t_i64+x));
        break;
      }
      case nsXPTType::T_U8: {
        PRUint8 *t_u8 = (PRUint8 *)result;
        perlXPUtil::ConvertSVToUint8(element, (t_u8+x));
        break;
      }
      case nsXPTType::T_U16: {
        PRUint16 *t_u16 = (PRUint16 *)result;
        perlXPUtil::ConvertSVToUint16(element, (t_u16+x));
        break;
      }
      case nsXPTType::T_U32: {
        PRUint32 *t_u32 = (PRUint32 *)result;
        perlXPUtil::ConvertSVToUint32(element, (t_u32+x));
        break;
      }
      case nsXPTType::T_U64: {
        PRUint64 *t_u64 = (PRUint64 *)result;
        perlXPUtil::ConvertSVToUint64(element, (t_u64+x));
        break;
      }
      case nsXPTType::T_FLOAT: {
        float *t_f = (float *)result;
        perlXPUtil::ConvertSVToFloat(element, (t_f+x));
        break;
      }
      case nsXPTType::T_DOUBLE: {
        double *t_d = (double *)result;
        perlXPUtil::ConvertSVToDouble(element, (t_d+x));
        break;
      }
      case nsXPTType::T_BOOL: {
        PRBool *t_b = (PRBool *)result;
        perlXPUtil::ConvertSVToBool(element, (t_b+x));
        break;
      }
      case nsXPTType::T_CHAR: {
        char *t_c = (char *)result;
        perlXPUtil::ConvertSVToChar(element, (t_c+x));
        break;
      }
      case nsXPTType::T_WCHAR: {
        PRUnichar *t_wc = (PRUnichar *)result;
        perlXPUtil::ConvertSVToWChar(element, (t_wc+x));
        break;
      }
      case nsXPTType::T_IID: {
        nsID *t_id = (nsID *)result;
        perlXPUtil::ConvertSVToID(element, (t_id+x));
        break;
      }
      case nsXPTType::T_CHAR_STR: {
        char **t_cstr = (char **)result;
        // XXX - Individual strings must be freed, same for wchar strings.
        perlXPUtil::ConvertSVToString(element, PR_FALSE, &(t_cstr[x]));
        break;
      }
      case nsXPTType::T_WCHAR_STR: {
        PRUnichar **t_wstr = (PRUnichar **)result;
        perlXPUtil::ConvertSVToWString(element, &(t_wstr[x]));
        break;
      }
      case nsXPTType::T_INTERFACE: {
        nsISupports **t_iface = (nsISupports **)result;
        // XXX - Make sure this gets released!
        fetchXPObject(element, nsnull, nsnull, &(t_iface[x]));
        break;
      }
    }
  }
  *res = rv;
  return result;
}

SV *perlXPUtil::convertArrayToSV(void *arrPtr,
                                 PRUint16 dimension,
                                 nsIInterfaceInfo *iface,
                                 nsXPTParamInfo *theParam,
                                 PRUint16 methodIndex,
                                 nsresult *res) {
  if(!arrPtr || !iface || !theParam) {
    *res = NS_ERROR_NULL_POINTER;
    return nsnull;
  }
  AV *theArray = newAV();
  SV *refArray = nsnull;
  PRUint16 x;
  SV *theSV = nsnull;
  nsresult rv = NS_OK;
  nsXPTType elementType;

  rv = iface->GetTypeForParam(methodIndex, theParam, 1, &elementType);
  if(NS_FAILED(rv)) {
    *res = rv;
    return nsnull;
  }

  av_extend(theArray, dimension);

  for(x = 0;x < dimension; x++) {
    theSV = sv_newmortal();
    switch(elementType.TagPart()) {
      case nsXPTType::T_U8: {
        PRUint8 *t_u8 = NS_STATIC_CAST(PRUint8 *, arrPtr);
        rv = perlXPUtil::ConvertUint8ToSV(t_u8[x], &theSV);
        break;
      }
      case nsXPTType::T_U16: {
        PRUint16 *t_u16 = NS_STATIC_CAST(PRUint16 *, arrPtr);
        rv = perlXPUtil::ConvertUint16ToSV(t_u16[x], &theSV);
        break;
      }
      case nsXPTType::T_U32: {
        PRUint32 *t_u32 = NS_STATIC_CAST(PRUint32 *, arrPtr);
        rv = perlXPUtil::ConvertUint32ToSV(t_u32[x], &theSV);
        break;
      }
      case nsXPTType::T_U64: {
        PRUint64 *t_u64 = NS_STATIC_CAST(PRUint64 *, arrPtr);
        rv = perlXPUtil::ConvertUint64ToSV(t_u64[x], &theSV);
        break;
      }
      case nsXPTType::T_I8: {
        PRInt8 *t_i8 = NS_STATIC_CAST(PRInt8 *,arrPtr);
        rv = perlXPUtil::ConvertInt8ToSV(t_i8[x], &theSV);
        break;
      }
      case nsXPTType::T_I16: {
        PRInt16 *t_i16 = NS_STATIC_CAST(PRInt16 *, arrPtr);
        rv = perlXPUtil::ConvertInt16ToSV(t_i16[x], &theSV);
        break;
      }
      case nsXPTType::T_I32: {
        PRInt32 *t_i32 = NS_STATIC_CAST(PRInt32 *, arrPtr);
        rv = perlXPUtil::ConvertInt32ToSV(t_i32[x], &theSV);
        break;
      }
      case nsXPTType::T_I64: {
        PRInt64 *t_i64 = NS_STATIC_CAST(PRInt64 *, arrPtr);
        rv = perlXPUtil::ConvertInt64ToSV(t_i64[x], &theSV);
        break;
      }
      case nsXPTType::T_FLOAT: {
        float *t_f = NS_STATIC_CAST(float *, arrPtr);
        rv = perlXPUtil::ConvertFloatToSV(t_f[x], &theSV);
        break;
      }
      case nsXPTType::T_DOUBLE: {
        double *t_d = NS_STATIC_CAST(double *, arrPtr);
        rv = perlXPUtil::ConvertDoubleToSV(t_d[x], &theSV);
        break;
      }
      case nsXPTType::T_BOOL: {
        PRBool *t_b = NS_STATIC_CAST(PRBool *, arrPtr);
        rv = perlXPUtil::ConvertBoolToSV(t_b[x], &theSV);
        break;
      }
      case nsXPTType::T_CHAR: {
        char *t_c = NS_STATIC_CAST(char *, arrPtr);
        rv = perlXPUtil::ConvertCharToSV(t_c[x], &theSV);
        break;
      }
      case nsXPTType::T_WCHAR: {
        PRUnichar *t_wc = NS_STATIC_CAST(PRUnichar *, arrPtr);
        rv = perlXPUtil::ConvertWCharToSV(t_wc[x], &theSV);
        break;
      }
      case nsXPTType::T_IID: {
        nsID **t_iid = NS_STATIC_CAST(nsID**, arrPtr);
        rv = perlXPUtil::ConvertIDToSV(*t_iid[x], &theSV);
        break;
      }
      // These last three are assuming the method is a proper getter and 
      // cleaning up, they need to include appropriate checking.
      case nsXPTType::T_CHAR_STR: {
        char **t_cstr = NS_STATIC_CAST(char **, arrPtr);
        rv = perlXPUtil::ConvertStringToSV(t_cstr[x], PR_FALSE, &theSV);
        nsMemory::Free(t_cstr[x]);
        break;
      }
      case nsXPTType::T_WCHAR_STR: {
        PRUnichar **t_str = NS_STATIC_CAST(PRUnichar **, arrPtr);
        rv = perlXPUtil::ConvertWStringToSV(t_str[x], &theSV);
        nsMemory::Free(t_str[x]);
        break;
      }
      case nsXPTType::T_INTERFACE: {
        nsISupports **t_iptr = NS_STATIC_CAST(nsISupports **, arrPtr);
        nsIID *t_iidptr = nsnull;
        // This is a proper getter
        rv = iface->GetIIDForParam(methodIndex, theParam, &t_iidptr);
        if(NS_SUCCEEDED(rv)) {
          rv = perlXPUtil::ConvertISupportsToSV(t_iptr[x], t_iidptr, nsnull, &theSV);
          nsMemory::Free(t_iidptr);
          NS_IF_RELEASE(t_iptr[x]);
        }
        break;
      }
    }
    if(NS_SUCCEEDED(rv)) {
      SvREFCNT_inc(theSV);
      av_store(theArray, x, theSV);
    }
  }
  refArray = sv_2mortal(newRV((SV*)theArray));
  *res = NS_OK;
  return refArray;
}

nsresult perlXPUtil::fetchIIDFromSV(SV *theIID, nsIID& anIID) {
  nsIInterfaceInfoManager *iim = XPTI_GetInterfaceInfoManager();
  nsCOMPtr<nsIInterfaceInfo> theIFace = nsnull;
  nsIID *IIDptr = nsnull;
  const char *csptr = nsnull;

  if(SvGMAGICAL(theIID)) {
    SvGETMAGIC(theIID);
  }
  if(SvROK(theIID)) {
    IIDptr = (nsIID*)SvIV((SV*)SvRV(theIID));
    anIID = *IIDptr;
  } else {
    if(!SvPOK(theIID)) {
      NS_RELEASE(iim);
      return NS_ERROR_FAILURE;
    }
    warn("Warning: String specified IIDs are deprecated.\n"
         "Use %%Components::interfaces.\n");
    csptr = (const char *)SvPV(theIID, PL_na);
    anIID.Parse(csptr);
    if(NS_FAILED(iim->GetInfoForIID(&anIID, getter_AddRefs(theIFace)))) {
      if(NS_FAILED(iim->GetIIDForName(csptr, &IIDptr))) {
        NS_IF_RELEASE(iim);
        return NS_ERROR_FAILURE;
      } else {
        anIID = *IIDptr;
        nsMemory::Free(IIDptr);
      }
    }
  }
  NS_IF_RELEASE(iim);
  return NS_OK;
}

nsresult perlXPUtil::GetInterfaceInfo(nsIInterfaceInfoManager *iim,
                             const nsIID& iid,
                             nsIInterfaceInfo **iface) {
  nsresult res = NS_OK;
  nsIInterfaceInfo *retval = nsnull;
  PRBool scriptable = PR_FALSE;
  
  res = iim->GetInfoForIID(&iid, &retval);
  if(NS_FAILED(res)) {
    return res;
  }
  retval->IsScriptable(&scriptable);
  if(scriptable == PR_FALSE) {
    NS_IF_RELEASE(retval);
    return NS_ERROR_NOT_AVAILABLE;
  }
  *iface = retval;
  return NS_OK;
}

nsresult perlXPUtil::GetMethodInfo(nsIInterfaceInfo *iface, 
                          const char *methName,
                          PRUint16 inputCount,
                          PRUint16 *methIndex,
                          const nsXPTMethodInfo **info) {
  if(!methIndex || !iface || !methName || !info) {
    return NS_ERROR_NULL_POINTER;
  }
  *methIndex = 0;
  *info = nsnull;
  
  nsXPTMethodInfo *meth = nsnull;
  PRUint16 x, totMethods;
  nsresult res = NS_OK;
  const char *curMethName = nsnull;
  PRUint32 inlen, curlen;
  
  iface->GetMethodCount(&totMethods);
  for(x = 0; x < totMethods; x++) {
    res = iface->GetMethodInfo(x, (const nsXPTMethodInfo **)&meth);
    if(NS_FAILED(res)) {
      return res;
    }
    if(!meth->IsNotXPCOM() && !meth->IsHidden()) {
      curMethName = meth->GetName();
      inlen = nsCRT::strlen(methName);
      curlen = nsCRT::strlen(curMethName);
      if(inlen == curlen) {
        if(nsCRT::strncmp(methName, curMethName, inlen) == 0) {
          if(meth->IsGetter()) {
            if(inputCount == 0) {
              *info = meth;
              *methIndex = x;
              return NS_OK;
            }
          } else if(meth->IsSetter()) {
            if(inputCount > 0) {
              *info = meth;
              *methIndex = x;
              return NS_OK;
            }
          } else {
            *info = meth;
            *methIndex = x;
            return NS_OK;
          }
        }
      }
    }
  }
  return NS_ERROR_FAILURE;
}
