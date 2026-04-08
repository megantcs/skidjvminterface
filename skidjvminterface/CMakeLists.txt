<<<<<<< HEAD
﻿add_library (skidjvminterface STATIC  "includes/skidjvminterface.h" "src/sjisysmem.c" "src/sjjvminterface.c" "src/sjihotspot.c" "src/sjiprocess.c" "src/sjihotspot17.c" "src/sjihotspot21.c")
=======
﻿add_library (skidjvminterface STATIC  "includes/skidjvminterface.h" "src/sjisysmem.c" "src/sjjvminterface.c" "src/sjihotspot.c" "src/sjiprocess.c" "src/sjihotspot17.c")
>>>>>>> 01e1a52d8fb32884195b12d1f4e32380b4729336

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET skidjvminterface PROPERTY CXX_STANDARD 20)
endif()

if(MSVC)
  target_compile_options(skidjvminterface PRIVATE /W3 /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(skidjvminterface PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()