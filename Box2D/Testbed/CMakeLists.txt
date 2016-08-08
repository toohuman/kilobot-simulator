

# Some flags for Freeglut and GLUI.
add_definitions( -DFREEGLUT_EXPORTS -DFREEGLUT_STATIC -D_CRT_SECURE_NO_WARNINGS )

# Define the framework files.
set(Testbed_Framework_SRCS
	Framework/Main.cpp
	Framework/Render.cpp
	Framework/Render.h
	Framework/Test.cpp
	Framework/Test.h
)

#define the test files.
set(Testbed_Tests_SRCS
	Tests/TestEntries.cpp
	Tests/kilolib.h
	Tests/kilolib.cpp
	Tests/kiloworld.h
	Tests/kiloworld.cpp
    Tests/worldfile.cpp
    Tests/worldfile.h
	Tests/evokilo1.h
	Tests/evokilo1.cpp
)

# These are used to create visual studio folders.
source_group(Framework FILES ${Testbed_Framework_SRCS})
source_group(Tests FILES ${Testbed_Tests_SRCS})

include_directories (
	${OPENGL_INCLUDE_DIR}
	${Box2D_SOURCE_DIR}
)

if(APPLE)
	# We are not using the Apple's framework version, but X11's
	include_directories( /usr/X11/include )
	link_directories( /usr/X11/lib )
    message(Apple)
	#set (OPENGL_LIBRARIES GL GLU GLUT X11)
	set (OPENGL_LIBRARIES "-framework OpenGL -framework GLUT")
elseif(WIN32)
	set (ADDITIONAL_LIBRARIES winmm)
else(APPLE)
    set (ADDITIONAL_LIBRARIES freeglut_static)
endif(APPLE)

add_executable(Testbed
	${Testbed_Framework_SRCS}
	${Testbed_Tests_SRCS}
)

target_link_libraries (
	Testbed
	Box2D
	#freeglut_static
	glui
	X11
	${ADDITIONAL_LIBRARIES}
	${OPENGL_LIBRARIES}
)
