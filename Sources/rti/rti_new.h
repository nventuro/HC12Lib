

struct rti_crit {
	void (*f)(void *, rti_time);
	void *data;
	rti_time period;
	rti_time delay0;
	rti_time fase_reload;
};

struct rti_crit_rtd {
	rti_time cnt;
	rti_time fase_cnt;
};

struct rti_soft {
	rti_time (*f)(void *data, rti_time pw, rti_time fase);
	void *data;
}

struct rti_soft_rtd {
	rti_time delay0;
	rti_time fase_reload;
	rti_time period;
	rti_time cnt;		// Cada cuánto llamar
	rti_time fase_cnt;	// Dónde estoy
}

//fase de col_gen: dice en qué fila del teclado estoy/ qué display tengo que prender
//fase de display: sirve para dimming (par o impar, el display tiene 2 niveles nada más), y para el blink.

const struct rti_crit rti_crit_tbl[] = {
#define COL_GEN_ID 0
/* 0 */	{col_gen, NULL, RTI_ALWAYS, RTI_NOW, N_COLS},
/* 1 */ {display_srv, NULL, RTI_ALWAYS, RTI_NOW, DISP_FASOR},
/* 2 */ {led_srv, NULL, RTI_ALWAYS, RTI_NOW, DISP_FASOR},
/* 3 */ {kbd_srv, NULL, RTI_HALF, RTI_NOW, DISP_FASOR},
}