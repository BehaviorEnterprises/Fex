
/* set */
{ "threshold", CONF_SET,
	{ C_TYPE_LF, C_TYPE__, C_TYPE__, C_TYPE__, C_TYPE__ },
	{ &conf.thresh, NULL, NULL, NULL, NULL}, },
{ "floor", CONF_SET,
	{ C_TYPE_LF, C_TYPE__, C_TYPE__, C_TYPE__, C_TYPE__ },
	{ &conf.spect_floor, NULL, NULL, NULL, NULL}, },
{ "samples", CONF_SET,
	{ C_TYPE_D, C_TYPE__, C_TYPE__, C_TYPE__, C_TYPE__ },
	{ &conf.winlen, NULL, NULL, NULL, NULL}, },
{ "hop", CONF_SET,
	{ C_TYPE_D, C_TYPE__, C_TYPE__, C_TYPE__, C_TYPE__ },
	{ &conf.hop, NULL, NULL, NULL, NULL}, },
{ "bandpass", CONF_SET,
	{ C_TYPE_LF, C_TYPE_LF, C_TYPE__, C_TYPE__, C_TYPE__ },
	{ &conf.hipass, &conf.lopass, NULL, NULL, NULL}, },
{ "scale", CONF_SET,
	{ C_TYPE_D, C_TYPE_LF, C_TYPE__, C_TYPE__, C_TYPE__ },
	{ &conf.scale, NULL, NULL, NULL, NULL}, },
{ "window", CONF_SET,
	{ C_TYPE_S, C_TYPE_LF, C_TYPE_LF, C_TYPE_LF, C_TYPE_LF },
	{ window, &custom.a[0], &custom.a[1], &custom.a[2], &custom.a[3]}, },
{ "fontFamily", CONF_SET,
	{ C_TYPE_LN, C_TYPE__, C_TYPE__, C_TYPE__, C_TYPE__ },
	{ font_fam, NULL, NULL, NULL, NULL}, },
{ "fontSize", CONF_SET,
	{ C_TYPE_D, C_TYPE__, C_TYPE__, C_TYPE__, C_TYPE__ },
	{ &conf.font_size, NULL, NULL, NULL, NULL}, },
{ "help", CONF_SET,
	{ C_TYPE_LN, C_TYPE__, C_TYPE__, C_TYPE__, C_TYPE__ },
	{ &help_cmd, NULL, NULL, NULL, NULL}, },

/* color */
{ "spectrogram", CONF_COL,
	{ C_TYPE_LF, C_TYPE_LF, C_TYPE_LF, C_TYPE_LF, C_TYPE_LF },
	{ &conf.col[0].r, &conf.col[0].g, &conf.col[0].b, &conf.col[0].a,
			&conf.col[0].w } },
{ "threshold", CONF_COL,
	{ C_TYPE_LF, C_TYPE_LF, C_TYPE_LF, C_TYPE_LF, C_TYPE_LF },
	{ &conf.col[1].r, &conf.col[1].g, &conf.col[1].b, &conf.col[1].a,
			&conf.col[1].w } },
{ "points", CONF_COL,
	{ C_TYPE_LF, C_TYPE_LF, C_TYPE_LF, C_TYPE_LF, C_TYPE_LF },
	{ &conf.col[2].r, &conf.col[2].g, &conf.col[2].b, &conf.col[2].a,
			&conf.col[2].w } },
{ "lines", CONF_COL,
	{ C_TYPE_LF, C_TYPE_LF, C_TYPE_LF, C_TYPE_LF, C_TYPE_LF },
	{ &conf.col[3].r, &conf.col[3].g, &conf.col[3].b, &conf.col[3].a,
			&conf.col[3].w } },
{ "eraser1", CONF_COL,
	{ C_TYPE_LF, C_TYPE_LF, C_TYPE_LF, C_TYPE_LF, C_TYPE_LF },
	{ &conf.col[4].r, &conf.col[4].g, &conf.col[4].b, &conf.col[4].a,
			&conf.col[4].w } },
{ "eraser2", CONF_COL,
	{ C_TYPE_LF, C_TYPE_LF, C_TYPE_LF, C_TYPE_LF, C_TYPE_LF },
	{ &conf.col[5].r, &conf.col[5].g, &conf.col[5].b, &conf.col[5].a,
			&conf.col[5].w } },
{ "crop", CONF_COL,
	{ C_TYPE_LF, C_TYPE_LF, C_TYPE_LF, C_TYPE_LF, C_TYPE_LF },
	{ &conf.col[6].r, &conf.col[6].g, &conf.col[6].b, &conf.col[6].a,
			&conf.col[6].w } },

