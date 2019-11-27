#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <getopt.h>
#include <math.h>
#include "threewire.h"
#include "frap.h"
#include "adrf6806.h"

int finish = 0;

void INTHandler(int dummy) {
	finish = 1;
}

static void print_usage(const char *prog)
{
	printf("Usage: %s [-tifmhvscalekno]\n", prog);
	puts("  -t --tune     tune to particular frequency, argument required.\n"
			"  -i --int      integer part of divider (int). Default 75.\n"
			"  -f --frac     fractional part of divider, numerator (int). Default 0.\n"
			"  -m --mod      fractional part of divider, denominator (int). Default n/a (integer mode).\n"
			"  -h --divider  LO divider (int). Allowed values: {2,3,4,5}*{4,8,16}. Default 8.\n"
			"  -v --vco      VCO amplitude (int). Allowed values: 0-63. Default 55.\n"
			"  -s --refsel   clock reference selector (int). Default 1 (x1).\n"
			"  -c --currmul  charge pump current multiplier. Default ? (?uA).\n"
			"  -a --abldly   anti-backlash delay. Default 0.\n"
			"  -l --cpctrl   charge pump control. Default ? (both on).\n"
			"  -e --edgesens PFD clock edge sensitivity. Default 0 (both falling).\n"
			"  -k --refclk   reference clock in MHz. Default 25MHz.\n");
}

int parse_opts(int argc, char *argv[], t_adrf6806_settings *settings)
{

	const struct option lopts[] = {
		{ "tune",     1, 0, 't' },
		{ "int",      1, 0, 'i' },
		{ "frac",     1, 0, 'f' },
		{ "mod",      1, 0, 'm' },
		{ "divider",  1, 0, 'h' },
		{ "vco",      1, 0, 'v' },
		{ "refsel",   1, 0, 's' },
		{ "currmul",  1, 0, 'c' },
		{ "abldly",   1, 0, 'a' },
		{ "cpctrl",   1, 0, 'l' },
		{ "edgesens", 1, 0, 'e' },
		{ "refclk",   1, 0, 'k' },
		{ NULL, 0, 0, 0 },
	};
	int c;


	while (1) {

		c = getopt_long(argc, argv, "t:i:f:m:h:v:s:c:k:", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
			case 't':
				(*settings).tune_freq=atof(optarg);
				break;
			case 'i':
				(*settings).INT=atoi(optarg);
				break;
			case 'f':
				(*settings).FRAC=atoi(optarg);
				(*settings).DIV_MODE=0;
				break;
			case 'm':
				(*settings).MOD=atoi(optarg);
				(*settings).DIV_MODE=0;
				break;
			case 'h':
				(*settings).lo_divider=atoi(optarg);
				break;
			case 'v':
				(*settings).VCO_AMPL=atoi(optarg);
				break;
			case 's':
				(*settings).REF_MUX_SEL=atoi(optarg);
				break;
			case 'c':
				(*settings).currmul_val=atoi(optarg);
				break;
			case 'a':
				(*settings).PFD_ABLDLY=atoi(optarg);
				break;
			case 'l':
				(*settings).CPCTRL=atoi(optarg);
				break;
			case 'e':
				(*settings).PFD_EDGE_SENS=atoi(optarg);
				break;
			case 'k':
				(*settings).ref_in=atof(optarg);
				break;
			default:
				return(-1);
				break;
		}
	}
	return(0);
}

int main(int argc, char* argv[])
{
	int regctr, retval, tmp;
	uint32_t regs[8];
	double frac_divider,ideal_divider,ideal_vco_freq;
	t_spipintriple spipins;
	t_adrf6806_settings settings;
	int automode=0;

	uint8_t writeorder[8] = {6, 5, 3, 4, 7, 1, 2, 0};

	settings.tune_freq=0;
	settings.INT=56;
	settings.DIV_MODE=1;
	settings.MOD=1536;
	settings.FRAC=768;
	settings.DITH_MAG=3;
	settings.DITH_EN=0;
	settings.DITH_RESTART_VAL=1;
	settings.REF_MUX_SEL=0;
	settings.REF_INPUT=1;
	settings.CP_REF=0;
	settings.PFD_Polarity=1;
	settings.PFD_PH_OFFS=10;
	settings.currmul_val=2;
	settings.CP_CURR_MUL=1;
	settings.CP_CTL_SRC=1;
	settings.CPCTRL=3;
	settings.PFD_EDGE_SENS=0;
	settings.PFD_ABLDLY=0;
	settings.DEMOD_BIAS_EN=1;
	settings.LP_EN=0;
	settings.LO_INOUT_CTL=0;
	settings.LO_DRV_EN=1;
	settings.CP_EN=1;
	settings.L3_EN=1;
	settings.LV_EN=1;
	settings.VCO_EN=1;
	settings.VCO_SW=0;
	settings.VCO_AMPL=55;
	settings.VCO_BS_SRC=0;
	settings.VCO_BANDSEL=32;
	settings.DIV_AB=0;
	settings.DIV_SEL=1;
	settings.ODIV_CTL=1;
	settings.lo_divider=8;
	settings.ext_lo_divider=4;


	settings.ref_in=25.0;


	retval = parse_opts(argc, argv, &settings);
	if (retval==-1)
	{
		print_usage(argv[0]);
		return(-2);
	}
	if (settings.tune_freq > 0)
	{
		automode = 1;
	}
	else
	{
		if ((settings.DIV_MODE==0) && ((settings.FRAC==0) || (settings.MOD==0)))
		{
			fprintf(stderr, "If one of --frac or --mod is given, the other one must be given as well for fractional mode.\n");
			return(-1);
		}
		if ((settings.INT<21)||(settings.INT>123))
		{
			fprintf(stderr, "--int must lie between 21 and 123.\n");
			return(-1);
		}
		if ((settings.DIV_MODE==0) && ((settings.FRAC<1)||(settings.FRAC>2047)||(settings.MOD<1)||(settings.MOD>2047)))
		{
			fprintf(stderr, "--frac and --mod must lie between 1 and 2047.\n");
			return(-1);
		}

		if ((settings.lo_divider % 4) != 0)
		{
			fprintf(stderr, "invalid --divider, must be multiple of 4.\n");
			return(-1);
		}
#ifdef DEBUG 
		printf("divider is multiple of 4\n");
#endif
		tmp = settings.lo_divider >> 2;
		if ((tmp < 2) || (tmp > 20))
		{
			fprintf(stderr, "invalid --divider, must lie between 8 and 80.\n");
			return(-1);
		}
#ifdef DEBUG 
		printf("divider is %d times 4\n",tmp);
#endif
		if (tmp < 6)
		{
			settings.DIV_AB=tmp-2;
			settings.DIV_SEL=1;

		}
		else
		{
#ifdef DEBUG
			printf("divider is more than 5 times 4, trying dividing by 8\n");
#endif

			if ((tmp % 1) != 0)
			{
				fprintf(stderr, "invalid --divider, must be multiple of 8.\n");
				return(-1);
			}

			tmp = tmp >> 1;
#ifdef DEBUG
			printf("divider is %d times 8\n",tmp);
#endif
			if (tmp < 6)
			{
				settings.DIV_AB=tmp-2;
				settings.DIV_SEL=2;
			}
			else
			{
#ifdef DEBUG
				printf("divider is more than 5 times 8, trying dividing by 16\n");
#endif
				if ((tmp % 1) != 0)
				{
					fprintf(stderr, "invalid --divider, must be multiple of 16.\n");
					return(-1);
				}

				tmp = tmp >> 1;
#ifdef DEBUG
				printf("divider is %d times 16\n",tmp);
#endif
				if (tmp < 6)
				{
					settings.DIV_AB=tmp-2;
					settings.DIV_SEL=3;
				}
				else
				{
					fprintf(stderr, "invalid --divider.\n");
					return(-1);
				}
			}


		}
	}


	if ((settings.VCO_AMPL<0)||(settings.VCO_AMPL>63))
	{
		fprintf(stderr, "--vco must lie between 0 and 63.\n");
		return(-1);
	}

	if ((settings.REF_MUX_SEL<0)||(settings.REF_MUX_SEL>3))
	{
		fprintf(stderr, "--refsel must lie between 0 and 3.\n");
		return(-1);
	}
	if ((settings.currmul_val<1)||(settings.currmul_val>4))
	{
		fprintf(stderr, "--currmul must lie between 1 and 4.\n");
		return(-1);
	}
	if ((settings.PFD_ABLDLY<0)||(settings.PFD_ABLDLY>3))
	{
		fprintf(stderr, "--abldly must lie between 0 and 3.\n");
		return(-1);
	}
	if ((settings.CPCTRL<0)||(settings.CPCTRL>3))
	{
		fprintf(stderr, "--cpctrl must lie between 0 and 3.\n");
		return(-1);
	}
	if ((settings.PFD_EDGE_SENS<0)||(settings.PFD_EDGE_SENS>3))
	{
		fprintf(stderr, "--edgesens must lie between 0 and 3.\n");
		return(-1);
	}

	settings.pll_ref_div = pow(2,(double)settings.REF_INPUT-1.0);
	settings.pfd_freq = settings.ref_in / settings.pll_ref_div;

	if (automode)
	{
		if (settings.tune_freq < 35)
		{
			fprintf(stderr, "tune frequency too low\n");
			return(-1);
		}
		if ((settings.tune_freq >= 35) && (settings.tune_freq < 43.75))
		{
			settings.DIV_AB = 5-2;
			settings.DIV_SEL = 3;
		}
		if ((settings.tune_freq >= 43.75) && (settings.tune_freq < 58.333))
		{
			settings.DIV_AB = 4-2;
			settings.DIV_SEL = 3;
		}
		if ((settings.tune_freq >= 58.333) && (settings.tune_freq < 70))
		{
			settings.DIV_AB = 3-2;
			settings.DIV_SEL = 3;
		}
		if ((settings.tune_freq >= 70) && (settings.tune_freq < 87.5))
		{
			settings.DIV_AB = 5-2;
			settings.DIV_SEL = 2;
		}
		if ((settings.tune_freq >= 87.5) && (settings.tune_freq < 116.7))
		{
			settings.DIV_AB = 4-2;
			settings.DIV_SEL = 2;
		}
		if ((settings.tune_freq >= 116.7) && (settings.tune_freq < 140))
		{
			settings.DIV_AB = 3-2;
			settings.DIV_SEL = 2;
		}
		if ((settings.tune_freq >= 140) && (settings.tune_freq < 175))
		{
			settings.DIV_AB = 5-2;
			settings.DIV_SEL = 1;
		}
		if ((settings.tune_freq >= 175) && (settings.tune_freq < 233.33))
		{
			settings.DIV_AB = 4-2;
			settings.DIV_SEL = 1;
		}
		if ((settings.tune_freq >= 233.33) && (settings.tune_freq < 350))
		{
			settings.DIV_AB = 3-2;
			settings.DIV_SEL = 1;
		}
		if ((settings.tune_freq >= 350) && (settings.tune_freq <= 525))
		{
			settings.DIV_AB = 2-2;
			settings.DIV_SEL = 1;
		}
		if (settings.tune_freq > 525)
		{
			fprintf(stderr, "tune frequency too high\n");
			return(-1);
		}
		settings.lo_divider = (settings.DIV_AB + 2) * (1 << settings.DIV_SEL) * 2;
		printf("choosing LO divider %d\n", settings.lo_divider);
		ideal_vco_freq = settings.tune_freq * settings.lo_divider; 
		ideal_divider = ideal_vco_freq / 2.0 / settings.pfd_freq;


	}

	signal(SIGINT, INTHandler);

	retval=threewire_init(&spipins);
	if (retval)
	{
		return(-2);
	}

	{
		if (automode)
		{
			settings.DIV_MODE=0;
			settings.INT = (int)floor(ideal_divider);
			frap(fmod(ideal_divider,1.0),2047,&(settings.FRAC),&(settings.MOD));
			if (settings.FRAC==0)
			{
				settings.DIV_MODE=1;
			}

		}
		if(settings.DIV_MODE==0)  //fractional mode
		{
			while (settings.MOD < 40)
			{
				settings.MOD  *= 2;
				settings.FRAC *= 2;
			}
			printf("setting up fractional mode with divider %d %d/%d\n",settings.INT,settings.FRAC,settings.MOD);
			regs[2] = (uint16_t)settings.FRAC;
			regs[1] = (uint16_t)settings.MOD;
			frac_divider = (double)settings.INT + ((double)settings.FRAC/(double)settings.MOD);
		}
		else  //integer mode
		{
			printf("setting up integer mode with divider %d\n",settings.INT);
			regs[2] = 0;
			regs[1] = 0;
			frac_divider = (double)settings.INT;
		}

		settings.vco_freq = frac_divider * 2.0 * settings.pfd_freq;
		settings.lo_freq = settings.vco_freq / settings.lo_divider; 
		settings.lo_out_freq = settings.vco_freq / settings.ext_lo_divider; 

		printf("f_PFD=%f, f_VCO=%f, f_c=%f\n", settings.pfd_freq, settings.vco_freq, settings.lo_freq);
		if ((settings.vco_freq < 2800) || (settings.vco_freq > 4200))
		{
			printf("VCO frequency out of range, PLL might not lock!\n");
		}

		regs[0] = (settings.DIV_MODE ? ADRF6806_FLAG_DIV_MODE : 0)
			|  ADRF6806_BITS_INT_DIV(settings.INT);

		regs[6] = (settings.CP_EN ? ADRF6806_FLAG_CP_EN : 0)
			| (settings.L3_EN ? ADRF6806_FLAG_L3_EN : 0)
			| (settings.LV_EN ? ADRF6806_FLAG_LV_EN : 0)
			| (settings.VCO_EN ? ADRF6806_FLAG_VCO_EN : 0)
			| (settings.VCO_SW ? ADRF6806_FLAG_VCO_SW : 0)
			| ADRF6806_BITS_VCO_AMPL(settings.VCO_AMPL)
			| (settings.VCO_BS_SRC ? ADRF6806_FLAG_VCO_BS_SRC : 0)
			| (settings.L3_EN ? ADRF6806_FLAG_L3_EN : 0)
			| ADRF6806_BITS_VCO_BANDSEL(settings.VCO_BANDSEL);

		settings.CP_CURR_MUL = settings.currmul_val - 1;

		regs[4] = ADRF6806_BITS_REF_MUX_SEL(settings.REF_MUX_SEL)
			| ADRF6806_BITS_INPUT_REF(settings.REF_INPUT)
			| (settings.CP_REF ? ADRF6806_FLAG_CP_REF : 0)
			| (settings.PFD_Polarity ? ADRF6806_FLAG_PFD_POL : 0)
			| ADRF6806_BITS_PFD_PH_OFFS(settings.PFD_PH_OFFS)
			| ADRF6806_BITS_CP_CURRMUL(settings.CP_CURR_MUL)
			| (settings.CP_CTL_SRC ? ADRF6806_FLAG_CP_CTL_SRC : 0)
			| ADRF6806_BITS_CP_CTL(settings.CPCTRL)
			| ADRF6806_BITS_PFD_EDGE_SENS(settings.PFD_EDGE_SENS)
			| ADRF6806_BITS_PFD_ABD(settings.PFD_ABLDLY);



		regs[7] = ADRF6806_BITS_DIV_AB(settings.DIV_AB)
			| ADRF6806_BITS_DIV_SEL(settings.DIV_SEL)
			| ADRF6806_BITS_ODIV_CTL(settings.ODIV_CTL);

		regs[5] = (settings.DEMOD_BIAS_EN ? ADRF6806_FLAG_DEMOD_BIAS_EN : 0)
			| (settings.LP_EN ? ADRF6806_FLAG_LP_EN : 0)
			| (settings.LO_INOUT_CTL ? ADRF6806_FLAG_LO_INOUT_CTL : 0)
			| (settings.LO_DRV_EN ? ADRF6806_FLAG_LO_DRV_EN : 0);
		if (settings.LO_DRV_EN && (settings.LO_INOUT_CTL==0))
		{
			printf("LO output enabled, freq %f\n",settings.lo_out_freq);
		}
		else
			printf("LO output disabled\n");

		regs[3] = (settings.DITH_EN ? ADRF6806_FLAG_DITH_EN : 0 )
			|  ADRF6806_BITS_DITH_MAG(settings.DITH_MAG)
			|  ADRF6806_BITS_DITH_RESTART(settings.DITH_RESTART_VAL);


		for (regctr=0; regctr < 8;regctr++)
		{
#ifdef DEBUG 
			printf("writing %x=%x\n", writeorder[regctr], regs[writeorder[regctr]]);
#endif
			threewire_writeraw24(spipins, writeorder[regctr] | regs[writeorder[regctr]]);
		}
		usleep(10);

		finish=0;
	}
	threewire_close(spipins);
	return(0);
}
