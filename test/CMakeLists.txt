include(RoqAutogen)

set(TARGET_NAME "${PROJECT_NAME}-test")

set(SOURCES
    json_account_assets_ack.cpp
    json_account.cpp
    json_account_settings_ack.cpp
    json_books.cpp
    json_cancel_all_orders_ack.cpp
    json_cancel_order_ack.cpp
    json_cancel_order.cpp
    json_current_positions_ack.cpp
    json_encoder.cpp
    json_error.cpp
    json_fill.cpp
    json_instruments_ack.cpp
    json_message.cpp
    json_modify_order_ack.cpp
    json_modify_order.cpp
    json_order.cpp
    json_place_order_ack.cpp
    json_place_order.cpp
    json_position.cpp
    json_public_trade.cpp
    json_ticker.cpp
    json_trade_fills_ack.cpp
    json_unfilled_orders_ack.cpp
    main.cpp)

roq_gitignore(OUTPUT .gitignore SOURCES "${TARGET_NAME}")

add_executable("${TARGET_NAME}" ${SOURCES})

target_link_libraries("${TARGET_NAME}" "${PROJECT_NAME}-json" Catch2::Catch2)

set_target_properties("${TARGET_NAME}" PROPERTIES LINK_FLAGS_RELEASE -s)

# run

add_test(NAME "${TARGET_NAME}" COMMAND "${TARGET_NAME}")
