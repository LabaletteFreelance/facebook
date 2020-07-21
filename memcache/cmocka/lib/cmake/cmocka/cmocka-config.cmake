get_filename_component(CMOCKA_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

if (EXISTS "${CMOCKA_CMAKE_DIR}/CMakeCache.txt")
    # In build tree
    include(${CMOCKA_CMAKE_DIR}/cmocka-build-tree-settings.cmake)
else()
    set(CMOCKA_INCLUDE_DIR /home/trahay/Documents/enseignement/csc4508/csc4508_facebook/memcache/cmocka/include)
endif()

set(CMOCKA_LIBRARY /home/trahay/Documents/enseignement/csc4508/csc4508_facebook/memcache/cmocka/lib/libcmocka.so)
set(CMOCKA_LIBRARIES /home/trahay/Documents/enseignement/csc4508/csc4508_facebook/memcache/cmocka/lib/libcmocka.so)
