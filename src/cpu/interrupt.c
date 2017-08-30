#include "interrupt.h"
#include <stdint.h>
#include <kprintf.h>
#include <cpu/idt.h>
#include <cpu/gdt.h>
#include <stdint.h>
#include <libc.h>

static interrupt_handler_t interrupt_handlers[INTERRUPTS_MAX] = {NULL};

uint8_t interrupt_set(uint8_t irq, interrupt_handler_t handler)
{

}


void interrupt_install_jumper_gates()
{
	/* Setup IDT */
	uint8_t ist = 0x00;
	uint8_t type_attr = IDTE_PRESENT | IDTE_DPL_RING0 | IDTE_TYPE_INTERRUPT_32;

	/* Install the gate */
	/* TODO: Is there a smarter way? */
	idt_install_gate(0  , (uint64_t)interrupt_handler_0, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(1  , (uint64_t)interrupt_handler_1, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(2  , (uint64_t)interrupt_handler_2, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(3  , (uint64_t)interrupt_handler_3, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(4  , (uint64_t)interrupt_handler_4, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(5  , (uint64_t)interrupt_handler_5, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(6  , (uint64_t)interrupt_handler_6, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(7  , (uint64_t)interrupt_handler_7, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(8  , (uint64_t)interrupt_handler_ec_8, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(9  , (uint64_t)interrupt_handler_9, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(10 , (uint64_t)interrupt_handler_ec_10, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(11 , (uint64_t)interrupt_handler_ec_11, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(12 , (uint64_t)interrupt_handler_ec_12, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(13 , (uint64_t)interrupt_handler_ec_13, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(14 , (uint64_t)interrupt_handler_ec_14, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(15 , (uint64_t)interrupt_handler_15, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(16 , (uint64_t)interrupt_handler_16, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(17 , (uint64_t)interrupt_handler_ec_17, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(18 , (uint64_t)interrupt_handler_18, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(19 , (uint64_t)interrupt_handler_19, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(20 , (uint64_t)interrupt_handler_20, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(21 , (uint64_t)interrupt_handler_21, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(22 , (uint64_t)interrupt_handler_22, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(23 , (uint64_t)interrupt_handler_23, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(24 , (uint64_t)interrupt_handler_24, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(25 , (uint64_t)interrupt_handler_25, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(26 , (uint64_t)interrupt_handler_26, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(27 , (uint64_t)interrupt_handler_27, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(28 , (uint64_t)interrupt_handler_28, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(29 , (uint64_t)interrupt_handler_29, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(30 , (uint64_t)interrupt_handler_30, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(31 , (uint64_t)interrupt_handler_31, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(32 , (uint64_t)interrupt_handler_32, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(33 , (uint64_t)interrupt_handler_33, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(34 , (uint64_t)interrupt_handler_34, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(35 , (uint64_t)interrupt_handler_35, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(36 , (uint64_t)interrupt_handler_36, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(37 , (uint64_t)interrupt_handler_37, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(38 , (uint64_t)interrupt_handler_38, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(39 , (uint64_t)interrupt_handler_39, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(40 , (uint64_t)interrupt_handler_40, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(41 , (uint64_t)interrupt_handler_41, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(42 , (uint64_t)interrupt_handler_42, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(43 , (uint64_t)interrupt_handler_43, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(44 , (uint64_t)interrupt_handler_44, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(45 , (uint64_t)interrupt_handler_45, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(46 , (uint64_t)interrupt_handler_46, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(47 , (uint64_t)interrupt_handler_47, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(48 , (uint64_t)interrupt_handler_48, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(49 , (uint64_t)interrupt_handler_49, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(50 , (uint64_t)interrupt_handler_50, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(51 , (uint64_t)interrupt_handler_51, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(52 , (uint64_t)interrupt_handler_52, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(53 , (uint64_t)interrupt_handler_53, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(54 , (uint64_t)interrupt_handler_54, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(55 , (uint64_t)interrupt_handler_55, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(56 , (uint64_t)interrupt_handler_56, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(57 , (uint64_t)interrupt_handler_57, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(58 , (uint64_t)interrupt_handler_58, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(59 , (uint64_t)interrupt_handler_59, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(60 , (uint64_t)interrupt_handler_60, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(61 , (uint64_t)interrupt_handler_61, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(62 , (uint64_t)interrupt_handler_62, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(63 , (uint64_t)interrupt_handler_63, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(64 , (uint64_t)interrupt_handler_64, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(65 , (uint64_t)interrupt_handler_65, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(66 , (uint64_t)interrupt_handler_66, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(67 , (uint64_t)interrupt_handler_67, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(68 , (uint64_t)interrupt_handler_68, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(69 , (uint64_t)interrupt_handler_69, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(70 , (uint64_t)interrupt_handler_70, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(71 , (uint64_t)interrupt_handler_71, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(72 , (uint64_t)interrupt_handler_72, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(73 , (uint64_t)interrupt_handler_73, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(74 , (uint64_t)interrupt_handler_74, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(75 , (uint64_t)interrupt_handler_75, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(76 , (uint64_t)interrupt_handler_76, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(77 , (uint64_t)interrupt_handler_77, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(78 , (uint64_t)interrupt_handler_78, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(79 , (uint64_t)interrupt_handler_79, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(80 , (uint64_t)interrupt_handler_80, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(81 , (uint64_t)interrupt_handler_81, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(82 , (uint64_t)interrupt_handler_82, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(83 , (uint64_t)interrupt_handler_83, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(84 , (uint64_t)interrupt_handler_84, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(85 , (uint64_t)interrupt_handler_85, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(86 , (uint64_t)interrupt_handler_86, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(87 , (uint64_t)interrupt_handler_87, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(88 , (uint64_t)interrupt_handler_88, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(89 , (uint64_t)interrupt_handler_89, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(90 , (uint64_t)interrupt_handler_90, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(91 , (uint64_t)interrupt_handler_91, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(92 , (uint64_t)interrupt_handler_92, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(93 , (uint64_t)interrupt_handler_93, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(94 , (uint64_t)interrupt_handler_94, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(95 , (uint64_t)interrupt_handler_95, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(96 , (uint64_t)interrupt_handler_96, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(97 , (uint64_t)interrupt_handler_97, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(98 , (uint64_t)interrupt_handler_98, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(99 , (uint64_t)interrupt_handler_99, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(100, (uint64_t)interrupt_handler_100, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(101, (uint64_t)interrupt_handler_101, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(102, (uint64_t)interrupt_handler_102, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(103, (uint64_t)interrupt_handler_103, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(104, (uint64_t)interrupt_handler_104, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(105, (uint64_t)interrupt_handler_105, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(106, (uint64_t)interrupt_handler_106, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(107, (uint64_t)interrupt_handler_107, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(108, (uint64_t)interrupt_handler_108, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(109, (uint64_t)interrupt_handler_109, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(110, (uint64_t)interrupt_handler_110, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(111, (uint64_t)interrupt_handler_111, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(112, (uint64_t)interrupt_handler_112, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(113, (uint64_t)interrupt_handler_113, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(114, (uint64_t)interrupt_handler_114, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(115, (uint64_t)interrupt_handler_115, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(116, (uint64_t)interrupt_handler_116, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(117, (uint64_t)interrupt_handler_117, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(118, (uint64_t)interrupt_handler_118, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(119, (uint64_t)interrupt_handler_119, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(120, (uint64_t)interrupt_handler_120, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(121, (uint64_t)interrupt_handler_121, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(122, (uint64_t)interrupt_handler_122, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(123, (uint64_t)interrupt_handler_123, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(124, (uint64_t)interrupt_handler_124, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(125, (uint64_t)interrupt_handler_125, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(126, (uint64_t)interrupt_handler_126, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(127, (uint64_t)interrupt_handler_127, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(128, (uint64_t)interrupt_handler_128, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(129, (uint64_t)interrupt_handler_129, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(130, (uint64_t)interrupt_handler_130, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(131, (uint64_t)interrupt_handler_131, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(132, (uint64_t)interrupt_handler_132, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(133, (uint64_t)interrupt_handler_133, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(134, (uint64_t)interrupt_handler_134, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(135, (uint64_t)interrupt_handler_135, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(136, (uint64_t)interrupt_handler_136, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(137, (uint64_t)interrupt_handler_137, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(138, (uint64_t)interrupt_handler_138, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(139, (uint64_t)interrupt_handler_139, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(140, (uint64_t)interrupt_handler_140, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(141, (uint64_t)interrupt_handler_141, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(142, (uint64_t)interrupt_handler_142, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(143, (uint64_t)interrupt_handler_143, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(144, (uint64_t)interrupt_handler_144, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(145, (uint64_t)interrupt_handler_145, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(146, (uint64_t)interrupt_handler_146, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(147, (uint64_t)interrupt_handler_147, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(148, (uint64_t)interrupt_handler_148, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(149, (uint64_t)interrupt_handler_149, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(150, (uint64_t)interrupt_handler_150, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(151, (uint64_t)interrupt_handler_151, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(152, (uint64_t)interrupt_handler_152, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(153, (uint64_t)interrupt_handler_153, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(154, (uint64_t)interrupt_handler_154, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(155, (uint64_t)interrupt_handler_155, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(156, (uint64_t)interrupt_handler_156, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(157, (uint64_t)interrupt_handler_157, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(158, (uint64_t)interrupt_handler_158, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(159, (uint64_t)interrupt_handler_159, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(160, (uint64_t)interrupt_handler_160, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(161, (uint64_t)interrupt_handler_161, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(162, (uint64_t)interrupt_handler_162, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(163, (uint64_t)interrupt_handler_163, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(164, (uint64_t)interrupt_handler_164, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(165, (uint64_t)interrupt_handler_165, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(166, (uint64_t)interrupt_handler_166, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(167, (uint64_t)interrupt_handler_167, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(168, (uint64_t)interrupt_handler_168, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(169, (uint64_t)interrupt_handler_169, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(170, (uint64_t)interrupt_handler_170, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(171, (uint64_t)interrupt_handler_171, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(172, (uint64_t)interrupt_handler_172, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(173, (uint64_t)interrupt_handler_173, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(174, (uint64_t)interrupt_handler_174, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(175, (uint64_t)interrupt_handler_175, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(176, (uint64_t)interrupt_handler_176, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(177, (uint64_t)interrupt_handler_177, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(178, (uint64_t)interrupt_handler_178, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(179, (uint64_t)interrupt_handler_179, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(180, (uint64_t)interrupt_handler_180, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(181, (uint64_t)interrupt_handler_181, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(182, (uint64_t)interrupt_handler_182, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(183, (uint64_t)interrupt_handler_183, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(184, (uint64_t)interrupt_handler_184, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(185, (uint64_t)interrupt_handler_185, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(186, (uint64_t)interrupt_handler_186, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(187, (uint64_t)interrupt_handler_187, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(188, (uint64_t)interrupt_handler_188, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(189, (uint64_t)interrupt_handler_189, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(190, (uint64_t)interrupt_handler_190, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(191, (uint64_t)interrupt_handler_191, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(192, (uint64_t)interrupt_handler_192, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(193, (uint64_t)interrupt_handler_193, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(194, (uint64_t)interrupt_handler_194, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(195, (uint64_t)interrupt_handler_195, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(196, (uint64_t)interrupt_handler_196, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(197, (uint64_t)interrupt_handler_197, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(198, (uint64_t)interrupt_handler_198, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(199, (uint64_t)interrupt_handler_199, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(200, (uint64_t)interrupt_handler_200, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(201, (uint64_t)interrupt_handler_201, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(202, (uint64_t)interrupt_handler_202, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(203, (uint64_t)interrupt_handler_203, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(204, (uint64_t)interrupt_handler_204, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(205, (uint64_t)interrupt_handler_205, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(206, (uint64_t)interrupt_handler_206, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(207, (uint64_t)interrupt_handler_207, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(208, (uint64_t)interrupt_handler_208, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(209, (uint64_t)interrupt_handler_209, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(210, (uint64_t)interrupt_handler_210, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(211, (uint64_t)interrupt_handler_211, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(212, (uint64_t)interrupt_handler_212, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(213, (uint64_t)interrupt_handler_213, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(214, (uint64_t)interrupt_handler_214, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(215, (uint64_t)interrupt_handler_215, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(216, (uint64_t)interrupt_handler_216, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(217, (uint64_t)interrupt_handler_217, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(218, (uint64_t)interrupt_handler_218, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(219, (uint64_t)interrupt_handler_219, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(220, (uint64_t)interrupt_handler_220, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(221, (uint64_t)interrupt_handler_221, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(222, (uint64_t)interrupt_handler_222, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(223, (uint64_t)interrupt_handler_223, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(224, (uint64_t)interrupt_handler_224, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(225, (uint64_t)interrupt_handler_225, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(226, (uint64_t)interrupt_handler_226, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(227, (uint64_t)interrupt_handler_227, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(228, (uint64_t)interrupt_handler_228, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(229, (uint64_t)interrupt_handler_229, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(230, (uint64_t)interrupt_handler_230, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(231, (uint64_t)interrupt_handler_231, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(232, (uint64_t)interrupt_handler_232, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(233, (uint64_t)interrupt_handler_233, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(234, (uint64_t)interrupt_handler_234, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(235, (uint64_t)interrupt_handler_235, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(236, (uint64_t)interrupt_handler_236, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(237, (uint64_t)interrupt_handler_237, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(238, (uint64_t)interrupt_handler_238, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(239, (uint64_t)interrupt_handler_239, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(240, (uint64_t)interrupt_handler_240, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(241, (uint64_t)interrupt_handler_241, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(242, (uint64_t)interrupt_handler_242, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(243, (uint64_t)interrupt_handler_243, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(244, (uint64_t)interrupt_handler_244, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(245, (uint64_t)interrupt_handler_245, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(246, (uint64_t)interrupt_handler_246, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(247, (uint64_t)interrupt_handler_247, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(248, (uint64_t)interrupt_handler_248, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(249, (uint64_t)interrupt_handler_249, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(250, (uint64_t)interrupt_handler_250, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(251, (uint64_t)interrupt_handler_251, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(252, (uint64_t)interrupt_handler_252, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(253, (uint64_t)interrupt_handler_253, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(254, (uint64_t)interrupt_handler_254, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
	idt_install_gate(255, (uint64_t)interrupt_handler_255, GDT_KERNEL_CODE_OFFSET, ist, type_attr);
}

void interrupt_init()
{
	/* Initialize IDT */
	idt_init();

	/* Setup the jumper gates */
	interrupt_install_jumper_gates();

	/* TODO:
	 *	apic
	 *	syscall
	 *	exceptions */

	/* Enable interrupts */
	//STI;
}



void interrupt_handler(pt_regs_t *regs)
{
	kprintf("Exception: 0x%x caught\n", regs->interrupt_num);
}

uint8_t interrupt_install(uint8_t irq, interrupt_handler_t handler)
{
	/* SANITY CHECK */
	if(irq >= INTERRUPTS_MAX) {
		ERROR("Could not install interrupt. IRQ: 0x%x >= 0x%0x", irq,
		      INTERRUPTS_MAX);
		return EPERM;
	}

	interrupt_handlers[irq] = handler;

	return EOK;
}

void interrupt_uninstall(uint8_t irq)
{
	if(irq >= INTERRUPTS_MAX) {
		ERROR("Could not uninstall interrupt. IRQ number too high: 0x%x
		      >= 0x%x", irq, INTERRUPTS_MAX);
		return EPERM;
	}

	interrupt_handlers[irq] = NULL;

	return EOK;
}
