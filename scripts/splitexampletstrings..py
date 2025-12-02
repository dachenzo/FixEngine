




def splitter(message: str, delimiter: str = '|') -> list[str]:
    fields = message.split(delimiter)
    for f in fields:

        print('"' + f + '"', end="|\n")



splitter("8=FIX.4.4|9=250|35=D|49=ORIG_FIRM|56=DEST_FIRM|115=TP_COMPANY|627=2|628=HOP1_COMP|629=20251202-12:00:01.000|630=1001|628=HOP2_COMP|629=20251202-12:00:02.000|630=1002|34=10|52=20251202-12:15:45.555|212=24|213=<Fixml>...</Fixml>|")


