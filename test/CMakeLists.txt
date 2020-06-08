find_package(GTest CONFIG REQUIRED)


if(GTest_FOUND)
  include_directories(${PROJECT_SOURCE_DIR}/thirdparty/my-gui/src)
  add_executable(rxcpp_test test.cc)
  target_link_libraries(rxcpp_test
    GTest::gtest_main
    my-gui_lib
    )
endif(GTest_FOUND)
