#include "lcd12864_charset.hpp"
#include "drawbuf.hpp"

namespace lcd {
	/* NUMBERS */
	const uint8_t CHAR_0_DATA[] = {
		0x70,
		0x88,
		0x88,
		0x98,
		0xa8,
		0xc8,
		0x88,
		0x88,
		0x70,
	};
	const Image CHAR_0(CHAR_0_DATA, 1, 5, 9);
	const uint8_t CHAR_1_DATA[] = {
        0x40,
        0xc0,
        0x40,
        0x40,
        0x40,
        0x40,
        0x40,
        0x40,
        0xe0,
	};
	const Image CHAR_1(CHAR_1_DATA, 1, 3, 9);
	const uint8_t CHAR_2_DATA[] = {
		0x70,
		0x88,
		0x88,
		0x08,
		0x10,
		0x20,
		0x40,
		0x80,
		0xf8,
	};
	const Image CHAR_2(CHAR_2_DATA, 1, 5, 9);
	const uint8_t CHAR_3_DATA[] = {
		0x70,
		0x88,
		0x08,
		0x08,
		0x70,
		0x08,
		0x08,
		0x88,
		0x70,
	};
	const Image CHAR_3(CHAR_3_DATA, 1, 5, 9);
	const uint8_t CHAR_4_DATA[] = {
		0x10,
		0x30,
		0x50,
		0x50,
		0x90,
		0xf8,
		0x10,
		0x10,
		0x10,
	};
	const Image CHAR_4(CHAR_4_DATA, 1, 5, 9);
	const uint8_t CHAR_5_DATA[] = {
		0xf8,
		0x80,
		0x80,
		0xf0,
		0x88,
		0x08,
		0x08,
		0x88,
		0x70,
	};
	const Image CHAR_5(CHAR_5_DATA, 1, 5, 9);
	const uint8_t CHAR_6_DATA[] = {
		0x70,
		0x88,
		0x80,
		0x80,
		0xf0,
		0x88,
		0x88,
		0x88,
		0x70,
	};
	const Image CHAR_6(CHAR_6_DATA, 1, 5, 9);
	const uint8_t CHAR_7_DATA[] = {
		0xf8,
		0x08,
		0x08,
		0x10,
		0x10,
		0x20,
		0x20,
		0x20,
		0x20,
	};
	const Image CHAR_7(CHAR_7_DATA, 1, 5, 9);
	const uint8_t CHAR_8_DATA[] = {
		0x70,
		0x88,
		0x88,
		0x88,
		0x70,
		0x88,
		0x88,
		0x88,
		0x70,
	};
	const Image CHAR_8(CHAR_8_DATA, 1, 5, 9);
	const uint8_t CHAR_9_DATA[] = {
		0x70,
		0x88,
		0x88,
		0x88,
		0x78,
		0x08,
		0x08,
		0x88,
		0x70,
	};
	const Image CHAR_9(CHAR_9_DATA, 1, 5, 9);

	/* ASCII 32-126 */
	const uint8_t CHAR_SPC_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	};
	const Image CHAR_SPC(CHAR_SPC_DATA, 1, 5, 9);
	const uint8_t CHAR_EXMK_DATA[] = {
		0x80,
		0x80,
		0x80,
		0x80,
		0x80,
		0x80,
		0x00,
		0x00,
		0x80,
	};
	const Image CHAR_EXMK(CHAR_EXMK_DATA, 1, 1, 9);
	// const uint8_t CHAR_DBLQ_DATA[] = {
    //     0xa0,
    //     0xa0,
    //     0xa0,
    //     0x00,
    //     0x00,
    //     0x00,
    //     0x00,
    //     0x00,
    //     0x00,
	// };
	// const Image CHAR_DBLQ(CHAR_DBLQ_DATA, 1, 3, 9);
	// const uint8_t CHAR_NSGN_DATA[] = {
	//     0x50,
	//     0x50,
	//     0xf8,
	//     0x50,
	//     0x50,
	//     0x50,
	//     0xf8,
	//     0x50,
	//     0x50,
	// };
	// const Image CHAR_NSGN(CHAR_NSGN_DATA, 1, 5, 9);
	// const uint8_t CHAR_DLR_DATA[] = {
	//     0x20,
	//     0x70,
	//     0xa8,
	//     0xa0,
	//     0x70,
	//     0x28,
	//     0xa8,
	//     0x70,
	//     0x20,
	// };
	// const Image CHAR_DLR(CHAR_DLR_DATA, 1, 5, 9);
	const uint8_t CHAR_PCT_DATA[] = {
		0x08,
		0xc8,
		0xd0,
		0x10,
		0x20,
		0x20,
		0x40,
		0x58,
		0x98,
	};
	const Image CHAR_PCT(CHAR_PCT_DATA, 1, 5, 9);
	const uint8_t CHAR_AND_DATA[] = {
	    0x60,
	    0x90,
	    0x90,
	    0x90,
	    0x60,
	    0xa8,
	    0x90,
	    0x90,
	    0x68,
	};
	const Image CHAR_AND(CHAR_AND_DATA, 1, 5, 9);
	// const uint8_t CHAR_SGLQ_DATA[] = {
	//     0x80,
	//     0x80,
	//     0x80,
	//     0x00,
	//     0x00,
	//     0x00,
	//     0x00,
	//     0x00,
	//     0x00,
	// };
	// const Image CHAR_SGLQ(CHAR_SGLQ_DATA, 1, 1, 9);
    const uint8_t CHAR_LBKT_DATA[] = {
        0x20,
        0x40,
        0x80,
        0x80,
        0x80,
        0x80,
        0x80,
        0x40,
        0x20,
    };
    const Image CHAR_LBKT(CHAR_LBKT_DATA, 1, 3, 9);
    const uint8_t CHAR_RBKT_DATA[] = {
        0x80,
        0x40,
        0x20,
        0x20,
        0x20,
        0x20,
        0x20,
        0x40,
        0x80,
    };
    const Image CHAR_RBKT(CHAR_RBKT_DATA, 1, 3, 9);
	const uint8_t CHAR_ASTK_DATA[] = {
	    0x00,
	    0x20,
	    0xa8,
	    0x70,
	    0xa8,
	    0x20,
	    0x00,
	    0x00,
	    0x00,
	};
	const Image CHAR_ASTK(CHAR_ASTK_DATA, 1, 5, 9);
	const uint8_t CHAR_PLUS_DATA[] = {
		0x00,
		0x00,
		0x20,
		0x20,
		0xf8,
		0x20,
		0x20,
		0x00,
		0x00,
	};
	const Image CHAR_PLUS(CHAR_PLUS_DATA, 1, 5, 9);
	const uint8_t CHAR_CMMA_DATA[] = {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0xc0,
        0xc0,
        0x40,
        0x80,
	};
	const Image CHAR_CMMA(CHAR_CMMA_DATA, 1, 2, 9);
	const uint8_t CHAR_MNUS_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0xf8,
		0x00,
		0x00,
		0x00,
		0x00,
	};
	const Image CHAR_MNUS(CHAR_MNUS_DATA, 1, 5, 9);
	const uint8_t CHAR_DOT_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0xc0,
		0xc0,
	};
	const Image CHAR_DOT(CHAR_DOT_DATA, 1, 2, 9);
	// const uint8_t CHAR_SLSH_DATA[] = {
	//     0x08,
	//     0x08,
	//     0x10,
	//     0x10,
	//     0x20,
	//     0x20,
	//     0x40,
	//     0x40,
	//     0x80,
	// };
	// const Image CHAR_SLSH(CHAR_SLSH_DATA, 1, 5, 9);
	const uint8_t CHAR_CLN_DATA[] = {
        0x00,
        0xc0,
        0xc0,
        0x00,
        0x00,
        0xc0,
        0xc0,
        0x00,
        0x00,
	};
	const Image CHAR_CLN(CHAR_CLN_DATA, 1, 2, 9);
	// const uint8_t CHAR_SMCN_DATA[] = {
    //     0x00,
    //     0xc0,
    //     0xc0,
    //     0x00,
    //     0x00,
    //     0xc0,
    //     0xc0,
    //     0x40,
    //     0x80,
	// };
	// const Image CHAR_SMCN(CHAR_SMCN_DATA, 1, 2, 9);
	const uint8_t CHAR_LSTN_DATA[] = {
		0x00,
        0x10,
        0x20,
        0x40,
        0x80,
        0x40,
        0x20,
        0x10,
        0x00,
	};
	const Image CHAR_LSTN(CHAR_LSTN_DATA, 1, 4, 9);
	const uint8_t CHAR_EQL_DATA[] = {
		0x00,
		0x00,
		0x00,
		0xf8,
		0x00,
		0xf8,
		0x00,
		0x00,
		0x00,
	};
	const Image CHAR_EQL(CHAR_EQL_DATA, 1, 5, 9);
	const uint8_t CHAR_GTTN_DATA[] = {
		0x00,
        0x80,
        0x40,
        0x20,
        0x10,
        0x20,
        0x40,
        0x80,
        0x00,
	};
	const Image CHAR_GTTN(CHAR_GTTN_DATA, 1, 4, 9);
	const uint8_t CHAR_QNMK_DATA[] = {
	    0x70,
	    0x88,
	    0x88,
	    0x08,
	    0x10,
	    0x20,
	    0x20,
	    0x00,
	    0x20,
	};
	const Image CHAR_QNMK(CHAR_QNMK_DATA, 1, 5, 9);
	// const uint8_t CHAR_ATSB_DATA[] = {
	//     0x70,
	//     0x88,
	//     0x88,
	//     0x08,
	//     0x68,
	//     0xa8,
	//     0xa8,
	//     0xa8,
	//     0x70,
	// };
	// const Image CHAR_ATSB(CHAR_ATSB_DATA, 1, 5, 9);
	const uint8_t CHAR_A_DATA[] = {
		0x20,
		0x50,
		0x50,
		0x88,
		0x88,
		0xf8,
		0x88,
		0x88,
		0x88,
	};
	const Image CHAR_A(CHAR_A_DATA, 1, 5, 9);
	const uint8_t CHAR_B_DATA[] = {
		0xf0,
		0x88,
		0x88,
		0x88,
		0xf0,
		0x88,
		0x88,
		0x88,
		0xf0,
	};
	const Image CHAR_B(CHAR_B_DATA, 1, 5, 9);
	const uint8_t CHAR_C_DATA[] = {
		0x70,
		0x88,
		0x88,
		0x80,
		0x80,
		0x80,
		0x88,
		0x88,
		0x70,
	};
	const Image CHAR_C(CHAR_C_DATA, 1, 5, 9);
	const uint8_t CHAR_D_DATA[] = {
		0xe0,
		0x90,
		0x88,
		0x88,
		0x88,
		0x88,
		0x88,
		0x90,
		0xe0,
	};
	const Image CHAR_D(CHAR_D_DATA, 1, 5, 9);
	const uint8_t CHAR_E_DATA[] = {
		0xf8,
		0x80,
		0x80,
		0x80,
		0xf0,
		0x80,
		0x80,
		0x80,
		0xf8,
	};
	const Image CHAR_E(CHAR_E_DATA, 1, 5, 9);
	const uint8_t CHAR_F_DATA[] = {
		0xf8,
		0x80,
		0x80,
		0x80,
		0xf0,
		0x80,
		0x80,
		0x80,
		0x80,
	};
	const Image CHAR_F(CHAR_F_DATA, 1, 5, 9);
	const uint8_t CHAR_G_DATA[] = {
		0x70,
		0x88,
		0x88,
		0x80,
		0xb0,
		0x88,
		0x88,
		0x88,
		0x70,
	};
	const Image CHAR_G(CHAR_G_DATA, 1, 5, 9);
	const uint8_t CHAR_H_DATA[] = {
		0x88,
		0x88,
		0x88,
		0x88,
		0xf8,
		0x88,
		0x88,
		0x88,
		0x88,
	};
	const Image CHAR_H(CHAR_H_DATA, 1, 5, 9);
	const uint8_t CHAR_I_DATA[] = {
		0x70,
		0x20,
		0x20,
		0x20,
		0x20,
		0x20,
		0x20,
		0x20,
		0x70,
	};
	const Image CHAR_I(CHAR_I_DATA, 1, 5, 9);
	const uint8_t CHAR_J_DATA[] = {
		0x78,
		0x10,
		0x10,
		0x10,
		0x10,
		0x10,
		0x90,
		0x90,
		0x60,
	};
	const Image CHAR_J(CHAR_J_DATA, 1, 5, 9);
	const uint8_t CHAR_K_DATA[] = {
		0x88,
		0x88,
		0x90,
		0x90,
		0xe0,
		0x90,
		0x90,
		0x88,
		0x88,
	};
	const Image CHAR_K(CHAR_K_DATA, 1, 5, 9);
	const uint8_t CHAR_L_DATA[] = {
		0x80,
		0x80,
		0x80,
		0x80,
		0x80,
		0x80,
		0x80,
		0x80,
		0xf8,
	};
	const Image CHAR_L(CHAR_L_DATA, 1, 5, 9);
	const uint8_t CHAR_M_DATA[] = {
		0x88,
		0xd8,
		0xa8,
		0xa8,
		0x88,
		0x88,
		0x88,
		0x88,
		0x88,
	};
	const Image CHAR_M(CHAR_M_DATA, 1, 5, 9);
	const uint8_t CHAR_N_DATA[] = {
		0x88,
		0xc8,
		0xc8,
		0xa8,
		0xa8,
		0x98,
		0x98,
		0x88,
		0x88,
	};
	const Image CHAR_N(CHAR_N_DATA, 1, 5, 9);
	const uint8_t CHAR_O_DATA[] = {
		0x70,
		0x88,
		0x88,
		0x88,
		0x88,
		0x88,
		0x88,
		0x88,
		0x70,
	};
	const Image CHAR_O(CHAR_O_DATA, 1, 5, 9);
	const uint8_t CHAR_P_DATA[] = {
		0xf0,
		0x88,
		0x88,
		0x88,
		0xf0,
		0x80,
		0x80,
		0x80,
		0x80,
	};
	const Image CHAR_P(CHAR_P_DATA, 1, 5, 9);
	const uint8_t CHAR_Q_DATA[] = {
		0x70,
		0x88,
		0x88,
		0x88,
		0x88,
		0x88,
		0xa8,
		0x90,
		0x68,
	};
	const Image CHAR_Q(CHAR_Q_DATA, 1, 5, 9);
	const uint8_t CHAR_R_DATA[] = {
		0xf0,
		0x88,
		0x88,
		0x88,
		0xf0,
		0xa0,
		0x90,
		0x88,
		0x88,
	};
	const Image CHAR_R(CHAR_R_DATA, 1, 5, 9);
	const uint8_t CHAR_S_DATA[] = {
		0x70,
		0x88,
		0x80,
		0x80,
		0x70,
		0x08,
		0x08,
		0x88,
		0x70,
	};
	const Image CHAR_S(CHAR_S_DATA, 1, 5, 9);
	const uint8_t CHAR_T_DATA[] = {
		0xf8,
		0x20,
		0x20,
		0x20,
		0x20,
		0x20,
		0x20,
		0x20,
		0x20,
	};
	const Image CHAR_T(CHAR_T_DATA, 1, 5, 9);
	const uint8_t CHAR_U_DATA[] = {
		0x88,
		0x88,
		0x88,
		0x88,
		0x88,
		0x88,
		0x88,
		0x88,
		0x70,
	};
	const Image CHAR_U(CHAR_U_DATA, 1, 5, 9);
	const uint8_t CHAR_V_DATA[] = {
		0x88,
		0x88,
		0x88,
		0x88,
		0x88,
		0x50,
		0x50,
		0x50,
		0x20,
	};
	const Image CHAR_V(CHAR_V_DATA, 1, 5, 9);
	const uint8_t CHAR_W_DATA[] = {
		0x88,
		0x88,
		0x88,
		0x88,
		0x88,
		0xa8,
		0xa8,
		0xa8,
		0x50,
	};
	const Image CHAR_W(CHAR_W_DATA, 1, 5, 9);
	const uint8_t CHAR_X_DATA[] = {
		0x88,
		0x88,
		0x88,
		0x50,
		0x20,
		0x50,
		0x88,
		0x88,
		0x88,
	};
	const Image CHAR_X(CHAR_X_DATA, 1, 5, 9);
	const uint8_t CHAR_Y_DATA[] = {
		0x88,
		0x88,
		0x88,
		0x50,
		0x50,
		0x20,
		0x20,
		0x20,
		0x20,
	};
	const Image CHAR_Y(CHAR_Y_DATA, 1, 5, 9);
	const uint8_t CHAR_Z_DATA[] = {
		0xf8,
		0x08,
		0x10,
		0x10,
		0x20,
		0x40,
		0x40,
		0x80,
		0xf8,
	};
	const Image CHAR_Z(CHAR_Z_DATA, 1, 5, 9);
	const uint8_t CHAR_LSQB_DATA[] = {
		0xe0,
		0x80,
		0x80,
		0x80,
		0x80,
		0x80,
		0x80,
		0x80,
		0xe0,
	};
	const Image CHAR_LSQB(CHAR_LSQB_DATA, 1, 3, 9);
	// const uint8_t CHAR_BKSL_DATA[] = {
	// 	0x80,
	// 	0x80,
	// 	0x40,
	// 	0x40,
	// 	0x20,
	// 	0x20,
	// 	0x10,
	// 	0x10,
	// 	0x08,
	// };
	// const Image CHAR_BKSL(CHAR_BKSL_DATA, 1, 5, 9);
	const uint8_t CHAR_RSQB_DATA[] = {
		0xe0,
		0x20,
		0x20,
		0x20,
		0x20,
		0x20,
		0x20,
		0x20,
		0xe0,
	};
	const Image CHAR_RSQB(CHAR_RSQB_DATA, 1, 3, 9);
	const uint8_t CHAR_CART_DATA[] = {
		0x20,
		0x50,
		0x88,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
	};
	const Image CHAR_CART(CHAR_CART_DATA, 1, 5, 9);
	const uint8_t CHAR_USCR_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0xf8,
	};
	const Image CHAR_USCR(CHAR_USCR_DATA, 1, 5, 9);
	// const uint8_t CHAR_BKTK_DATA[] = {
	// 	0x40,
	// 	0x20,
	// 	0x10,
	// 	0x00,
	// 	0x00,
	// 	0x00,
	// 	0x00,
	// 	0x00,
	// 	0x00,
	// };
	// const Image CHAR_BKTK(CHAR_BKTK_DATA, 1, 5, 9);
	const uint8_t CHAR_LCA_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0x78,
		0x88,
		0x88,
		0x98,
		0x68,
	};
	const Image CHAR_LCA(CHAR_LCA_DATA, 1, 5, 9);
	const uint8_t CHAR_LCB_DATA[] = {
		0x80,
		0x80,
		0x80,
		0x80,
		0xb0,
		0xc8,
		0x88,
		0x88,
		0xf0,
	};
	const Image CHAR_LCB(CHAR_LCB_DATA, 1, 5, 9);
	const uint8_t CHAR_LCC_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0x70,
		0x80,
		0x80,
		0x88,
		0x70,
	};
	const Image CHAR_LCC(CHAR_LCC_DATA, 1, 5, 9);
	const uint8_t CHAR_LCD_DATA[] = {
		0x08,
		0x08,
		0x08,
		0x08,
		0x68,
		0x98,
		0x88,
		0x88,
		0x78,
	};
	const Image CHAR_LCD(CHAR_LCD_DATA, 1, 5, 9);
	const uint8_t CHAR_LCE_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0x70,
		0x88,
		0xf8,
		0x80,
		0x70,
	};
	const Image CHAR_LCE(CHAR_LCE_DATA, 1, 5, 9);
	const uint8_t CHAR_LCF_DATA[] = {
		0x00,
        0x20,
        0x50,
        0x40,
        0xe0,
        0x40,
        0x40,
        0x40,
        0x40,
	};
	const Image CHAR_LCF(CHAR_LCF_DATA, 1, 4, 9);
	const uint8_t CHAR_LCG_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x78,
		0x88,
		0x88,
		0x78,
		0x08,
		0x70,
	};
	const Image CHAR_LCG(CHAR_LCG_DATA, 1, 5, 9);
	const uint8_t CHAR_LCH_DATA[] = {
		0x80,
		0x80,
		0x80,
		0x80,
		0xb0,
		0xc8,
		0x88,
		0x88,
		0x88,
	};
	const Image CHAR_LCH(CHAR_LCH_DATA, 1, 5, 9);
	const uint8_t CHAR_LCI_DATA[] = {
		0x00,
        0x00,
        0x40,
        0x00,
        0xc0,
        0x40,
        0x40,
        0x40,
        0xe0,
	};
	const Image CHAR_LCI(CHAR_LCI_DATA, 1, 3, 9);
	const uint8_t CHAR_LCJ_DATA[] = {
		0x00,
		0x00,
		0x10,
		0x00,
		0x30,
		0x10,
		0x10,
		0x90,
		0x60,
	};
	const Image CHAR_LCJ(CHAR_LCJ_DATA, 1, 4, 9);
	const uint8_t CHAR_LCK_DATA[] = {
		0xc0,
		0x40,
		0x40,
		0x40,
		0x48,
		0x50,
		0x60,
		0x50,
		0x48,
	};
	const Image CHAR_LCK(CHAR_LCK_DATA, 1, 5, 9);
	const uint8_t CHAR_LCL_DATA[] = {
        0xc0, // 11000000
        0x40, // 01000000
        0x40, // 01000000
        0x40, // 01000000
        0x40, // 01000000
        0x40, // 01000000
        0x40, // 01000000
        0x40, // 01000000
        0xe0, // 11100000
	};
	const Image CHAR_LCL(CHAR_LCL_DATA, 1, 3, 9);
	const uint8_t CHAR_LCM_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0xd0,
		0xa8,
		0xa8,
		0xa8,
		0xa8,
	};
	const Image CHAR_LCM(CHAR_LCM_DATA, 1, 5, 9);
	const uint8_t CHAR_LCN_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0xb0,
		0xc8,
		0x88,
		0x88,
		0x88,
	};
	const Image CHAR_LCN(CHAR_LCN_DATA, 1, 5, 9);
	const uint8_t CHAR_LCO_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0x70,
		0x88,
		0x88,
		0x88,
		0x70,
	};
	const Image CHAR_LCO(CHAR_LCO_DATA, 1, 5, 9);
	const uint8_t CHAR_LCP_DATA[] = {
		0x00,
		0x00,
		0x00,
		0xf0,
		0x88,
		0x88,
		0xf0,
		0x80,
		0x80,
	};
	const Image CHAR_LCP(CHAR_LCP_DATA, 1, 5, 9);
	const uint8_t CHAR_LCQ_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x68,
		0x98,
		0x88,
		0x78,
		0x08,
		0x08,
	};
	const Image CHAR_LCQ(CHAR_LCQ_DATA, 1, 5, 9);
	const uint8_t CHAR_LCR_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0xb0,
		0xc8,
		0x80,
		0x80,
		0x80,
	};
	const Image CHAR_LCR(CHAR_LCR_DATA, 1, 5, 9);
	const uint8_t CHAR_LCS_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0x78,
		0x80,
		0x70,
		0x08,
		0xf0,
	};
	const Image CHAR_LCS(CHAR_LCS_DATA, 1, 5, 9);
	const uint8_t CHAR_LCT_DATA[] = {
        0x00, // 00000000
        0x00, // 00000000
        0x40, // 01000000
        0x40, // 01000000
        0xe0, // 11100000
        0x40, // 01000000
        0x40, // 01000000
        0x50, // 01010000
        0x20, // 00100000
	};
	const Image CHAR_LCT(CHAR_LCT_DATA, 1, 4, 9);
	const uint8_t CHAR_LCU_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0x88,
		0x88,
		0x88,
		0x98,
		0x68,
	};
	const Image CHAR_LCU(CHAR_LCU_DATA, 1, 5, 9);
	const uint8_t CHAR_LCV_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0x88,
		0x88,
		0x88,
		0x50,
		0x20,
	};
	const Image CHAR_LCV(CHAR_LCV_DATA, 1, 5, 9);
	const uint8_t CHAR_LCW_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0x88,
		0x88,
		0xa8,
		0xa8,
		0x50,
	};
	const Image CHAR_LCW(CHAR_LCW_DATA, 1, 5, 9);
	const uint8_t CHAR_LCX_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0x88,
		0x50,
		0x20,
		0x50,
		0x88,
	};
	const Image CHAR_LCX(CHAR_LCX_DATA, 1, 5, 9);
	const uint8_t CHAR_LCY_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0x88,
		0x88,
		0x78,
		0x08,
		0x70,
	};
	const Image CHAR_LCY(CHAR_LCY_DATA, 1, 5, 9);
	const uint8_t CHAR_LCZ_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0xf8,
		0x10,
		0x20,
		0x40,
		0xf8,
	};
	const Image CHAR_LCZ(CHAR_LCZ_DATA, 1, 5, 9);
	// const uint8_t CHAR_LCVB_DATA[] = {
    //     0x20, // 00100000
    //     0x40, // 01000000
    //     0x40, // 01000000
    //     0x40, // 01000000
    //     0x80, // 10000000
    //     0x40, // 01000000
    //     0x40, // 01000000
    //     0x40, // 01000000
    //     0x20, // 00100000
	// };
	// const Image CHAR_LCVB(CHAR_LCVB_DATA, 1, 3, 9);
	const uint8_t CHAR_PIPE_DATA[] = {
        0x80, // 10000000
        0x80, // 10000000
        0x80, // 10000000
        0x80, // 10000000
        0x80, // 10000000
        0x80, // 10000000
        0x80, // 10000000
        0x80, // 10000000
        0x80, // 10000000
	};
	const Image CHAR_PIPE(CHAR_PIPE_DATA, 1, 1, 9);
	// const uint8_t CHAR_RCVB_DATA[] = {
	// 	0x80, // 10000000
    //     0x40, // 01000000
    //     0x40, // 01000000
    //     0x40, // 01000000
    //     0x20, // 00100000
    //     0x40, // 01000000
    //     0x40, // 01000000
    //     0x40, // 01000000
    //     0x80, // 10000000
	// };
	// const Image CHAR_RCVB(CHAR_RCVB_DATA, 1, 3, 9);
	// const uint8_t CHAR_TLDE_DATA[] = {
	// 	0x00,
	// 	0x00,
	// 	0x00,
	// 	0x40,
	// 	0xa8,
	// 	0x10,
	// 	0x00,
	// 	0x00,
	// 	0x00,
	// };
	// const Image CHAR_TLDE(CHAR_TLDE_DATA, 1, 5, 9);
	const uint8_t CHAR_UNKNOWN_DATA[] = {
		0xe0, 0x10, 0x20, 0x1c,
		0x10, 0x20, 0x10, 0x20,
		0x10, 0x42, 0x48, 0x20,
		0x10, 0x49, 0x48, 0x20,
		0x08, 0x44, 0x48, 0x40,
		0x08, 0x40, 0x48, 0x40,
		0x04, 0x40, 0x88, 0x80,
		0x04, 0x21, 0x10, 0x80,
		0x03, 0x96, 0x27, 0x00,
	};
	const Image CHAR_UNKNOWN(CHAR_UNKNOWN_DATA, 4, 30, 9);
	
	// Special characters
    const uint8_t CHAR_MU_DATA[] = {
		0x00, // 00000000
		0x00, // 00000000
		0x88, // 10001000
		0x88, // 10001000
		0x88, // 10001000
		0x98, // 10011000
		0xe8, // 11101000
		0x80, // 10000000
		0x80, // 10000000
	};
	const Image CHAR_MU(CHAR_MU_DATA, 1, 5, 9);
    const uint8_t CHAR_RARW_DATA[] = {
		0x00, // 00000000
		0x00, // 00000000
		0x20, // 00100000
		0x10, // 00010000
		0xf8, // 11111000
		0x10, // 00010000
		0x20, // 00100000
		0x00, // 00000000
		0x00, // 00000000
	};
	const Image CHAR_RARW(CHAR_RARW_DATA, 1, 5, 9);
    const uint8_t CHAR_LAND_DATA[] = {
		0x00, // 00000000
		0x00, // 00000000
		0x20, // 00100000
		0x50, // 01010000
		0x50, // 01010000
		0x88, // 10001000
		0x88, // 10001000
		0x00, // 00000000
		0x00, // 00000000
	};
	const Image CHAR_LAND(CHAR_LAND_DATA, 1, 5, 9);
    const uint8_t CHAR_LOR_DATA[] = {
		0x00, // 00000000
		0x00, // 00000000
		0x88, // 10001000
		0x88, // 10001000
		0x50, // 01010000
		0x50, // 01010000
		0x20, // 00100000
		0x00, // 00000000
		0x00, // 00000000
	};
	const Image CHAR_LOR(CHAR_LOR_DATA, 1, 5, 9);
    const uint8_t CHAR_LXOR_DATA[] = {
		0x00, // 00000000
		0x00, // 00000000
		0x70, // 01110000
		0xa8, // 10101000
		0xf8, // 11111000
		0xa8, // 10101000
		0x70, // 01110000
		0x00, // 00000000
		0x00, // 00000000
	};
	const Image CHAR_LXOR(CHAR_LXOR_DATA, 1, 5, 9);
    const uint8_t CHAR_LNOT_DATA[] = {
		0x00, // 00000000
		0x00, // 00000000
		0x00, // 00000000
		0xf8, // 11111000
		0x08, // 00001000
		0x08, // 00001000
		0x00, // 00000000
		0x00, // 00000000
		0x00, // 00000000
	};
	const Image CHAR_LNOT(CHAR_LNOT_DATA, 1, 5, 9);
    const uint8_t CHAR_ECB_DATA[] = {
        0x00, // 00000000
        0xfe, // 11111110
        0x82, // 10000010
        0x82, // 10000010
        0x82, // 10000010
        0x82, // 10000010
        0x82, // 10000010
        0xfe, // 11111110
        0x00, // 00000000
    };
    const Image CHAR_ECB(CHAR_ECB_DATA, 1, 7, 9);
    const uint8_t CHAR_CCB_DATA[] = {
        0x00, // 00000000
        0xfe, // 11111110
        0xc6, // 11000110
        0xaa, // 10101010
        0x92, // 10010010
        0xaa, // 10101010
        0xc6, // 11000110
        0xfe, // 11111110
        0x00, // 00000000
    };
    const Image CHAR_CCB(CHAR_CCB_DATA, 1, 7, 9);
    const uint8_t CHAR_LEQ_DATA[] = {
        0x10,
        0x20,
        0x40,
        0x80,
        0x40,
        0x20,
        0x10,
        0x00,
        0xf0,
    };
    const Image CHAR_LEQ(CHAR_LEQ_DATA, 1, 4, 9);
    const uint8_t CHAR_GEQ_DATA[] = {
        0x80,
        0x40,
        0x20,
        0x10,
        0x20,
        0x40,
        0x80,
        0x00,
        0xf0,
    };
    const Image CHAR_GEQ(CHAR_GEQ_DATA, 1, 4, 9);
	const uint8_t CHAR_CRS_DATA[] = {
		0x00,
		0x00,
		0x88,
		0x50,
		0x20,
		0x50,
		0x88,
		0x00,
		0x00,
	};
	const Image CHAR_CRS(CHAR_CRS_DATA, 1, 5, 9);
	const uint8_t CHAR_AGV_DATA[] = {
		0x00, 0x00,
		0x00, 0x00,
		0x00, 0x00,
		0x78, 0x00,
		0x88, 0x00,
		0x89, 0x00,
		0x7a, 0x80,
		0x0a, 0x80,
		0x71, 0x00,
	};
	const Image CHAR_AGV(CHAR_AGV_DATA, 2, 9, 9);
	const uint8_t CHAR_AVGO_DATA[] = {
		0x88, 0x00,
		0xc8, 0x00,
		0xc8, 0x00,
		0xa8, 0x00,
		0xa8, 0x00,
		0x99, 0x00,
		0x9a, 0x80,
		0x8b, 0x80,
		0x8a, 0x80,
	};
	const Image CHAR_AVGO(CHAR_AVGO_DATA, 2, 9, 9);
	const uint8_t CHAR_ECHG_DATA[] = {
		0x00,
		0x00,
		0x07,
		0x00,
		0x70,
		0x88,
		0xf8,
		0x80,
		0x70,
	};
	const Image CHAR_ECHG(CHAR_ECHG_DATA, 1, 8, 9);
	const uint8_t CHAR_VLIG_DATA[] = {
		0x00,
		0x00,
		0x30,
		0x48,
		0x80,
		0x80,
		0x80,
		0x88,
		0x70,
	};
	const Image CHAR_VLIG(CHAR_VLIG_DATA, 1, 5, 9);
	const uint8_t CHAR_EE_DATA[] = {
		0x00, // 00000000
        0x00, // 00000000
        0x00, // 00000000
        0x00, // 00000000
        0xe0, // 11100000
        0x80, // 10000000
        0xe0, // 11100000
        0x80, // 10000000
        0xe0, // 11100000
	};
	const Image CHAR_EE(CHAR_EE_DATA, 1, 3, 9);
	const uint8_t CHAR_EULR_DATA[] = {
		0x00,
		0x00,
		0x30,
		0x48,
		0xd0,
		0xe0,
		0xc0,
		0xc8,
		0x70,
	};
	const Image CHAR_EULR(CHAR_EULR_DATA, 1, 5, 9);
	const uint8_t CHAR_THETA_DATA[] = {
		0x70,
		0x88,
		0x88,
		0x88,
		0xf8,
		0x88,
		0x88,
		0x88,
		0x70,
	};
	const Image CHAR_THETA(CHAR_THETA_DATA, 1, 5, 9);
	const uint8_t CHAR_SERR_DATA[] = {
		0x28, 0x00, 0x82, 0x00, 0x00,
		0x49, 0x02, 0x42, 0x00, 0x88,
		0x8a, 0x85, 0x22, 0x00, 0x88,
		0xb1, 0x02, 0x2c, 0x00, 0x88,
		0x80, 0x78, 0x20, 0x03, 0xfe,
		0x80, 0x48, 0x20, 0x00, 0x00,
		0x80, 0x48, 0x20, 0xf0, 0x00,
		0x40, 0x78, 0x41, 0x08, 0x00,
		0x20, 0x00, 0x82, 0x04, 0x00,
	};
	const Image CHAR_SERR(CHAR_SERR_DATA, 5, 39, 9);
	const uint8_t CHAR_PI_DATA[] = {
		0x00,
		0x00,
		0x00,
		0x00,
		0xf8,
		0x50,
		0x50,
		0x50,
		0x98,
	};
	const Image CHAR_PI(CHAR_PI_DATA, 1, 5, 9);
	const uint8_t CHAR_MUL_DATA[] = {
        0x00, // 00000000
        0x00, // 00000000
        0x00, // 00000000
        0xc0, // 11000000
        0xc0, // 11000000
        0x00, // 00000000
        0x00, // 00000000
        0x00, // 00000000
        0x00, // 00000000
	};
	const Image CHAR_MUL(CHAR_MUL_DATA, 1, 2, 9);
	const uint8_t CHAR_DIV_DATA[] = {
		0x08,
	    0x08,
	    0x10,
	    0x10,
	    0x20,
	    0x20,
	    0x40,
	    0x40,
	    0x80,
	};
	const Image CHAR_DIV(CHAR_DIV_DATA, 1, 5, 9);

	const uint8_t CHAR_SUMMATION_DATA[] = {
		0xff, 0xc0,
		0xff, 0xc0,
		0xc0, 0x40,
		0xe0, 0x00,
		0x70, 0x00,
		0x38, 0x00,
		0x1c, 0x00,
		0x0e, 0x00,
		0x1c, 0x00,
		0x38, 0x00,
		0x70, 0x00,
		0xe0, 0x00,
		0xc0, 0x40,
		0xff, 0xc0,
		0xff, 0xc0,
	};
	const Image CHAR_SUMMATION(CHAR_SUMMATION_DATA, 2, 10, 15);
	const uint8_t CHAR_PRODUCT_DATA[] = {
		0xff, 0xc0,
		0xff, 0xc0,
		0x61, 0x80,
		0x61, 0x80,
		0x61, 0x80,
		0x61, 0x80,
		0x61, 0x80,
		0x61, 0x80,
		0x61, 0x80,
		0x61, 0x80,
		0x61, 0x80,
		0x61, 0x80,
		0x61, 0x80,
		0x61, 0x80,
		0xf3, 0xc0,
	};
	const Image CHAR_PRODUCT(CHAR_PRODUCT_DATA, 2, 10, 15);

    /* Small Charset */
    const uint8_t CHAR_SMALL_0_DATA[] = {
        0x40, // 01000000
        0xa0, // 10100000
        0xa0, // 10100000
        0xa0, // 10100000
        0x40, // 01000000
    };
    const Image CHAR_SMALL_0(CHAR_SMALL_0_DATA, 1, 3, 5);
        const uint8_t CHAR_SMALL_1_DATA[] = {
        0x40, // 01000000
        0xc0, // 11000000
        0x40, // 01000000
        0x40, // 01000000
        0xe0, // 11100000
    };
    const Image CHAR_SMALL_1(CHAR_SMALL_1_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_2_DATA[] = {
        0xc0, // 11000000
        0x20, // 00100000
        0x40, // 01000000
        0x80, // 10000000
        0xe0, // 11100000
    };
    const Image CHAR_SMALL_2(CHAR_SMALL_2_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_3_DATA[] = {
        0xc0, // 11000000
        0x20, // 00100000
        0x40, // 01000000
        0x20, // 00100000
        0xc0, // 11000000
    };
    const Image CHAR_SMALL_3(CHAR_SMALL_3_DATA, 1, 3, 5);
        const uint8_t CHAR_SMALL_4_DATA[] = {
        0xa0, // 10100000
        0xa0, // 10100000
        0xe0, // 11100000
        0x20, // 00100000
        0x20, // 00100000
    };
    const Image CHAR_SMALL_4(CHAR_SMALL_4_DATA, 1, 3, 5);
        const uint8_t CHAR_SMALL_5_DATA[] = {
        0xe0, // 11100000
        0x80, // 10000000
        0xc0, // 11000000
        0x20, // 00100000
        0xc0, // 11000000
    };
    const Image CHAR_SMALL_5(CHAR_SMALL_5_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_6_DATA[] = {
		0x60, // 01100000
		0x80, // 10000000
		0xe0, // 11100000
		0xa0, // 10100000
		0xe0, // 11100000
	};
	const Image CHAR_SMALL_6(CHAR_SMALL_6_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_7_DATA[] = {
		0xe0, // 11100000
		0x20, // 00100000
		0x40, // 01000000
		0x40, // 01000000
		0x40, // 01000000
	};
	const Image CHAR_SMALL_7(CHAR_SMALL_7_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_8_DATA[] = {
		0xe0, // 11100000
		0xa0, // 10100000
		0xe0, // 11100000
		0xa0, // 10100000
		0xe0, // 11100000
	};
	const Image CHAR_SMALL_8(CHAR_SMALL_8_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_9_DATA[] = {
		0xe0, // 11100000
		0xa0, // 10100000
		0xe0, // 11100000
		0x20, // 00100000
		0xc0, // 11000000
	};
	const Image CHAR_SMALL_9(CHAR_SMALL_9_DATA, 1, 3, 5);

    const uint8_t CHAR_SMALL_A_DATA[] = {
		0x40, // 01000000
		0xa0, // 10100000
		0xe0, // 11100000
		0xa0, // 10100000
		0xa0, // 10100000
	};
	const Image CHAR_SMALL_A(CHAR_SMALL_A_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_B_DATA[] = {
		0xc0, // 11000000
		0xa0, // 10100000
		0xc0, // 11000000
		0xa0, // 10100000
		0xc0, // 11000000
	};
	const Image CHAR_SMALL_B(CHAR_SMALL_B_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_C_DATA[] = {
		0x60, // 01100000
		0x80, // 10000000
		0x80, // 10000000
		0x80, // 10000000
		0x60, // 01100000
	};
	const Image CHAR_SMALL_C(CHAR_SMALL_C_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_D_DATA[] = {
		0xc0, // 11000000
		0xa0, // 10100000
		0xa0, // 10100000
		0xa0, // 10100000
		0xc0, // 11000000
	};
	const Image CHAR_SMALL_D(CHAR_SMALL_D_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_E_DATA[] = {
		0xe0, // 11100000
		0x80, // 10000000
		0xc0, // 11000000
		0x80, // 10000000
		0xe0, // 11100000
	};
	const Image CHAR_SMALL_E(CHAR_SMALL_E_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_F_DATA[] = {
		0xe0, // 11100000
		0x80, // 10000000
		0xc0, // 11000000
		0x80, // 10000000
		0x80, // 10000000
	};
	const Image CHAR_SMALL_F(CHAR_SMALL_F_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_G_DATA[] = {
		0x70, // 01110000
		0x80, // 10000000
		0xb0, // 10110000
		0x90, // 10010000
		0x70, // 01110000
	};
	const Image CHAR_SMALL_G(CHAR_SMALL_G_DATA, 1, 4, 5);
    const uint8_t CHAR_SMALL_H_DATA[] = {
		0xa0, // 10100000
		0xa0, // 10100000
		0xe0, // 11100000
		0xa0, // 10100000
		0xa0, // 10100000
	};
	const Image CHAR_SMALL_H(CHAR_SMALL_H_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_I_DATA[] = {
		0xe0, // 11100000
		0x40, // 01000000
		0x40, // 01000000
		0x40, // 01000000
		0xe0, // 11100000
	};
	const Image CHAR_SMALL_I(CHAR_SMALL_I_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_J_DATA[] = {
		0x60, // 01100000
		0x20, // 00100000
		0x20, // 00100000
		0xa0, // 10100000
		0x40, // 01000000
	};
	const Image CHAR_SMALL_J(CHAR_SMALL_J_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_K_DATA[] = {
		0xa0, // 10100000
		0xa0, // 10100000
		0xc0, // 11000000
		0xa0, // 10100000
		0xa0, // 10100000
	};
	const Image CHAR_SMALL_K(CHAR_SMALL_K_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_L_DATA[] = {
		0x80, // 10000000
		0x80, // 10000000
		0x80, // 10000000
		0x80, // 10000000
		0xe0, // 11100000
	};
	const Image CHAR_SMALL_L(CHAR_SMALL_L_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_M_DATA[] = {
		0x88, // 10001000
		0xd8, // 11011000
		0xa8, // 10101000
		0x88, // 10001000
		0x88, // 10001000
	};
	const Image CHAR_SMALL_M(CHAR_SMALL_M_DATA, 1, 5, 5);
    const uint8_t CHAR_SMALL_N_DATA[] = {
		0x90, // 10010000
		0xd0, // 11010000
		0xb0, // 10110000
		0xb0, // 10110000
		0x90, // 10010000
	};
	const Image CHAR_SMALL_N(CHAR_SMALL_N_DATA, 1, 4, 5);
    const uint8_t CHAR_SMALL_O_DATA[] = {
		0xe0, // 11100000
		0xa0, // 10100000
		0xa0, // 10100000
		0xa0, // 10100000
		0xe0, // 11100000
	};
	const Image CHAR_SMALL_O(CHAR_SMALL_O_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_P_DATA[] = {
		0xe0, // 11100000
		0xa0, // 10100000
		0xe0, // 11100000
		0x80, // 10000000
		0x80, // 10000000
	};
	const Image CHAR_SMALL_P(CHAR_SMALL_P_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_Q_DATA[] = {
		0xf0, // 11110000
		0x90, // 10010000
		0x90, // 10010000
		0xb0, // 10110000
		0xf0, // 11110000
	};
	const Image CHAR_SMALL_Q(CHAR_SMALL_Q_DATA, 1, 4, 5);
    const uint8_t CHAR_SMALL_R_DATA[] = {
		0xe0, // 11100000
		0xa0, // 10100000
		0xe0, // 11100000
		0xc0, // 11000000
		0xa0, // 10100000
	};
	const Image CHAR_SMALL_R(CHAR_SMALL_R_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_S_DATA[] = {
		0xe0, // 11100000
		0x80, // 10000000
		0xe0, // 11100000
		0x20, // 00100000
		0xe0, // 11100000
	};
	const Image CHAR_SMALL_S(CHAR_SMALL_S_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_T_DATA[] = {
		0xe0, // 11100000
		0x40, // 01000000
		0x40, // 01000000
		0x40, // 01000000
		0x40, // 01000000
	};
	const Image CHAR_SMALL_T(CHAR_SMALL_T_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_U_DATA[] = {
		0xa0, // 10100000
		0xa0, // 10100000
		0xa0, // 10100000
		0xa0, // 10100000
		0xe0, // 11100000
	};
	const Image CHAR_SMALL_U(CHAR_SMALL_U_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_V_DATA[] = {
		0xa0, // 10100000
		0xa0, // 10100000
		0xa0, // 10100000
		0xa0, // 10100000
		0x40, // 01000000
	};
	const Image CHAR_SMALL_V(CHAR_SMALL_V_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_W_DATA[] = {
		0x88, // 10001000
		0xa8, // 10101000
		0xa8, // 10101000
		0xa8, // 10101000
		0x50, // 01010000
	};
	const Image CHAR_SMALL_W(CHAR_SMALL_W_DATA, 1, 5, 5);
    const uint8_t CHAR_SMALL_X_DATA[] = {
		0xa0, // 10100000
		0xa0, // 10100000
		0x40, // 01000000
		0xa0, // 10100000
		0xa0, // 10100000
	};
	const Image CHAR_SMALL_X(CHAR_SMALL_X_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_Y_DATA[] = {
		0xa0, // 10100000
		0xa0, // 10100000
		0x40, // 01000000
		0x40, // 01000000
		0x40, // 01000000
	};
	const Image CHAR_SMALL_Y(CHAR_SMALL_Y_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_Z_DATA[] = {
		0xe0, // 11100000
		0x20, // 00100000
		0x40, // 01000000
		0x80, // 10000000
		0xe0, // 11100000
	};
	const Image CHAR_SMALL_Z(CHAR_SMALL_Z_DATA, 1, 3, 5);

    const uint8_t CHAR_SMALL_DOT_DATA[] = {
		0x00, // 00000000
		0x00, // 00000000
		0x00, // 00000000
		0x00, // 00000000
		0x80, // 10000000
	};
	const Image CHAR_SMALL_DOT(CHAR_SMALL_DOT_DATA, 1, 1, 5);
    const uint8_t CHAR_SMALL_EQL_DATA[] = {
		0x00, // 00000000
		0xe0, // 11100000
		0x00, // 00000000
		0xe0, // 11100000
		0x00, // 00000000
	};
	const Image CHAR_SMALL_EQL(CHAR_SMALL_EQL_DATA, 1, 3, 5);
    const uint8_t CHAR_SMALL_EE_DATA[] = {
		0x00, 0x00, // 00000000 00000000
		0x09, 0x10, // 00001001 00010000
		0xaa, 0xa8, // 10101010 10101000
		0x4a, 0x80, // 01001010 10000000
		0xa9, 0x00, // 10101001 00000000
	};
	const Image CHAR_SMALL_EE(CHAR_SMALL_EE_DATA, 2, 13, 5);
    const uint8_t CHAR_SMALL_MINUS_DATA[] = {
		0x00, // 00000000
		0x00, // 00000000
		0xe0, // 11100000
		0x00, // 00000000
		0x00, // 00000000
	};
	const Image CHAR_SMALL_MINUS(CHAR_SMALL_MINUS_DATA, 1, 3, 5);
	const uint8_t CHAR_SMALL_COMMA_DATA[] = {
		0x00, // 00000000
		0x00, // 00000000
		0x00, // 00000000
		0x80, // 10000000
		0x80, // 10000000
	};
	const Image CHAR_SMALL_COMMA(CHAR_SMALL_COMMA_DATA, 1, 1, 5);

    /* Other Images */
    const uint8_t IMG_PTABLE_DATA[] = {
		0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, // 11111100 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000111 11100000
		0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x20, // 10000100 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000100 00100000
		0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x20, // 10000100 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000100 00100000
		0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x20, // 10000100 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000100 00100000
		0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x20, // 10000100 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000100 00100000
		0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xe0, // 11111111 11100000 00000000 00000000 00000000 00000000 00000000 00001111 11111111 11111111 11111111 11100000
		0x84, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100000 00000000 00000000 00000000 00000000 00000000 00001000 01000010 00010000 10000100 00100000
		0x84, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100000 00000000 00000000 00000000 00000000 00000000 00001000 01000010 00010000 10000100 00100000
		0x84, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100000 00000000 00000000 00000000 00000000 00000000 00001000 01000010 00010000 10000100 00100000
		0x84, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100000 00000000 00000000 00000000 00000000 00000000 00001000 01000010 00010000 10000100 00100000
		0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xe0, // 11111111 11100000 00000000 00000000 00000000 00000000 00000000 00001111 11111111 11111111 11111111 11100000
		0x84, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100000 00000000 00000000 00000000 00000000 00000000 00001000 01000010 00010000 10000100 00100000
		0x84, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100000 00000000 00000000 00000000 00000000 00000000 00001000 01000010 00010000 10000100 00100000
		0x84, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100000 00000000 00000000 00000000 00000000 00000000 00001000 01000010 00010000 10000100 00100000
		0x84, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100000 00000000 00000000 00000000 00000000 00000000 00001000 01000010 00010000 10000100 00100000
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, // 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11100000
		0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00100000
		0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00100000
		0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00100000
		0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00100000
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, // 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11100000
		0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00100000
		0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00100000
		0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00100000
		0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00100000
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, // 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11100000
		0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00100000
		0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00100000
		0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00100000
		0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00100000
		0xff, 0xe1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, // 11111111 11100001 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11100000
		0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00100000
		0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00100000
		0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00100000
		0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x20, // 10000100 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00100000
		0xff, 0xe1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, // 11111111 11100001 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11100000
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
		0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, // 00000000 00111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111100 00000000
		0x00, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x00, // 00000000 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00000000
		0x00, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x00, // 00000000 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00000000
		0x00, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x00, // 00000000 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00000000
		0x00, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x00, // 00000000 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00000000
		0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, // 00000000 00111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111100 00000000
		0x00, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x00, // 00000000 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00000000
		0x00, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x00, // 00000000 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00000000
		0x00, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x00, // 00000000 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00000000
		0x00, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x00, // 00000000 00100001 00001000 01000010 00010000 10000100 00100001 00001000 01000010 00010000 10000100 00000000
		0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, // 00000000 00111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111100 00000000
	};
	const Image IMG_PTABLE(IMG_PTABLE_DATA, 12, 91, 51);

} // namespace lcd
