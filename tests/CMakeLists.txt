add_executable(tests src/main.c)

target_link_libraries(tests skidjvminterface)
target_include_directories(tests PRIVATE "../skidjvminterface/includes")