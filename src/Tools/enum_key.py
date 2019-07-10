from sys import argv
from random import randrange

numstart = 0x0000
keys = open(argv[1], "r")

used_nums = set()

include_guard = None
randnum = False

for key in keys:
    key = key[:-1]
    if key == "":
        print()
        continue
    if key.startswith('EK_RESTART_NUM'):
        n = key.split()[1]
        if n.startswith('0x') or n.startswith('0X'):
            numstart = int(n[2:], base=16)
        else:
            numstart = int(n)
        randnum = False
        continue
    elif key.startswith('EK_INCLUDE_GUARD'):
        include_guard = key.split()[1]
        print(f"#ifndef {include_guard}")
        print(f"#define {include_guard}")
        print()
        continue
    elif key.startswith('EK_RANDOM_NUM'):
        randnum = True
        continue

    eq = key.find('=')
    num = 0
    if eq != -1:
        num = int(key.split('=')[1])
    else:
        if not randnum:
            num = numstart
            numstart += 1
            while num in used_nums:
                num = numstart
                numstart += 1
        else:
            num = randrange(0, 0x10000)
            while num in used_nums:
                num = randrange(0, 0x10000)
    
    used_nums.add(num)

    name = key.upper() if eq == -1 else key.split('=')[0].upper()
    
    print(f"#define KEY_{name}{' ' * (10 - len(name))}0x{'0' * (6 - len(hex(num)))}{hex(num).upper()[2:]}")

if include_guard != None:
    print()
    print("#endif")
    
keys.close()
