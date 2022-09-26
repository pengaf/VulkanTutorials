
set(HeaderFiles
    "Tutorial01.h"
)
source_group("Header Files" FILES ${HeaderFiles})

set(SourceFiles
    "main.cpp"
    "Tutorial01.cpp"
)
source_group("Source Files" FILES ${SourceFiles})

set(ShaderFiles
    "shader.vert"
    "shader.frag"
)
source_group("Shader Files" FILES ${ShaderFiles})

set(FormFiles
    "Tutorial01.ui"
)
source_group("Form Files" FILES ${FormFiles})

set(ResourceFiles
    "Tutorial01.qrc"
)
source_group("Resource Files" FILES ${ResourceFiles})

set(AllFiles
    ${HeaderFiles}
    ${SourceFiles}
    ${ShaderFiles}
    ${FormFiles}
    ${ResourceFiles}
)

foreach(ShaderFile IN LISTS ShaderFiles)
	set(BinShaderFile ${CMAKE_CURRENT_SOURCE_DIR}/../bin/${ShaderFile}.spv)
    add_custom_command(
        OUTPUT ${BinShaderFile}
	    COMMAND glslangValidator -H -V -o ${BinShaderFile} "%(FullPath)" >> ${BinShaderFile}.txt
	    MAIN_DEPENDENCY ${ShaderFile})
endforeach()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS" ) 

add_executable(Tutorial01 ${AllFiles})
target_link_libraries(Tutorial01 Qt5::Widgets)
target_link_libraries(Tutorial01 optimized qtmain)
target_link_libraries(Tutorial01 debug qtmaind)
target_link_libraries(Tutorial01 vulkan-1)

set_target_properties(Tutorial01 PROPERTIES DEBUG_POSTFIX _d)
