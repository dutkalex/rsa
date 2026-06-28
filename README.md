This is a toy project inspired by [Alex Stepanov's lectures](https://www.youtube.com/watch?v=wrmXDxn_Zuc&list=PLHxtyCq_WDLV5N5zUCBCDC2WqF1VBDGg1), which implements RSA encryption.

# Todos / ideas

- [x] implement fundamental algorithms (russian peasant, gcd, miller-rabin, etc.)
- [x] implement public/private key generation
- [x] implement encoding/decoding function
- [x] implement the `rsa::bigint` arbitrary precision integer class
- [x] implement large integer arithmetic
- [x] implement large integer random number generation
- [x] generalize keygen to support `rsa::bigint`
- [ ] generalize `rsa::impl::le_unsigned_divide` to variable length encodings
- [ ] optimize arithmetic for very large integers (Knuth algorithm D)
- [ ] decouple storage policy from `rsa::bigint` implementation
- [ ] extend encoding/decoding to arbitrary streams of bytes
- [ ] implement on the fly compression/decompression
