Fix::Arenahandle references Fix::Arena with a pointer so there will be lifetime issues if arena is destroyed before its handle. 
ResendStream and Messagestore assumes the seq numbers map directly to the underlying messag store buffer
Seq next out updates require that u only build messages that u send 
The results of the validation must be used in the same scope as the validator