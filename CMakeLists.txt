project(reverseKeylayoutGenerator)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
cmake_minimum_required(VERSION 2.4.0)
include(FindBoost)

include_directories(${CMAKE_SOURCE_DIR}/src/utils)
include_directories(${CMAKE_SOURCE_DIR}/src/core)
include_directories(${CMAKE_SOURCE_DIR}/src/configs/autogen)
include_directories(${CMAKE_SOURCE_DIR}/src/configs/variant)
include_directories(${CMAKE_SOURCE_DIR}/src/configs/include)
include_directories(${CMAKE_SOURCE_DIR}/src/mod)
include_directories(${CMAKE_SOURCE_DIR}/src/front)
include_directories(${CMAKE_SOURCE_DIR}/src/acl)
include_directories(${CMAKE_SOURCE_DIR}/src/capture)
include_directories(${CMAKE_SOURCE_DIR}/src/keyboard)
include_directories(${CMAKE_SOURCE_DIR}/src/transport)
include_directories(${CMAKE_SOURCE_DIR}/src/regex)
include_directories(${CMAKE_SOURCE_DIR}/src/headers)
include_directories(${CMAKE_SOURCE_DIR}/src/main)
include_directories(${CMAKE_SOURCE_DIR}/modules/includes)
include_directories(/usr/include)
include_directories(${CMAKE_SOURCE_DIR}/tests)

set(spreadsheet_SRCS
    tools/reverseKeylayoutGenerator.cpp
)
set(spreadsheet_MOC_SRCS

)
set(spreadsheet_UIS

)
set(spreadsheet_RCCS

)



set(FIXTURES_PATH "./tests/fixtures")
set(INSTALLDIR)
set(PREFIX "/usr/local")
set(BIN_PREFIX "$(PREFIX)/bin")
set(LIB_PREFIX "$(PREFIX)/lib")
set(SHARE_PREFIX "$(PREFIX)/share/rdpproxy")
set(RECORD_TMP_PATH "/var/rdpproxy/tmp")
set(RECORD_PATH "/var/rdpproxy/recorded")
set(ETC_PREFIX "/etc/rdpproxy")
set(CERT_PREFIX" /etc/rdpproxy/cert")
set(HASH_PATH "/var/rdpproxy/hash")
set(PERSISTENT_PATH "/var/lib/redemption/cache")
set(DRIVE_REDIRECTION_PATH "/var/rdpproxy/drive_redirection")

add_definitions(-DSHARE_PATH='\"${PREFIX}/share/rdpproxy\"')
add_definitions(-DCFG_PATH='\"${ETC_PREFIX}\"')
add_definitions(-DRECORD_PATH='\"${RECORD_PATH}\"')
add_definitions(-DRECORD_TMP_PATH='\"${RECORD_TMP_PATH}\"')
add_definitions(-DFLV_PATH='\"${RECORD_TMP_PATH}\"')
add_definitions(-DOCR_PATH='\"${RECORD_TMP_PATH}\"')
add_definitions(-DPNG_PATH='\"${RECORD_TMP_PATH}\"')
add_definitions(-DWRM_PATH='\"${RECORD_PATH}\"')
add_definitions(-DHASH_PATH='\"${HASH_PATH}\"')
add_definitions(-DLICENSE_PATH='\"${CERT_PREFIX}/rdplicense\"')
add_definitions(-DCERTIF_PATH='\"${CERTIF_PATH}/rdp\"')
add_definitions(-DFIXTURES_PATH='\"${FIXTURES_PATH}\"')
add_definitions(-DPERSISTENT_PATH='\"${PERSISTENT_PATH}\"')
add_definitions(-DDRIVE_REDIRECTION_PATH='\"${DRIVE_REDIRECTION_PATH}\"')

add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DQT_NO_DEBUG)
add_executable(reverseKeylayoutGenerator
    ${spreadsheet_SRCS}
    ${spreadsheet_MOCS}
    ${spreadsheet_MOC_UI}
    ${spreadsheet_RCC_SRCS})

#link_directories(${BOOST_LIBRARYDIR})
#include_directories(SYSTEM ${BOOST_INCLUDEDIR})

target_link_libraries( reverseKeylayoutGenerator pthread krb5 gssglue z dl )
