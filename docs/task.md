Current Focus: 
SeqNumTracking -> Integrated

Specific:  Integrate Wirewriter and Arena with send stream in session
-


BroadTasks:
- Integrate Wirewriter and Arena with send stream in session
- Pin down when to update out seq num
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
