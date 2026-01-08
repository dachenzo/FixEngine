Current Focus: 
SeqNumTracking -> Integrated

Specific: Integrate fatcory scratch with message factory
- finishing logogn and the rest of the admin message generation in message factory

BroadTasks:
- Pin down when to update out seq num
- Integrate fatcory scratch with message factory
- Test 
- test_wire_writer
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
