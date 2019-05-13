# pyctpclient Change History

## 0.2.2a2

1. Add request queue for query.
2. Add `atomic_bool` as request lock.

## 0.2.1a1

1. `on_rsp_market_data` 增加 `request_id`
2. `query_market_data` 增加 `request_id`，默认值为 0

## 0.2.0a3

1. Change architecture of the program: add a message queue between CTP SPI and CTP client.
2. Change `run` to `init`
3. Must call `join` after `init`
4. FIX copy nullptr of response.

## 0.1.4a0

1. Add GIL for each callback functions.

## 0.1.3a2

1. Compute volume and turnover in one minute
2. Add 1s timer.
3. FIX return string cannot be save
4. Use `extract<char*>` instead of alloc new memory in subscribe/unsubscribe market data.

## 0.1.2a0

1. Add `join` for client.
2. Remove `on_rsp_order_action`, merge into `on_err_order_action`

## 0.1.1a0

1. `delete_order` and `modify_order` are binding to `insert_order`

## 0.1.0a0

Initial release.
