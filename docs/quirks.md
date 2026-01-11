Fix::Arenahandle references Fix::Arena with a pointer so there will be lifetime issues if arena is destroyed before its handle. 
ResendStream assumes the seq numbers map directly to the underlying messag store buffer
