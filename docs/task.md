Current Focus: 
SeqNumTracking -> Integrated

Tasks:
- Message Factory Scratch
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
- Message Factory is broken 


BackLog:
- Implement Reject Messages
- Implement Logout Messages
