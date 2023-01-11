import math

print("const uint32_t sqrt_integer_guess_table[33] = {")
for i in range(33):
    #x = math.sqrt((0xFFFFFFFF >> i) * math.sqrt(2) / 2)
    x = math.sqrt((0xFFFFFFFF >> i) * math.sqrt(2) / 2)
    x = math.ceil(x)
    print(f"{x},")
print("};")
