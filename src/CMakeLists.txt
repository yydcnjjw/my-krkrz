set(my-krkrz_SRC
  main.cc
  krkrz_application.cc)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUNICODE -DTJS_TEXT_OUT_CRLF -DTJS_JP_LOCALIZED -DTVP_ENABLE_EXECUTE_AT_EXCEPTION -DTVP_LOG_TO_COMMANDLINE_CONSOLE")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNICODE -DTJS_TEXT_OUT_CRLF -DTJS_JP_LOCALIZED -DTVP_ENABLE_EXECUTE_AT_EXCEPTION -DTVP_LOG_TO_COMMANDLINE_CONSOLE")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(
  base
  tjs2
  msg
  msg/linux
  utils
  ${PROJECT_SOURCE_DIR}/thirdparty/my-gui/src
  )

aux_source_directory(tjs2 my-krkrz_SRC)
list(REMOVE_ITEM my-krkrz_SRC tjs2/tjsDateWordMap.cc)

aux_source_directory(base my-krkrz_SRC)
aux_source_directory(msg my-krkrz_SRC)
aux_source_directory(msg/linux my-krkrz_SRC)
aux_source_directory(utils my-krkrz_SRC)
aux_source_directory(tjs2_lib my-krkrz_SRC)
aux_source_directory(tjs2_plugin my-krkrz_SRC)

find_package(Onig REQUIRED)

add_executable(my-krkrz ${my-krkrz_SRC})
target_link_libraries(my-krkrz
  ${ONIGURUMA_LIBRARIES}
  my-gui_lib)

add_custom_target(i18n)
add_custom_command(
  TARGET i18n
  COMMAND ${CMAKE_COMMAND} -E copy
  ${PROJECT_SOURCE_DIR}/src/msg/text/linux/i18n
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  )
add_dependencies(my-krkrz i18n)
