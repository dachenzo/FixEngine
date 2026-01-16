Current Focus: 
SeqNumTracking -> Integrated

Specific: Refactor Parser
- Make parser purely functional: one frame -> parse res
- Fix Session Genric message types
- Fix make valid message
- add session and message back to the compilation source files
- add test_validator back to compilation source



Specific:  loop in session over resend stream
- Replace session vector buffer with plain std::byte buffer
- send the begining and end sequence number into the resend processeror
- track "in recovery state"
- Handle poss dup semantics
- Buffer out_of_order msg
- decide what to do when seq number < expected seq number
- test message factory resend request 




BroadTasks:
- Refactor Parser
- Implement reject messages correctly
- Pin down when to update out seq num
- test_wire_writer
- Fix Reject sends // they are commented out rn
- loop in session over resend stream
- gapfill message generation
- resend wire generation 
- stub out core functionality for message store
- implement the functionality
- test make valid message

Bugs:
- Resend stream currently confuses seq num for message type 


Later:
- Integrate seq provider with message store
- Session::send_message_ is broken
- handle gap fills and correctly update both message store and the seq provider
 


BackLog:
- Schematic message generation in message factory
- clock is still a dummy
- Implement Reject Messages
- Implement Logout Messages
