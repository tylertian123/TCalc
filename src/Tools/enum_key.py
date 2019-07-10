from sys import argv
from random import randrange

numstart = 0x0000
keys = open(argv[1], "r")

used_nums = set()

include_guard = None
randnum = False

num_range_low = 0
num_range_high = 0x10000
num_length = 6
num_padding_length = 10

def parse_int_param(s):
    n = s.split()[1]
    if n.startswith('0x') or n.startswith('0X'):
        return int(n[2:], base=16)
    else:
        return int(n)

for key in keys:
    key = key[:-1]
    if key == "":
        print()
        continue
    if key.startswith('EK_RESTART_NUM'):
        numstart = parse_int_param(key)
        randnum = False
        continue
    elif key.startswith('EK_INCLUDE_GUARD'):
        include_guard = key.split()[1]
        print(f"#ifndef {include_guard}")
        print(f"#define {include_guard}")
        continue
    elif key.startswith('EK_RANDOM_NUM'):
        randnum = True
        continue
    elif key.startswith('EK_NUM_LOWBOUND'):
        num_range_low = parse_int_param(key)
        continue
    elif key.startswith('EK_NUM_HIGHBOUND'):
        num_range_high = parse_int_param(key)
        continue
    elif key.startswith('EK_NUM_LENGTH'):
        num_length = parse_int_param(key)
        continue
    elif key.startswith('EK_NUM_PADDING'):
        num_padding_length = parse_int_param(key)
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
            num = randrange(num_range_low, num_range_high)
            while num in used_nums:
                num = randrange(num_range_low, num_range_high)
    
    used_nums.add(num)

    name = key.upper() if eq == -1 else key.split('=')[0].upper()
    
    print(f"#define KEY_{name}{' ' * (num_padding_length - len(name))}0x{'0' * (num_length - len(hex(num)))}{hex(num).upper()[2:]}")

if include_guard != None:
    print()
    print("#endif")
    
keys.close()
