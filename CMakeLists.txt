cmake_minimum_required(VERSION 3.18)
project(wxmeasure)

add_executable(wxmeasure src/wxmeasure.cpp)
add_subdirectory(src)

add_custom_target(build_win32_installer
	COMMAND sed -e s/\#VERSION\#/${CMAKE_PROJECT_VERSION}/ ${CMAKE_CURRENT_SOURCE_DIR}/wxmeasure.iss.in > ${CMAKE_CURRENT_BINARY_DIR}/wxmeasure.iss
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/inno.sh wxmeasure-${CMAKE_PROJECT_VERSION}-x86_64 ${CMAKE_CURRENT_BINARY_DIR}/wxmeasure.iss WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	VERBATIM
)

add_custom_target(build_win32_zip
	COMMAND zip -j ${CMAKE_CURRENT_BINARY_DIR}/wxmeasure-${CMAKE_PROJECT_VERSION}-x86_64.zip ${CMAKE_CURRENT_BINARY_DIR}/wxmeasure.exe  ${CMAKE_CURRENT_BINARY_DIR}/wxmeasure.conf
	VERBATIM
)

#WxWidgets:
#if WXDIR contains a path to a built wxWidgets library set, wx-config there will be used to provide the includes and libs.  Otherwise, a system package will be found.
if(DEFINED WXDIR)
        execute_process(COMMAND bash -c "${WXDIR}/wx-config --cxxflags" OUTPUT_VARIABLE WX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(COMMAND bash -c "${WXDIR}/wx-config --libs all" OUTPUT_VARIABLE WX_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE )
        string(APPEND CMAKE_CXX_FLAGS " ${WX_FLAGS}")
        target_link_libraries(wxmeasure ${WX_LIBS})
        message(STATUS "wxWidgets configured with wx-config at ${WXDIR}")
else()
        find_package(wxwidgets REQUIRED COMPONENTS all)
        include(${wxWidgets_USE_FILE})
        target_link_libraries(wxmeasure ${wxWidgets_LIBRARIES})
endif(DEFINED WXDIR)


