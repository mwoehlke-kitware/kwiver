#
# Script to set up environment for python pytest
#
# To allow for windows configuration based paths in the build
# directory WIN_TEST_CONFIG_TYPE must be set to, in some fashion,
# evaluate to the configuration type of the build
# Note, this only need be set for Windows


# results of tests being run will be exported to an Xunit xml file
if (PYTEST_RUNNER)

  set(no_install TRUE)
  string(TOLOWER "${CMAKE_PROJECT_NAME}" project_name)

  if (WIN32)
    set(kwiver_test_output_path    "${KWIVER_BINARY_DIR}/${WIN_TEST_CONFIG_TYPE}/bin")
  else ()
    set(kwiver_test_output_path    "${KWIVER_BINARY_DIR}/tests/bin")
  endif ()
  if(VENV_CREATED)
      if(Python3_INTERPRETER_ID STREQUAL "Anaconda")
        set(PYTEST_COMMAND "$ENV{CONDA_EXE} activate {VENV_DIR} && ")
      else()
        set(PYTEST_COMMAND "source ${VENV_DIR}/bin/activate && ")
      endif()
  else()
    set(PYTEST_COMMAND)
  endif()
  if (CTEST_BINARY_DIRECTORY)
    set(kwiver_test_working_path "${CTEST_BINARY_DIRECTORY}")
  else()
    set(kwiver_test_working_path "${KWIVER_BINARY_DIR}")
  endif()

  set(kwiver_test_runner "${PYTEST_RUNNER} ${mod_dst}")
endif()
