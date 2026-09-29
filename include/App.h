#ifndef SENTINEL_APP_H
#define SENTINEL_APP_H

#include "Common.h"

namespace Sentinel {

    /**
     * @class SentinelApp
     * @brief Core Application Controller managing SentinelOS lifecycle,
     *        module initializations, and system execution loops.
     */
    class SentinelApp {
    private:
        bool m_is_running;

    public:
        SentinelApp();
        ~SentinelApp() = default;

        /**
         * @brief Initializes core configuration, logs, and subsystem handles.
         * @return Status code (SUCCESS if initialized cleanly).
         */
        Status initialize();

        /**
         * @brief Starts the application event loop.
         */
        void run();

        /**
         * @brief Performs graceful shutdown of all modules and handles.
         */
        void shutdown();
    };

} // namespace Sentinel

#endif // SENTINEL_APP_H
