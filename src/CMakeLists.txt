set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

add_library(
	dstrack STATIC
	DSTracker.cpp
	Biquad.cpp
)

if(BUILD_WAV)

	add_executable(
		dstrack-wav
		main.cpp
		DSTracker.cpp
		Biquad.cpp
		)

	target_link_libraries(dstrack-wav dstrack)

endif()
