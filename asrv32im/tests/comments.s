        .global _start
        .data
l_a:    .space 84
        .text
// comment
_start: li     s0, 0            # line 8
        li     s1, 0
        la     s2, l_a
/* multiline comment
continues...
/* cursed non-nested comment
*/
        sw     s1, 0(s2)
        li /* nyaaaa */ s0, 1            // line 9
        li     s1, 1
        la     s2, l_a
# comment
 #comment
        li     s3, 4
        mul    s0, s0, s3
        add    s2, s2, s0
        sw     s1, 0(s2)
        li     s0, 2            /* line 10 */
        li     s1, 2
        li     s3, 4
