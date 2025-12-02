#include <fix/core/Validator.hpp>


std::string validheader1 = "8=FIX.4.4\x01"
"9=112\x01"
"35=D\x01"
"49=BUY_SIDE\x01"
"56=SELL_SIDE\x01"
"34=1\x01"
"52=20251202-12:00:00.000\x01";

std::string validheader2 = "8=FIX.4.4\x01"
"9=145\x01"
"35=8\x01"
"49=EXEC_BROKER\x01"
"56=CLIENT_FIRM\x01"
"115=TP_COMPANY\x01"
"128=DEST_COMPANY\x01"
"34=42\x01"
"50=TRADER_1\x01"
"57=DESK_A\x01"
"142=LON-DESK1\x01"
"143=NY-DESK2\x01"
"52=20251202-12:05:30.123\x01"
;

std::string validheader3 = "9=250\x01"
"35=D\x01"
"49=ORIG_FIRM\x01"
"56=DEST_FIRM\x01"
"115=TP_COMPANY\x01"
"627=2\x01"
"628=HOP1_COMP\x01"
"629=20251202-12:00:01.000\x01"
"630=1001\x01"
"628=HOP2_COMP\x01"
"629=20251202-12:00:02.000\x01"
"630=1002\x01"
"34=10\x01"
"52=20251202-12:15:45.555\x01"
"212=24\x01"
"213=<Fixml>...</Fixml>\x01"
;


int main() {
    return 0;
}