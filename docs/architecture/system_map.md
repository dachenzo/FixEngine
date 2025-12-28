FIX Engine System Map

1. Transport Layer
   - TCP Session lifecycle
   - Reconnect logic
   - Heartbeats
   - Timeouts

2. Session Layer
   - SeqNum tracking
   - Logon / Logout state machine
   - ResendRequest handling
   - GapFill logic

3. Message Processing
   - Parsing
   - Validation
   - Generation
   - Normalisation
   - Routing

4. Schema & Metadata
   - Message schemas
   - Field schemas
   - Repeating groups
   - Versioning (FIX 4.2/4.4)

5. Validation
   - Required fields
   - Field order
   - Type validation
   - Conditional rules
   - Group constraints

6. Persistence
   - Message store
   - SeqNum store
   - Recovery on restart

7. Concurrency Model
   - Threading model
   - Queues / ring buffers
   - Memory ownership

8. Error Handling
   - Reject generation
   - Session-level errors
   - Recoverable vs fatal

9. Testing & Tooling
   - Unit tests
   - Golden messages
   - Fuzzing
   - Benchmarks

10. Logging 
   - Mpsc ring
   - Dedicated Drain thread
   - Json Output

11. Interaction
   - CLI