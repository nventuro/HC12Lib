/*
 * graphics.c
 * 
 * Control de LED
 * Lab de micros - 2012 - Grupo X
 *
 */
 
 #include <string.h>
 #include "graphics.h"
 
 struct blink_msg_data
 {
 	u8 done;
 	uint idx;
 	timer_id func_id;
 };
 
 struct blink_msg_data data;
 
 void set_global_blink(int blink);
 rti_time move_dim (void* _data, rti_time pw);
 rti_time grapics_stop_stage(void* _data, rti_time pw);
 
 void disp_blinking_message (char *msg, rti_time dim_dur, rti_time blink_dur, rti_time dim_rate, rti_time blink_rate) 
 {
 	int i;
 	rti_time orig_blink_rate;
 	
 	strncpy(disp_ram,msg,DISP_SIZE);	
 		
 	data.done = 0;
 	 
 	set_global_blink(DISP_NO_BLINK);
 	data.idx = 0;
 	data.func_id = rti_register(move_dim,NULL,dim_rate,RTI_NOW);
 	rti_register(grapics_stop_stage,NULL,RTI_ONCE,dim_dur);
 	
 	while (!data.done)
 		;
 		
 	data.done = 0;
 	
 	for (i = 0; i < DISP_SIZE; i++)
 		disp_att_ram[i].dim = DISP_FULL_DIM;
 		
 	orig_blink_rate = disp_get_blink_rate();
 	
 	disp_set_blink_rate(blink_rate);
 	set_global_blink(DISP_BLINK);
 	
 	data.func_id = RTI_INVALID_ID;
 	rti_register(grapics_stop_stage,NULL,RTI_ONCE,blink_dur);
 	
 	while (!data.done)
 		;
 	
 	set_global_blink(DISP_NO_BLINK);
 	disp_set_blink_rate(orig_blink_rate);
 
 	return; 	
 }
 
 rti_time move_dim (void* _data, rti_time pw)
 {
 	int i;
 	
 	for (i = 0; i < DISP_SIZE; i++)
 		if (i == data.idx)
 			disp_att_ram[i].dim = DISP_FULL_DIM;
 		else
 			disp_att_ram[i].dim = DISP_HALF_DIM;
 	
 	data.idx++;
 	data.idx %= DISP_SIZE;

 	return pw;
 }
 
 
 rti_time grapics_stop_stage(void* _data, rti_time pw)
 {
 	data.done = 1;
 	if (data.func_id != RTI_INVALID_ID)
 		rti_cancel(data.func_id);
 	return pw;
 }
 	
 void set_global_blink(int blink)
 {
 	int i;
 	for (i = 0; i < DISP_SIZE; i++)
 		disp_att_ram[i].blink = blink;
 }