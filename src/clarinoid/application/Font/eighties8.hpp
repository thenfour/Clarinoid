const uint8_t Eighties8pt7bBitmaps[] PROGMEM = {
    0x00, 0xF4, 0xB4, 0x57, 0xD4, 0xAF, 0xA8, 0x4E, 0x73, 0x90, 0x92, 0x49, 0x4A, 0x4B, 0xA5, 0x60, 0x6A, 0x90, 0x95,
    0x60, 0xAA, 0x80, 0x5D, 0x00, 0x60, 0xE0, 0x80, 0x12, 0x48, 0x56, 0xDA, 0x80, 0x59, 0x25, 0xC0, 0x54, 0xA9, 0xC0,
    0xC5, 0x13, 0x80, 0x2E, 0xF2, 0x40, 0xF3, 0x13, 0x80, 0x53, 0x5A, 0x80, 0xE5, 0x24, 0x80, 0x55, 0x5A, 0x80, 0x56,
    0xB2, 0x80, 0xA0, 0x46, 0x64, 0xE3, 0x80, 0x98, 0x54, 0xA0, 0x80, 0x38, 0x8A, 0x4D, 0x5A, 0xB3, 0x90, 0x00, 0x56,
    0xFB, 0x40, 0xD7, 0x5B, 0x80, 0x72, 0x48, 0xC0, 0xD6, 0xDB, 0x80, 0x73, 0xC9, 0xC0, 0x73, 0xC9, 0x00, 0x72, 0xDA,
    0xC0, 0xB7, 0xDB, 0x40, 0xFC, 0x55, 0x60, 0xB7, 0x5B, 0x40, 0xAA, 0xB0, 0xDD, 0x6B, 0x5A, 0xD4, 0x9D, 0xB9, 0x99,
    0x56, 0xDA, 0x80, 0xD6, 0xE9, 0x00, 0x56, 0xDA, 0xC0, 0xD6, 0xEB, 0x40, 0x72, 0x13, 0x80, 0xE9, 0x24, 0x80, 0xB6,
    0xDA, 0xC0, 0xB6, 0xDA, 0x80, 0xAD, 0x6B, 0x5A, 0xA8, 0xB5, 0x5B, 0x40, 0xB6, 0xA4, 0x80, 0xE5, 0x49, 0xC0, 0xEA,
    0xB0, 0x84, 0x21, 0xD5, 0x70, 0xFC, 0x63, 0x18, 0xC6, 0x31, 0x8C, 0x7E, 0xE0, 0x90, 0x56, 0xB0, 0x93, 0x5B, 0x80,
    0x72, 0x30, 0x25, 0xDA, 0xC0, 0x57, 0x30, 0x6E, 0xA0, 0x75, 0x94, 0x93, 0x5B, 0x40, 0xBC, 0x45, 0x58, 0x92, 0xEB,
    0x40, 0xAA, 0x90, 0x55, 0x6B, 0x50, 0xD6, 0xD0, 0x56, 0xA0, 0x56, 0xE8, 0x56, 0xB2, 0x56, 0x40, 0x70, 0xE0, 0xAE,
    0x90, 0xB6, 0xB0, 0xB6, 0xA0, 0xAD, 0x6A, 0xA0, 0xA9, 0x50, 0xB5, 0x94, 0xE5, 0x70, 0xFC, 0x63, 0x18, 0xC6, 0x31,
    0x8C, 0x7E, 0xFC, 0xFC, 0x63, 0x18, 0xC6, 0x31, 0x8C, 0x7E, 0x45, 0x44};

GFXglyph Eighties8pt7bGlyphs[] = {{0, 1, 1, 5, 0, 0},      // 0x20 ' '
                                  {1, 1, 6, 2, 0, -5},     // 0x21 '!'
                                  {2, 3, 2, 4, 0, -5},     // 0x22 '"'
                                  {3, 5, 6, 6, 0, -5},     // 0x23 '#'
                                  {7, 3, 7, 4, 0, -5},     // 0x24 '$'
                                  {10, 4, 4, 5, 0, -4},    // 0x25 '%'
                                  {12, 4, 6, 5, 0, -5},    // 0x26 '&'
                                  {15, 2, 2, 3, 0, -5},    // 0x27 '''
                                  {16, 2, 6, 3, 0, -5},    // 0x28 '('
                                  {18, 2, 6, 3, 0, -5},    // 0x29 ')'
                                  {20, 3, 3, 4, 0, -3},    // 0x2A '*'
                                  {22, 3, 3, 4, 0, -3},    // 0x2B '+'
                                  {24, 2, 2, 3, 0, 0},     // 0x2C ','
                                  {25, 3, 1, 4, 0, -2},    // 0x2D '-'
                                  {26, 1, 1, 2, 0, 0},     // 0x2E '.'
                                  {27, 4, 4, 5, 0, -4},    // 0x2F '/'
                                  {29, 3, 6, 4, 0, -5},    // 0x30 '0'
                                  {32, 3, 6, 4, 0, -5},    // 0x31 '1'
                                  {35, 3, 6, 4, 0, -5},    // 0x32 '2'
                                  {38, 3, 6, 4, 0, -5},    // 0x33 '3'
                                  {41, 3, 6, 4, 0, -5},    // 0x34 '4'
                                  {44, 3, 6, 4, 0, -5},    // 0x35 '5'
                                  {47, 3, 6, 4, 0, -5},    // 0x36 '6'
                                  {50, 3, 6, 4, 0, -5},    // 0x37 '7'
                                  {53, 3, 6, 4, 0, -5},    // 0x38 '8'
                                  {56, 3, 6, 4, 0, -5},    // 0x39 '9'
                                  {59, 1, 3, 2, 0, -3},    // 0x3A ':'
                                  {60, 2, 4, 3, 0, -2},    // 0x3B ';'
                                  {61, 2, 3, 3, 0, -3},    // 0x3C '<'
                                  {62, 3, 3, 4, 0, -3},    // 0x3D '='
                                  {64, 2, 3, 3, 0, -3},    // 0x3E '>'
                                  {65, 3, 6, 4, 0, -5},    // 0x3F '?'
                                  {68, 7, 7, 8, 0, -5},    // 0x40 '@'
                                  {75, 3, 6, 4, 0, -5},    // 0x41 'A'
                                  {78, 3, 6, 4, 0, -5},    // 0x42 'B'
                                  {81, 3, 6, 4, 0, -5},    // 0x43 'C'
                                  {84, 3, 6, 4, 0, -5},    // 0x44 'D'
                                  {87, 3, 6, 4, 0, -5},    // 0x45 'E'
                                  {90, 3, 6, 4, 0, -5},    // 0x46 'F'
                                  {93, 3, 6, 4, 0, -5},    // 0x47 'G'
                                  {96, 3, 6, 4, 0, -5},    // 0x48 'H'
                                  {99, 1, 6, 2, 0, -5},    // 0x49 'I'
                                  {100, 2, 6, 3, 0, -5},   // 0x4A 'J'
                                  {102, 3, 6, 4, 0, -5},   // 0x4B 'K'
                                  {105, 2, 6, 3, 0, -5},   // 0x4C 'L'
                                  {107, 5, 6, 6, 0, -5},   // 0x4D 'M'
                                  {111, 4, 6, 5, 0, -5},   // 0x4E 'N'
                                  {114, 3, 6, 4, 0, -5},   // 0x4F 'O'
                                  {117, 3, 6, 4, 0, -5},   // 0x50 'P'
                                  {120, 3, 6, 4, 0, -5},   // 0x51 'Q'
                                  {123, 3, 6, 4, 0, -5},   // 0x52 'R'
                                  {126, 3, 6, 4, 0, -5},   // 0x53 'S'
                                  {129, 3, 6, 4, 0, -5},   // 0x54 'T'
                                  {132, 3, 6, 4, 0, -5},   // 0x55 'U'
                                  {135, 3, 6, 4, 0, -5},   // 0x56 'V'
                                  {138, 5, 6, 6, 0, -5},   // 0x57 'W'
                                  {142, 3, 6, 4, 0, -5},   // 0x58 'X'
                                  {145, 3, 6, 4, 0, -5},   // 0x59 'Y'
                                  {148, 3, 6, 4, 0, -5},   // 0x5A 'Z'
                                  {151, 2, 6, 3, 0, -5},   // 0x5B '['
                                  {153, 4, 4, 5, 0, -4},   // 0x5C '\'
                                  {155, 2, 6, 3, 0, -5},   // 0x5D ']'
                                  {157, 5, 11, 6, 1, -10}, // 0x5E '^'
                                  {164, 3, 1, 4, 0, 0},    // 0x5F '_'
                                  {165, 2, 2, 3, 0, -5},   // 0x60 '`'
                                  {166, 3, 4, 4, 0, -3},   // 0x61 'a'
                                  {168, 3, 6, 4, 0, -5},   // 0x62 'b'
                                  {171, 3, 4, 4, 0, -3},   // 0x63 'c'
                                  {173, 3, 6, 4, 0, -5},   // 0x64 'd'
                                  {176, 3, 4, 4, 0, -3},   // 0x65 'e'
                                  {178, 2, 6, 3, 0, -5},   // 0x66 'f'
                                  {180, 3, 5, 4, 0, -3},   // 0x67 'g'
                                  {182, 3, 6, 4, 0, -5},   // 0x68 'h'
                                  {185, 1, 6, 2, 0, -5},   // 0x69 'i'
                                  {186, 2, 7, 3, 0, -5},   // 0x6A 'j'
                                  {188, 3, 6, 4, 0, -5},   // 0x6B 'k'
                                  {191, 2, 6, 3, 0, -5},   // 0x6C 'l'
                                  {193, 5, 4, 6, 0, -3},   // 0x6D 'm'
                                  {196, 3, 4, 4, 0, -3},   // 0x6E 'n'
                                  {198, 3, 4, 4, 0, -3},   // 0x6F 'o'
                                  {200, 3, 5, 4, 0, -3},   // 0x70 'p'
                                  {202, 3, 5, 4, 0, -3},   // 0x71 'q'
                                  {204, 3, 4, 4, 0, -3},   // 0x72 'r'
                                  {206, 3, 4, 4, 0, -3},   // 0x73 's'
                                  {208, 2, 6, 3, 0, -5},   // 0x74 't'
                                  {210, 3, 4, 4, 0, -3},   // 0x75 'u'
                                  {212, 3, 4, 4, 0, -3},   // 0x76 'v'
                                  {214, 5, 4, 6, 0, -3},   // 0x77 'w'
                                  {217, 3, 4, 4, 0, -3},   // 0x78 'x'
                                  {219, 3, 5, 4, 0, -3},   // 0x79 'y'
                                  {221, 3, 4, 4, 0, -3},   // 0x7A 'z'
                                  {223, 5, 11, 6, 1, -10}, // 0x7B '{'
                                  {230, 1, 6, 2, 0, -5},   // 0x7C '|'
                                  {231, 5, 11, 6, 1, -10}, // 0x7D '}'
                                  {238, 5, 3, 6, 0, -3}};  // 0x7E '~'

GFXfont Eighties8pt7b = {(uint8_t *)Eighties8pt7bBitmaps, (GFXglyph *)Eighties8pt7bGlyphs, 0x20, 0x7E, 6};

// Approx. 912 bytes
