#pragma once

namespace app {
    /**
     * @brief Base interface for all application views.
     */
    class IView {
    public:
        virtual ~IView() = default;
        
        // Called by Controller to start the view's event loop or render cycle
        virtual void run() = 0;
    };
}