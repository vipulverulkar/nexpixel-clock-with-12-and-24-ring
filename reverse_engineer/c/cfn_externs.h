extern cfn_main();
extern cfn_initworld();
extern cfn_receive();
extern cfn_set_alarm();
extern cfn_send2sock();
extern cfn_sendidpdata();
extern cfn_error();
extern char *  cfn_fmt_txt_pkt();
extern uchar   cfn_cksum();
extern int     cfn_ascii_to_hex();
extern int     cfn_gethash();

extern cfn_tcreate();
extern cfn_update();
extern BD *    cfn_str_to_bd();
extern cfn_panic();
extern cfn_panic2();
extern cfn_dumptable();
extern cfn_print1_tentry();
extern cfn_tableinit();
extern cfn_reboot();
extern PSPPKT  cx_psp_packet();
extern int     cx_atoh_n();
extern uchar   cx_psp_cksum();

extern cx_init();
extern cx_main();
extern cx_listenmsg();
extern cx_connectmsg();
extern cx_disconnectmsg();
extern cx_IDPdatamsg();
extern cx_printpkt();
extern BOOL    cx_xr_packet();
extern BOOL    cfn_31();
extern BOOL    cx_xr_psp_valid();
extern cfn_33();
extern cx_psp_reply();

extern cx_SAdatamsg();
extern BOOL    cfn_36();
extern cx_37();
extern BOOL    cfn_38();
extern BOOL    cfn_39();
extern BOOL    cfn_40();
extern BOOL    cfn_41();
extern char *  cfn_42();
extern BOOL    cx_43();
extern short   cx_countbytes();
extern cx_fill_ff();
