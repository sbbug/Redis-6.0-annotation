
SHA1(Secure Hash Algorithm 1)加密算法原理:
        
    SHA-1可以生成一个被称为消息摘要的160位（20字节）散列值，散列值通常的呈现形式为40个十六进制数。
    该算法的思想是接收一段明文，然后以一种不可逆的方式将它转换成一段密文。
    
SHA1具体实现细节：
    
    Note: 所有的变量都是无符号的32位，在计算时对2^32取模进行封装
    初始化变量:
    h0 := 0x67452301
    h1 := 0xEFCDAB89
    h2 := 0x98BADCFE
    h3 := 0x10325476
    h4 := 0xC3D2E1F0
    预处理:
    将比特‘1’追加到message
    追加k个比特位 '0', where k is the minimum number >= 0 such that the resulting message
        length (in bits) is congruent to 448(mod 512)
    append length of message (before pre-processing), in bits, as 64-bit big-endian integer
    以连续的512chunk对消息做处理:
    将消息分成以512位为单位的chunk
    for each chunk：
        分成16个32位，使用大端模式存储
        break chunk into sixteen 32-bit big-endian words w[i], 0 ≤ i ≤ 15
        16个32的词集扩展为80个32位
        Extend the sixteen 32-bit words into eighty 32-bit words:
        for i from 16 to 79
            w[i] := (w[i-3] xor w[i-8] xor w[i-14] xor w[i-16]) leftrotate 1
        Initialize hash value for this chunk:
        a := h0
        b := h1
        c := h2
        d := h3
        e := h4
        Main loop:
        对这80个词循环
        for i from 0 to 79
            if 0 ≤ i ≤ 19 then
                f := (b and c) or ((not b) and d)
                k := 0x5A827999
            else if 20 ≤ i ≤ 39
                f := b xor c xor d
                k := 0x6ED9EBA1
            else if 40 ≤ i ≤ 59
                f := (b and c) or (b and d) or(c and d)
                k := 0x8F1BBCDC
            else if 60 ≤ i ≤ 79
                f := b xor c xor d
                k := 0xCA62C1D6
            temp := (a leftrotate 5) + f + e + k + w[i]
            e := d
            d := c
            c := b leftrotate 30
            b := a
            a := temp
        Add this chunk's hash to result so far:
        h0 := h0 + a
        h1 := h1 + b
        h2 := h2 + c
        h3 := h3 + d
        h4 := h4 + e
    Produce the final hash value (big-endian):
    digest = hash = h0 append h1 append h2 append h3 append h4