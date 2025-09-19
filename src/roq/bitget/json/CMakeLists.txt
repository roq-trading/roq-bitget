set(TARGET_NAME "${PROJECT_NAME}-json")

set(SOURCES encoder.cpp map.cpp parser.cpp utils.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    account_assets_data_assets_item.json
    account_assets_data.json
    account_assets.json
    account_data_item_coin_item.json
    account_data_item.json
    account_info_data_coin_config_item.json
    account_info_data.json
    account_info_data_symbol_config_item.json
    account_info.json
    account.json
    account_mode.json
    action.json
    arg.json
    asset_mode.json
    books_data.json
    books_item.json
    books.json
    cancel_all_orders_ack_data.json
    cancel_all_orders_ack_data_list_item.json
    cancel_all_orders_ack.json
    cancel_order_ack_data.json
    cancel_order_ack.json
    category.json
    category_ws.json
    delivery_period.json
    error.json
    event.json
    fill_data_item_fee_detail_item.json
    fill_data_item.json
    fill_history_data.json
    fill_history_data_list_item_fee_detail_item.json
    fill_history_data_list_item.json
    fill_history.json
    fill.json
    futures_type.json
    hold_mode.json
    hold_side.json
    instruments_data_item.json
    instruments.json
    login.json
    margin_mode.json
    message.json
    modify_order_ack_data.json
    modify_order_ack.json
    open_orders_data.json
    open_orders_data_list_item_fee_detail_item.json
    open_orders_data_list_item.json
    open_orders.json
    order_data_item_fee_detail_item.json
    order_data_item.json
    order.json
    order_source.json
    order_status.json
    order_type.json
    place_order_ack_data.json
    place_order_ack.json
    position_data_item.json
    position_info_data.json
    position_info_data_list_item.json
    position_info.json
    position.json
    position_status.json
    pos_mode.json
    pos_side.json
    public_trade_data_item.json
    public_trade.json
    side.json
    stp_mode.json
    subscribe.json
    symbol_status.json
    symbol_type.json
    ticker_data_item.json
    ticker.json
    time_in_force.json
    topic.json
    trade_scope.json
    trade_side.json
    trading_status.json)

roq_autogen_hpp(
  OUTPUT
  AUTOGEN_HEADERS
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "json")

add_custom_target(autogen-json-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen_cpp(
  OUTPUT
  AUTOGEN_SOURCES
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "json")

roq_gitignore(OUTPUT .gitignore SOURCES "${TARGET_NAME}" ${AUTOGEN_HEADERS} ${AUTOGEN_SOURCES})

add_library("${TARGET_NAME}" OBJECT ${SOURCES} ${AUTOGEN_SOURCES})

add_dependencies("${TARGET_NAME}" autogen-json-headers)

target_link_libraries("${TARGET_NAME}" PRIVATE roq-core::roq-core roq-codec::roq-codec roq-utils::roq-utils roq-logging::roq-logging fmt::fmt)
