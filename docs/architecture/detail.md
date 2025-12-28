markdown
// filepath: docs/architecture/system_map.md
# FIX Engine System Map

This document maps *code modules* to the engine’s functional areas.

---

## 1. Transport Layer
**Responsibility**
- TCP session lifecycle (connect/accept, read/write)
- Backpressure (write queue)
- Reconnect logic (planned)
- Timeouts/heartbeats timers (partial)

**Key code**
- Reactor / IO threads: [`Fix::Reactor`](include/fix/core/Reactor.hpp), implementation in [src/Reactor.cpp](src/Reactor.cpp)
- Connections: [`Fix::IConnection`](include/fix/core/IConnection.hpp) + ASIO implementation in [src/AsioConnection.cpp](src/AsioConnection.cpp)
- Connection factory: [src/AsioConnectionFactory.cpp](src/AsioConnectionFactory.cpp)
- Timers: [`Fix::ITimerFactory`](include/fix/core/ITimer.hpp), ASIO impl in [src/AsioTimer.cpp](src/AsioTimer.cpp)

**Notes**
- Write path uses a queued async loop in [`Fix::Session::do_write`](src/Session.cpp).
- Read path uses async read + parse loop in [`Fix::Session::do_read`](src/Session.cpp).

---

## 2. Session Layer
**Responsibility**
- SeqNum tracking
- Logon / Logout state machine (partial)
- ResendRequest / GapFill (stubs present)

**Key code**
- Core session: [`Fix::Session`](include/fix/core/Session.hpp) in [src/Session.cpp](src/Session.cpp)
- Session manager: [`Fix::SessionManager`](include/fix/core/SessionManager.hpp) in [src/SessionManager.cpp](src/SessionManager.cpp)
- Sequence provider: [src/SeqProvider.cpp](src/SeqProvider.cpp)
- Message store: [src/MessageStore.cpp](src/MessageStore.cpp), [`Fix::InboundStore`](include/fix/core/InboundMessageStore.hpp) (in-progress)

**Notes**
- Dispatch currently branches on tag 35 in [`Fix::Session::dispatch`](src/Session.cpp).
- Admin handlers exist: `handle_logon/logout/heartbeat/test_request/resend_request/sequence_reset` in [`Fix::Session`](include/fix/core/Session.hpp).

---

## 3. Message Processing
**Responsibility**
- Parsing (wire → message)
- Serialization (message → wire)
- Generation (admin messages)
- Normalisation / routing (partial)

**Key code**
- Parser: [`Fix::Parser`](include/fix/core/Parser.hpp) in [src/Parser.cpp](src/Parser.cpp)
- Builder: [`Fix::MessageBuilder`](include/fix/core/MessageBuilder.hpp) in [src/MessageBuilder.cpp](src/MessageBuilder.cpp)
- Message model: [`Fix::Message`](include/fix/core/Message.hpp) in [src/Message.cpp](src/Message.cpp)
- Serializer: [`Fix::Serializer`](include/fix/core/Serializer.hpp) in [src/Serializer.cpp](src/Serializer.cpp)
- Message generation: [`Fix::MessageFactory`](include/fix/core/MessageFactory.hpp) in [src/MessageFactory.cpp](src/MessageFactory.cpp)

---

## 4. Schema & Metadata
**Responsibility**
- Message schemas, field schemas, repeating groups
- FIX version support (currently defaults to FIX 4.4)

**Key code**
- Registry: [`Fix::Schema::Registry`](include/fix/schema/Registry.hpp)
- Header schema: [include/fix/message/Header.hpp](include/fix/message/Header.hpp)
- Trailer schema: [include/fix/message/Trailer.hpp](include/fix/message/Trailer.hpp)
- Admin schemas (example): [include/fix/message/admin/Logon.hpp](include/fix/message/admin/Logon.hpp)
- Field typing helpers: [include/fix/schema/Field.hpp](include/fix/schema/Field.hpp)

---

## 5. Validation
**Responsibility**
- Required fields, order, types
- Conditional rules & group constraints (partial)

**Key code**
- Validator: [`Fix::Validator`](include/fix/core/Validator.hpp) in [src/Validator.cpp](src/Validator.cpp)
- Validator error enums: [include/fix/error/ValidatorErrors.hpp](include/fix/error/ValidatorErrors.hpp)

---

## 6. Persistence
**Responsibility**
- Message store
- SeqNum store
- Recovery on restart (planned)

**Key code**
- Message store skeleton: [src/MessageStore.cpp](src/MessageStore.cpp), [`Fix::InboundStore`](include/fix/core/InboundMessageStore.hpp)

---

## 7. Concurrency Model
**Responsibility**
- IO threading model
- Queues / ring buffers
- Memory ownership and lifetimes

**Key code**
- Reactor threads: [`Fix::Reactor`](include/fix/core/Reactor.hpp)
- Per-session strand: `exec_` in [`Fix::Session`](include/fix/core/Session.hpp)
- Logging queue: [`Fix::Log::MpscRing`](include/fix/log/MpscRing.hpp)

---

## 8. Error Handling
**Responsibility**
- Transport vs FIX vs peripheral categorization
- Recoverable vs fatal
- Reject generation (planned)

**Key code**
- Error enums + stringification: [include/fix/error/utility.hpp](include/fix/error/utility.hpp)
- Parser errors: [include/fix/error/ParserErrors.hpp](include/fix/error/ParserErrors.hpp)
- Severity: [include/fix/error/Severity.hpp](include/fix/error/Severity.hpp)
- Session read/write classification (transport): [`Fix::Session::do_write`](src/Session.cpp)

---

## 9. Testing & Tooling
**Responsibility**
- Unit tests
- Multi-session / transport tests (ad-hoc executables)

**Key code**
- GTest suite: [tests/test_validator.cpp](tests/test_validator.cpp), [tests/test_tagscratch.cpp](tests/test_tagscratch.cpp)
- Standalone tests/examples: [tests/client.cpp](tests/client.cpp), [tests/server.cpp](tests/server.cpp), [tests/test_multi.cpp](tests/test_multi.cpp), [tests/test_parser.cpp](tests/test_parser.cpp)

---

## 10. Logging
**Responsibility**
- MPSC ring buffer
- Dedicated drain thread
- JSON output to core + per-session files

**Key code**
- Core: [`Fix::Log::LogCore`](include/fix/log/LogCore.hpp) in [src/LogCore.cpp](src/LogCore.cpp)
- Ring buffer: [`Fix::Log::MpscRing`](include/fix/log/MpscRing.hpp)
- Sink: [`Fix::Log::LogFileSink`](include/fix/log/LogFileSink.hpp) in [src/LogFileSink.cpp](src/LogFileSink.cpp)
- Session logger: [`Fix::Log::SessionLogger`](include/fix/log/SessionLogger.hpp) in [src/SessionLogger.cpp](src/SessionLogger.cpp)
- Entry format: [`Fix::Log::Entry`](include/fix/log/LogEntry.hpp)

---

## 11. Interaction
**Responsibility**
- CLI / operator controls (planned)

**Current**
- Manual “Press ENTER to stop…” loops in [tests/client.cpp](tests/client.cpp) and [tests/server.cpp](tests/server.cpp)

---