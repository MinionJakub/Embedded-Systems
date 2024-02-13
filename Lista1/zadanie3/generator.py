a = PD0 = 1;
b = PD1 = 2;
c = PD2 = 4;
d = PD3 = 8;
e = PD4 = 16;
f = PD5 = 32;
g = PD6 = 64;

tab = [
    a + b + c + d + e + f, #0
    b + c, #1
    a + b + g + e + d, #2
    a + b + g + c + d, #3
    f + g + b + c, #4
    a + f + g + c + d, #5
    a + f + g + e + d + c,  #6
    a + b + c, #7
    a + b + c + d + e + f + g, #8
    a + b + c + d + f + g #9
]
# for i in range(10):
print(255-tab[4],",")