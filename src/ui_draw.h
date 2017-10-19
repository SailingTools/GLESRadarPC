/* Global Scan Buffer (2048 angles of 256 resolution each
 * OR 512 data_packets of 1024 bytes each) 
 * BUT for some reason the angles are counted up to 2046*/
unsigned char global_scan_buffer[2048*256];
int current_angle;
int scanning;
int first_draw;
