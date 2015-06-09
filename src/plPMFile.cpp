/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

/* plPMFile
 *
 * Abstraction of a Dll. Stores modifiedTime and size for easy detection of
 * change in dll.
 *
 * dp Suresh <dp@netscape.com>
 */

#include "plPMFile.h"
#include "nsDebug.h"
#include "nsIComponentManager.h"
#include "nsIModule.h"
#include "nsILocalFile.h"
#include "nsCOMPtr.h"
#include "nsCRT.h"
#include "nsReadableUtils.h"
#include "nsNativeComponentLoader.h"
#if defined(VMS) && defined(DEBUG)
#include <lib$routines.h>
#include <ssdef.h>
#endif

plPMFile::plPMFile(const char *codeDllName, int type)
  : m_dllName(NULL),
    m_instance(NULL), m_status(PMFILE_OK), m_moduleObject(NULL),
    m_persistentDescriptor(NULL), m_nativePath(NULL),
    m_markForUnload(PR_FALSE), m_registryLocation(0)

{
	m_modDate = LL_Zero();
	m_size = LL_Zero();
	
    if (!codeDllName || !*codeDllName)
    {
        m_status = PMFILE_INVALID_PARAM;
        return;
    }
    m_dllName = nsCRT::strdup(codeDllName);
    if (!m_dllName)
    {
        m_status = PMFILE_NO_MEM;
        return;
    }
}

plPMFile::plPMFile(nsIFile *dllSpec, const char *registryLocation)
  : m_dllName(NULL),
    m_instance(NULL), m_status(PMFILE_OK), m_moduleObject(NULL),
    m_persistentDescriptor(NULL), m_nativePath(NULL), m_markForUnload(PR_FALSE)

{
	m_modDate = LL_Zero();
	m_size = LL_Zero();
    m_dllSpec = dllSpec;

    m_registryLocation = nsCRT::strdup(registryLocation);
    Init(dllSpec);
    // Populate m_modDate and m_size
    if (NS_FAILED(Sync()))
    {
        m_status = PMFILE_INVALID_PARAM;
    }
}

plPMFile::plPMFile(nsIFile *dllSpec, const char *registryLocation, PRInt64* modDate, PRInt64* fileSize)
  : m_dllName(NULL),
    m_instance(NULL), m_status(PMFILE_OK), m_moduleObject(NULL),
    m_persistentDescriptor(NULL), m_nativePath(NULL), m_markForUnload(PR_FALSE)

{
    m_modDate = LL_Zero();
    m_size = LL_Zero();
    m_dllSpec = dllSpec;

    m_registryLocation = nsCRT::strdup(registryLocation);
    Init(dllSpec);

    if (modDate)
        m_modDate = *modDate;
    else
        m_modDate = LL_Zero();
    
    if (fileSize)
        m_size = *fileSize;
    else
        m_size = LL_Zero();

}

plPMFile::plPMFile(const nsAString& libPersistentDescriptor)
  : m_dllName(NULL),
    m_instance(NULL), m_status(PMFILE_OK), m_moduleObject(NULL),
    m_persistentDescriptor(NULL), m_nativePath(NULL),
    m_markForUnload(PR_FALSE), m_registryLocation(0)

{
    m_modDate = LL_Zero();
    m_size = LL_Zero();

    Init(libPersistentDescriptor);
    // Populate m_modDate and m_size
    if (NS_FAILED(Sync()))
    {
        m_status = PMFILE_INVALID_PARAM;
    }
}

plPMFile::plPMFile(const nsAString& libPersistentDescriptor, PRInt64* modDate, PRInt64* fileSize)
  : m_dllName(NULL),
    m_instance(NULL), m_status(PMFILE_OK), m_moduleObject(NULL),
    m_persistentDescriptor(NULL), m_nativePath(NULL),
    m_markForUnload(PR_FALSE), m_registryLocation(0)

{
    m_modDate = LL_Zero();
    m_size = LL_Zero();

    Init(libPersistentDescriptor);

    // and overwrite the modData and fileSize
	
    if (modDate)
        m_modDate = *modDate;
    else
        m_modDate = LL_Zero();
    
    if (fileSize)
        m_size = *fileSize;
    else
        m_size = LL_Zero();
}
 
void
plPMFile::Init(nsIFile *dllSpec)
{
    // Addref the m_dllSpec
    m_dllSpec = dllSpec;

    // Make sure we are dealing with a file
    PRBool isFile = PR_FALSE;
    nsresult rv = m_dllSpec->IsFile(&isFile);
    if (NS_FAILED(rv))
    {
        m_status = PMFILE_INVALID_PARAM;
        return;
    }

    if (isFile == PR_FALSE)
    {
      // Not a file. Cant work with it.
      m_status = PMFILE_NOT_FILE;
      return;
    }
    
	m_status = PMFILE_OK;			
}

void
plPMFile::Init(const nsAString& libPersistentDescriptor)
{
    nsresult rv;
    m_modDate = LL_Zero();
    m_size = LL_Zero();
	
	if (libPersistentDescriptor.IsEmpty())
	{
		m_status = PMFILE_INVALID_PARAM;
		return;
	}

    // Create a FileSpec from the persistentDescriptor
    nsCOMPtr<nsILocalFile> dllSpec;
    
    nsCID clsid;
    nsComponentManager::ContractIDToClassID(NS_LOCAL_FILE_CONTRACTID, &clsid);
    rv = nsComponentManager::CreateInstance(clsid, nsnull, 
                                            NS_GET_IID(nsILocalFile),
                                            (void**)getter_AddRefs(dllSpec));
    if (NS_FAILED(rv))
    {
        m_status = PMFILE_INVALID_PARAM;
        return;
    }

    rv = dllSpec->InitWithPath(libPersistentDescriptor);
    if (NS_FAILED(rv))
    {
        m_status = PMFILE_INVALID_PARAM;
        return;
    }

}


plPMFile::~plPMFile(void)
{
#if 0
    // The dll gets deleted when the dllStore is destroyed. This happens on
    // app shutdown. At that point, unloading dlls can cause crashes if we have
    // - dll dependencies
    // - callbacks
    // - static dtors
    // Hence turn it back on after all the above have been removed.
    Unload();
#endif
    if (m_dllName)
        nsCRT::free(m_dllName);
    if (m_persistentDescriptor)
        nsCRT::free(m_persistentDescriptor);
    if (m_nativePath)
        nsCRT::free(m_nativePath);
    if (m_registryLocation)
        nsCRT::free(m_registryLocation);

}

nsresult
plPMFile::Sync()
{
    if (!m_dllSpec)
        return NS_ERROR_FAILURE;

    // Populate m_modDate and m_size
    nsresult rv = m_dllSpec->GetLastModifiedTime(&m_modDate);
    if (NS_FAILED(rv)) return rv;
    rv = m_dllSpec->GetFileSize(&m_size);
    return rv;
}


const char *
plPMFile::GetDisplayPath()
{
    if (m_dllName)
        return m_dllName;
    if (m_nativePath)
        return m_nativePath;
    nsString path;
    m_dllSpec->GetPath(path);
    m_nativePath = ToNewCString(path);
    return m_nativePath;
}

const char *
plPMFile::GetPersistentDescriptorString()
{
    if (m_dllName)
        return m_dllName;
    if (m_persistentDescriptor)
        return m_persistentDescriptor;
    nsString path;
    m_dllSpec->GetPath(path);
    m_persistentDescriptor = ToNewCString(path);
    return m_persistentDescriptor;
}

PRBool
plPMFile::HasChanged()
{
    if (m_dllName)
        return PR_FALSE;

    // If mod date has changed, then dll has changed
    PRInt64 currentDate;

    nsresult rv = m_dllSpec->GetLastModifiedTime(&currentDate);
    
    if (NS_FAILED(rv) || LL_NE(currentDate, m_modDate))
      return PR_TRUE;

    // If size has changed, then dll has changed
    PRInt64 aSize;
    rv = m_dllSpec->GetFileSize(&aSize);
    if (NS_FAILED(rv) || LL_NE(aSize, m_size))
      return PR_TRUE;

    return PR_FALSE;
}




PRBool plPMFile::Load(void)
{
	if (m_status != PMFILE_OK)
	{
		return (PR_FALSE);
	}
	if (m_instance != NULL)
	{
		// Already loaded
		return (PR_TRUE);
	}

    if (m_dllSpec)
    {
        nsCOMPtr<nsILocalFile> localFile = do_QueryInterface(m_dllSpec);

#ifdef NS_BUILD_REFCNT_LOGGING
        nsTraceRefcnt::SetActivityIsLegal(PR_FALSE);
#endif
        if (localFile)
            localFile->Load(&m_instance);
        
#ifdef NS_BUILD_REFCNT_LOGGING
        nsTraceRefcnt::SetActivityIsLegal(PR_TRUE);
        if (m_instance) {
            // Inform refcnt tracer of new library so that calls through the
            // new library can be traced.
            nsString path;
            char* displayPath;
            m_dllSpec->GetPath(path);
            displayPath = ToNewCString(path);
            nsTraceRefcnt::LoadLibrarySymbols(displayPath, m_instance);
            nsMemory::Free(displayPath);
        }
#endif
    }
    else if (m_dllName)
    {
        // if there is not an nsIFile, but there is a dll name, just try to load that..
#ifdef NS_BUILD_REFCNT_LOGGING
        nsTraceRefcnt::SetActivityIsLegal(PR_FALSE);
#endif
        m_instance = PR_LoadLibrary(m_dllName);

#ifdef NS_BUILD_REFCNT_LOGGING
        nsTraceRefcnt::SetActivityIsLegal(PR_TRUE);
        if (m_instance) {
            // Inform refcnt tracer of new library so that calls through the
            // new library can be traced.
            nsTraceRefcnt::LoadLibrarySymbols(m_dllName, m_instance);
        }
#endif    
    }

#if defined(DEBUG) && defined(XP_UNIX)
    // Debugging help for components. Component dlls need to have their
    // symbols loaded before we can put a breakpoint in the debugger.
    // This will help figureing out the point when the dll was loaded.
    BreakAfterLoad(GetDisplayPath());
#endif

    return ((m_instance == NULL) ? PR_FALSE : PR_TRUE);
}

PRBool plPMFile::Unload(void)
{
	if (m_status != PMFILE_OK || m_instance == NULL)
		return (PR_FALSE);

    // Shutdown the dll
    Shutdown();

#ifdef NS_BUILD_REFCNT_LOGGING
    nsTraceRefcnt::SetActivityIsLegal(PR_FALSE);
#endif
	PRStatus ret = PR_UnloadLibrary(m_instance);
#ifdef NS_BUILD_REFCNT_LOGGING
    nsTraceRefcnt::SetActivityIsLegal(PR_TRUE);
#endif

	if (ret == PR_SUCCESS)
	{
		m_instance = NULL;
		return (PR_TRUE);
	}
	else
		return (PR_FALSE);
}

void * plPMFile::FindSymbol(const char *symbol)
{
	if (symbol == NULL)
		return (NULL);
	
	// If not already loaded, load it now.
	if (Load() != PR_TRUE)
		return (NULL);

    return(PR_FindSymbol(m_instance, symbol));
}


// Component dll specific functions
nsresult plPMFile::GetDllSpec(nsIFile **fsobj)
{
    NS_ASSERTION(m_dllSpec, "m_dllSpec NULL");
    NS_ASSERTION(fsobj, "plPMFile::GetModule : Null argument" );

    *fsobj = m_dllSpec;
    NS_ADDREF(*fsobj);
    return NS_OK;
}

nsresult plPMFile::GetModule(nsISupports *servMgr, nsIModule **cobj)
{
    nsIComponentManager *compMgr;
    nsresult rv = NS_GetGlobalComponentManager(&compMgr);
    NS_ASSERTION(compMgr, "Global Component Manager is null" );
    if (NS_FAILED(rv)) return rv;

    NS_ASSERTION(cobj, "plPMFile::GetModule : Null argument" );

    if (m_moduleObject)
    {
        NS_ADDREF(m_moduleObject);
        *cobj = m_moduleObject;
        return NS_OK;
    }

	// If not already loaded, load it now.
	if (Load() != PR_TRUE) return NS_ERROR_FAILURE;

    // We need a nsIFile for location. If we dont
    // have one, create one.
    if (!m_dllSpec && m_dllName)
    {
        // Create m_dllSpec from m_dllName
    }

    nsGetModuleProc proc =
      (nsGetModuleProc) FindSymbol(NS_GET_MODULE_SYMBOL);

    if (proc == NULL)
        return NS_ERROR_FACTORY_NOT_LOADED;

    rv = (*proc) (compMgr, m_dllSpec, &m_moduleObject);
    if (NS_SUCCEEDED(rv))
    {
        NS_ADDREF(m_moduleObject);
        *cobj = m_moduleObject;
    }
    return rv;
}

#if defined(DEBUG) && !defined(XP_BEOS)
#define SHOULD_IMPLEMENT_BREAKAFTERLOAD
#endif

// These are used by BreakAfterLoad, below.
#ifdef SHOULD_IMPLEMENT_BREAKAFTERLOAD
static nsCString *sBreakList[16];
static int sBreakListCount = 0;
#endif

nsresult plPMFile::Shutdown(void)
{
    // Release the module object if we got one
    nsrefcnt refcnt;
    if (m_moduleObject)
    {
        NS_RELEASE2(m_moduleObject, refcnt);
        NS_ASSERTION(refcnt == 0, "Dll moduleObject refcount non zero");
    }
#ifdef SHOULD_IMPLEMENT_BREAKAFTERLOAD
    for (int i = 0; i < sBreakListCount; i++)
    {
        delete sBreakList[i];
        sBreakList[i] = nsnull;
    }
    sBreakListCount = 0;
#endif
    return NS_OK;

}

void plPMFile::BreakAfterLoad(const char *nsprPath)
{
#ifdef SHOULD_IMPLEMENT_BREAKAFTERLOAD
    static PRBool firstTime = PR_TRUE;

    // return if invalid input
    if (!nsprPath || !*nsprPath) return;

    // return if nothing to break on
    if (!firstTime && sBreakListCount == 0) return;

    if (firstTime)
    {
        firstTime = PR_FALSE;
        // Form the list of dlls to break on load
        nsCAutoString envList(getenv("XPCOM_BREAK_ON_LOAD"));
        if (envList.IsEmpty()) return;
        PRInt32 ofset = 0;
        PRInt32 start = 0;
        do
        {
            ofset = envList.FindChar(':', PR_TRUE, start);
            sBreakList[sBreakListCount] = new nsCString();
            envList.Mid(*(sBreakList[sBreakListCount]), start, ofset);
            sBreakListCount++;
            start = ofset + 1;
        }
        while (ofset != -1 && 16 > sBreakListCount); // avoiding vc6.0 compiler issue. count < thinks it is starting a template
    }

    // Find the dllname part of the string
    nsCString currentPath(nsprPath);
    PRInt32 lastSep = currentPath.RFindCharInSet(":\\/");

    for (int i=0; i<sBreakListCount; i++)
        if (currentPath.Find(*(sBreakList[i]), PR_TRUE, lastSep) > 0)
        {
            // Loading a dll that we want to break on
            // Put your breakpoint here
            printf("...Loading module %s\n", nsprPath);
            // Break in the debugger here.
#if defined(linux) && defined(__i386)
            asm("int $3");
#elif defined(VMS)
            lib$signal(SS$_DEBUG);
#endif
        }
#endif /* SHOULD_IMPLEMENT_BREAKAFTERLOAD */
    return;
}
