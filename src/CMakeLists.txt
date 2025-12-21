add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

add_executable(ICG_2025_HW4
"main.cpp"
"stb_image.cpp"
"shader.cpp"
)
target_link_libraries(ICG_2025_HW4
glfw
glm::glm
glad
tinyobjloader
)
add_custom_command(TARGET ICG_2025_HW4 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders)

add_custom_command(TARGET ICG_2025_HW4 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory  
    ${CMAKE_CURRENT_SOURCE_DIR}/asset ${CMAKE_CURRENT_BINARY_DIR}/asset)