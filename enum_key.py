numstart = 0x0000
keys = open("D:\\Microcontroller Development\\STM32\\Projects\\TCalc\\keys.txt", "r")

for key in keys:
    print(f"#define KEY_{key.upper()[:-1]}{' ' * (10 - len(key[:-1]))}0x{'0' * (6 - len(hex(numstart)))}{hex(numstart).upper()[2:]}")
    numstart += 1
keys.close()
