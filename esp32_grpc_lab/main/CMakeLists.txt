# 1. 生成先ディレクトリのパスを確定し、即座に作成する
set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GEN_DIR}")

# 2. コンポーネントの登録
# main.c で使用する ESP-IDF 標準機能（nvs, wifi, http_client）を REQUIRES に追加
idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "." "${GEN_DIR}"
    REQUIRES nanopb nvs_flash esp_wifi esp_http_client esp_event
)

# 3. 実際のビルド処理（依存関係スキャン時はスキップ）
if(NOT CMAKE_BUILD_EARLY_EXPANSION)
    set(NANOPB_GEN "${CMAKE_SOURCE_DIR}/components/nanopb/generator/nanopb_generator.py")
    set(PROTO_DIR "${CMAKE_CURRENT_LIST_DIR}/proto")
    set(PROTO_FILE "${PROTO_DIR}/sensor.proto")
    set(OPTIONS_FILE "${PROTO_DIR}/sensor.options") # オプションファイル
    
    set(GEN_SRC "${GEN_DIR}/sensor.pb.c")
    set(GEN_HDR "${GEN_DIR}/sensor.pb.h")

    # 自動生成コマンド
    add_custom_command(
        OUTPUT "${GEN_SRC}" "${GEN_HDR}"
        # -I でプロトファイルのディレクトリを指定し、.proto をコンパイル
        COMMAND python "${NANOPB_GEN}" -I "${PROTO_DIR}" -D "${GEN_DIR}" "${PROTO_FILE}"
        # .proto だけでなく .options が更新された時も再生成するように設定
        DEPENDS "${PROTO_FILE}" "${OPTIONS_FILE}"
        VERBATIM
    )

    # 生成されたソースをターゲットに追加
    target_sources(${COMPONENT_LIB} PRIVATE "${GEN_SRC}")
    
    # main.c のコンパイル前にヘッダーの生成を保証する
    set_source_files_properties("main.c" PROPERTIES OBJECT_DEPENDS "${GEN_HDR}")
endif()