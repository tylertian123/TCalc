#include "ptable.hpp"

namespace pt {
	const Element PERIOD_1_ELEMENTS[] = {
		{
			"Hydrogen",
			"H",
			"1",
			"1s1",
			13.99,
			20.271,
			0.08988,
			72.769,
			1.008,
			1,
			0,
			1,
			DIATOMIC_NONMETAL,
			220,
		},
		{
			"Helium",
			"He",
			"",
			"1s2",
			0.95,
			4.222,
			0.1786,
			-48,
			4.0026022,
			2,
			2,
			18,
			NOBLE_GAS,
			0,
		},
	};

	const Element PERIOD_2_ELEMENTS[] = {
		{
			"Lithium",
			"Li",
			"1",
			"[He] 2s1",
			453.65,
			1603,
			0.534,
			59.6326,
			6.94,
			3,
			4,
			1,
			ALKALI_METAL,
			98,
		},
		{
			"Beryllium",
			"Be",
			"2",
			"[He] 2s2",
			1560,
			2742,
			1.85,
			-48,
			9.01218315,
			4,
			5,
			2,
			ALKALINE_EARTH_METAL,
			157,
		},
		{
			"Boron",
			"B",
			"3",
			"[He] 2s2 2p1",
			2349,
			4200,
			2.08,
			26.989,
			10.81,
			5,
			6,
			13,
			METALLOID,
			204,
		},
		{
			"Carbon",
			"C",
			"4",
			"[He] 2s2 2p2",
			NAN,
			NAN,
			1.821,
			121.7763,
			12.011,
			6,
			6,
			14,
			POLYATOMIC_NONMETAL,
			255,
		},
		{
			"Nitrogen",
			"N",
			"3",
			"[He] 2s2 2p3",
			63.15,
			77.355,
			1.251,
			-6.8,
			14.007,
			7,
			7,
			15,
			DIATOMIC_NONMETAL,
			304,
		},
		{
			"Oxygen",
			"O",
			"2",
			"[He] 2s2 2p4",
			54.36,
			90.188,
			1.429,
			140.976,
			15.999,
			8,
			8,
			16,
			DIATOMIC_NONMETAL,
			344,
		},
		{
			"Fluorine",
			"F",
			"1",
			"[He] 2s2 2p5",
			53.48,
			85.03,
			1.696,
			328.1649,
			18.9984031636,
			9,
			10,
			17,
			DIATOMIC_NONMETAL,
			398,
		},
		{
			"Neon",
			"Ne",
			"",
			"[He] 2s2 2p6",
			24.56,
			27.104,
			0.9002,
			-116,
			20.17976,
			10,
			10,
			18,
			NOBLE_GAS,
			0,
		},
	};

	const Element PERIOD_3_ELEMENTS[] = {
		{
			"Sodium",
			"Na",
			"1",
			"[Ne] 3s1",
			370.944,
			1156.09,
			0.968,
			52.867,
			22.989769282,
			11,
			12,
			1,
			ALKALI_METAL,
			93,
		},
		{
			"Magnesium",
			"Mg",
			"2",
			"[Ne] 3s2",
			923,
			1363,
			1.738,
			-40,
			24.305,
			12,
			12,
			2,
			ALKALINE_EARTH_METAL,
			131,
		},
		{
			"Aluminium",
			"Al",
			"3",
			"[Ne] 3s2 3p1",
			933.47,
			2743,
			2.7,
			41.762,
			26.98153857,
			13,
			14,
			13,
			POST_TRANSITION_METAL,
			161,
		},
		{
			"Silicon",
			"Si",
			"4",
			"[Ne] 3s2 3p2",
			1687,
			3538,
			2.329,
			134.0684,
			28.085,
			14,
			14,
			14,
			METALLOID,
			190,
		},
		{
			"Phosphorus",
			"P",
			"3",
			"[Ne] 3s2 3p3",
			NAN,
			NAN,
			1.823,
			72.037,
			30.9737619985,
			15,
			16,
			15,
			POLYATOMIC_NONMETAL,
			219,
		},
		{
			"Sulfur",
			"S",
			"2",
			"[Ne] 3s2 3p4",
			388.36,
			717.8,
			2.07,
			200.4101,
			32.06,
			16,
			16,
			16,
			POLYATOMIC_NONMETAL,
			258,
		},
		{
			"Chlorine",
			"Cl",
			"1",
			"[Ne] 3s2 3p5",
			171.6,
			239.11,
			3.2,
			348.575,
			35.45,
			17,
			18,
			17,
			DIATOMIC_NONMETAL,
			316,
		},
		{
			"Argon",
			"Ar",
			"",
			"[Ne] 3s2 3p6",
			83.81,
			87.302,
			1.784,
			-96,
			39.9481,
			18,
			22,
			18,
			NOBLE_GAS,
			0,
		},
	};

	const Element PERIOD_4_ELEMENTS[] = {
		{
			"Potassium",
			"K",
			"1",
			"[Ar] 4s1",
			336.7,
			1032,
			0.862,
			48.383,
			39.09831,
			19,
			20,
			1,
			ALKALI_METAL,
			82,
		},
		{
			"Calcium",
			"Ca",
			"2",
			"[Ar] 4s2",
			1115,
			1757,
			1.55,
			2.37,
			40.0784,
			20,
			20,
			2,
			ALKALINE_EARTH_METAL,
			100,
		},
		{
			"Scandium",
			"Sc",
			"3",
			"[Ar] 3d1 4s2",
			1814,
			3109,
			2.985,
			18,
			44.9559085,
			21,
			24,
			3,
			TRANSITION_METAL,
			136,
		},
		{
			"Titanium",
			"Ti",
			"4,3",
			"[Ar] 3d2 4s2",
			1941,
			3560,
			4.506,
			7.289,
			47.8671,
			22,
			26,
			4,
			TRANSITION_METAL,
			154,
		},
		{
			"Vanadium",
			"V",
			"5,4",
			"[Ar] 3d3 4s2",
			2183,
			3680,
			6.0,
			50.911,
			50.94151,
			23,
			28,
			5,
			TRANSITION_METAL,
			163,
		},
		{
			"Chromium",
			"Cr",
			"3,2",
			"[Ar] 3d5 4s1",
			2180,
			2944,
			7.19,
			65.21,
			51.99616,
			24,
			28,
			6,
			TRANSITION_METAL,
			166,
		},
		{
			"Manganese",
			"Mn",
			"2,4",
			"[Ar] 3d5 4s2",
			1519,
			2334,
			7.21,
			-50,
			54.9380443,
			25,
			30,
			7,
			TRANSITION_METAL,
			155,
		},
		{
			"Iron",
			"Fe",
			"3,2",
			"[Ar] 3d6 4s2",
			1811,
			3134,
			7.874,
			14.785,
			55.8452,
			26,
			30,
			8,
			TRANSITION_METAL,
			183,
		},
		{
			"Cobalt",
			"Co",
			"2,3",
			"[Ar] 3d7 4s2",
			1768,
			3200,
			8.9,
			63.898,
			58.9331944,
			27,
			32,
			9,
			TRANSITION_METAL,
			188,
		},
		{
			"Nickel",
			"Ni",
			"2,3",
			"[Ar] 3d8 4s2",
			1728,
			3003,
			8.908,
			111.65,
			58.69344,
			28,
			31,
			10,
			TRANSITION_METAL,
			191,
		},
		{
			"Copper",
			"Cu",
			"2,1",
			"[Ar] 3d10 4s1",
			1357.77,
			2835,
			8.96,
			119.235,
			63.5463,
			29,
			35,
			11,
			TRANSITION_METAL,
			190,
		},
		{
			"Zinc",
			"Zn",
			"2",
			"[Ar] 3d10 4s2",
			692.68,
			1180,
			7.14,
			-58,
			65.382,
			30,
			35,
			12,
			TRANSITION_METAL,
			165,
		},
		{
			"Gallium",
			"Ga",
			"3",
			"[Ar] 3d10 4s2 4p1",
			302.9146,
			2673,
			5.91,
			41,
			69.7231,
			31,
			39,
			13,
			POST_TRANSITION_METAL,
			181,
		},
		{
			"Germanium",
			"Ge",
			"4",
			"[Ar] 3d10 4s2 4p2",
			1211.4,
			3106,
			5.323,
			118.9352,
			72.6308,
			32,
			41,
			14,
			METALLOID,
			201,
		},
		{
			"Arsenic",
			"As",
			"3",
			"[Ar] 3d10 4s2 4p3",
			NAN,
			NAN,
			5.727,
			77.65,
			74.9215956,
			33,
			42,
			15,
			METALLOID,
			218,
		},
		{
			"Selenium",
			"Se",
			"2",
			"[Ar] 3d10 4s2 4p4",
			494,
			958,
			4.81,
			194.9587,
			78.9718,
			34,
			45,
			16,
			POLYATOMIC_NONMETAL,
			255,
		},
		{
			"Bromine",
			"Br",
			"1",
			"[Ar] 3d10 4s2 4p5",
			265.8,
			332.0,
			3.1028,
			324.537,
			79.904,
			35,
			45,
			17,
			DIATOMIC_NONMETAL,
			296,
		},
		{
			"Krypton",
			"Kr",
			"",
			"[Ar] 3d10 4s2 4p6",
			115.78,
			119.93,
			3.749,
			-96,
			83.7982,
			36,
			48,
			18,
			NOBLE_GAS,
			300,
		},
	};

	const Element PERIOD_5_ELEMENTS[] = {
		{
			"Rubidium",
			"Rb",
			"1",
			"[Kr] 5s1",
			312.45,
			961,
			1.532,
			46.884,
			85.46783,
			37,
			48,
			1,
			ALKALI_METAL,
			82,
		},
		{
			"Strontium",
			"Sr",
			"2",
			"[Kr] 5s2",
			1050,
			1650,
			2.64,
			5.023,
			87.621,
			38,
			50,
			2,
			ALKALINE_EARTH_METAL,
			95,
		},
		{
			"Yttrium",
			"Y",
			"3",
			"[Kr] 4d1 5s2",
			1799,
			3203,
			4.472,
			29.6,
			88.905842,
			39,
			50,
			3,
			TRANSITION_METAL,
			122,
		},
		{
			"Zirconium",
			"Zr",
			"4",
			"[Kr] 4d2 5s2",
			2128,
			4650,
			6.52,
			41.806,
			91.2242,
			40,
			51,
			4,
			TRANSITION_METAL,
			133,
		},
		{
			"Niobium",
			"Nb",
			"5,3",
			"[Kr] 4d4 5s1",
			2750,
			5017,
			8.57,
			88.516,
			92.906372,
			41,
			52,
			5,
			TRANSITION_METAL,
			160,
		},
		{
			"Molybdenum",
			"Mo",
			"6",
			"[Kr] 4d5 5s1",
			2896,
			4912,
			10.28,
			72.1,
			95.951,
			42,
			54,
			6,
			TRANSITION_METAL,
			216,
		},
		{
			"Technetium",
			"Tc",
			"7",
			"[Kr] 4d5 5s2",
			2430,
			4538,
			11,
			53,
			98,
			43,
			55,
			7,
			TRANSITION_METAL,
			190,
		},
		{
			"Ruthenium",
			"Ru",
			"3,4",
			"[Kr] 4d7 5s1",
			2607,
			4423,
			12.45,
			100.96,
			101.072,
			44,
			57,
			8,
			TRANSITION_METAL,
			220,
		},
		{
			"Rhodium",
			"Rh",
			"3",
			"[Kr] 4d8 5s1",
			2237,
			3968,
			12.41,
			110.27,
			102.905502,
			45,
			58,
			9,
			TRANSITION_METAL,
			228,
		},
		{
			"Palladium",
			"Pd",
			"2,4",
			"[Kr] 4d10",
			1828.05,
			3236,
			12.023,
			54.24,
			106.421,
			46,
			60,
			10,
			TRANSITION_METAL,
			220,
		},
		{
			"Silver",
			"Ag",
			"1",
			"[Kr] 4d10 5s1",
			1234.93,
			2435,
			10.49,
			125.862,
			107.86822,
			47,
			61,
			11,
			TRANSITION_METAL,
			193,
		},
		{
			"Cadmium",
			"Cd",
			"2",
			"[Kr] 4d10 5s2",
			594.22,
			1040,
			8.65,
			-68,
			112.4144,
			48,
			64,
			12,
			TRANSITION_METAL,
			169,
		},
		{
			"Indium",
			"In",
			"3",
			"[Kr] 4d10 5s2 5p1",
			429.7485,
			2345,
			7.31,
			37.043,
			114.8181,
			49,
			66,
			13,
			POST_TRANSITION_METAL,
			178,
		},
		{
			"Tin",
			"Sn",
			"4,2",
			"[Kr] 4d10 5s2 5p2",
			505.08,
			2875,
			7.365,
			107.2984,
			118.7107,
			50,
			69,
			14,
			POST_TRANSITION_METAL,
			196,
		},
		{
			"Antimony",
			"Sb",
			"3,5",
			"[Kr] 4d10 5s2 5p3",
			903.78,
			1908,
			6.697,
			101.059,
			121.7601,
			51,
			71,
			15,
			METALLOID,
			205,
		},
		{
			"Tellurium",
			"Te",
			"2",
			"[Kr] 4d10 5s2 5p4",
			722.66,
			1261,
			6.24,
			190.161,
			127.603,
			52,
			76,
			16,
			METALLOID,
			210,
		},
		{
			"Iodine",
			"I",
			"1",
			"[Kr] 4d10 5s2 5p5",
			386.85,
			457.4,
			4.933,
			295.1531,
			126.904473,
			53,
			74,
			17,
			DIATOMIC_NONMETAL,
			266,
		},
		{
			"Xenon",
			"Xe",
			"",
			"[Kr] 4d10 5s2 5p6",
			161.4,
			165.051,
			5.894,
			-77,
			131.2936,
			54,
			77,
			18,
			NOBLE_GAS,
			260,
		},
	};

	const Element PERIOD_6_ELEMENTS[] = {
		{
			"Cesium",
			"Cs",
			"1",
			"[Xe] 6s1",
			301.7,
			944,
			1.93,
			45.505,
			132.905451966,
			55,
			78,
			1,
			ALKALI_METAL,
			79,
		},
		{
			"Barium",
			"Ba",
			"2",
			"[Xe] 6s2",
			1000,
			2118,
			3.51,
			13.954,
			137.3277,
			56,
			81,
			2,
			ALKALINE_EARTH_METAL,
			89,
		},
		{
			"Hafnium",
			"Hf",
			"4",
			"[Xe] 4f14 5d2 6s2",
			2506,
			4876,
			13.31,
			17.18,
			178.492,
			72,
			106,
			4,
			TRANSITION_METAL,
			130,
		},
		{
			"Tantalum",
			"Ta",
			"5",
			"[Xe] 4f14 5d3 6s2",
			3290,
			5731,
			16.69,
			31,
			180.947882,
			73,
			108,
			5,
			TRANSITION_METAL,
			150,
		},
		{
			"Tungsten",
			"W",
			"6",
			"[Xe] 4f14 5d4 6s2",
			3695,
			6203,
			19.25,
			78.76,
			183.841,
			74,
			110,
			6,
			TRANSITION_METAL,
			236,
		},
		{
			"Rhenium",
			"Re",
			"7",
			"[Xe] 4f14 5d5 6s2",
			3459,
			5869,
			21.02,
			5.8273,
			186.2071,
			75,
			111,
			7,
			TRANSITION_METAL,
			190,
		},
		{
			"Osmium",
			"Os",
			"4",
			"[Xe] 4f14 5d6 6s2",
			3306,
			5285,
			22.59,
			103.99,
			190.233,
			76,
			114,
			8,
			TRANSITION_METAL,
			220,
		},
		{
			"Iridium",
			"Ir",
			"4",
			"[Xe] 4f14 5d7 6s2",
			2719,
			4403,
			22.56,
			150.94,
			192.2173,
			77,
			115,
			9,
			TRANSITION_METAL,
			220,
		},
		{
			"Platinum",
			"Pt",
			"4,2",
			"[Xe] 4f14 5d9 6s1",
			2041.4,
			4098,
			21.45,
			205.041,
			195.0849,
			78,
			117,
			10,
			TRANSITION_METAL,
			228,
		},
		{
			"Gold",
			"Au",
			"3,1",
			"[Xe] 4f14 5d10 6s1",
			1337.33,
			3243,
			19.3,
			222.747,
			196.9665695,
			79,
			118,
			11,
			TRANSITION_METAL,
			254,
		},
		{
			"Mercury",
			"Hg",
			"2,1",
			"[Xe] 4f14 5d10 6s2",
			234.321,
			629.88,
			13.534,
			-48,
			200.5923,
			80,
			121,
			12,
			TRANSITION_METAL,
			200,
		},
		{
			"Thallium",
			"Tl",
			"1,3",
			"[Xe] 4f14 5d10 6s2 6p1",
			577,
			1746,
			11.85,
			36.4,
			204.38,
			81,
			123,
			13,
			POST_TRANSITION_METAL,
			162,
		},
		{
			"Lead",
			"Pb",
			"2,4",
			"[Xe] 4f14 5d10 6s2 6p2",
			600.61,
			2022,
			11.34,
			34.4204,
			207.21,
			82,
			125,
			14,
			POST_TRANSITION_METAL,
			187,
		},
		{
			"Bismuth",
			"Bi",
			"3,5",
			"[Xe] 4f14 5d10 6s2 6p3",
			544.7,
			1837,
			9.78,
			90.924,
			208.980401,
			83,
			126,
			15,
			POST_TRANSITION_METAL,
			202,
		},
		{
			"Polonium",
			"Po",
			"2,4",
			"[Xe] 4f14 5d10 6s2 6p4",
			527,
			1235,
			9.196,
			136,
			209,
			84,
			125,
			16,
			POST_TRANSITION_METAL,
			200,
		},
		{
			"Astatine",
			"At",
			"1",
			"[Xe] 4f14 5d10 6s2 6p5",
			575,
			610,
			6.35,
			233,
			210,
			85,
			125,
			17,
			METALLOID,
			220,
		},
		{
			"Radon",
			"Rn",
			"",
			"[Xe] 4f14 5d10 6s2 6p6",
			202,
			211.5,
			9.73,
			-68,
			222,
			86,
			136,
			18,
			NOBLE_GAS,
			220,
		},
	};

	const Element PERIOD_7_ELEMENTS[] = {
		{
			"Francium",
			"Fr",
			"1",
			"[Rn] 7s1",
			300,
			950,
			1.87,
			46.89,
			223,
			87,
			136,
			1,
			ALKALI_METAL,
			79,
		},
		{
			"Radium",
			"Ra",
			"2",
			"[Rn] 7s2",
			1233,
			2010,
			5.5,
			9.6485,
			226,
			88,
			138,
			2,
			ALKALINE_EARTH_METAL,
			90,
		},
		{
			"Rutherfordium",
			"Rf",
			"?",
			"[Rn] 5f14 6d2 7s2",
			2400,
			5800,
			23.2,
			NAN,
			267,
			104,
			163,
			4,
			TRANSITION_METAL,
			0,
		},
		{
			"Dubnium",
			"Db",
			"?",
			"[Rn] 5f14 6d3 7s2",
			NAN,
			NAN,
			29.3,
			NAN,
			268,
			105,
			163,
			5,
			TRANSITION_METAL,
			0,
		},
		{
			"Seaborgium",
			"Sg",
			"?",
			"[Rn] 5f14 6d4 7s2",
			NAN,
			NAN,
			35.0,
			NAN,
			269,
			106,
			163,
			6,
			TRANSITION_METAL,
			0,
		},
		{
			"Bohrium",
			"Bh",
			"?",
			"[Rn] 5f14 6d5 7s2",
			NAN,
			NAN,
			37.1,
			NAN,
			270,
			107,
			163,
			7,
			TRANSITION_METAL,
			0,
		},
		{
			"Hassium",
			"Hs",
			"?",
			"[Rn] 5f14 6d6 7s2",
			126,
			NAN,
			40.7,
			NAN,
			269,
			108,
			161,
			8,
			TRANSITION_METAL,
			0,
		},
		{
			"Meitnerium",
			"Mt",
			"?",
			"[Rn] 5f14 6d7 7s2",
			NAN,
			NAN,
			37.4,
			NAN,
			278,
			109,
			169,
			9,
			UNKNOWN,
			0,
		},
		{
			"Darmstadtium",
			"Ds",
			"?",
			"[Rn] 5f14 6d9 7s1",
			NAN,
			NAN,
			34.8,
			NAN,
			281,
			110,
			171,
			10,
			UNKNOWN,
			0,
		},
		{
			"Roentgenium",
			"Rg",
			"?",
			"[Rn] 5f14 6d10 7s1",
			NAN,
			NAN,
			28.7,
			151.0,
			282,
			111,
			171,
			11,
			UNKNOWN,
			0,
		},
		{
			"Copernicium",
			"Cn",
			"?",
			"[Rn] 5f14 6d10 7s2",
			NAN,
			3570,
			23.7,
			NAN,
			285,
			112,
			173,
			12,
			TRANSITION_METAL,
			0,
		},
		{
			"Nihonium",
			"Nh",
			"?",
			"[Rn] 5f14 6d10 7s2 7p1",
			700,
			1430,
			16,
			66.6,
			286,
			113,
			173,
			13,
			UNKNOWN,
			0,
		},
		{
			"Flerovium",
			"Fl",
			"?",
			"[Rn] 5f14 6d10 7s2 7p2",
			340,
			420,
			14,
			NAN,
			289,
			114,
			175,
			14,
			POST_TRANSITION_METAL,
			0,
		},
		{
			"Moscovium",
			"Mc",
			"?",
			"[Rn] 5f14 6d10 7s2 7p3",
			670,
			1400,
			13.5,
			35.3,
			289,
			115,
			174,
			15,
			UNKNOWN,
			0,
		},
		{
			"Livermorium",
			"Lv",
			"?",
			"[Rn] 5f14 6d10 7s2 7p4",
			709,
			1085,
			12.9,
			74.9,
			293,
			116,
			177,
			16,
			UNKNOWN,
			0,
		},
		{
			"Tennessine",
			"Ts",
			"?",
			"[Rn] 5f14 6d10 7s2 7p5",
			723,
			883,
			7.17,
			165.9,
			294,
			117,
			177,
			17,
			UNKNOWN,
			0,
		},
		{
			"Oganesson",
			"Og",
			"?",
			"[Rn] 5f14 6d10 7s2 7p6",
			NAN,
			350,
			4.95,
			5.40318,
			294,
			118,
			176,
			18,
			UNKNOWN,
			0,
		},
	};

	const Element LANTHANIDES[] = {
		{
			"Lanthanum",
			"La",
			"3,2",
			"[Xe] 5d1 6s2",
			1193,
			3737,
			6.162,
			53,
			138.905477,
			57,
			82,
			3,
			LANTHANIDE,
			110,
		},
		{
			"Cerium",
			"Ce",
			"3",
			"[Xe] 4f1 5d1 6s2",
			1068,
			3716,
			6.77,
			55,
			140.1161,
			58,
			82,
			4,
			LANTHANIDE,
			112,
		},
		{
			"Praseodymium",
			"Pr",
			"3",
			"[Xe] 4f3 6s2",
			1208,
			3403,
			6.77,
			93,
			140.907662,
			59,
			82,
			5,
			LANTHANIDE,
			113,
		},
		{
			"Neodymium",
			"Nd",
			"3",
			"[Xe] 4f4 6s2",
			1297,
			3347,
			7.01,
			184.87,
			144.2423,
			60,
			84,
			6,
			LANTHANIDE,
			114,
		},
		{
			"Promethium",
			"Pm",
			"3",
			"[Xe] 4f5 6s2",
			1315,
			3273,
			7.26,
			12.45,
			145,
			61,
			84,
			7,
			LANTHANIDE,
			113,
		},
		{
			"Samarium",
			"Sm",
			"3,2",
			"[Xe] 4f6 6s2",
			1345,
			2173,
			7.52,
			15.63,
			150.362,
			62,
			88,
			8,
			LANTHANIDE,
			117,
		},
		{
			"Europium",
			"Eu",
			"3,2",
			"[Xe] 4f7 6s2",
			1099,
			1802,
			5.264,
			11.2,
			151.9641,
			63,
			89,
			9,
			LANTHANIDE,
			120,
		},
		{
			"Gadolinium",
			"Gd",
			"3",
			"[Xe] 4f7 5d1 6s2",
			1585,
			3273,
			7.9,
			13.22,
			157.253,
			64,
			93,
			10,
			LANTHANIDE,
			120,
		},
		{
			"Terbium",
			"Tb",
			"3",
			"[Xe] 4f9 6s2",
			1629,
			3396,
			8.23,
			112.4,
			158.925352,
			65,
			94,
			11,
			LANTHANIDE,
			110,
		},
		{
			"Dysprosium",
			"Dy",
			"3",
			"[Xe] 4f10 6s2",
			1680,
			2840,
			8.54,
			33.96,
			162.5001,
			66,
			97,
			12,
			LANTHANIDE,
			122,
		},
		{
			"Holmium",
			"Ho",
			"3",
			"[Xe] 4f11 6s2",
			1734,
			2873,
			8.79,
			32.61,
			164.930332,
			67,
			98,
			13,
			LANTHANIDE,
			123,
		},
		{
			"Erbium",
			"Er",
			"3",
			"[Xe] 4f12 6s2",
			1802,
			3141,
			9.066,
			30.1,
			167.2593,
			68,
			99,
			14,
			LANTHANIDE,
			124,
		},
		{
			"Thulium",
			"Tm",
			"3",
			"[Xe] 4f13 6s2",
			1818,
			2223,
			9.32,
			99,
			168.934222,
			69,
			100,
			15,
			LANTHANIDE,
			125,
		},
		{
			"Ytterbium",
			"Yb",
			"3,2",
			"[Xe] 4f14 6s2",
			1097,
			1469,
			6.9,
			-1.93,
			173.0451,
			70,
			103,
			16,
			LANTHANIDE,
			110,
		},
		{
			"Lutetium",
			"Lu",
			"3",
			"[Xe] 4f14 5d1 6s2",
			1925,
			3675,
			9.841,
			33.4,
			174.96681,
			71,
			104,
			17,
			LANTHANIDE,
			127,
		},
	};

	const Element ACTINIDES[] = {
		{
			"Actinium",
			"Ac",
			"3,2",
			"[Rn] 6d1 7s2",
			1500,
			3500,
			10,
			33.77,
			227,
			89,
			138,
			3,
			ACTINIDE,
			110,
		},
		{
			"Thorium",
			"Th",
			"4",
			"[Rn] 6d2 7s2",
			2023,
			5061,
			11.724,
			112.72,
			232.03774,
			90,
			142,
			4,
			ACTINIDE,
			130,
		},
		{
			"Protactinium",
			"Pa",
			"5,4",
			"[Rn] 5f2 6d1 7s2",
			1841,
			4300,
			15.37,
			53.03,
			231.035882,
			91,
			140,
			5,
			ACTINIDE,
			150,
		},
		{
			"Uranium",
			"U",
			"6,4",
			"[Rn] 5f3 6d1 7s2",
			1405.3,
			4404,
			19.1,
			50.94,
			238.028913,
			92,
			146,
			6,
			ACTINIDE,
			138,
		},
		{
			"Neptunium",
			"Np",
			"5",
			"[Rn] 5f4 6d1 7s2",
			912,
			4447,
			20.45,
			45.85,
			237,
			93,
			144,
			7,
			ACTINIDE,
			136,
		},
		{
			"Plutonium",
			"Pu",
			"4,6",
			"[Rn] 5f6 7s2",
			912.5,
			3505,
			19.816,
			-48.33,
			244,
			94,
			150,
			8,
			ACTINIDE,
			128,
		},
		{
			"Americium",
			"Am",
			"3,4",
			"[Rn] 5f7 7s2",
			1449,
			2880,
			12,
			9.93,
			243,
			95,
			148,
			9,
			ACTINIDE,
			113,
		},
		{
			"Curium",
			"Cm",
			"3",
			"[Rn] 5f7 6d1 7s2",
			1613,
			3383,
			13.51,
			27.17,
			247,
			96,
			151,
			10,
			ACTINIDE,
			128,
		},
		{
			"Berkelium",
			"Bk",
			"3,4",
			"[Rn] 5f9 7s2",
			1259,
			2900,
			14.78,
			-165.24,
			247,
			97,
			150,
			11,
			ACTINIDE,
			130,
		},
		{
			"Californium",
			"Cf",
			"3",
			"[Rn] 5f10 7s2",
			1173,
			1743,
			15.1,
			-97.31,
			251,
			98,
			153,
			12,
			ACTINIDE,
			130,
		},
		{
			"Einsteinium",
			"Es",
			"3",
			"[Rn] 5f11 7s2",
			1133,
			1269,
			8.84,
			-28.6,
			252,
			99,
			153,
			13,
			ACTINIDE,
			130,
		},
		{
			"Fermium",
			"Fm",
			"3",
			"[Rn] 5f12 7s2",
			1800,
			NAN,
			NAN,
			33.96,
			257,
			100,
			157,
			14,
			ACTINIDE,
			130,
		},
		{
			"Mendelevium",
			"Md",
			"2,3",
			"[Rn] 5f13 7s2",
			1100,
			NAN,
			NAN,
			93.91,
			258,
			101,
			157,
			15,
			ACTINIDE,
			130,
		},
		{
			"Nobelium",
			"No",
			"2,3",
			"[Rn] 5f14 7s2",
			1100,
			NAN,
			NAN,
			-223.22,
			259,
			102,
			157,
			16,
			ACTINIDE,
			130,
		},
		{
			"Lawrencium",
			"Lr",
			"3",
			"[Rn] 5f14 7s2 7p1",
			1900,
			NAN,
			NAN,
			-30.04,
			266,
			103,
			163,
			17,
			ACTINIDE,
			130,
		},
	};
}
