/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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

/* Dll
 *
 * Programmatic representation of a dll. Stores modifiedTime and size for
 * easy detection of change in dll.
 *
 * dp Suresh <dp@netscape.com>
 */

#ifndef plPMFile_h__
#define plPMFile_h__

#include "prio.h"
#include "prlink.h"
#include "nsISupports.h"
#include "nsIFile.h"
#include "nsCOMPtr.h"
#include "nsString.h"

class nsIModule;
class nsIServiceManager;

typedef enum plPMFileStatus
{
	PMFILE_OK = 0,
	PMFILE_NO_MEM = 1,
	PMFILE_STAT_ERROR = 2,
	PMFILE_NOT_FILE = 3,
	PMFILE_INVALID_PARAM = 4
} plPMFileStatus;

class plPMFile
{
private:
    char *m_dllName;			// Stores the dllName to load.

    nsCOMPtr<nsIFile> m_dllSpec;	    // Filespec representing the component
	PRInt64 m_modDate;		// last modified time at creation of this object
	PRInt64 m_size;		// size of the dynamic library at creation of this object

	PRLibrary *m_instance;	// Load instance
	plPMFileStatus m_status;	// holds current status
    nsIModule *m_moduleObject;

    // Cache frequent queries
    char *m_persistentDescriptor;
    char *m_nativePath;

    PRBool m_markForUnload;
    char *m_registryLocation;

    void Init(nsIFile *dllSpec);
    void Init(const nsAString& persistentDescriptor);

    void BreakAfterLoad(const char *nsprPath);

public:
 
	plPMFile(nsIFile *dllSpec, const char *registryLocation);
	plPMFile(nsIFile *dllSpec, const char *registryLocation, PRInt64* modDate, PRInt64* fileSize);
	plPMFile(const nsAString& persistentDescriptor);
	plPMFile(const nsAString& persistentDescriptor, PRInt64* modDate, PRInt64* fileSize);
    plPMFile(const char *dll, int type /* dummy */);

	~plPMFile(void);

    // Sync : Causes update of file information
    nsresult Sync(void);

	// Status checking on operations completed
	plPMFileStatus GetStatus(void) { return (m_status); }

	// Dll Loading
	PRBool Load(void);
	PRBool Unload(void);
	PRBool IsLoaded(void)
	{
		return ((m_instance != 0) ? PR_TRUE : PR_FALSE);
	}
    void MarkForUnload(PRBool mark) { m_markForUnload = mark; }
    PRBool IsMarkedForUnload(void) { return m_markForUnload; }

    // Shutdown the dll. This will call any on unload hook for the dll.
    // This wont unload the dll. Unload() implicitly calls Shutdown().
    nsresult Shutdown(void);

	void *FindSymbol(const char *symbol);
	
    PRBool HasChanged(void);

    // WARNING: DONT FREE string returned.
    const char *GetDisplayPath(void);
    // WARNING: DONT FREE string returned.
    const char *GetPersistentDescriptorString(void);
    // WARNING: DONT FREE string returned.
    const char *GetRegistryLocation(void) { return m_registryLocation; }
	void GetLastModifiedTime(PRInt64 *val) { *val = m_modDate; }
	void GetSize(PRInt64 *val) { *val = m_size; }
	PRLibrary *GetInstance(void) { return (m_instance); }

    // NS_RELEASE() is required to be done on objects returned
    nsresult GetDllSpec(nsIFile **dllSpec);
    nsresult GetModule(nsISupports *servMgr, nsIModule **mobj);
};

#endif /* plPMFile_h__ */
