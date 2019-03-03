#include "HalfDuplexSimpleSerial.h"
#include "GPSSmartModule.h"
#include "gps_type_define.h"

/* connect the GPS Smart Module to Arduino UNO pin 12, and set the baud rate to 9600 bps */
#define         GPS_SIO                    (12)
#define         GPS_BAUDRATE               (9600)

// global variants definition
static uint8_t set_point_flag = 0; // Set Point Flag, indicates the original position(after power up,the GPS get the current position) 
static uint8_t signal_valid_flag = 0; // signal valid 

// types are re-defined for the following variants, please find the infomation in file "gps_type_define.h".
utc_t utc;// UTC time, union, hour, minute, second
date_t date; // date, union, day, month, year
version_t ver;// version info, union, hardware ver, firmware ver
satellite_t satellites; // satellites, 8-bit val
latitude_t latitude; // latitude, union, degree, minute, second(the module will return degree(8-bit), minute(8-bit), mill-minute(16-bit))
longitude_t longitude;// longitude, union, degree, minute, second(the module will return degree(8-bit), minute(8-bit), mill-minute(16-bit))
altitude_t altitude;// altitude, union, 16-bit value, -3276.8~3276.7 meters
speed_knot_t speed_knot;// speed, union, 16-bit value, 0~6553.5 knots
heading_t heading; // direction of travel to North, union, 16-bit value, 0~359.9 degree
distance_t distance; // distance between two set points, 32-bit value, 0~4294967294 meters(4294967295 == -1, error occured)
speed_average_t speed_average; // average speed between two set points,, 32-bit value, 0~4294967294 m/s(4294967295 == -1, error occured)


/*
Function      : gps_send_cmd
Parameter     : command_basic, command_sub
Return        : none
Description   : Send a command packet to the GPS module require data
*/
void gps_send_cmd(uint8_t cmd_b, uint8_t cmd_s)
{  
  HDSS_write(0xA5);
  HDSS_write(cmd_b);
  HDSS_write(cmd_s);
  HDSS_write(0x5A);
}


/*
Function      : dec0_to_string
Parameter     : The value need to be converted, how many bits be display(count from the end), result save at                
Return        : none
Description   : Covert a dec value to string
                examples:
                dec0_to_string(12345, 4, pstr) ==> will store the string to pstr "2345".
                dec0_to_string(12345, 7, pstr) ==> will store the string to pstr "0012345".
                dec0_to_string(-12345, 4, pstr) ==> will store the string to pstr "-2345".
                dec0_to_string(-12345, 7, pstr) ==> will store the string to pstr "-0012345".
*/

void dec0_to_string(int32_t dec_value, uint8_t bits, int8_t * str_buffer)
{
    uint8_t j;
    uint8_t bits_ = bits;
    int32_t i;
    // For a 32-bit int, 11 bits will be maximum.
    if(bits < 11)
    {
        if (dec_value < 0)
        {
            dec_value = 0 - dec_value;
            *str_buffer = '-';
            str_buffer++;    
        }
        
        i = 1;
        while((--bits)>0)
        {i *=10;}
        dec_value %= (i*10);        
        
        for(j = 0; j < bits_;  j++)
        {
            *str_buffer = ((dec_value / i) + '0');
            dec_value %= i;                 
            str_buffer++;     
            i /= 10;
        }
    }
    // put the string ended indicator to the end.
    *str_buffer = '\0'; 
}



/*
Function      : setup
Parameter     : none
Return        : none
Description   : initialize the arduino and gps connected io, start the Half Duplex Simple Serial driver
                initialize the basic infomation display on Parallax Serial Terminal
*/

void setup(void)
{
  /* initialize the serial drivers */
  //TODO : Replace with RS-232 Serial Comunication initialization
  Serial.begin(SERIAL_MONITOR_BR);
  HDSS_start(GPS_SIO, 0, GPS_BAUDRATE);
}

/*
Function      : get_gps_info
Parameter     : none
Return        : none
Description   : get data from gps module and display on Parallax Serial Terminal
*/

uint32_t get_gps_info(void)
{

    uint8_t i=0;
    uint8_t temp8;   // 8-bit temperate
    uint16_t temp16; // 16-bit temperate
    uint32_t temp32; // 32-bit temperate
    uint32_t rxdata =0; // rxdata from module (32-bit), when error occur, will get 32-bit value, otherwise will get a 8-bit value(LSB)
    int8_t convert_buff[16];
    
    /* Get the Hardware and Software version */
    gps_send_cmd(CMD_INFO_BASIC, CMD_GET_INFO);
    // The module will return 2 bytes, after received the command
    for(i = 0; i < 2; i++)
    {
        rxdata = HDSS_read_time(8);
        if(rxdata == ERROR_RX_TIMEOUT)
        {
            return ERROR_RX_TIMEOUT;
        }
        ver.buff[i] = rxdata;
    }
    /* update the Hardware and Firmware version */
    PST_SET_X_Y(18, HARDWARE_INFO_AT_ROW);
    Serial.print(ver.hardware_ver >> 4, HEX);
    Serial.print('.');
    Serial.print(ver.hardware_ver & 0x0F, HEX);
    PST_CREOL();

    PST_SET_X_Y(18, FIRMWARE_INFO_AT_ROW);
    Serial.print(ver.firmware_ver >> 4, HEX);
    Serial.print('.');
    Serial.print(ver.firmware_ver & 0x0F, HEX);
    PST_CREOL();

    /* Get status, signal valid or not */
    gps_send_cmd(CMD_INFO_BASIC, CMD_GET_STATUS);
    // The module will return 1 bytes, after received the command
    rxdata = HDSS_read_time(8);
    if(rxdata == ERROR_RX_TIMEOUT)
    {
        return ERROR_RX_TIMEOUT;
    }
    signal_valid_flag = rxdata;

    PST_SET_X_Y(18,  VALID_INFO_AT_ROW);
    if (signal_valid_flag == 0)
    {
        Serial.print("No ");
    }
    else
    {
        Serial.print("Yes");
        // If signal valid, and not set the orignal point yet, then save current information.
        if(set_point_flag == 0)
        {
            set_point_flag = 1;
            gps_send_cmd(CMD_SET_POINT_BASIC, 0x01);  // Set the point 1, save the current gps position to RAM
            // The module will return 0 bytes, after received the command
            // Wait for the previous command done.
            delay(10);
            // Start data log.
            gps_send_cmd(CMD_DATALOGGER_BASIC, CMD_START_NEW_LOG);
            // The module will return 0 bytes, after received the command
            // Wait for the previous command done.
            delay(10);
        }
    }
    PST_CREOL();

    /* Get current visible satellites */
    gps_send_cmd(CMD_INFO_BASIC, CMD_GET_SATELLITES);
    // The module will return 1 bytes, after received the command
    rxdata = HDSS_read_time(8);
    if(rxdata == ERROR_RX_TIMEOUT)
    {
        return ERROR_RX_TIMEOUT;
    }
    satellites = rxdata;

    PST_SET_X_Y(18,  SATELLITES_INFO_AT_ROW);
    dec0_to_string(satellites, 2, convert_buff);
    Serial.print((char *)convert_buff);
    PST_CREOL();


    /* Get UTC time */
    gps_send_cmd(CMD_INFO_BASIC, CMD_GET_TIME);
    // The module will return 3 bytes, after received the command
    // Hour, Minute, Second
    for(i = 0; i < 3; i++)
    {
        rxdata = HDSS_read_time(8);
        if(rxdata == ERROR_RX_TIMEOUT)
        {
            return ERROR_RX_TIMEOUT;
        }
        utc.buff[i] = rxdata;
    }
    PST_SET_X_Y(18,  TIME_INFO_AT_ROW);
    dec0_to_string(utc.hour, 2, convert_buff);
    Serial.print((char *)convert_buff);
    Serial.print(':');
    dec0_to_string(utc.minute, 2, convert_buff);
    Serial.print((char *)convert_buff);
    Serial.print(':');
    dec0_to_string(utc.second, 2, convert_buff);
    Serial.print((char *)convert_buff);
    PST_CREOL();

    /* Get Date */
    PST_SET_X_Y(18, DATE_INFO_AT_ROW );
    gps_send_cmd(CMD_INFO_BASIC, CMD_GET_DATE);
    // The module will return 3 bytes, after received the command
    // Day, Month, Year(00~99)
    for(i = 0; i < 3; i++)
    {
        rxdata = HDSS_read_time(8);
        if(rxdata == ERROR_RX_TIMEOUT)
        {
            return ERROR_RX_TIMEOUT;
        }
        date.buff[i] = rxdata;
    }
    PST_SET_X_Y(18,  DATE_INFO_AT_ROW);
    dec0_to_string(date.day, 2, convert_buff);
    Serial.print((char *)convert_buff);
    Serial.print('/');
    dec0_to_string(date.month, 2, convert_buff);
    Serial.print((char *)convert_buff);
    Serial.print("/20");
    dec0_to_string(date.year, 2, convert_buff);
    Serial.print((char *)convert_buff);
    PST_CREOL();

    /* Get Latitude */
    gps_send_cmd(CMD_INFO_BASIC, CMD_GET_LATITUDE);
    // The module will return 5 bytes, after received the command
    // Degree, minute, second(16-bit, MSB first), N/S
    for(i = 0; i < 5; i++)
    {
        rxdata = HDSS_read_time(8);
        if(rxdata == ERROR_RX_TIMEOUT)
        {
            return ERROR_RX_TIMEOUT;
        }
        latitude.buff[i] = rxdata;
    }
    PST_SET_X_Y(18, LATITUDE_INFO_AT_ROW);
    dec0_to_string(latitude.degree, 2, convert_buff);
    Serial.print((char *)convert_buff);
    Serial.print('`');
    dec0_to_string(latitude.minute, 2, convert_buff);
    Serial.print((char *)convert_buff);
    Serial.print('\'');
    temp16 = latitude.buff[2] << 8 | latitude.buff[3];
    latitude.second = temp16 * 6 / 10;
    dec0_to_string(latitude.second / 100, 2, convert_buff);
    Serial.print((char *)convert_buff);
    Serial.print(".");
    dec0_to_string(latitude.second % 100  , 2, convert_buff);
    Serial.print((char *)convert_buff);
    if(latitude.indicator == 0)
    {
        Serial.print("\" N");
    }
    else
    {
        Serial.print("\" S");
    }
    PST_CREOL();

    /* Get Longitude */
    gps_send_cmd(CMD_INFO_BASIC, CMD_GET_LONGITUDE);
    // The module will return 5 bytes, after received the command
    // Degree, minute, second(16-bit, MSB first), E/W
    
    for(i = 0; i < 5; i++)
    {
        rxdata = HDSS_read_time(8);
        if(rxdata == ERROR_RX_TIMEOUT)
        {
            return ERROR_RX_TIMEOUT;
        }
        longitude.buff[i] = rxdata;
    }
    PST_SET_X_Y(18, LONGITUDE_INFO_AT_ROW);
    dec0_to_string(longitude.degree, 3, convert_buff);
    Serial.print((char *)convert_buff);
    Serial.print('`');
    dec0_to_string(longitude.minute, 2, convert_buff);
    Serial.print((char *)convert_buff);
    Serial.print('\'');
    temp16 = longitude.buff[2] << 8 | longitude.buff[3];
    longitude.second = temp16 * 6 / 10;
    dec0_to_string(longitude.second / 100, 2, convert_buff);
    Serial.print((char *)convert_buff);
    Serial.print(".");
    dec0_to_string(longitude.second % 100, 2, convert_buff);
    Serial.print((char *)convert_buff);
    if(longitude.indicator == 0)
    {
        Serial.print("\" E");
    }
    else
    {
        Serial.print("\" W");
    }
    PST_CREOL();

    /* Get Altitude*/
    gps_send_cmd(CMD_INFO_BASIC, CMD_GET_ALTITUDE);
    // The module will return 2 bytes, after received the command
    // 16-bit value, (altitude x 10), 0.0 ~ 6553.5 meter   
    for(i = 0; i < 2; i++)
    {
        rxdata = HDSS_read_time(8);
        if(rxdata == ERROR_RX_TIMEOUT)
        {
            return ERROR_RX_TIMEOUT;
        }
        altitude.buff[i] = rxdata;
    }
    PST_SET_X_Y(18, ALTITUDE_INFO_AT_ROW);
    temp16 = (altitude.buff[0] << 8 | altitude.buff[1]);
    altitude.altitude = temp16;
    if(altitude.altitude < 0)
    {
        Serial.print("-");
        temp16 = 0 - temp16;
    }
    dec0_to_string(temp16 / 10, 4, convert_buff);
    Serial.print((char *)convert_buff);
    Serial.write('.');
    Serial.write((char)((temp16 % 10) + '0'));
    Serial.print(" meters");
    PST_CREOL();

    gps_send_cmd(CMD_INFO_BASIC, CMD_GET_SPEED_KNOT);
    // The module will return 2 bytes, after received the command   
    // 16-bit value, (speed x 10), 0.0 ~ 6553.5 knot(s)    
    for(i = 0; i < 2; i++)
    {
        rxdata = HDSS_read_time(8);
        if(rxdata == ERROR_RX_TIMEOUT)
        {
            return ERROR_RX_TIMEOUT;
        }
        speed_knot.buff[i] = rxdata;
    }
    PST_SET_X_Y(18,  SPEED_INFO_AT_ROW);
    temp16 = (speed_knot.buff[0] << 8 | speed_knot.buff[1]);
    speed_knot.speed = temp16;
    dec0_to_string(speed_knot.speed / 10, 4, convert_buff);
    Serial.print((char *)convert_buff);
    Serial.write('.');
    Serial.write((char)((speed_knot.speed % 10) + '0'));
    Serial.print(" knots");
    PST_CREOL();

    /* If the Speed is 0, then Direction of Travel will not available */
    PST_SET_X_Y(18, HEADING_INFO_AT_ROW);
    if (speed_knot.speed != 0)
    {
        gps_send_cmd(CMD_INFO_BASIC, CMD_GET_HEADING);
      // The module will return 2 bytes, after received the command   
      // 16-bit value, (degree x 10), 0.0 ~ 359.9 knot(s)           
        for(i = 0; i < 2; i++)
        {
            rxdata = HDSS_read_time(8);
            if(rxdata == ERROR_RX_TIMEOUT)
            {
                return ERROR_RX_TIMEOUT;
            }
            heading.buff[i] = rxdata;
        }
        temp16 = (heading.buff[0] << 8 | heading.buff[1]) % 10000;
        heading.degree = temp16;
        dec0_to_string(heading.degree / 10, 3, convert_buff);
        Serial.print((char *)convert_buff);
        Serial.print('.');
        Serial.print((char)heading.degree % 10 + '0');
        Serial.print(" `");
    }
    else
    {
        Serial.print("N/A");
    }
    PST_CREOL();


    /* Set current position to point 0, and get the distance between original position(Point 1) */
    if(set_point_flag == 1)
    {
        gps_send_cmd(CMD_SET_POINT_BASIC, 0x00);// Set the point 0, save the current gps position to RAM
        // The module will return 1 bytes, after received the command
        // Wait for the previous command done.
        rxdata = HDSS_read_time(8);
        if(rxdata == ERROR_RX_TIMEOUT)
        {
                return ERROR_RX_TIMEOUT;
        }
        
        gps_send_cmd(CMD_GET_DISTANCE_BASIC, 0x01);
        // The module will return 4 bytes, after received the command
        for(i = 0; i < 4; i++)
        {
            rxdata = HDSS_read_time(8);
            if(rxdata == ERROR_RX_TIMEOUT)
            {
                return ERROR_RX_TIMEOUT;
            }
            distance.buff[i] = rxdata;
        }
        PST_SET_X_Y(18,  DISTANCE_INFO_AT_ROW);
        temp32 = ((uint32_t)speed_average.buff[0] << 24 | (uint32_t)speed_average.buff[1] << 16 | (uint32_t)speed_average.buff[2] << 8 | (uint32_t)speed_average.buff[3]);
        distance.distance = temp32;
        Serial.print((int32_t)distance.distance);
        Serial.print(" meters");
        PST_CREOL();

        gps_send_cmd(CMD_GET_SPEED_AVER_BASIC, 0x01);        
        // The module will return 4 bytes, after received the command
        for(i = 0; i < 4; i++)
        {
            rxdata = HDSS_read_time(8);
            if(rxdata == ERROR_RX_TIMEOUT)
            {
                return ERROR_RX_TIMEOUT;
            }
            speed_average.buff[i] = rxdata;
        }
        PST_SET_X_Y(18,  SPEED_AVER_INFO_AT_ROW);
        temp32 = ((uint32_t)speed_average.buff[0] << 24 | (uint32_t)speed_average.buff[1] << 16 | (uint32_t)speed_average.buff[2] << 8 | (uint32_t)speed_average.buff[3]);
        speed_average.speed = temp32;
        Serial.print((int32_t)speed_average.speed);
        Serial.print(" m/s");
        PST_CREOL();
    }
    // no error occured, return 0
    return 0;
}
