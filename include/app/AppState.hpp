#pragma once

namespace app {
    /**
     * @brief Defines the high-level states of the application.
     */
    enum class AppState {
        ListsBrowser,   // Browsing folders and .json list files
        DeckEditor,     // Managing flashcards inside a specific list
        StudyConfig,    // Pre-study configuration screen
        StudySession,   // Active learning loop
        Settings,       // Global app configuration
        Exit            // Graceful shutdown
    };
}
