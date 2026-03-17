# Most likely, users of the library have already setup FTXUI:
if (TARGET ftxui::component)
    return()
endif()

# Fallback on the system manager:
# find_package(ftxui QUIET)
find_package(ftxui REQUIRED)
if (ftxui_FOUND)
    message(STATUS "FXTUI found at ${ftxui_DIR}")
    return()
else()
    message(WARNING "FTXUI not found. Attempting to clone and build from GitHub...")
endif()

# Fallback on a specific version of FTXUI using FetchContent.
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v6.1.9
)
FetchContent_MakeAvailable(ftxui)
