from sys import argv

numstart = 0x0000
keys = open(argv[1], "r")

include_guard = None

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
        continue
    elif key.startswith('EK_INCLUDE_GUARD'):
        include_guard = key.split()[1]
        print(f"#ifndef {include_guard}")
        print(f"#define {include_guard}")
        print()
        continue

    eq = key.find('=')
    
    if eq == -1:
        print(f"#define KEY_{key.upper()}{' ' * (10 - len(key))}0x{'0' * (6 - len(hex(numstart)))}{hex(numstart).upper()[2:]}")
        numstart += 1
    else:
        print(f"#define KEY_{key.split('=')[0].upper()}{' ' * (10 - len(key.split('=')[0]))}{key.split('=')[1]}")

if include_guard != None:
    print()
    print("#endif")
    
keys.close()
