MORSE_CODE_DICT = { 'A':'.-', 'B':'-...',
                    'C':'-.-.', 'D':'-..', 'E':'.',
                    'F':'..-.', 'G':'--.', 'H':'....',
                    'I':'..', 'J':'.---', 'K':'-.-',
                    'L':'.-..', 'M':'--', 'N':'-.',
                    'O':'---', 'P':'.--.', 'Q':'--.-',
                    'R':'.-.', 'S':'...', 'T':'-',
                    'U':'..-', 'V':'...-', 'W':'.--',
                    'X':'-..-', 'Y':'-.--', 'Z':'--..'}

def converter(code):
    #43210
    result = 0
    shift = 2**4
    for i in code:
        if(i == '-'):
            result+= shift
        shift //= 2
    result += len(code)*(2**5)
    return result
        


for i in range(26):
    print(converter(MORSE_CODE_DICT[chr(ord('A')+i)]),format(converter(MORSE_CODE_DICT[chr(ord('A')+i)]),'b'))