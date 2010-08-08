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

#include "plugin.h"
#include "pluginbase.h"
#include "../common/memory.h"
#include "../bankid/bankid.h"
#include "npfunctions.h"


static NPIdentifier sGetVersion_id;
static NPIdentifier sGetParam_id;
static NPIdentifier sSetParam_id;
static NPIdentifier sPerformAction_id;
static NPIdentifier sGetLastError_id;


string
Lower(const string &input)
{
    string ret(input);
    for (string::iterator it = ret.begin(); it != ret.end(); ++it)
        *it = tolower(*it);
    return ret;
}

string
NPVariantToString(NPVariant variant)
{
    NPString str = NPVARIANT_TO_STRING(variant);
    return string(str.UTF8Characters, str.UTF8Length);
}


Param::Param()
    : mValue(""), mMaxSize(0), mCanSet(false)
{
}

Param::Param(const string &value, int maxSize, bool canSet)
    : mValue(value), mMaxSize(maxSize), mCanSet(canSet)
{
}

bool
Param::canSet(bool force) const
{
    return force || this->mCanSet;
}

const string &
Param::get() const
{
    return this->mValue;
}

void
Param::set(const string &value)
{
    // Validate that the value is a base64 string. So valid characters are A-Z, a-z, 0-9 and + /
    // and it could end with = or ==.
    // COMPAT: The good way of doing this is to check the length first but Nexus does it in this order
    // COMPAT: Nexus allow the =-character in the last two positions. Even if "aa=a" isn't a valid base64 string
    int len = value.length();
    for (int i = 0; i < len; i++) {
        const char c = value[i];
        if (!(c == 43 || (c >= 47 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122) || (c == 61 && i >= len -2)))
            throw PluginException(PE_BadCharValue);
    }

    if (len > this->mMaxSize)
        throw PluginException(PE_TooLongValue);

    this->mValue = value;
}


void
ParamData::add(const string &name, Param param)
{
    this->mParams[name] = param;
};

const string &
ParamData::get(const string &name) const
{
    map<string, Param>::const_iterator it = this->mParams.find(name);
    if (it == this->mParams.end())
        throw PluginException(PE_UnknownParam);

    return it->second.get();
}

void
ParamData::set(const string &name, const string &value, bool force)
{
    map<string, Param>::iterator it = this->mParams.find(name);
    if (it == this->mParams.end() || !it->second.canSet(force))
        throw PluginException(PE_UnknownParam);

    it->second.set(value);
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
    virtual bool Invoke(NPIdentifier name, const NPVariant *args,
                        uint32_t argCount, NPVariant *result);
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

                string ret = plugin->GetVersion();
                STRINGZ_TO_NPVARIANT(Memory::AllocString(ret.c_str()), *result);
                plugin->SetError(PE_OK);
                return true;
            }
            return false;
        case PT_Authentication:
        case PT_Signer:
            if (name == sGetParam_id) {
                if (argCount != 1 || !NPVARIANT_IS_STRING(args[0]))
                    return false;

                PluginError error = PE_OK;
                try {
                    string ret = plugin->GetParam(Lower(NPVariantToString(args[0])));
                    STRINGZ_TO_NPVARIANT(Memory::AllocString(ret.c_str()), *result);
                } catch (PluginException &exc) {
                    error = exc.getError();
                    NULL_TO_NPVARIANT(*result);
                }

                plugin->SetError(error);
                return true;
            }
            if (name == sSetParam_id) {
                if (argCount != 2 || !NPVARIANT_IS_STRING(args[0]) ||
                    !NPVARIANT_IS_STRING(args[0]))
                    return false;

                PluginError error = PE_OK;
                try {
                    plugin->SetParam(Lower(NPVariantToString(args[0])), Lower(NPVariantToString(args[1])));
                } catch (PluginException &exc) {
                    error = exc.getError();
                }

                plugin->SetError(error);
                INT32_TO_NPVARIANT((int32_t)error, *result);
                return true;
            }
            if (name == sPerformAction_id) {
                if (argCount != 1 || !NPVARIANT_IS_STRING(args[0]))
                    return false;

                PluginError error = PE_OK;
                try {
                    plugin->PerformAction(Lower(NPVariantToString(args[0])));
                } catch (PluginException &exc) {
                    error = exc.getError();
                }

                plugin->SetError(error);
                INT32_TO_NPVARIANT((int32_t)error, *result);
                return true;
            }
            if (name == sGetLastError_id) {
                if (argCount != 0)
                    return false;

                INT32_TO_NPVARIANT((int32_t)plugin->GetLastError(), *result);
                return true;
            }
            return false;
    }

    return false;
}


FriBIDPlugin::FriBIDPlugin(NPMIMEType pluginType, NPP instance) :
    mInstance(instance),
    mScriptableObject(NULL),
    mWindow(NULL)
{
    if (strcmp(MIME_VERSION, pluginType) == 0) {
        mType = PT_Version;
    } else if (strcmp(MIME_AUTHENTICATION, pluginType) == 0) {
        mType = PT_Authentication;
    } else if (strcmp(MIME_SIGNER, pluginType) == 0) {
        mType = PT_Signer;
    }

    if (mType != PT_Version) {
        if (mType == PT_Signer) {
            this->mParams.add("nonce", Param("", 5462, true));
            this->mParams.add("texttobesigned", Param("", 136534, true));
            this->mParams.add("nonvisibledata", Param("", 10*1024*1024, true));
        } else {
            this->mParams.add("challenge", Param("", 5462, true));
        }

        this->mParams.add("policys", Param("", 5462, true));
        this->mParams.add("subjects", Param("", 5462, true));
        this->mParams.add("signature", Param("", 5462, false));
    }

    sGetVersion_id = NPN_GetStringIdentifier("GetVersion");
    sGetParam_id = NPN_GetStringIdentifier("GetParam");
    sSetParam_id = NPN_GetStringIdentifier("SetParam");
    sPerformAction_id = NPN_GetStringIdentifier("PerformAction");
    sGetLastError_id = NPN_GetStringIdentifier("GetLastError");

    static const char *const hrefIdentifiers[] = {
        "location", "href", NULL
    };
    this->mUrl = this->GetWindowProperty(hrefIdentifiers);
    static const char *const hostnameIdentifiers[] = {
        "location", "hostname", NULL
    };
    this->mHostname = this->GetWindowProperty(hostnameIdentifiers);
    this->mIpAddress = string("127.0.0.1");
}

FriBIDPlugin::~FriBIDPlugin()
{
    if (this->mScriptableObject)
        NPN_ReleaseObject(this->mScriptableObject);
}

NPBool
FriBIDPlugin::init(NPWindow* window)
{
    if(window == NULL)
        return false;

    this->mWindow = window;
    return true;
}

bool
FriBIDPlugin::isHttps()
{
    return this->mUrl.find("https://") == 0;
}

string
FriBIDPlugin::GetWindowProperty(const char *const identifiers[])
{
    NPObject *obj = NULL;
    NPN_GetValue(this->mInstance, NPNVWindowNPObject, &obj);
    if (!obj)
        return "";

    NPVariant value;
    string ret = "";
    const char *const *identifier = &identifiers[0];
    while (true) {
        NPIdentifier ident = NPN_GetStringIdentifier(*identifier);
        if (!ident)
            break;

        NPN_GetProperty(this->mInstance, obj, ident, &value);
        NPN_ReleaseObject(obj);

        identifier++;
        if (*identifier) {
            if (!NPVARIANT_IS_OBJECT(value))
                break;
            obj = NPVARIANT_TO_OBJECT(value);
        } else {
            if (NPVARIANT_IS_STRING(value))
                ret = NPVariantToString(value);
            break;
        }
    }

    if (obj)
        NPN_ReleaseObject(obj);
    NPN_ReleaseVariantValue(&value);
    return ret;
}

string
FriBIDPlugin::GetVersion()
{
    return "Personal_exe=4.10.4.3&persinst_exe=4.10.4.3&tokenapi_dll=4.10.4.2&personal_dll=4.10.4.2&np_prsnl_dll=4.10.4.3&lng_svse_dll=4.10.4.3&lng_frfr_dll=4.10.4.3&crdsiem_dll=4.10.4.3&crdsetec_dll=4.10.4.3&crdprism_dll=4.10.4.3&br_svse_dll=1.5.0.5&br_enu_dll=1.5.0.5&branding_dll=1.5.0.5&CSP_INSTALLED=TRUE&Personal=4.10.4.3&platform=win32&os_version=winvista&best_before=1283547033&";
}

const string &
FriBIDPlugin::GetParam(const string &name)
{
    return this->mParams.get(name);
}

void
FriBIDPlugin::SetParam(const string &name, const string &value)
{
    return this->mParams.set(name, value, false);
}

void
FriBIDPlugin::PerformAction(const string &action)
{
    switch(this->mType) {
        case PT_Authentication: {
            if (action.compare("authenticate") != 0)
                break;

            const string &nonce = this->mParams.get("challenge");
            if (nonce.empty())
                throw PluginException(PE_MissingParameter);

            if (!this->isHttps())
                throw PluginException(PE_NotSSL);

            try {
                /*
                string signature = BankId::authenticate(nonce, this->mParams.get("subjectFilter"),
                                                        this->mHostname, this->mIpAddress);
                */
                string signature = "test";
                this->mParams.set("signature", signature, true);
            } catch (exception) {
                throw PluginException(PE_UnknownError);
            }
            return;
        }
        case PT_Signer: {
            if (action.compare("sign") != 0)
                break;

            const string &nonce = this->mParams.get("nonce");
            const string &message = this->mParams.get("texttobesigned");
            if (nonce.empty() || message.empty())
                throw PluginException(PE_MissingParameter);

            if (!this->isHttps())
                throw PluginException(PE_NotSSL);

            try {
                /*
                string signature = BankId::sign(nonce, this->mParams.get("subjectfilter"),
                                                message, this->mParams.get("nonvisibledata"),
                                                this->mHostname, this->mIpAddress);
                */
                string signature = "test";
                this->mParams.set("signature", signature, true);
            } catch (exception) {
                throw PluginException(PE_UnknownError);
            }
            return;
        }
    }

    throw PluginException(PE_InvalidAction);
}

void
FriBIDPlugin::SetError(PluginError error)
{
    this->mLastError = error;
}

NPObject *
FriBIDPlugin::GetScriptableObject()
{
    if (!this->mScriptableObject) {
        this->mScriptableObject = NPN_CreateObject(mInstance,
            GET_NPOBJECT_CLASS(ScriptablePluginObject));
    }

    if (this->mScriptableObject)
        NPN_RetainObject(this->mScriptableObject);

    return this->mScriptableObject;
}
