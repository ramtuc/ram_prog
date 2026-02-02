set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GEN_DIR}")

idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "." "${GEN_DIR}"
    REQUIRES nanopb nvs_flash esp_wifi esp_http_client esp_event esp_timer
    EMBED_TXTFILES "certs/azure_root_ca.pem"
)

if(NOT CMAKE_BUILD_EARLY_EXPANSION)
    set(NANOPB_GEN "${CMAKE_SOURCE_DIR}/components/nanopb/generator/nanopb_generator.py")
    set(PROTO_DIR "${CMAKE_CURRENT_LIST_DIR}/proto")
    set(PROTO_FILE "${PROTO_DIR}/sensor.proto")
    set(OPTIONS_FILE "${PROTO_DIR}/sensor.options")
    set(GEN_SRC "${GEN_DIR}/sensor.pb.c")
    set(GEN_HDR "${GEN_DIR}/sensor.pb.h")

    add_custom_command(
        OUTPUT "${GEN_SRC}" "${GEN_HDR}"
        COMMAND python "${NANOPB_GEN}" -I "${PROTO_DIR}" -D "${GEN_DIR}" "${PROTO_FILE}"
        DEPENDS "${PROTO_FILE}" "${OPTIONS_FILE}"
        VERBATIM
    )
    target_sources(${COMPONENT_LIB} PRIVATE "${GEN_SRC}")
    set_source_files_properties("main.c" PROPERTIES OBJECT_DEPENDS "${GEN_HDR}")
endif()