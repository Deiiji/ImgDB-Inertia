if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(WINDOWS ON BOOL FORCE)
  add_definitions(-DWINDOWS)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LINUX ON BOOL FORCE)
  add_definitions(-DLINUX)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(DARWIN ON BOOL FORCE)
  add_definitions(-DDARWIN)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
