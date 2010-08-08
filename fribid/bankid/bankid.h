#include "../plugin/plugin.h"

class BankId
{
private:
    static PluginError _sign(const char *challenge, const char *hostname, const char *ipAddress,
                             const char *purpose, const char *extra, char **signature);

public:
    static PluginError authenticate(const char *challenge, const char *subjectFilter,
                                    const char *hostname, const char *ipAddress,
                                    char **signature);
    static PluginError sign(const char *nonce, const char *subjectFilter, const char *message,
                            const char *invisibleMessage, const char *hostname,
                            const char *ipAddress, char **signature);
};