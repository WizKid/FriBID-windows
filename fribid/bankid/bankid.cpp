#include "../common/memory.h"
#include "../common/format.h"
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
    "</usrVisibleData>";

static const char signedInvisibleText_template[] =
    "<usrNonVisibleData>"
        "%s"
    "</usrNonVisibleData>";

static const char signobj_id[] = "bidSignedData";

PluginError BankId::authenticate(const char *challenge, const char *subjectFilter,
                                 const char *hostname, const char *ipAddress,
                                 char **signature)
{
    *signature = CreateFormatString(signedInvisibleText_template, "NonVisibleData!!");
    return PE_OK;
}

PluginError BankId::sign(const char *nonce, const char *subjectFilter, const char *message,
                         const char *invisibleMessage, const char *hostname,
                         const char *ipAddress, char **signature)
{
    *signature = CreateFormatString(signedText_template, "VisibleData!!");
    return PE_OK;
}