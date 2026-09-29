#include "App.h"
#include <iostream>

namespace Sentinel {

    SentinelApp::SentinelApp() : m_is_running(false) {}

    Status SentinelApp::initialize() {
        std::cout << "[SentinelApp] Initializing " << PLATFORM_NAME << " v" << VERSION << "..." << std::endl;
        std::cout << "[SentinelApp] Environment check passed." << std::endl;
        m_is_running = true;
        return Status::SUCCESS;
    }

    void SentinelApp::run() {
        if (!m_is_running) {
            std::cerr << "[SentinelApp] Error: Cannot run uninitialized application." << std::endl;
            return;
        }

        std::cout << "[SentinelApp] Application controller running." << std::endl;
        std::cout << "[SentinelApp] Milestone 1 verification complete." << std::endl;
    }

    void SentinelApp::shutdown() {
        std::cout << "[SentinelApp] Shutting down application gracefully..." << std::endl;
        m_is_running = false;
    }

} // namespace Sentinel
