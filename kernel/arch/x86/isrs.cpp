#include <vix/arch/gdt.h>
#include <vix/arch/idt.h>
#include <vix/arch/isrs.h>

extern "C" void except_entry0();
extern "C" void except_entry1();
extern "C" void except_entry2();
extern "C" void except_entry3();
extern "C" void except_entry4();
extern "C" void except_entry5();
extern "C" void except_entry6();
extern "C" void except_entry7();
extern "C" void except_entry8();
extern "C" void except_entry9();
extern "C" void except_entry10();
extern "C" void except_entry11();
extern "C" void except_entry12();
extern "C" void except_entry13();
extern "C" void except_entry14();
extern "C" void except_entry15();
extern "C" void except_entry16();
extern "C" void except_entry17();
extern "C" void except_entry18();
extern "C" void except_entry19();
extern "C" void except_entry20();
extern "C" void except_entry21();
extern "C" void except_entry22();
extern "C" void except_entry23();
extern "C" void except_entry24();
extern "C" void except_entry25();
extern "C" void except_entry26();
extern "C" void except_entry27();
extern "C" void except_entry28();
extern "C" void except_entry29();
extern "C" void except_entry30();
extern "C" void except_entry31();
extern "C" void i686_ISR32();
extern "C" void i686_ISR33();
extern "C" void i686_ISR34();
extern "C" void i686_ISR35();
extern "C" void i686_ISR36();
extern "C" void i686_ISR37();
extern "C" void i686_ISR38();
extern "C" void i686_ISR39();
extern "C" void i686_ISR40();
extern "C" void i686_ISR41();
extern "C" void i686_ISR42();
extern "C" void i686_ISR43();
extern "C" void i686_ISR44();
extern "C" void i686_ISR45();
extern "C" void i686_ISR46();
extern "C" void i686_ISR47();
extern "C" void i686_ISR48();
extern "C" void i686_ISR49();
extern "C" void i686_ISR50();
extern "C" void i686_ISR51();
extern "C" void i686_ISR52();
extern "C" void i686_ISR53();
extern "C" void i686_ISR54();
extern "C" void i686_ISR55();
extern "C" void i686_ISR56();
extern "C" void i686_ISR57();
extern "C" void i686_ISR58();
extern "C" void i686_ISR59();
extern "C" void i686_ISR60();
extern "C" void i686_ISR61();
extern "C" void i686_ISR62();
extern "C" void i686_ISR63();
extern "C" void i686_ISR64();
extern "C" void i686_ISR65();
extern "C" void i686_ISR66();
extern "C" void i686_ISR67();
extern "C" void i686_ISR68();
extern "C" void i686_ISR69();
extern "C" void i686_ISR70();
extern "C" void i686_ISR71();
extern "C" void i686_ISR72();
extern "C" void i686_ISR73();
extern "C" void i686_ISR74();
extern "C" void i686_ISR75();
extern "C" void i686_ISR76();
extern "C" void i686_ISR77();
extern "C" void i686_ISR78();
extern "C" void i686_ISR79();
extern "C" void i686_ISR80();
extern "C" void i686_ISR81();
extern "C" void i686_ISR82();
extern "C" void i686_ISR83();
extern "C" void i686_ISR84();
extern "C" void i686_ISR85();
extern "C" void i686_ISR86();
extern "C" void i686_ISR87();
extern "C" void i686_ISR88();
extern "C" void i686_ISR89();
extern "C" void i686_ISR90();
extern "C" void i686_ISR91();
extern "C" void i686_ISR92();
extern "C" void i686_ISR93();
extern "C" void i686_ISR94();
extern "C" void i686_ISR95();
extern "C" void i686_ISR96();
extern "C" void i686_ISR97();
extern "C" void i686_ISR98();
extern "C" void i686_ISR99();
extern "C" void i686_ISR100();
extern "C" void i686_ISR101();
extern "C" void i686_ISR102();
extern "C" void i686_ISR103();
extern "C" void i686_ISR104();
extern "C" void i686_ISR105();
extern "C" void i686_ISR106();
extern "C" void i686_ISR107();
extern "C" void i686_ISR108();
extern "C" void i686_ISR109();
extern "C" void i686_ISR110();
extern "C" void i686_ISR111();
extern "C" void i686_ISR112();
extern "C" void i686_ISR113();
extern "C" void i686_ISR114();
extern "C" void i686_ISR115();
extern "C" void i686_ISR116();
extern "C" void i686_ISR117();
extern "C" void i686_ISR118();
extern "C" void i686_ISR119();
extern "C" void i686_ISR120();
extern "C" void i686_ISR121();
extern "C" void i686_ISR122();
extern "C" void i686_ISR123();
extern "C" void i686_ISR124();
extern "C" void i686_ISR125();
extern "C" void i686_ISR126();
extern "C" void i686_ISR127();
extern "C" void syscall_entry();
extern "C" void i686_ISR129();
extern "C" void i686_ISR130();
extern "C" void i686_ISR131();
extern "C" void i686_ISR132();
extern "C" void i686_ISR133();
extern "C" void i686_ISR134();
extern "C" void i686_ISR135();
extern "C" void i686_ISR136();
extern "C" void i686_ISR137();
extern "C" void i686_ISR138();
extern "C" void i686_ISR139();
extern "C" void i686_ISR140();
extern "C" void i686_ISR141();
extern "C" void i686_ISR142();
extern "C" void i686_ISR143();
extern "C" void i686_ISR144();
extern "C" void i686_ISR145();
extern "C" void i686_ISR146();
extern "C" void i686_ISR147();
extern "C" void i686_ISR148();
extern "C" void i686_ISR149();
extern "C" void i686_ISR150();
extern "C" void i686_ISR151();
extern "C" void i686_ISR152();
extern "C" void i686_ISR153();
extern "C" void i686_ISR154();
extern "C" void i686_ISR155();
extern "C" void i686_ISR156();
extern "C" void i686_ISR157();
extern "C" void i686_ISR158();
extern "C" void i686_ISR159();
extern "C" void i686_ISR160();
extern "C" void i686_ISR161();
extern "C" void i686_ISR162();
extern "C" void i686_ISR163();
extern "C" void i686_ISR164();
extern "C" void i686_ISR165();
extern "C" void i686_ISR166();
extern "C" void i686_ISR167();
extern "C" void i686_ISR168();
extern "C" void i686_ISR169();
extern "C" void i686_ISR170();
extern "C" void i686_ISR171();
extern "C" void i686_ISR172();
extern "C" void i686_ISR173();
extern "C" void i686_ISR174();
extern "C" void i686_ISR175();
extern "C" void i686_ISR176();
extern "C" void i686_ISR177();
extern "C" void i686_ISR178();
extern "C" void i686_ISR179();
extern "C" void i686_ISR180();
extern "C" void i686_ISR181();
extern "C" void i686_ISR182();
extern "C" void i686_ISR183();
extern "C" void i686_ISR184();
extern "C" void i686_ISR185();
extern "C" void i686_ISR186();
extern "C" void i686_ISR187();
extern "C" void i686_ISR188();
extern "C" void i686_ISR189();
extern "C" void i686_ISR190();
extern "C" void i686_ISR191();
extern "C" void i686_ISR192();
extern "C" void i686_ISR193();
extern "C" void i686_ISR194();
extern "C" void i686_ISR195();
extern "C" void i686_ISR196();
extern "C" void i686_ISR197();
extern "C" void i686_ISR198();
extern "C" void i686_ISR199();
extern "C" void i686_ISR200();
extern "C" void i686_ISR201();
extern "C" void i686_ISR202();
extern "C" void i686_ISR203();
extern "C" void i686_ISR204();
extern "C" void i686_ISR205();
extern "C" void i686_ISR206();
extern "C" void i686_ISR207();
extern "C" void i686_ISR208();
extern "C" void i686_ISR209();
extern "C" void i686_ISR210();
extern "C" void i686_ISR211();
extern "C" void i686_ISR212();
extern "C" void i686_ISR213();
extern "C" void i686_ISR214();
extern "C" void i686_ISR215();
extern "C" void i686_ISR216();
extern "C" void i686_ISR217();
extern "C" void i686_ISR218();
extern "C" void i686_ISR219();
extern "C" void i686_ISR220();
extern "C" void i686_ISR221();
extern "C" void i686_ISR222();
extern "C" void i686_ISR223();
extern "C" void i686_ISR224();
extern "C" void i686_ISR225();
extern "C" void i686_ISR226();
extern "C" void i686_ISR227();
extern "C" void i686_ISR228();
extern "C" void i686_ISR229();
extern "C" void i686_ISR230();
extern "C" void i686_ISR231();
extern "C" void i686_ISR232();
extern "C" void i686_ISR233();
extern "C" void i686_ISR234();
extern "C" void i686_ISR235();
extern "C" void i686_ISR236();
extern "C" void i686_ISR237();
extern "C" void i686_ISR238();
extern "C" void i686_ISR239();
extern "C" void i686_ISR240();
extern "C" void i686_ISR241();
extern "C" void i686_ISR242();
extern "C" void i686_ISR243();
extern "C" void i686_ISR244();
extern "C" void i686_ISR245();
extern "C" void i686_ISR246();
extern "C" void i686_ISR247();
extern "C" void i686_ISR248();
extern "C" void i686_ISR249();
extern "C" void i686_ISR250();
extern "C" void i686_ISR251();
extern "C" void i686_ISR252();
extern "C" void i686_ISR253();
extern "C" void i686_ISR254();
extern "C" void i686_ISR255();

void isrs::i686_ISR_InitializeGates() {
    idt::i686_IDT_SetGate(0, (void *)&except_entry0, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(1, (void *)&except_entry1, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(2, (void *)&except_entry2, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(3, (void *)&except_entry3, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(4, (void *)&except_entry4, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(5, (void *)&except_entry5, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(6, (void *)&except_entry6, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(7, (void *)&except_entry7, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(8, (void *)&except_entry8, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(9, (void *)&except_entry9, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(10, (void *)&except_entry10, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(11, (void *)&except_entry11, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(12, (void *)&except_entry12, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(13, (void *)&except_entry13, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(14, (void *)&except_entry14, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(15, (void *)&except_entry15, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(16, (void *)&except_entry16, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(17, (void *)&except_entry17, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(18, (void *)&except_entry18, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(19, (void *)&except_entry19, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(20, (void *)&except_entry20, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(21, (void *)&except_entry21, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(22, (void *)&except_entry22, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(23, (void *)&except_entry23, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(24, (void *)&except_entry24, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(25, (void *)&except_entry25, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(26, (void *)&except_entry26, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(27, (void *)&except_entry27, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(28, (void *)&except_entry28, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(29, (void *)&except_entry29, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(30, (void *)&except_entry30, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(31, (void *)&except_entry31, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(32, (void *)&i686_ISR32, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(33, (void *)&i686_ISR33, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(34, (void *)&i686_ISR34, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(35, (void *)&i686_ISR35, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(36, (void *)&i686_ISR36, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(37, (void *)&i686_ISR37, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(38, (void *)&i686_ISR38, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(39, (void *)&i686_ISR39, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(40, (void *)&i686_ISR40, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(41, (void *)&i686_ISR41, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(42, (void *)&i686_ISR42, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(43, (void *)&i686_ISR43, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(44, (void *)&i686_ISR44, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(45, (void *)&i686_ISR45, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(46, (void *)&i686_ISR46, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(47, (void *)&i686_ISR47, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(48, (void *)&i686_ISR48, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(49, (void *)&i686_ISR49, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(50, (void *)&i686_ISR50, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(51, (void *)&i686_ISR51, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(52, (void *)&i686_ISR52, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(53, (void *)&i686_ISR53, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(54, (void *)&i686_ISR54, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(55, (void *)&i686_ISR55, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(56, (void *)&i686_ISR56, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(57, (void *)&i686_ISR57, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(58, (void *)&i686_ISR58, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(59, (void *)&i686_ISR59, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(60, (void *)&i686_ISR60, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(61, (void *)&i686_ISR61, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(62, (void *)&i686_ISR62, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(63, (void *)&i686_ISR63, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(64, (void *)&i686_ISR64, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(65, (void *)&i686_ISR65, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(66, (void *)&i686_ISR66, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(67, (void *)&i686_ISR67, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(68, (void *)&i686_ISR68, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(69, (void *)&i686_ISR69, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(70, (void *)&i686_ISR70, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(71, (void *)&i686_ISR71, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(72, (void *)&i686_ISR72, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(73, (void *)&i686_ISR73, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(74, (void *)&i686_ISR74, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(75, (void *)&i686_ISR75, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(76, (void *)&i686_ISR76, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(77, (void *)&i686_ISR77, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(78, (void *)&i686_ISR78, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(79, (void *)&i686_ISR79, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(80, (void *)&i686_ISR80, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(81, (void *)&i686_ISR81, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(82, (void *)&i686_ISR82, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(83, (void *)&i686_ISR83, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(84, (void *)&i686_ISR84, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(85, (void *)&i686_ISR85, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(86, (void *)&i686_ISR86, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(87, (void *)&i686_ISR87, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(88, (void *)&i686_ISR88, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(89, (void *)&i686_ISR89, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(90, (void *)&i686_ISR90, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(91, (void *)&i686_ISR91, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(92, (void *)&i686_ISR92, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(93, (void *)&i686_ISR93, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(94, (void *)&i686_ISR94, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(95, (void *)&i686_ISR95, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(96, (void *)&i686_ISR96, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(97, (void *)&i686_ISR97, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(98, (void *)&i686_ISR98, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(99, (void *)&i686_ISR99, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(100, (void *)&i686_ISR100, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(101, (void *)&i686_ISR101, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(102, (void *)&i686_ISR102, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(103, (void *)&i686_ISR103, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(104, (void *)&i686_ISR104, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(105, (void *)&i686_ISR105, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(106, (void *)&i686_ISR106, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(107, (void *)&i686_ISR107, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(108, (void *)&i686_ISR108, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(109, (void *)&i686_ISR109, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(110, (void *)&i686_ISR110, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(111, (void *)&i686_ISR111, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(112, (void *)&i686_ISR112, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(113, (void *)&i686_ISR113, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(114, (void *)&i686_ISR114, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(115, (void *)&i686_ISR115, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(116, (void *)&i686_ISR116, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(117, (void *)&i686_ISR117, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(118, (void *)&i686_ISR118, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(119, (void *)&i686_ISR119, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(120, (void *)&i686_ISR120, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(121, (void *)&i686_ISR121, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(122, (void *)&i686_ISR122, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(123, (void *)&i686_ISR123, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(124, (void *)&i686_ISR124, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(125, (void *)&i686_ISR125, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(126, (void *)&i686_ISR126, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(127, (void *)&i686_ISR127, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(128, (void *)&syscall_entry, GDT_KERNEL_CODE, idt::IDT_FLAG_RING3 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(129, (void *)&i686_ISR129, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(130, (void *)&i686_ISR130, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(131, (void *)&i686_ISR131, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(132, (void *)&i686_ISR132, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(133, (void *)&i686_ISR133, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(134, (void *)&i686_ISR134, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(135, (void *)&i686_ISR135, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(136, (void *)&i686_ISR136, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(137, (void *)&i686_ISR137, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(138, (void *)&i686_ISR138, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(139, (void *)&i686_ISR139, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(140, (void *)&i686_ISR140, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(141, (void *)&i686_ISR141, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(142, (void *)&i686_ISR142, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(143, (void *)&i686_ISR143, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(144, (void *)&i686_ISR144, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(145, (void *)&i686_ISR145, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(146, (void *)&i686_ISR146, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(147, (void *)&i686_ISR147, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(148, (void *)&i686_ISR148, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(149, (void *)&i686_ISR149, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(150, (void *)&i686_ISR150, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(151, (void *)&i686_ISR151, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(152, (void *)&i686_ISR152, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(153, (void *)&i686_ISR153, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(154, (void *)&i686_ISR154, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(155, (void *)&i686_ISR155, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(156, (void *)&i686_ISR156, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(157, (void *)&i686_ISR157, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(158, (void *)&i686_ISR158, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(159, (void *)&i686_ISR159, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(160, (void *)&i686_ISR160, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(161, (void *)&i686_ISR161, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(162, (void *)&i686_ISR162, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(163, (void *)&i686_ISR163, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(164, (void *)&i686_ISR164, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(165, (void *)&i686_ISR165, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(166, (void *)&i686_ISR166, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(167, (void *)&i686_ISR167, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(168, (void *)&i686_ISR168, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(169, (void *)&i686_ISR169, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(170, (void *)&i686_ISR170, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(171, (void *)&i686_ISR171, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(172, (void *)&i686_ISR172, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(173, (void *)&i686_ISR173, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(174, (void *)&i686_ISR174, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(175, (void *)&i686_ISR175, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(176, (void *)&i686_ISR176, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(177, (void *)&i686_ISR177, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(178, (void *)&i686_ISR178, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(179, (void *)&i686_ISR179, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(180, (void *)&i686_ISR180, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(181, (void *)&i686_ISR181, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(182, (void *)&i686_ISR182, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(183, (void *)&i686_ISR183, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(184, (void *)&i686_ISR184, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(185, (void *)&i686_ISR185, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(186, (void *)&i686_ISR186, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(187, (void *)&i686_ISR187, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(188, (void *)&i686_ISR188, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(189, (void *)&i686_ISR189, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(190, (void *)&i686_ISR190, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(191, (void *)&i686_ISR191, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(192, (void *)&i686_ISR192, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(193, (void *)&i686_ISR193, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(194, (void *)&i686_ISR194, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(195, (void *)&i686_ISR195, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(196, (void *)&i686_ISR196, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(197, (void *)&i686_ISR197, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(198, (void *)&i686_ISR198, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(199, (void *)&i686_ISR199, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(200, (void *)&i686_ISR200, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(201, (void *)&i686_ISR201, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(202, (void *)&i686_ISR202, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(203, (void *)&i686_ISR203, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(204, (void *)&i686_ISR204, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(205, (void *)&i686_ISR205, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(206, (void *)&i686_ISR206, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(207, (void *)&i686_ISR207, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(208, (void *)&i686_ISR208, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(209, (void *)&i686_ISR209, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(210, (void *)&i686_ISR210, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(211, (void *)&i686_ISR211, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(212, (void *)&i686_ISR212, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(213, (void *)&i686_ISR213, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(214, (void *)&i686_ISR214, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(215, (void *)&i686_ISR215, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(216, (void *)&i686_ISR216, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(217, (void *)&i686_ISR217, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(218, (void *)&i686_ISR218, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(219, (void *)&i686_ISR219, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(220, (void *)&i686_ISR220, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(221, (void *)&i686_ISR221, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(222, (void *)&i686_ISR222, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(223, (void *)&i686_ISR223, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(224, (void *)&i686_ISR224, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(225, (void *)&i686_ISR225, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(226, (void *)&i686_ISR226, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(227, (void *)&i686_ISR227, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(228, (void *)&i686_ISR228, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(229, (void *)&i686_ISR229, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(230, (void *)&i686_ISR230, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(231, (void *)&i686_ISR231, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(232, (void *)&i686_ISR232, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(233, (void *)&i686_ISR233, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(234, (void *)&i686_ISR234, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(235, (void *)&i686_ISR235, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(236, (void *)&i686_ISR236, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(237, (void *)&i686_ISR237, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(238, (void *)&i686_ISR238, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(239, (void *)&i686_ISR239, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(240, (void *)&i686_ISR240, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(241, (void *)&i686_ISR241, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(242, (void *)&i686_ISR242, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(243, (void *)&i686_ISR243, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(244, (void *)&i686_ISR244, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(245, (void *)&i686_ISR245, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(246, (void *)&i686_ISR246, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(247, (void *)&i686_ISR247, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(248, (void *)&i686_ISR248, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(249, (void *)&i686_ISR249, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(250, (void *)&i686_ISR250, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(251, (void *)&i686_ISR251, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(252, (void *)&i686_ISR252, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(253, (void *)&i686_ISR253, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(254, (void *)&i686_ISR254, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
    idt::i686_IDT_SetGate(255, (void *)&i686_ISR255, GDT_KERNEL_CODE, idt::IDT_FLAG_RING0 | idt::IDT_FLAG_GATE_32BIT_INT);
}
