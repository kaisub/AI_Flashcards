#include "app/AppController.hpp"
#include "app/views/FtxuiDeckEditorView.hpp"
#include "app/views/FtxuiListsBrowserView.hpp"
#include "app/views/FtxuiStudyConfigView.hpp"
#include "app/views/FtxuiStudySessionView.hpp"
#include "core/DeckManager.hpp"
#include "storage/JsonStorage.hpp"

int main() {
    try {
        // 1. Set up the data directory
        auto dataPath = std::filesystem::current_path() / "data";
        if (!std::filesystem::exists(dataPath)) {
            std::filesystem::create_directories(dataPath);
        }
        std::cout << "Using data directory: " << dataPath.string() << '\n';

        // 2. Instantiate core components
        auto storage = std::make_unique<storage::JsonStorage>(dataPath);
        auto deckManager = std::make_unique<core::DeckManager>(std::move(storage));

        // 3. Instantiate the main application controller
        app::AppController controller(std::move(deckManager));

        // 4. Instantiate and inject the views
        auto browserView = std::make_shared<app::FtxuiListsBrowserView>();
        controller.setListsBrowserView(browserView);
        auto studyView = std::make_shared<app::FtxuiStudySessionView>();
        controller.setStudySessionView(studyView);
        auto deckView = std::make_shared<app::FtxuiDeckEditorView>();
        controller.setDeckEditorView(deckView);
        auto configView = std::make_shared<app::FtxuiStudyConfigView>();
        controller.setStudyConfigView(configView);

        // 5. Run the application
        controller.run();

    } catch (const std::exception& e) {
        std::cerr << "An unhandled exception occurred: " << e.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "An unknown exception occurred." << '\n';
        return 1;
    }

    return 0;
}
