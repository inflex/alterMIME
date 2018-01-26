CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)

PROJECT ("altermime")

SET(CMAKE_C_FLAGS "-Wall -Werror -g -I. -O2")

ADD_DEFINITIONS(-DALTERMIME_PRETEXT)
FILE(GLOB COMPILE_SRCS   ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_EXECUTABLE(altermime ${COMPILE_SRCS})

INSTALL( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/altermime
         DESTINATION /usr/bin
       )


set(CPACK_PACKAGE_NAME                                  "altermime")
set(CPACK_PACKAGE_CONTACT                               "Altermime")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY                   "Provides a binary to alter the MIME contents in a mail.")
set(CPACK_PACKAGE_VENDOR                                "Open source")
set(CPACK_PACKAGE_VERSION_MAJOR                         0)
set(CPACK_PACKAGE_VERSION_MINOR                         3)
set(CPACK_PACKAGE_VERSION_PATCH                         11)
set(CPACK_PACKAGE_RELEASE                               "1")
set(CPACK_PACKAGE_FILE_NAME                             ${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
set(CPACK_DEBIAN_PACKAGE_SECTION                        "main")

SET(CPACK_DEBIAN_PACKAGE_REPLACES                    "altermime (<= 0.3.10)")

SET(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION    "/usr/bin")

FIND_PROGRAM(DPKG NAMES dpkg)
FIND_PROGRAM(ZYPPER NAMES zypper)
FIND_PROGRAM(YUM NAMES yum)

IF (DPKG)
    SET (CPACK_GENERATOR "DEB")
ELSE (DPKG)
    IF (ZYPPER)
        SET (CPACK_GENERATOR "RPM")
    ELSEIF(YUM)
        SET (CPACK_GENERATOR "RPM")
    ELSE ()
        SET (CPACK_GENERATOR "")
    ENDIF ()
ENDIF(DPKG)

include(CPack)
