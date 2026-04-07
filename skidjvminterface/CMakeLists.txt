add_library (skidjvminterface STATIC "src/main.c" "includes/skidjvminterface.h" "src/sjisysmem.c" "src/sjjvminterface.c" "src/sjihotspot.c" "src/sjiprocess.c")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET skidjvminterface PROPERTY CXX_STANDARD 20)
endif()

if(MSVC)
  target_compile_options(skidjvminterface PRIVATE /W3 /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(skidjvminterface PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()