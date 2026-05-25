.. _roq-bitget:

.. |dagger| unicode:: U+2020
.. |double-dagger| unicode:: U+2021
.. |right-arrow| unicode:: U+2192
.. |right-double-arrow| unicode:: U+21D2
.. |left-right-double-arrow| unicode:: U+21D4
.. |check-mark| unicode:: U+2705
.. |cross-mark| unicode:: U+274C
.. |negative-cross-mark| unicode:: U+274E
.. |footnote-1| unicode:: U+2776
.. |footnote-2| unicode:: U+2777
.. |footnote-3| unicode:: U+2778


`roq-bitget <https://github.com/roq-trading/roq-bitget/>`__
===========================================================


.. tab:: Unstable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/unstable \
           roq-bitget

.. tab:: Stable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/stable \
           roq-bitget


Supports
--------

.. grid::  2
  :gutter: 2

  .. grid-item-card::  Products

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:enumerator:`Spot <roq::SecurityType::SPOT>`
        - |check-mark|
        -
      * - :cpp:enumerator:`Futures <roq::SecurityType::FUTURES>`
        - |check-mark|
        -
      * - :cpp:enumerator:`Swap <roq::SecurityType::SWAP>`
        - |check-mark|
        -
      * - :cpp:enumerator:`Option <roq::SecurityType::OPTION>`
        - |cross-mark|
        -

  .. grid-item-card::  Market Data

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`ReferenceData <roq::ReferenceData>`
        - |check-mark|
        - |footnote-1|
      * - :cpp:class:`MarketStatus <roq::MarketStatus>`
        - |check-mark|
        - |footnote-1|
      * - :cpp:class:`TopOfBook <roq::TopOfBook>`
        - |check-mark|
        -
      * - :cpp:class:`MarketByPrice <roq::MarketByPriceUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`MarketByOrder <roq::MarketByOrderUpdate>`
        - |cross-mark|
        -
      * - :cpp:class:`TradeSummary <roq::TradeSummary>`
        - |check-mark|
        -
      * - :cpp:class:`Statistics <roq::StatisticsUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`TimeSeries <roq::TimeSeriesUpdate>`
        - |check-mark|
        -

  .. grid-item-card::  Orders & Quotes

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`CreateOrder <roq::CreateOrder>`
        - |check-mark|
        - |footnote-2|
      * - :cpp:class:`ModifyOrder <roq::ModifyOrder>`
        - |check-mark|
        -
      * - :cpp:class:`CancelOrder <roq::CancelOrder>`
        - |check-mark|
        -
      * - :cpp:class:`CancelAllOrders <roq::CancelAllOrders>`
        - |check-mark|
        -
      * - :cpp:class:`MassQuote <roq::MassQuote>`
        - |cross-mark|
        -
      * - :cpp:class:`CancelQuotes <roq::CancelQuotes>`
        - |cross-mark|
        -

  .. grid-item-card::  Account

    .. list-table::
      :widths: auto
      :align: left

      * - :cpp:class:`Funds <roq::FundsUpdate>`
        - |check-mark|
        -
      * - :cpp:class:`Position <roq::PositionUpdate>`
        - |check-mark|
        -

.. note::

   |check-mark| = Available.

   |negative-cross-mark| = Not implemented.

   |cross-mark| = Unavailable.

   |footnote-1| The exchange protocol does not support streaming updates for reference data and market status.

   |footnote-2| The exchange protocol does not support stop orders.


Using
-----

.. code-block:: shell

   $ roq-bitget [FLAGS]


.. _roq-bitget-flags:

Flags
-----

.. code-block:: shell

   $ roq-bitget --help

.. tab:: Flags

   .. include:: flags/flags.rstinc

.. tab:: REST

   .. include:: flags/rest.rstinc

.. tab:: WS

   .. include:: flags/ws.rstinc

.. tab:: Misc

   .. include:: flags/misc.rstinc


Environments
------------

.. tab:: Prod

   .. code-block:: shell

      $ --flagfile $CONDA_PREFIX/share/roq-bitget/flags/prod/flags.cfg

   .. include:: flags/prod/flags.cfg
     :code: shell


Configuration
-------------

.. code-block:: shell

   $ --config_file $CONDA_PREFIX/share/roq-bitget/config.toml

.. important::

   This template will be replaced when the software is upgraded.
   Make a copy and modify to your own needs.

.. include:: config.toml
   :code: toml


Market Data
-----------


Inbound
~~~~~~~

.. tab:: TradingStatus

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Enum
       -
       -

     * - :code:`listed`
       - |right-double-arrow|
       - :cpp:enumerator:`UNDEFINED <roq::TradingStatus::UNDEFINED>`

     * - :code:`online`
       - |right-double-arrow|
       - :cpp:enumerator:`OPEN <roq::TradingStatus::OPEN>`

     * - :code:`limit_open`
       - |right-double-arrow|
       - :cpp:enumerator:`PRE_OPEN <roq::TradingStatus::PRE_OPEN>`

     * - :code:`offline`
       - |right-double-arrow|
       - :cpp:enumerator:`CLOSE <roq::TradingStatus::CLOSE>`

     * - :code:`restrictedAPI`
       - |right-double-arrow|
       - :cpp:enumerator:`HALT <roq::TradingStatus::HALT>`


.. tab:: StatisticsType

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Event
       - Field
       -
       -

     * - :code:`Ticker`
       - :code:`openPrice24h`
       - |right-double-arrow|
       - :cpp:enumerator:`OPEN_PRICE <roq::StatisticsType::OPEN_PRICE>`

     * - :code:`Ticker`
       - :code:`highPrice24h`
       - |right-double-arrow|
       - :cpp:enumerator:`HIGHEST_TRADED_PRICE <roq::StatisticsType::HIGHEST_TRADED_PRICE>`

     * - :code:`Ticker`
       - :code:`lowPrice24h`
       - |right-double-arrow|
       - :cpp:enumerator:`LOWEST_TRADED_PRICE <roq::StatisticsType::LOWEST_TRADED_PRICE>`

     * - :code:`Ticker`
       - :code:`indexPrice`
       - |right-double-arrow|
       - :cpp:enumerator:`INDEX_VALUE <roq::StatisticsType::INDEX_VALUE>`

     * - :code:`Ticker`
       - :code:`volume24h`
       - |right-double-arrow|
       - :cpp:enumerator:`TRADE_VOLUME <roq::StatisticsType::TRADE_VOLUME>`

     * - :code:`Ticker`
       - :code:`fundingRate`
       - |right-double-arrow|
       - :cpp:enumerator:`FUNDING_RATE <roq::StatisticsType::FUNDING_RATE>`

     * - :code:`Ticker`
       - :code:`openInterest`
       - |right-double-arrow|
       - :cpp:enumerator:`OPEN_INTEREST <roq::StatisticsType::OPEN_INTEREST>`



Order Management
----------------


Inbound
~~~~~~~

.. tab:: OrderType

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Enum
       -
       -

     * - :code:`MARKET`
       - |right-double-arrow|
       - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`

     * - :code:`LIMIT`
       - |right-double-arrow|
       - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`


.. tab:: TimeInForce

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Enum
       -
       -

     * - :code:`ioc`
       - |right-double-arrow|
       - :cpp:enumerator:`IOC <roq::TimeInForce::IOC>`

     * - :code:`fok`
       - |right-double-arrow|
       - :cpp:enumerator:`FOK <roq::TimeInForce::FOK>`

     * - :code:`gtc`
       - |right-double-arrow|
       - :cpp:enumerator:`GTC <roq::TimeInForce::GTC>`

     * - :code:`post_only`
       - |right-double-arrow|
       - :cpp:enumerator:`GTC <roq::TimeInForce::GTC>`


.. tab:: OrderStatus

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - Enum
       -
       -

     * - :code:`live`
       - |right-double-arrow|
       - :cpp:enumerator:`WORKING <roq::OrderStatus::WORKING>`

     * - :code:`new`
       - |right-double-arrow|
       - :cpp:enumerator:`WORKING <roq::OrderStatus::WORKING>`

     * - :code:`partially_filled`
       - |right-double-arrow|
       - :cpp:enumerator:`WORKING <roq::OrderStatus::WORKING>`

     * - :code:`filled`
       - |right-double-arrow|
       - :cpp:enumerator:`COMPLETED <roq::OrderStatus::COMPLETED>`

     * - :code:`cancelled`
       - |right-double-arrow|
       - :cpp:enumerator:`CANCELED <roq::OrderStatus::CANCELED>`


Outbound
~~~~~~~~

.. tab:: CreateOrder

   .. list-table::
     :header-rows: 1
     :widths: auto
     :align: left

     * - :cpp:member:`order_type <roq::CreateOrder::order_type>`
       - :cpp:member:`execution_instructions <roq::CreateOrder::execution_instructions>`
       - :cpp:member:`price <roq::CreateOrder::price>`
       - :cpp:member:`stop_price <roq::CreateOrder::stop_price>`
       -
       - :code:`orderType`
       - :code:`timeInForce`
       - :code:`price`
       - :code:`reduceOnly`

     * - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`
       -
       - :code:`NaN`
       - :code:`NaN`
       - |right-double-arrow|
       - :code:`market`
       -
       - |cross-mark|
       - |cross-mark|

     * - :cpp:enumerator:`MARKET <roq::OrderType::MARKET>`
       -
       - :code:`NaN`
       - |check-mark|
       - |right-double-arrow|
       -
       -
       - |cross-mark|
       -

     * - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`
       -
       - |check-mark|
       - :code:`NaN`
       - |right-double-arrow|
       - :code:`limit`
       -
       - |check-mark|
       -

     * - :cpp:enumerator:`LIMIT <roq::OrderType::LIMIT>`
       - :cpp:enumerator:`PARTICIPATE_DO_NOT_INITIATE <roq::ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE>`
       - |check-mark|
       - :code:`NaN`
       - |right-double-arrow|
       -
       -
       - |check-mark|
       -

     * - :cpp:enumerator:`Limit <roq::OrderType::LIMIT>`
       -
       - |check-mark|
       - |check-mark|
       - |right-double-arrow|
       -
       -
       - |check-mark|
       -


.. tab:: ModifyOrder

   TBD


.. tab:: CancelOrder

   TBD


.. tab:: CancelAllOrders

   TBD


Comments
--------

* Stop orders are not supported


References
----------

Common
~~~~~~

* :ref:`Using Conda <tutorial-conda>`
* :ref:`Using Flags <abseil-cpp>`
* :ref:`Gateway Flags <gateway-flags>`
* :ref:`Gateway Config <gateway-config>`


GitHub
~~~~~~

* `roq-bitget <https://github.com/roq-trading/roq-bitget/>`__


Exchange
~~~~~~~~

* `Website <https://www.bitget.com/>`__
* `Documentation <https://www.bitget.com/api-doc/common/intro>`__
