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

#include <map>
#include <string>

#include "npapi.h"
#include "npruntime.h"

#define MIME_VERSION "application/x-personal-version-test"
#define MIME_AUTHENTICATION "application/x-personal-authentication-test"
#define MIME_SIGNER "application/x-personal-signer2-test"

#define NO_FILE_EXTENSIONS ""

using namespace std;

enum PluginType {
    PT_Version,
    PT_Authentication,
    PT_Signer,
};

enum PluginError {
    PE_OK = 0,
    PE_UnknownError =     1, // Maybe this is used for something else in the original plugin?
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

class PluginException : exception
{
private:
    PluginError error;
public:
    PluginException(PluginError error) : error(error) {}
    PluginError getError() { return error; };
};

class Param {
private:
    int mMaxSize;
    string mValue;
    bool mCanSet;

public:
    Param();
    Param(const string &value, int maxSize, bool canSet);

    const string &get() const;
    void set(const string &value);
    bool canSet(bool force) const;
};

class ParamData {
protected:
    map<string, Param> mParams;

public:
    void add(const string &name, Param param);

    const string &get(const string &name) const;
    void set(const string &name, const string &value, bool force);
};

class FriBIDPlugin
{
private:
    NPP mInstance;
    NPObject *mScriptableObject;
    NPWindow *mWindow;
    PluginType mType;
    PluginError mLastError;
    string mUrl;
    string mHostname;
    string mIpAddress;
    ParamData mParams;

    string GetWindowProperty(const char *const identifiers[]);
    bool isHttps();

public:
    FriBIDPlugin(NPMIMEType pluginType, NPP pNPInstance);
    ~FriBIDPlugin();

    PluginType GetType() { return this->mType; };
    PluginError GetLastError() { return this->mLastError; };
    void SetError(PluginError error);

    NPBool init(NPWindow* window);
  
    NPObject *GetScriptableObject();

    string GetVersion();
    const string &GetParam(const string &name);
    void SetParam(const string &name, const string &value);
    void PerformAction(const string &action);
};

#endif // __PLUGIN_H__
