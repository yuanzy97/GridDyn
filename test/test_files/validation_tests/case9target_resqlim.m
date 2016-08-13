function mpc = case9target_resqlim
%CASE9TARGET_RESQLIM

%% MATPOWER Case Format : Version 2
mpc.version = '2';

%%-----  Power Flow Data  -----%%
%% system MVA base
mpc.baseMVA = 100;

%% bus data
%	bus_i	type	Pd	Qd	Gs	Bs	area	Vm	Va	baseKV	zone	Vmax	Vmin
mpc.bus = [
	1	1	0	0	0	0	1	1.33762913	0	345	1	1.1	0.9;
	2	3	0	0	0	0	1	1	-153.809495	345	1	1.1	0.9;
	3	2	0	0	0	0	1	1	-188.435664	345	1	1.1	0.9;
	4	1	0	0	0	0	1	1.18204226	-214.29155	345	1	1.1	0.9;
	5	1	305.4	123.15	0	0	1	2.23762151	-118.886748	345	1	1.1	0.9;
	6	1	0	0	0	0	1	1.86595084	-174.95912	345	1	1.1	0.9;
	7	1	214.11	71.09	0	0	1	1.21061398	-269.027254	345	1	1.1	0.9;
	8	1	0	0	0	0	1	0.305385218	-30.5882403	345	1	1.1	0.9;
	9	1	235.61	81.44	0	0	1	0.0849011288	-53.1652667	345	1	1.1	0.9;
];

%% generator data
%	bus	Pg	Qg	Qmax	Qmin	Vg	mBase	status	Pmax	Pmin	Pc1	Pc2	Qc1min	Qc1max	Qc2min	Qc2max	ramp_agc	ramp_10	ramp_30	ramp_q	apf
mpc.gen = [
	1	436.149762	300	300	-300	1	100	1	250	10	0	0	0	0	0	0	0	0	0	0	0;
	2	-408.757443	1867.7	300	-300	1	100	1	300	10	0	0	0	0	0	0	0	0	0	0	0;
	3	124.59	-1390.05571	300	-300	1	100	1	270	10	0	0	0	0	0	0	0	0	0	0	0;
];

%% branch data
%	fbus	tbus	r	x	b	rateA	rateB	rateC	ratio	angle	status	angmin	angmax	Pf	Qf	Pt	Qt
mpc.branch = [
	1	4	0	0.0576	0	250	250	250	0	0	1	-360	360	-1546.5584	5374.2280	1546.5584	4693.6242;
	4	5	0.017	0.092	0.158	250	250	250	0	0	1	-360	360	-2447.9227	2230.8147	3788.5089	4973.5291;
	5	6	0.039	0.17	0.358	150	150	150	0	0	1	-360	360	2279.1365	961.9447	-1788.3983	1025.2225;
	3	6	0	0.0586	0	300	300	300	0	0	1	-360	360	-742.0729	-1390.0557	742.0729	2845.0510;
	6	7	0.0119	0.1008	0.209	150	150	150	0	0	1	-360	360	2625.3313	3266.8038	-2016.8462	1835.7229;
	7	8	0.0085	0.072	0.149	250	250	250	0	0	1	-360	360	699.8068	2210.7587	-385.1380	443.0578;
	8	2	0	0.0625	0	250	250	250	0	0	1	-360	360	408.7574	416.9162	-408.7574	1867.7000;
	8	9	0.032	0.161	0.306	250	250	250	0	0	1	-360	360	14.1802	38.8103	-7.9349	-8.9259;
	9	4	0.01	0.085	0.176	250	250	250	0	0	1	-360	360	51.6192	114.0627	166.0400	1723.6819;
];

%%-----  OPF Data  -----%%
%% generator cost data
%	1	startup	shutdown	n	x1	y1	...	xn	yn
%	2	startup	shutdown	n	c(n-1)	...	c0
mpc.gencost = [
	2	1500	0	3	0.11	5	150;
	2	2000	0	3	0.085	1.2	600;
	2	3000	0	3	0.1225	1	335;
];