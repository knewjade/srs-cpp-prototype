# srs-cpp-prototype

Legal Move Generation in Tetris (in Japanese):  
https://gist.github.com/knewjade/df3403a266c4eea33c2c94fb3fb7c3b2

This repository is a reimplementation of https://github.com/ImpleLee/fast-reachability. (Thanks to ImpleLee!)
The main difference is that fast-reachability uses a row-oriented board,
while this repository uses a column-oriented board.

The height of the board that can be handled changes depending on the bit size.
(The larger the bit size, the slower it becomes)

# Required Library

https://github.com/VcDevel/std-simd

# Output Example

```
...

# EMPTY
Elapsed time (I): 65.0375 ns
Elapsed time (J): 45.5963 ns
Elapsed time (L): 45.6839 ns
Elapsed time (O): 10.7034 ns
Elapsed time (S): 47.684 ns
Elapsed time (T): 47.5947 ns
Elapsed time (Z): 46.6259 ns
# LEMONTEA
Elapsed time (I): 76.2926 ns
Elapsed time (J): 67.7769 ns
Elapsed time (L): 83.123 ns
Elapsed time (O): 12.2906 ns
Elapsed time (S): 66.2474 ns
Elapsed time (T): 69.555 ns
Elapsed time (Z): 68.322 ns
# LZT
Elapsed time (I): 858.281 ns
Elapsed time (J): 218.615 ns
Elapsed time (L): 498.89 ns
Elapsed time (O): 12.8218 ns
Elapsed time (S): 172.93 ns
Elapsed time (T): 256.695 ns
Elapsed time (Z): 1063.87 ns
```
