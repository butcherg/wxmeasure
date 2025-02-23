cmake_minimum_required(VERSION 3.18)
project(wxmeasure)

add_executable(wxmeasure src/wxmeasure.cpp)
add_subdirectory(src)

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

if (DEFINED MXE_DEPS)
	set(WXMEASURE_DEPS "-lsharpyuv -lzstd")
	target_link_libraries(wxmeasure ${WXMEASURE_DEPS} )
endif(DEFINED MXE_DEPS)