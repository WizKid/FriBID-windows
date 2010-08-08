#include "../common/memory.h"
#include "../common/format.h"
#include "../common/base64.h"
#include "bankid.h"

/* Authentication and signing objects */
static const char sign_template[] =
    "<bankIdSignedData xmlns=\"http://www.bankid.com/signature/v1.0.0/types\" Id=\"bidSignedData\">"
        "%s"
        "<srvInfo>"
            "<nonce>%s</nonce>"
        "</srvInfo>"
        "<clientInfo>"
            "<funcId>%s</funcId>"
            "<host>"
                "<fqdn>%s</fqdn>"
                "<ip>%s</ip>"
            "</host>"
            "<version>%s</version>"
        "</clientInfo>"
    "</bankIdSignedData>";

static const char signedText_template[] =
    "<usrVisibleData charset=\"UTF-8\" visible=\"wysiwys\">"
        "%s"
    "</usrVisibleData>"
    "%s";

static const char signedInvisibleText_template[] =
    "<usrNonVisibleData>"
        "%s"
    "</usrNonVisibleData>";

static const char signobj_id[] = "bidSignedData";

PluginError BankId::_sign(const char *challenge, const char *hostname, const char *ipAddress,
                          const char *purpose, const char *extra, char **signature)
{
    char *version64 = Base64::encode("version", strlen("version"));
    char *xml = CreateFormatString(sign_template, extra, challenge, purpose, hostname, ipAddress, version64);
    Memory::Free(version64);

    char *xmlSign = ""; // __sign(signobj_id, xml);
    Memory::Free(xml);
    if (xmlSign) {
        *signature = Base64::encode(xmlSign, strlen(xmlSign));
        Memory::Free(xmlSign);
        return PE_OK;
    }

    *signature = NULL;
    return PE_UnknownError;
}

PluginError BankId::authenticate(const char *challenge, const char *subjectFilter, const char *hostname,
                                 const char *ipAddress, char **signature)
{
    return BankId::_sign(challenge, hostname, ipAddress, "Identification", "", signature);
}

PluginError BankId::sign(const char *nonce, const char *subjectFilter, const char *message,
                         const char *invisibleMessage, const char *hostname, const char *ipAddress,
                         char **signature)
{
    char *invisibleMessageData = NULL;
    if (invisibleMessage) {
        invisibleMessageData = CreateFormatString(signedInvisibleText_template, invisibleMessage);
    } else {
        invisibleMessageData = Memory::AllocString("");
    }
    char *extra = CreateFormatString(signedText_template, message, invisibleMessageData);

    PluginError ret = BankId::_sign(nonce, hostname, ipAddress, "Signing", extra, signature);

    Memory::Free(invisibleMessageData);
    Memory::Free(extra);

    return ret;
}