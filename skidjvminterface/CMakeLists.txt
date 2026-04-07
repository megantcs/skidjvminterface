add_library (skidjvminterface STATIC "src/main.c" "includes/skidjvminterface.h" "src/sjisysmem.c" "src/sjijvminterface.c")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET skidjvminterface PROPERTY CXX_STANDARD 20)
endif()

