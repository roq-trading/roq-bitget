include(RoqAutogen)

set(TARGET_NAME "${PROJECT_NAME}-test")

set(SOURCES
    json_account_assets.cpp
    json_account.cpp
    json_account_info.cpp
    json_books.cpp
    json_cancel_all_orders_ack.cpp
    json_cancel_all_orders.cpp
    json_cancel_order_ack.cpp
    json_cancel_order.cpp
    json_fill.cpp
    json_fill_history.cpp
    json_instruments.cpp
    json_message.cpp
    json_modify_order_ack.cpp
    json_modify_order.cpp
    json_open_orders.cpp
    json_order.cpp
    json_place_order_ack.cpp
    json_place_order.cpp
    json_position.cpp
    json_position_info.cpp
    json_public_trade.cpp
    json_ticker.cpp
    main.cpp)

roq_gitignore(OUTPUT .gitignore SOURCES "${TARGET_NAME}")

add_executable("${TARGET_NAME}" ${SOURCES})

target_link_libraries("${TARGET_NAME}" "${PROJECT_NAME}-json" Catch2::Catch2)

set_target_properties("${TARGET_NAME}" PROPERTIES LINK_FLAGS_RELEASE -s)

# run

add_test(NAME "${TARGET_NAME}" COMMAND "${TARGET_NAME}")
