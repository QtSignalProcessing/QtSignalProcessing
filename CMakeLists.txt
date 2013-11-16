project(SignalProcessing)

cmake_minimum_required(VERSION 2.8.8)

#####OPTIONS#########
option(QT5_BUILD "Build powerbike using the Qt5 framework" FALSE)
#####END OPTIONS#####

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set(SignalProcessing_SRCS
  src/audiohandle.cpp
  src/axislabel.cpp
  src/chebyshevfilter.cpp
  src/glbase.cpp
  src/glospectrum.cpp
  src/glsw.cpp
  src/iir.cpp
  src/main.cpp
  src/mainwindow.cpp
  src/plotwidget.cpp
  src/plotfilter.cpp
  src/utilities.cpp
)

if (QT5_BUILD)
	if(WIN32)
		set (CMAKE_PREFIX_PATH "C:\\Qt\\5.1.1\\msvc2010_opengl\\")
		cmake_policy(SET CMP0020 OLD)
	endif()
  find_package(Qt5Core REQUIRED)
endif()

add_executable(SignalProcessing ${SignalProcessing_SRCS})

if (Qt5Core_FOUND)
  qt5_use_modules(SignalProcessing Widgets OpenGL Multimedia)
else(Qt5Core_FOUND)
  find_package(Qt4 REQUIRED QtCore QtGui QtOpenGl)
	if (WIN32)
		set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
	endif()
  find_package(Phonon REQUIRED)
  include(${QT_USE_FILE})
  target_link_libraries(SignalProcessing ${QT_LIBRARIES} ${PHONON_LIBS})
endif(Qt5Core_FOUND)

if (WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/include)
	target_link_libraries(SignalProcessing OpenGL32 ${CMAKE_CURRENT_SOURCE_DIR}/lib/win32/sndfile.lib ${CMAKE_CURRENT_SOURCE_DIR}/lib/win32/fftw3.lib ${CMAKE_CURRENT_SOURCE_DIR}/lib/win32/samplerate.lib)
elseif()
	target_link_libraries(SignalProcessing GL sndfile fftw3 samplerate)
endif()

if (WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++0x")
endif()