# pyctpclient Change History

## 0.3.5rc1

1. Change to v6.3.15 front_se
2. Enter rc stage

## 0.3.4b2

1. Add `on_exception` callback.

## 0.3.3b0

1. Register `std::runtime_error` as `RequestError`

## 0.3.2b0

1. `delete_order` make `request_id` having a default value.

## 0.3.1a1

1. FIX: `query_market_data` response segment fault.
2. FIX: 1 minute bar volume/turnover compute error.

## 0.3.0a0

1. Use pybind11 instead of boost.python
2. Bundle libthostftdc*.so into wheel package.
3. Do some stuff in python to skip boost.filesystem

## 0.2.4a0

1. Use `dict` for extra options in `insert_order`

## 0.2.3a1

1. Use `moodycamel::ConcurrentQueue` instead of `std::queue`.
2. Change `OrderSubmitStatus` `OrderStatus` and `OrderActionStatus` to enumeration, instead of string.
3. Add TradingDay and ActionDay to M1Bar and TickBar.

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
