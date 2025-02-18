if(INSTALL_GIT_PRECOMMIT_HOOK)
  find_package(Git REQUIRED)
  find_program(CLANG_FORMAT clang-format REQUIRED)

  # Installing clang-format precommit hook if
  #prerequisites are met and it doesn't # exist yet.
  if(EXISTS ${PROJECT_SOURCE_DIR}/.git AND
    NOT EXISTS ${PROJECT_SOURCE_DIR}/.git/hooks/pre-commit)

    configure_file(${CMAKE_SOURCE_DIR}/cmake/util/git_pre-commit.in
                  ${PROJECT_SOURCE_DIR}/.git/hooks/pre-commit)
    execute_process(COMMAND
      bash -c "chmod +x ${PROJECT_SOURCE_DIR}/.git/hooks/pre-commit")
  endif()
endif()
