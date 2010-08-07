/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or 
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include "npapi.h"
#include "npruntime.h"

#define MIME_VERSION "application/x-personal-version-test"
#define MIME_AUTHENTICATION "application/x-personal-authentication-test"
#define MIME_SIGNER "application/x-personal-signer2-test"

#define NO_FILE_EXTENSIONS ""

enum PluginType {
    PT_Version,
    PT_Authentication,
    PT_Signer,
};

enum PluginError {
    PE_OK = 0,
    PE_UnknownError = 1, // Maybe this is used for something else in the original plugin?
    PE_BadData =          8001,
    PE_UserCancel =       8002,
    PE_UnknownParam =     8004,
    PE_InvalidAction =    8008,
    PE_InvalidPIN =       8013,
    PE_NotSSL =           8015,
    PE_MissingParameter = 8016,
    PE_BadCharValue =     8017,
    PE_TooLongValue =     8018,
    PE_HostnameIsIP =     8019,
    PE_BlockedPIN =       8102
};

union PluginInfo {
    struct {
        /* Input parameters */
        char *challenge;
        char *policys;
        char *subjectFilter;
        void *dummy0, *dummy1; // To be compatible with .sign below
        /* Output parameters */
        char *signature;
    } auth;
    struct {
        /* Input parameters */
        char *nonce;
        char *policys;
        char *subjectFilter;
        char *message;
        char *invisibleMessage;
        /* Output parameters */
        char *signature;
    } sign;
};

class FriBIDPlugin
{
private:
    NPP m_pNPInstance;
    NPObject *m_pScriptableObject;
    NPWindow *m_pWindow;
    PluginType m_eType;
    PluginError m_eLastError;
    PluginInfo m_Info;
    char *m_sUrl;

    char **GetInfoPointer(const char *name, bool set, int *maxLength);
    char *GetWindowProperty(const char *const identifiers[]);
    bool isHttps();

public:
    FriBIDPlugin(NPMIMEType pluginType, NPP pNPInstance);
    ~FriBIDPlugin();

    PluginInfo GetInfo() { return this->m_Info; };
    PluginType GetType() { return this->m_eType; };
    PluginError GetLastError() { return this->m_eLastError; };

    NPBool init(NPWindow* pNPWindow);
  
    NPObject *GetScriptableObject();

    const char *GetVersion();
    const char *GetParam(const char *name);
    bool SetParam(const char *name, const char *value);
    bool PerformAction(const char *action);
    void SetError(PluginError error);
};

#endif // __PLUGIN_H__
