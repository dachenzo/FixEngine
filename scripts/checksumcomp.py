

msg = "8=FIX.4.4\x019=97\x0135=2\x0134=5\x0149=SENDER\x0156=TARGET\x0152=20240606-12:00:00.000\x0143=Y\x01122=20240605-11:59:59.000\x017=10\x0116=20\x01"
     

checksum = sum(ord(c) for c in msg) % 256
print(f"Computed checksum: {checksum:03}")
print(f"Msg without checksum field: {msg[:-7]}")