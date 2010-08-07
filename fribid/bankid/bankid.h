#include "../plugin/plugin.h"

class BankId
{
public:
    static PluginError authenticate(const char *challenge, const char *subjectFilter,
                                    const char *hostname, const char *ipAddress,
                                    char **signature);
    static PluginError sign(const char *nonce, const char *subjectFilter, const char *message,
                            const char *invisibleMessage, const char *hostname,
                            const char *ipAddress, char **signature);
};