

msg = "8=FIX.4.4\x019=25\x0135=D\x0149=SENDER\x0156=TARGET\x0110=197\x01"
     

checksum = sum(ord(c) for c in msg[:-7]) % 256
print(f"Computed checksum: {checksum:03}")
print(f"Msg without checksum field: {msg[:-7]}")