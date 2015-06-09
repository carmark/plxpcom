# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is the Perl XPCOM language bindings.
#
# The Initial Developer of the Original Code is
# Jumpline.com, Inc. Portions created by Jumpline.com, Inc. are
# Copyright (C) 2001 Jumpline.com, Inc. All Rights Reserved.
#
# Contributor(s):
# Matt Kennedy <matt@jumpline.com> (original author)
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK ***** 

#include "../perlXPUtil.h"
    
MODULE = XPCOM		PACKAGE = XPCOM::nsISupports

void
nsISupports::DESTROY()
  INIT:
    HV *theHV;
  CODE:
    // All subordinate objects in the hash reference have their own 
    // destructors. We let them take care of their own, we just undef the 
    // hash
#ifdef XP_PERL_DEBUG
    warn("Hit Perl destructor for XPCOM::nsISupports object\n");
#endif
    theHV = (HV*)SvRV(ST(0));
    hv_undef(theHV);

void
QueryInterface(...)
  CASE: items == 2
  INIT:
    nsIID iid;
    nsCID tcid, *ocid = nsnull;
    nsIInterfaceInfoManager *iim = XPTI_GetInterfaceInfoManager();
    nsISupports *newIf, *curIf = nsnull;
    nsresult res = 0;
  CODE:
    if(NS_FAILED(perlXPUtil::fetchXPObject(ST(0), &ocid, nsnull, &curIf))) {
      warn("Object is not an nsISupports container.\n");
      VP_SET_RETCODE(NS_ERROR_ABORT);
      VP_RETURN_UNDEF;
    }
    if(!curIf) { // Really all we need is an interface pointer
      warn("Object does not contain all required elements.\n");
      VP_SET_RETCODE(NS_ERROR_ABORT);
      VP_RETURN_UNDEF;
    }
    if(ocid) {
      tcid = *ocid;
      delete ocid;
    }
    res = perlXPUtil::fetchIIDFromSV(ST(1), iid);
    if(NS_FAILED(res)) {
      NS_IF_RELEASE(iim);
      VP_SET_RETCODE(res);
      VP_RETURN_UNDEF;
    }
#ifdef XP_PERL_DEBUG
    char *iidstr = iid.ToString();
    warn("Calling QueryInterface to find %s\n", iidstr);
    nsMemory::Free(iidstr);
#endif
    // Now really query the interface    
    res = curIf->QueryInterface(iid, (void **)&newIf);
    if(NS_FAILED(res)) {
      NS_IF_RELEASE(iim);
      NS_IF_RELEASE(curIf);
      VP_SET_RETCODE(res);
      VP_RETURN_UNDEF;
    }
    ST(0) = perlXPUtil::makeXPObject(&tcid, &iid, newIf);
    NS_IF_RELEASE(iim);
    NS_IF_RELEASE(curIf);
    NS_IF_RELEASE(newIf);
    VP_SET_RETCODE(res);
    XSRETURN(1);
    
    
void
callXPCOMMethod(...)
  CASE: items < 2
    CODE:
#ifdef XP_PERL_DEBUG
      warn("Invalid numbers of arguments.\n");
#endif
      VP_RETURN_UNDEF;
  CASE: items >= 2
    INIT:
      const char *meth = (const char *)SvPV(ST(1), PL_na);
      nsCID cid;
      nsCID *ocid = nsnull;
      nsIID iid;
      nsIID *rtIID, *rtpiid, *oiid = nsnull;
      nsISupports *theObject = nsnull;
      nsIInterfaceInfoManager *iim = XPTI_GetInterfaceInfoManager();
      nsIInterfaceInfo *theIFace;
      const nsXPTMethodInfo *methodInfo;
      PRUint16 methodIndex = 0;
      nsresult res = 0;
      PRUint8 iCount = items - 2;
      PRUint8 pCount, oCount, x, y;
      nsAutoString t_convs;
      const char *t_ctou = nsnull;
      nsCOMPtr<nsISupports> argObj = nsnull;
      nsIID **ifArray = nsnull;
      PRUint32 ifcnt = 0;
    CODE:
#ifdef XP_PERL_DEBUG
      warn("Entering callXPCOMMethod to invoke %s\n", meth);
#endif
      // FIXME - This function is freakin huge and generally sucks ass. I'll 
      // try to elaborate on some of the steps where explanation is 
      // necessary, and hopefully I can get this crap cleaned up in the near 
      // future.
      nsXPTCVariant vars[MAX_XPTC_ARG_COUNT];
      // Grab the native objects stored in the perl hashref.
      if(NS_FAILED(perlXPUtil::fetchXPObject(ST(0),&ocid,&oiid, &theObject))) {
        warn("Error: Unable to obtain elements of XPCOM container.\n");
        NS_IF_RELEASE(iim);
        VP_SET_RETCODE(NS_ERROR_ABORT);
        VP_RETURN_UNDEF;
      }
      if(ocid) {
        cid = *ocid;
        delete ocid;
      }
      if(oiid) {
        iid = *oiid;
        delete oiid;
      } else {
        iid = NS_GET_IID(nsISupports);
      }
      // Lookup the information for this object's advertised interface 
      // with the interface info manager.
#ifdef XP_PERL_DEBUG
      char *tcd = cid.ToString();
      warn("CID is %s\n", tcd);
      nsMemory::Free(tcd);
      warn("Doing method %s\n", meth); 
#endif
      res = perlXPUtil::GetInterfaceInfo(iim, iid, &theIFace);
      if(NS_FAILED(res)) {
        NS_IF_RELEASE(theObject);
        NS_IF_RELEASE(iim);
        VP_SET_RETCODE(res);
        VP_RETURN_UNDEF;
      }
      // XXX nsIInterfaceInfo methods do not make copies for results
      // This will probably get called repeatedly in another sub once I get 
      // interface flattening via nsIClassInfo implemented.
      res = perlXPUtil::GetMethodInfo(   theIFace,meth,iCount,
                                            &methodIndex,&methodInfo);
      
      // XXX - If we haven't found a method, then check if the name we got 
      // is a constant and if so return the constant's value. I'm not 
      // doing flattening support for constants too, use the 
      // $Components::interfaces hash for that.
      // If the above failed, we check to see if the object can provide a 
      // class info interface from which we can find a usable method. This 
      // allows for interface flattening.
      if(NS_FAILED(res)) {
        const nsXPTConstant *constPtr = nsnull;
        if(perlXPUtil::lookupConstant(theIFace, meth, &constPtr)) {
          PRBool cRes = PR_FALSE;
          ST(0) = perlXPUtil::constantToSV(constPtr, &cRes);
          NS_IF_RELEASE(theObject);
          NS_IF_RELEASE(iim);
          NS_IF_RELEASE(theIFace);
          if(cRes) {
            XSRETURN(1);
          } else {
            VP_RETURN_UNDEF;
          }
        } else {
          NS_IF_RELEASE(theIFace);
          res = perlXPUtil::GetInterfaceList(theObject, &ifcnt, &ifArray);

          if(ifcnt) {
            res = perlXPUtil::FindMethod(iim, ifcnt, ifArray, meth, iCount,
                                &methodIndex, &methodInfo, &theIFace);
            while(ifcnt-- > 0) {
              nsMemory::Free(ifArray[ifcnt]);
            }
            nsMemory::Free(ifArray);
          }
        }
      }
      if(NS_FAILED(res)) {
          warn("Error: Unknown method %s().\n", meth);
          NS_IF_RELEASE(theObject);
          NS_IF_RELEASE(iim);
          NS_IF_RELEASE(theIFace);
          VP_SET_RETCODE(NS_ERROR_NOT_AVAILABLE);
          VP_RETURN_UNDEF;
      }
      // Get parameter count. There's undoubtedly a more elegant way to do 
      // this...
      pCount = methodInfo->GetParamCount();
      if(pCount > MAX_XPTC_ARG_COUNT) {
        warn("Error: method has too many parameters.\n");
        NS_IF_RELEASE(theObject);
        NS_IF_RELEASE(iim);
        NS_IF_RELEASE(theIFace);
        VP_SET_RETCODE(NS_ERROR_UNEXPECTED);
        VP_RETURN_UNDEF;
      }
#ifdef XP_PERL_DEBUG
      warn("Method has %d parameters.\n", pCount);
#endif
      // Tally up the number of outputs.
      oCount = 0;
      if(pCount) {
        nsXPTParamInfo paramInfo_(methodInfo->GetParam(0));
        for(x = 0; x < pCount; x++) {
          paramInfo_ = methodInfo->GetParam(x);
          if(paramInfo_.IsOut() || paramInfo_.IsRetval()) {
            oCount++;
          }
        }
      }
#ifdef XP_PERL_DEBUG
      warn("Method has %d output parameters.\n", oCount);
#endif
      // Now see if the number of args we got jives with the number of 
      // inputs the method wants
      if(iCount < (pCount - oCount)) {
        warn("Error: To few arguments for %s()\n", meth);
        NS_IF_RELEASE(theObject);
        NS_IF_RELEASE(iim);
        NS_IF_RELEASE(theIFace);
        VP_SET_RETCODE(NS_ERROR_NULL_POINTER);
        VP_RETURN_UNDEF;
      }
      if(iCount > (pCount - oCount)) {
        warn("Error: To many arguments for %s()\n", meth);
        NS_IF_RELEASE(theObject);
        NS_IF_RELEASE(iim);
        NS_IF_RELEASE(theIFace);
        VP_SET_RETCODE(NS_ERROR_UNEXPECTED);
        VP_RETURN_UNDEF;
      }
      // And so begins the enormous mass of switch statements...
      // Ok, put the args together.
      for(x = 0; x < pCount; x++) {
        nsXPTParamInfo paramInfo = methodInfo->GetParam(x);
        vars[x].type = paramInfo.GetType();
#ifdef XP_PERL_DEBUG
        warn("Processing param %d of %d\n", x, pCount);
#endif
        if(paramInfo.IsIn() && !paramInfo.IsRetval()) {
#ifdef XP_PERL_DEBUG
          warn("Param %d is an input.\n", x);
#endif
          switch(vars[x].type.TagPart()) {
            case nsXPTType::T_I8:
              vars[x].val.i8 = (PRInt8)SvIV(ST(x+2));
              vars[x].flags = 0;
              break;
            case nsXPTType::T_I16:
              vars[x].val.i16 = (PRInt16)SvIV(ST(x+2));
              vars[x].flags = 0;
              break;
            case nsXPTType::T_I32: 
              vars[x].val.i32 = (PRInt32)SvIV(ST(x+2));
              vars[x].flags = 0;
              break;
            case nsXPTType::T_I64: 
              vars[x].val.i64 = (PRInt64)SvIV(ST(x+2));
              vars[x].flags = 0;
              break;
            case nsXPTType::T_U8:
              vars[x].val.u8 = (PRUint8)SvUV(ST(x+2));
              vars[x].flags = 0;
              break;
            case nsXPTType::T_U16:
              vars[x].val.u16 = (PRUint16)SvUV(ST(x+2));
              vars[x].flags = 0;
              break;
            case nsXPTType::T_U32:
              vars[x].val.u32 = (PRUint32)SvUV(ST(x+2));
              vars[x].flags = 0;
              break;
            case nsXPTType::T_U64:
              vars[x].val.u64 = (PRUint64)SvUV(ST(x+2));
              vars[x].flags = 0;
              break;
            case nsXPTType::T_FLOAT:
              vars[x].val.f = (float)SvNV(ST(x+2));
              vars[x].flags = 0;
              break;
            case nsXPTType::T_DOUBLE:
              vars[x].val.d = (double)SvNV(ST(x+2));
              vars[x].flags = 0;
              break;
            case nsXPTType::T_BOOL:
              vars[x].val.b = (PRBool)SvIV(ST(x+2));
              vars[x].flags = 0;
              break;
            case nsXPTType::T_CHAR:
              vars[x].val.c = (char)*SvPV(ST(x+2), PL_na);
              vars[x].flags = 0;
              break;
            case nsXPTType::T_WCHAR:
              vars[x].val.wc = (PRUnichar)SvIV(ST(x+2));
              vars[x].flags = 0;
              break;
            case nsXPTType::T_VOID:
              vars[x].val.p = (void *)SvIV(ST(x+2));
              vars[x].ptr = vars[x].val.p;
              vars[x].flags = nsXPTCVariant::PTR_IS_DATA;
              break;
            case nsXPTType::T_IID: {
              nsIID tmpIID;
              nsIID *tmppiid = nsnull;
              if(SvPOK(ST(x+2))) {
                warn("Warning: Passing strings for an IID is deprecated.\n");
                char *tistr = (char *)SvPV(ST(x+2), PL_na);
                nsIInterfaceInfo *tiface = nsnull;
                // Verify that this is an interface, otherwise assume it's a
                // name and look it up. If both fail let the error handling of
                // the real call deal with it. We just want to allow for 
                // Scripting to specify interfaces by name.
                if(NS_FAILED(iim->GetIIDForName(tistr, &tmppiid))) {
                  tmpIID.Parse(tistr);
                  if(NS_FAILED(iim->GetInfoForIID(&tmpIID, &tiface))) {
                    tmpIID = NS_GET_IID(nsISupports);
                  }
                } else {
                  tmpIID = *tmppiid;
                  nsMemory::Free(tmppiid);
                }
                vars[x].val.p = (void *)&tmpIID;
                vars[x].ptr = vars[x].val.p;
                vars[x].flags = nsXPTCVariant::PTR_IS_DATA;
              } else if(SvROK(ST(x+2))) { // nsID blessed object
                tmppiid = (nsIID *)SvIV((SV*)SvRV(ST(x+2)));
                vars[x].val.p = (void *)tmppiid;
                vars[x].ptr = vars[x].val.p;
                vars[x].flags = nsXPTCVariant::PTR_IS_DATA;
              } else {
                warn("Error: Unrecognized value given for expected IID type\n");
                vars[x].val.p = nsnull;
                vars[x].ptr = vars[x].val.p;
                vars[x].flags = nsXPTCVariant::PTR_IS_DATA;
              }
              break;
            }
            case nsXPTType::T_DOMSTRING: {
              char *t_sptr = (char *)SvPV(ST(x+2), PL_na);
              vars[x].val.p = new nsString(
                      SvUTF8(ST(x+2)) ? 
                      NS_ConvertUTF8toUCS2(t_sptr).get() :
                      NS_ConvertASCIItoUCS2(t_sptr).get());
              vars[x].ptr = vars[x].val.p;
              vars[x].flags |= nsXPTCVariant::VAL_IS_DOMSTR;
              break;
            }
            case nsXPTType::T_PSTRING_SIZE_IS: /* fall through */
            case nsXPTType::T_CHAR_STR: /* fall through */
            {
              SV* target = ST(x+2);
              if(SvUTF8(target))
              {
                  target = sv_newmortal();
                  sv_setsv(target, ST(x+2));
                  sv_utf8_downgrade(target, TRUE);
              }
              vars[x].val.p = (void *)SvPV_nolen(target);
              vars[x].ptr = vars[x].val.p;
              vars[x].flags = nsXPTCVariant::PTR_IS_DATA;
              break;
            }
            case nsXPTType::T_PWSTRING_SIZE_IS: /* fall through */
            case nsXPTType::T_WCHAR_STR:
            {
              if(SvUTF8(ST(x+2)))
                  vars[x].val.p = ToNewUnicode(
                        NS_ConvertUTF8toUCS2(SvPV_nolen(ST(x+2))));
              else
                  vars[x].val.p = ToNewUnicode(
                        NS_ConvertASCIItoUCS2(SvPV_nolen(ST(x+2))));
              vars[x].ptr = vars[x].val.p;
              vars[x].flags = nsXPTCVariant::PTR_IS_DATA;
              break;
            }
            case nsXPTType::T_INTERFACE_IS: /* fall through */
            case nsXPTType::T_INTERFACE:
              // XXX - Implement interface checking
              // Multiple uses of argObj will drop the reference count. This 
              // is actually desirable behavior. The reference is to an object 
              // contained in the perl object that called us, so it won't die 
              // during the life of this function.
              if(NS_SUCCEEDED(
                    perlXPUtil::fetchXPObject(ST(x+2),
                                  nsnull,
                                  nsnull,
                                  getter_AddRefs(argObj)))) {
                vars[x].val.p = (void *)argObj;
              } else {
                vars[x].val.p = nsnull;
              }
              vars[x].ptr = vars[x].val.p;
              vars[x].flags = nsXPTCVariant::PTR_IS_DATA;
              break;
            case nsXPTType::T_ARRAY: {
              // Do the following:
              PRUint8 t_szarg = paramInfo.type.argnum;
              if(!t_szarg) {
                NS_IF_RELEASE(theObject);
                NS_IF_RELEASE(iim);
                NS_IF_RELEASE(theIFace);
                VP_SET_RETCODE(NS_ERROR_FAILURE);
                VP_RETURN_UNDEF;
              }
              PRUint16 t_dim = (PRUint16)SvUV(ST(t_szarg+2));
              vars[x].val.p = perlXPUtil::ConvertSVToArray(ST(x+2),
                                                  t_dim,
                                                  theIFace,
                                                  &paramInfo,
                                                  methodIndex,
                                                  &res);
              if(NS_FAILED(res)) {
                warn("Failed to convert SV to an array\n");
                NS_IF_RELEASE(theObject);
                NS_IF_RELEASE(iim);
                NS_IF_RELEASE(theIFace);
                VP_SET_RETCODE(NS_ERROR_FAILURE);
                VP_RETURN_UNDEF;
              }
              vars[x].ptr = vars[x].val.p;
              vars[x].flags |= nsXPTCVariant::VAL_IS_ARRAY;
              // pruint8 = vars[x].type.arnum (size_is arg)
              // get type/value out of param.
              // interfaceInfo->getTypeForParam(methodIndex, paramInfo, len);
              // make array size len of type.
              // Iterate perl array ref and convert elements.
              // Free at end.
              break;
            }
            default:
              warn("Error: Argument %d of %s() is an unsupported type.\n",
                     x,
                     meth);
#ifdef XP_PERL_DEBUG
              warn("Type ID is %d.\n", vars[x].type.TagPart());
#endif
              NS_IF_RELEASE(theObject);
              NS_IF_RELEASE(iim);
              NS_IF_RELEASE(theIFace);
              VP_SET_RETCODE(NS_ERROR_NOT_IMPLEMENTED);
              VP_RETURN_UNDEF;
              break;
          }
        // XXX Start of output argument prep.
        } else if(paramInfo.IsOut() || paramInfo.IsRetval()){ // Param IsOut
#ifdef XP_PERL_DEBUG
          warn("Param %d is an output. type is %u.\n", x,
                  vars[x].type.TagPart());
#endif
          vars[x].ClearFlags();
          vars[x].SetPtrIsData();
          switch(vars[x].type.TagPart()) {
            case nsXPTType::T_I8:
              vars[x].val.i8 = 0;
              vars[x].ptr = &vars[x].val.i8;
              break;
            case nsXPTType::T_I16:
              vars[x].val.i16 = 0;
              vars[x].ptr = &vars[x].val.i16;
              break;
            case nsXPTType::T_I32:
              vars[x].val.i32 = 0;
              vars[x].ptr = &vars[x].val.i32;
              break;
            case nsXPTType::T_I64:
              vars[x].val.i64 = 0;
              vars[x].ptr = &vars[x].val.i64;
              break;
            case nsXPTType::T_U8:
              vars[x].val.u8 = 0;
              vars[x].ptr = &vars[x].val.u8;
              break;
            case nsXPTType::T_U16:
              vars[x].val.u16 = 0;
              vars[x].ptr = &vars[x].val.u16;
              break;
            case nsXPTType::T_U32:
              vars[x].val.u32 = 0;
              vars[x].ptr = &vars[x].val.u32;
              break;
            case nsXPTType::T_U64:
              vars[x].val.u64 = 0;
              vars[x].ptr = &vars[x].val.u64;
              break;
            case nsXPTType::T_FLOAT:
              vars[x].val.f = 0.0;
              vars[x].ptr = &vars[x].val.f;
              break;
            case nsXPTType::T_DOUBLE:
              vars[x].val.d = 0.0;
              vars[x].ptr = &vars[x].val.d;
              break;
            case nsXPTType::T_BOOL:
              vars[x].val.b = 0;
              vars[x].ptr = &vars[x].val.b;
              break;
            case nsXPTType::T_CHAR:
              vars[x].val.c = 0;
              vars[x].ptr = &vars[x].val.c;
              break;
            case nsXPTType::T_WCHAR:
              vars[x].val.wc = 0;
              vars[x].ptr = &vars[x].val.wc;
              break;
            case nsXPTType::T_VOID:
              vars[x].val.p = 0;
              vars[x].ptr = &vars[x].val.p;
              break;
            case nsXPTType::T_IID:  /* fall through */
            case nsXPTType::T_CHAR_STR: /* fall through */
            case nsXPTType::T_WCHAR_STR:
              vars[x].val.p = 0;
              vars[x].SetValIsAllocated();
              vars[x].ptr = &vars[x].val.p;
              break;
            case nsXPTType::T_CSTRING:
              vars[x].SetValIsCString();
              vars[x].ptr = vars[x].val.p = new nsCString();
              if(!vars[x].ptr)
              {
                  NS_IF_RELEASE(theObject);
                  NS_IF_RELEASE(iim);
                  NS_IF_RELEASE(theIFace);
                  VP_SET_RETCODE(NS_ERROR_OUT_OF_MEMORY);
                  VP_RETURN_UNDEF;
              }
              break;
            case nsXPTType::T_UTF8STRING:
              vars[x].SetValIsUTF8String();
              vars[x].ptr = vars[x].val.p = new nsCString();
              if(!vars[x].ptr)
              {
                  NS_IF_RELEASE(theObject);
                  NS_IF_RELEASE(iim);
                  NS_IF_RELEASE(theIFace);
                  VP_SET_RETCODE(NS_ERROR_OUT_OF_MEMORY);
                  VP_RETURN_UNDEF;
              }
              break;
            case nsXPTType::T_DOMSTRING:
            case nsXPTType::T_ASTRING:
              vars[x].flags = nsXPTCVariant::VAL_IS_DOMSTR;
              vars[x].ptr = vars[x].val.p = new nsString();
              if(!vars[x].ptr)
              {
                  NS_IF_RELEASE(theObject);
                  NS_IF_RELEASE(iim);
                  NS_IF_RELEASE(theIFace);
                  VP_SET_RETCODE(NS_ERROR_OUT_OF_MEMORY);
                  VP_RETURN_UNDEF;
              }
              break;
            case nsXPTType::T_INTERFACE_IS: /* fall through */
            case nsXPTType::T_INTERFACE:
              vars[x].val.p = 0;
              vars[x].flags |= nsXPTCVariant::VAL_IS_IFACE;
              vars[x].flags |= nsXPTCVariant::VAL_IS_ALLOCD;
              vars[x].ptr = &vars[x].val.p;
              break;
            case nsXPTType::T_ARRAY:
              vars[x].val.p = nsnull;
              vars[x].flags |= nsXPTCVariant::VAL_IS_ARRAY;
              vars[x].flags |= nsXPTCVariant::VAL_IS_ALLOCD;
              vars[x].ptr = &vars[x].val.p;
              break;
            default:
              warn("Error: Argument %d of %s() is an unsupported type (%u).\n",
                     x,
                     meth,
                     vars[x].type.TagPart());
              NS_IF_RELEASE(theObject);
              NS_IF_RELEASE(iim);
              NS_IF_RELEASE(theIFace);
              VP_SET_RETCODE(NS_ERROR_NOT_IMPLEMENTED);
              VP_RETURN_UNDEF;
              break;
          }
        } else {
         warn("Param %d is neither input or output. WTF!\n", x);
        }
      }
#ifdef XP_PERL_DEBUG
      warn("OK, place the call...\n");
#endif
      // OK, now call the method.
      res = XPTC_InvokeByIndex(theObject, methodIndex, pCount, vars);
#ifdef XP_PERL_DEBUG
      warn("Call done.\n");
#endif
      VP_SET_RETCODE(res);
      if(NS_FAILED(res)) {
        NS_IF_RELEASE(theObject);
        NS_IF_RELEASE(iim);
        NS_IF_RELEASE(theIFace);
        VP_RETURN_UNDEF;
      }

      // The call succeeded, get the outputs if any
      if(oCount) {
        y = 0;
        for(x = 0;x < pCount;x++) {
          nsXPTParamInfo paramInfo = methodInfo->GetParam(x);
          if(paramInfo.IsOut() || paramInfo.IsRetval()) {
            ST(y) = sv_newmortal();
            switch(vars[x].type.TagPart()) {
              case nsXPTType::T_I8:
                sv_setiv(ST(y), (IV)vars[x].val.i8);
                break;
              case nsXPTType::T_I16:
                sv_setiv(ST(y), (IV)vars[x].val.i16);
                break;
              case nsXPTType::T_I32:
                sv_setiv(ST(y), (IV)vars[x].val.i32);
                break;
              case nsXPTType::T_I64:
                sv_setiv(ST(y), (IV)vars[x].val.i64);
                break;
              case nsXPTType::T_U8:
                sv_setuv(ST(y), (UV)vars[x].val.u8);
                break;
              case nsXPTType::T_U16:
                sv_setuv(ST(y), (UV)vars[x].val.u16);
                break;
              case nsXPTType::T_U32:
                sv_setuv(ST(y), (UV)vars[x].val.u32);
                break;
              case nsXPTType::T_U64:
                sv_setuv(ST(y), (UV)vars[x].val.u64);
                break;
              case nsXPTType::T_FLOAT:
                sv_setnv(ST(y), (double)vars[x].val.f);
                break;
              case nsXPTType::T_DOUBLE:
                sv_setnv(ST(y), (double)vars[x].val.d);
                break;
              case nsXPTType::T_BOOL:
                ST(y) = boolSV(vars[x].val.b);
                break;
              case nsXPTType::T_CHAR:
                sv_setpvn(ST(y), (char *)&vars[x].val.c, 1);
                break;
              case nsXPTType::T_WCHAR:
                sv_setiv(ST(y), (IV)vars[x].val.wc);
                break;
              case nsXPTType::T_VOID:
                // Value not copied *don't* free.
                sv_setiv(ST(y), (IV)vars[x].val.p);
                break;
              case nsXPTType::T_IID:
                // Dont delete rtIID, the perl nsID object's destructor will
                // take care of it.
                rtIID = (nsIID *)vars[x].val.p;
                sv_setref_pv(ST(y), "nsID", rtIID);
                break;
              case nsXPTType::T_CSTRING:
              case nsXPTType::T_UTF8STRING:
              {
                  sv_setpv(ST(y), ((nsCString*)vars[x].ptr)->get());
                  if(vars[x].type.TagPart() == nsXPTType::T_UTF8STRING)
                      SvUTF8_on(ST(y));
                  else
                      SvUTF8_off(ST(y));
                  delete vars[x].ptr;
                  break;
              }
              case nsXPTType::T_ASTRING:
              case nsXPTType::T_DOMSTRING:
              {
                sv_setpv(ST(y), 
                         NS_ConvertUCS2toUTF8(*(nsString*)vars[x].ptr).get());
                SvUTF8_on(ST(y));
                delete vars[x].ptr;
                break;
              }
              case nsXPTType::T_CHAR_STR: 
                sv_setpv(ST(y), (char *)vars[x].val.p);
                SvUTF8_off(ST(y));
                nsMemory::Free(vars[x].val.p);
                break;
              case nsXPTType::T_WCHAR_STR: {
                sv_setpv(ST(y), 
                        NS_ConvertUCS2toUTF8((PRUnichar *)vars[x].val.p).get());
                SvUTF8_on(ST(y));
                nsMemory::Free(vars[x].val.p);
                break;
              }
              case nsXPTType::T_INTERFACE_IS: {
#ifdef XP_PERL_DEBUG
                warn("Processing Interface Is output.\n");
#endif
                nsISupports *trodep = (nsISupports *)vars[x].val.p;
                nsXPTParamInfo t_pidep(methodInfo->GetParam(x));
#ifdef XP_PERL_DEBUG
                warn("Interface is dependant on arg %d\n", 
                                t_pidep.type.argnum);
#endif
                PRUint8 iarg = t_pidep.type.argnum;
                if(vars[iarg].type.TagPart() == nsXPTType::T_IID) {
#ifdef XP_PERL_DEBUG
                  warn("Allright..dependent arg is an IID.\n");
#endif
                  if(!vars[iarg].val.p) {
                    warn("Unable to obtain interface for return value.\n");
                    NS_IF_RELEASE(theObject);
                    NS_IF_RELEASE(iim);
                    NS_IF_RELEASE(theIFace);
                    NS_IF_RELEASE(trodep);
                    VP_SET_RETCODE(NS_ERROR_NO_INTERFACE);
                    VP_RETURN_UNDEF;
                  }
                  ST(y) = perlXPUtil::makeXPObject(nsnull, 
                                      (nsIID*)vars[iarg].val.p, 
                                      trodep);
                } else {
                  warn("Unable to obtain interface for return value.\n");
                  NS_IF_RELEASE(theObject);
                  NS_IF_RELEASE(iim);
                  NS_IF_RELEASE(theIFace);
                  NS_IF_RELEASE(trodep);
                  VP_SET_RETCODE(NS_ERROR_NO_INTERFACE);
                  VP_RETURN_UNDEF;
                }
#ifdef XP_PERL_DEBUG
                warn("Popping interface as %dth stack item.\n", y);
#endif
                NS_IF_RELEASE(trodep);
                break;
              }
              case nsXPTType::T_INTERFACE: {
#ifdef XP_PERL_DEBUG
                warn("Processing Interface output.\n");
#endif
                nsISupports *tro = (nsISupports *)vars[x].val.p;
                nsXPTParamInfo t_pi(methodInfo->GetParam(x));
                // This is a getter
                res = theIFace->GetIIDForParam(methodIndex, 
                                               &t_pi,
                                               &rtpiid);
                if(NS_FAILED(res)) {
                  warn("Unable to obtain interface for return value.\n");
                  NS_IF_RELEASE(theObject);
                  NS_IF_RELEASE(iim);
                  NS_IF_RELEASE(theIFace);
                  NS_IF_RELEASE(tro);
                  VP_SET_RETCODE(NS_ERROR_NO_INTERFACE);
                  VP_RETURN_UNDEF;
                }
#ifdef XP_PERL_DEBUG
                warn("Popping interface as %dth stack item.\n", y);
#endif
                ST(y) = perlXPUtil::makeXPObject(nsnull, rtpiid, tro);
                nsMemory::Free(rtpiid);
                NS_IF_RELEASE(tro);
                break;
              }
              case nsXPTType::T_ARRAY: {
                nsXPTParamInfo t_api(methodInfo->GetParam(x));
                nsresult aprv = NS_OK;
                PRUint8 t_sz = paramInfo.type.argnum;
                if(!t_sz) {
                  NS_IF_RELEASE(theObject);
                  NS_IF_RELEASE(iim);
                  NS_IF_RELEASE(theIFace);
                  VP_SET_RETCODE(NS_ERROR_FAILURE);
                  VP_RETURN_UNDEF;
                }
                ST(y) = perlXPUtil::ConvertArrayToSV(
                                            vars[x].val.p,
                                            vars[t_sz].val.u16,
                                            theIFace,
                                            &t_api,
                                            methodIndex,
                                            &aprv);
                if(NS_FAILED(res)) {
                  NS_IF_RELEASE(theObject);
                  NS_IF_RELEASE(iim);
                  NS_IF_RELEASE(theIFace);
                  VP_SET_RETCODE(NS_ERROR_NO_INTERFACE);
                  VP_RETURN_UNDEF;
                }
                nsMemory::Free(vars[x].val.p);
                break;
              }
            }
            y++;
            if(y == oCount) {
              NS_IF_RELEASE(theObject);
              NS_IF_RELEASE(iim);
              NS_IF_RELEASE(theIFace);
              XSRETURN(oCount);
              break;
            }
          }
        }
      }
      // Final cleanup of arguments scratch memory.
      for(x = 0;x < pCount;x++) {
        nsXPTParamInfo paramInfo = methodInfo->GetParam(x);
        if(paramInfo.IsIn()) {
          switch(vars[x].type.TagPart()) {
            case nsXPTType::T_DOMSTRING:
              if(!paramInfo.IsRetval()) {
                delete vars[x].val.p;
              }
              break;
            case nsXPTType::T_PWSTRING_SIZE_IS: /* fall through */
            case nsXPTType::T_WCHAR_STR:
              nsMemory::Free(vars[x].val.p);
              break;
            case nsXPTType::T_ARRAY: {
              PRUint8 t_sz = paramInfo.type.argnum;
              PRUint16 t_dimarr = vars[t_sz].val.u16;
              PRUint16 t_cnt;
              nsXPTType t_elType;
              res = theIFace->GetTypeForParam(methodIndex, 
                                              &paramInfo,
                                              1,
                                              &t_elType);
              // The following types have dynamic allocation in the individual
              // array elements (or are refcounted objects) which need dealt
              // with.
              if(t_elType.TagPart() == nsXPTType::T_CHAR_STR ||
                 t_elType.TagPart() == nsXPTType::T_WCHAR_STR ||
                 t_elType.TagPart() == nsXPTType::T_INTERFACE) {
                for(t_cnt = 0; t_cnt < t_dimarr; t_cnt++) {
                  switch(t_elType.TagPart()) {
                    case nsXPTType::T_CHAR_STR: {
                      char **t_elchar = (char **)vars[x].val.p;
                      nsMemory::Free(t_elchar[t_cnt]);
                      break;
                    }
                    case nsXPTType::T_WCHAR_STR: {
                      PRUnichar **t_elwchar = (PRUnichar **)vars[x].val.p;
                      nsMemory::Free(t_elwchar[t_cnt]);
                      break;
                    }
                    case nsXPTType::T_INTERFACE: {
                      nsISupports **t_eliface = (nsISupports**)vars[x].val.p;
                      NS_IF_RELEASE(t_eliface[t_cnt]);
                      break;
                    }
                  }
                }
              }
              nsMemory::Free(vars[x].val.p);
              break;
            }
          }
        } 
      }
      NS_IF_RELEASE(theObject);
      NS_IF_RELEASE(iim);
      NS_IF_RELEASE(theIFace);
      XSRETURN_EMPTY;
      
