




def splitter(message: str, delimiter: str = '|') -> list[str]:
    fields = message.split(delimiter)
    for f in fields:

        print('"' + f + '"', end="|\n")



splitter("8=FIX.4.4|9=123|35=D|34=2|49=SENDER|52=20251206-18:20:00.000|56=TARGET|11=ORDER123|21=1|38=1000|40=2|54=1|55=AAPL|60=20251206-18:20:00.000|10=172|")


