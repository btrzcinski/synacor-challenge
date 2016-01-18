ver 0 b c = b + 1
ver a 0 c = ver (a-1) c c
ver a b c = ver (a-1) (ver a (b-1) c) c

