add_library (skidjvminterface STATIC "src/main.c" "includes/skidjvminterface.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET skidjvminterface PROPERTY CXX_STANDARD 20)
endif()

