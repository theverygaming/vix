/*
calling convention:
args on stack last to first
move.l #0x69, %sp@-
move.l #0x420, %sp@-
(a1=0x420, a2=0x69)


*/

.global _vectest
.type _vectest, @function
_vectest:
clrl	%sp@-			/* stk_adj */
pea	-1:w			/* orig d0 */
movel	%d0,%sp@-		/* d0 */
moveml	%d1-%d5/%a0-%a2,%sp@-
bsr vectest_cpp
