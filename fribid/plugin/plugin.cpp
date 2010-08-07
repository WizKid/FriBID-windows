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

//////////////////////////////////////////////////
//
// FriBIDPlugin class implementation
//
#ifdef XP_WIN
#include <windows.h>
#include <windowsx.h>
#endif

#ifdef XP_MAC
#include <TextEdit.h>
#endif

#ifdef XP_UNIX
#include <string.h>
#endif

#include "../common/biderror.h"
#include "plugin.h"
#include "npfunctions.h"

static NPIdentifier sGetVersion_id;
static NPIdentifier sGetParam_id;
static NPIdentifier sSetParam_id;
static NPIdentifier sPerformAction_id;
static NPIdentifier sGetLastError_id;


char *AllocString(const char *str)
{
    char* npOutString = (char *)NPN_MemAlloc(strlen(str) + 1);
    if (!npOutString)
        return false;
    strcpy(npOutString, str);
    return npOutString;
}

char *AllocStringLower(const char *str) {
    char *lower = AllocString(str);
    for (int i = 0; i < strlen(str); i++)
        lower[i] = tolower(str[i]);
    return lower;
}


// Helper class that can be used to map calls to the NPObject hooks
// into virtual methods on instances of classes that derive from this
// class.
class ScriptablePluginObjectBase : public NPObject
{
public:
  ScriptablePluginObjectBase(NPP npp)
    : m_pNpp(npp)
  {
  }

  virtual ~ScriptablePluginObjectBase()
  {
  }

  // Virtual NPObject hooks called through this base class. Override
  // as you see fit.
  virtual void Invalidate();
  virtual bool HasMethod(NPIdentifier name);
  virtual bool Invoke(NPIdentifier name, const NPVariant *args,
                      uint32_t argCount, NPVariant *result);
  virtual bool InvokeDefault(const NPVariant *args, uint32_t argCount,
                             NPVariant *result);
  virtual bool HasProperty(NPIdentifier name);
  virtual bool GetProperty(NPIdentifier name, NPVariant *result);
  virtual bool SetProperty(NPIdentifier name, const NPVariant *value);
  virtual bool RemoveProperty(NPIdentifier name);
  virtual bool Enumerate(NPIdentifier **identifier, uint32_t *count);
  virtual bool Construct(const NPVariant *args, uint32_t argCount,
                         NPVariant *result);

public:
  static void _Deallocate(NPObject *npobj);
  static void _Invalidate(NPObject *npobj);
  static bool _HasMethod(NPObject *npobj, NPIdentifier name);
  static bool _Invoke(NPObject *npobj, NPIdentifier name,
                      const NPVariant *args, uint32_t argCount,
                      NPVariant *result);
  static bool _InvokeDefault(NPObject *npobj, const NPVariant *args,
                             uint32_t argCount, NPVariant *result);
  static bool _HasProperty(NPObject * npobj, NPIdentifier name);
  static bool _GetProperty(NPObject *npobj, NPIdentifier name,
                           NPVariant *result);
  static bool _SetProperty(NPObject *npobj, NPIdentifier name,
                           const NPVariant *value);
  static bool _RemoveProperty(NPObject *npobj, NPIdentifier name);
  static bool _Enumerate(NPObject *npobj, NPIdentifier **identifier,
                         uint32_t *count);
  static bool _Construct(NPObject *npobj, const NPVariant *args,
                         uint32_t argCount, NPVariant *result);

protected:
  NPP m_pNpp;
};

#define DECLARE_NPOBJECT_CLASS_WITH_BASE(_class, ctor)                        \
static NPClass s##_class##_NPClass = {                                        \
  NP_CLASS_STRUCT_VERSION_CTOR,                                               \
  ctor,                                                                       \
  ScriptablePluginObjectBase::_Deallocate,                                    \
  ScriptablePluginObjectBase::_Invalidate,                                    \
  ScriptablePluginObjectBase::_HasMethod,                                     \
  ScriptablePluginObjectBase::_Invoke,                                        \
  ScriptablePluginObjectBase::_InvokeDefault,                                 \
  ScriptablePluginObjectBase::_HasProperty,                                   \
  ScriptablePluginObjectBase::_GetProperty,                                   \
  ScriptablePluginObjectBase::_SetProperty,                                   \
  ScriptablePluginObjectBase::_RemoveProperty,                                \
  ScriptablePluginObjectBase::_Enumerate,                                     \
  ScriptablePluginObjectBase::_Construct                                      \
}

#define GET_NPOBJECT_CLASS(_class) &s##_class##_NPClass

void
ScriptablePluginObjectBase::Invalidate()
{
}

bool
ScriptablePluginObjectBase::HasMethod(NPIdentifier name)
{
    return false;
}

bool
ScriptablePluginObjectBase::Invoke(NPIdentifier name, const NPVariant *args,
                                   uint32_t argCount, NPVariant *result)
{
    return false;
}

bool
ScriptablePluginObjectBase::InvokeDefault(const NPVariant *args,
                                          uint32_t argCount, NPVariant *result)
{
  return false;
}

bool
ScriptablePluginObjectBase::HasProperty(NPIdentifier name)
{
  return false;
}

bool
ScriptablePluginObjectBase::GetProperty(NPIdentifier name, NPVariant *result)
{
  return false;
}

bool
ScriptablePluginObjectBase::SetProperty(NPIdentifier name,
                                        const NPVariant *value)
{
  return false;
}

bool
ScriptablePluginObjectBase::RemoveProperty(NPIdentifier name)
{
  return false;
}

bool
ScriptablePluginObjectBase::Enumerate(NPIdentifier **identifier,
                                      uint32_t *count)
{
  return false;
}

bool
ScriptablePluginObjectBase::Construct(const NPVariant *args, uint32_t argCount,
                                      NPVariant *result)
{
  return false;
}

// static
void
ScriptablePluginObjectBase::_Deallocate(NPObject *npobj)
{
  // Call the virtual destructor.
  delete (ScriptablePluginObjectBase *)npobj;
}

// static
void
ScriptablePluginObjectBase::_Invalidate(NPObject *npobj)
{
  ((ScriptablePluginObjectBase *)npobj)->Invalidate();
}

// static
bool
ScriptablePluginObjectBase::_HasMethod(NPObject *npobj, NPIdentifier name)
{
  return ((ScriptablePluginObjectBase *)npobj)->HasMethod(name);
}

// static
bool
ScriptablePluginObjectBase::_Invoke(NPObject *npobj, NPIdentifier name,
                                    const NPVariant *args, uint32_t argCount,
                                    NPVariant *result)
{
  return ((ScriptablePluginObjectBase *)npobj)->Invoke(name, args, argCount,
                                                       result);
}

// static
bool
ScriptablePluginObjectBase::_InvokeDefault(NPObject *npobj,
                                           const NPVariant *args,
                                           uint32_t argCount,
                                           NPVariant *result)
{
  return ((ScriptablePluginObjectBase *)npobj)->InvokeDefault(args, argCount,
                                                              result);
}

// static
bool
ScriptablePluginObjectBase::_HasProperty(NPObject * npobj, NPIdentifier name)
{
  return ((ScriptablePluginObjectBase *)npobj)->HasProperty(name);
}

// static
bool
ScriptablePluginObjectBase::_GetProperty(NPObject *npobj, NPIdentifier name,
                                         NPVariant *result)
{
  return ((ScriptablePluginObjectBase *)npobj)->GetProperty(name, result);
}

// static
bool
ScriptablePluginObjectBase::_SetProperty(NPObject *npobj, NPIdentifier name,
                                         const NPVariant *value)
{
  return ((ScriptablePluginObjectBase *)npobj)->SetProperty(name, value);
}

// static
bool
ScriptablePluginObjectBase::_RemoveProperty(NPObject *npobj, NPIdentifier name)
{
  return ((ScriptablePluginObjectBase *)npobj)->RemoveProperty(name);
}

// static
bool
ScriptablePluginObjectBase::_Enumerate(NPObject *npobj,
                                       NPIdentifier **identifier,
                                       uint32_t *count)
{
  return ((ScriptablePluginObjectBase *)npobj)->Enumerate(identifier, count);
}

// static
bool
ScriptablePluginObjectBase::_Construct(NPObject *npobj, const NPVariant *args,
                                       uint32_t argCount, NPVariant *result)
{
  return ((ScriptablePluginObjectBase *)npobj)->Construct(args, argCount,
                                                          result);
}


class ScriptablePluginObject : public ScriptablePluginObjectBase
{
private:
    FriBIDPlugin *GetPlugin();

public:
  ScriptablePluginObject(NPP npp)
    : ScriptablePluginObjectBase(npp)
  {
  }

  virtual bool HasMethod(NPIdentifier name);
  virtual bool HasProperty(NPIdentifier name);
  virtual bool GetProperty(NPIdentifier name, NPVariant *result);
  virtual bool SetProperty(NPIdentifier name, const NPVariant *value);
  virtual bool Invoke(NPIdentifier name, const NPVariant *args,
                      uint32_t argCount, NPVariant *result);
  virtual bool InvokeDefault(const NPVariant *args, uint32_t argCount,
                             NPVariant *result);
};

static NPObject *
AllocateScriptablePluginObject(NPP npp, NPClass *aClass)
{
  return new ScriptablePluginObject(npp);
}

DECLARE_NPOBJECT_CLASS_WITH_BASE(ScriptablePluginObject,
                                 AllocateScriptablePluginObject);

FriBIDPlugin *
ScriptablePluginObject::GetPlugin()
{
    if (m_pNpp == NULL)
        return false;

    return (FriBIDPlugin *)m_pNpp->pdata;
}

bool
ScriptablePluginObject::HasMethod(NPIdentifier name)
{
    FriBIDPlugin *plugin = this->GetPlugin();
    if (plugin == NULL)
        return false;

    switch (plugin->GetType()) {
        case PT_Version:
            return name == sGetVersion_id;
        case PT_Authentication:
        case PT_Signer:
            return name == sGetParam_id || name == sSetParam_id ||
                   name == sPerformAction_id || name == sGetLastError_id;
    }

    return false;
}

bool
ScriptablePluginObject::HasProperty(NPIdentifier name)
{
    return false;
}

bool
ScriptablePluginObject::GetProperty(NPIdentifier name, NPVariant *result)
{
    return false;
}

bool
ScriptablePluginObject::SetProperty(NPIdentifier name, const NPVariant *value)
{
    return false;
}

bool
ScriptablePluginObject::Invoke(NPIdentifier name, const NPVariant *args,
                               uint32_t argCount, NPVariant *result)
{
    FriBIDPlugin *plugin = this->GetPlugin();
    if (plugin == NULL)
        return false;

    switch (plugin->GetType()) {
        case PT_Version:
            if (name == sGetVersion_id) {
                if (argCount != 0)
                    return false;

                const char* ret = plugin->GetVersion();
                if (!ret)
                    return false;

                STRINGZ_TO_NPVARIANT(AllocString(ret), *result);
                return true;
            }
            return false;
        case PT_Authentication:
        case PT_Signer:
            if (name == sGetParam_id) {
                if (argCount != 1 || !NPVARIANT_IS_STRING(args[0]))
                    return false;

                char *param = AllocStringLower(NPVARIANT_TO_STRING(args[0]).UTF8Characters);
                const char *ret = plugin->GetParam(param);
                NPN_MemFree(param);

                if (ret) {
                    STRINGZ_TO_NPVARIANT(AllocString(ret), *result);
                } else {
                    NULL_TO_NPVARIANT(*result);
                }
                return true;
            }
            if (name == sSetParam_id) {
                if (argCount != 2 || !NPVARIANT_IS_STRING(args[0]) ||
                    !NPVARIANT_IS_STRING(args[0]))
                    return false;

                char *param = AllocStringLower(NPVARIANT_TO_STRING(args[0]).UTF8Characters);
                plugin->SetParam(param, NPVARIANT_TO_STRING(args[1]).UTF8Characters);
                NPN_MemFree(param);

                INT32_TO_NPVARIANT((int32_t)plugin->GetLastError(), *result);
                return true;
            }
            if (name == sPerformAction_id) {
                if (argCount != 1 || !NPVARIANT_IS_STRING(args[0]))
                    return false;

                char *action = AllocStringLower(NPVARIANT_TO_STRING(args[0]).UTF8Characters);
                int ret = plugin->PerformAction(action);
                NPN_MemFree(action);

                INT32_TO_NPVARIANT((int32_t)ret, *result);
                return true;
            }
            if (name == sGetLastError_id) {
                if (argCount != 0)
                    return false;

                int ret = plugin->GetLastError();

                INT32_TO_NPVARIANT((int32_t)ret, *result);
                return true;
            }
            return false;
    }

    return false;
}

bool
ScriptablePluginObject::InvokeDefault(const NPVariant *args, uint32_t argCount,
                                      NPVariant *result)
{
    return false;
}

FriBIDPlugin::FriBIDPlugin(NPMIMEType pluginType, NPP pNPInstance) :
    m_pNPInstance(pNPInstance),
    m_pScriptableObject(NULL),
    m_pWindow(NULL)
{
    if (strcmp(MIME_VERSION, pluginType) == 0) {
        m_eType = PT_Version;
    } else if (strcmp(MIME_AUTHENTICATION, pluginType) == 0) {
        m_eType = PT_Authentication;
    } else if (strcmp(MIME_SIGNER, pluginType) == 0) {
        m_eType = PT_Signer;
    }

    // Set default values
    this->m_Info.sign.nonce = AllocString("");
    this->m_Info.sign.policys = AllocString("");
    this->m_Info.sign.subjectFilter = AllocString("");
    this->m_Info.sign.message = AllocString("");
    this->m_Info.sign.invisibleMessage = AllocString("");
    this->m_Info.sign.signature = AllocString("");

    sGetVersion_id = NPN_GetStringIdentifier("GetVersion");
    sGetParam_id = NPN_GetStringIdentifier("GetParam");
    sSetParam_id = NPN_GetStringIdentifier("SetParam");
    sPerformAction_id = NPN_GetStringIdentifier("PerformAction");
    sGetLastError_id = NPN_GetStringIdentifier("GetLastError");
}

FriBIDPlugin::~FriBIDPlugin()
{
    if (m_pScriptableObject)
        NPN_ReleaseObject(m_pScriptableObject);
}

NPBool FriBIDPlugin::init(NPWindow* pNPWindow)
{
    if(pNPWindow == NULL)
        return false;

    m_pWindow = pNPWindow;

    return true;
}

const char *FriBIDPlugin::GetVersion()
{
    return "Personal_exe=4.10.4.3&persinst_exe=4.10.4.3&tokenapi_dll=4.10.4.2&personal_dll=4.10.4.2&np_prsnl_dll=4.10.4.3&lng_svse_dll=4.10.4.3&lng_frfr_dll=4.10.4.3&crdsiem_dll=4.10.4.3&crdsetec_dll=4.10.4.3&crdprism_dll=4.10.4.3&br_svse_dll=1.5.0.5&br_enu_dll=1.5.0.5&branding_dll=1.5.0.5&CSP_INSTALLED=TRUE&Personal=4.10.4.3&platform=win32&os_version=winvista&best_before=1283547033&";
}

char **FriBIDPlugin::GetInfoPointer(const char *name, bool set, int &maxLength)
{
    switch(this->m_eType) {
        case PT_Authentication:
            if (strcmp(name, "challenge") == 0)
                return &this->m_Info.auth.challenge;
            if (strcmp(name, "policys") == 0)
                return &this->m_Info.auth.policys;
            if (strcmp(name, "subjects") == 0)
                return &this->m_Info.auth.subjectFilter;
            if (!set && strcmp(name, "signature") == 0)
                return &this->m_Info.auth.signature;
            break;
        case PT_Signer:
            if (strcmp(name, "nonce") == 0)
                return &this->m_Info.sign.nonce;
            if (strcmp(name, "texttobesigned") == 0)
                return &this->m_Info.sign.message;
            if (strcmp(name, "nonvisibledata") == 0)
                return &this->m_Info.sign.invisibleMessage;
            if (strcmp(name, "policys") == 0)
                return &this->m_Info.sign.policys;
            if (strcmp(name, "subjects") == 0)
                return &this->m_Info.sign.subjectFilter;
            if (!set && strcmp(name, "signature") == 0)
                return &this->m_Info.sign.signature;
            break;
    }

    return NULL;
}

const char *FriBIDPlugin::GetParam(const char *name)
{
    char **valuePtr = this->GetInfoPointer(name, false);

    if (valuePtr == NULL || *valuePtr == NULL) {
        this->m_eLastError = PE_UnknownParam;
        return NULL;
    }

    this->m_eLastError = PE_OK;
    return *valuePtr;
}

bool FriBIDPlugin::SetParam(const char *name, const char *value)
{
    char **valuePtr = this->GetInfoPointer(name, true);

    if (valuePtr == NULL) {
        this->m_eLastError = PE_UnknownParam;
        return false;
    }

    // Validate that the value is a base64 string. So valid characters are A-Z, a-z, 0-9 and + /
    // and it could end with = or ==.
    // BUG: We should check the length first but Nexus does it in this order
    // BUG: Nexus allow the =-character in the last two positions. Even if "aa=a" isn't a valid base64 string
    int len = strlen(value);
    for (int i = 0; i < len; i++) {
        const char c = value[i];
        if (!(c == 43 || (c >= 47 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122) || (c == 61 && i >= len -2))) {
            this->m_eLastError = PE_BadCharValue;
            return false;
        }
    }

    // TODO: 5462 was tested with signer and Nonce. Other params could have other sizes
    if (len > 5462) {
        this->m_eLastError = PE_TooLongValue;
        return false;
    }

    if (!*valuePtr)
        NPN_MemFree(*valuePtr);

    *valuePtr = AllocString(value);
    if (*valuePtr == NULL) {
        this->m_eLastError = PE_UnknownError; // TODO: What error should it really be?
        return false;
    }

    this->m_eLastError = PE_OK;
    return true;
}

int FriBIDPlugin::PerformAction(const char *action)
{
    this->m_eLastError = PE_UnknownError;
    switch(this->m_eType) {
        case PT_Authentication:
            if (strcmp(action, "authenticate") != 0)
                break;

            if (!this->m_Info.auth.challenge || this->m_Info.auth.challenge[0] == '\0')
                return BIDERR_MissingParameter;

            return 0;
        case PT_Signer:
            if (strcmp(action, "sign") != 0)
                break;

            if (!this->m_Info.sign.nonce || this->m_Info.sign.nonce[0] == '\0' ||
                !this->m_Info.sign.message || this->m_Info.sign.message[0] == '\0')
                return BIDERR_MissingParameter;

            return 0;
    }
    return BIDERR_InvalidAction;
}

void FriBIDPlugin::SetError(PluginError error)
{
    this->m_eLastError = error;
}

NPObject *
FriBIDPlugin::GetScriptableObject()
{
    if (!m_pScriptableObject) {
        m_pScriptableObject = NPN_CreateObject(m_pNPInstance,
            GET_NPOBJECT_CLASS(ScriptablePluginObject));
    }

    if (m_pScriptableObject)
        NPN_RetainObject(m_pScriptableObject);

    return m_pScriptableObject;
}
