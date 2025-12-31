Current Focus: 
SeqNumTracking -> Integrated

Tasks:
- generic message to parsed message
- make validator take in the pasred message
- stub out core functionality for message store
- implement the functionality
- test make valid message


Later:
- Integrate seq provider with message store
- Session::send_message_ is broken
- handle gap fills and correctly update both message store and the seq provider
- Message Factory is broken 


BackLog:
- Implement Reject Messages
- Implement Logout Messages
