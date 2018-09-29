/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file i2c.h
*   \brief I2C master API
*   \author Montage
*/
void i2c_init(void);
void i2c_finish(void);
void i2c_start(unsigned char addr, unsigned char is_read);
//void i2c_stop(void);
int i2c_write(unsigned char byte, unsigned char is_stop);
void i2c_read(unsigned int *out, unsigned char reg, unsigned char is_stop);

//int i2c_read(unsigned char byte, unsigned char is_stop);
