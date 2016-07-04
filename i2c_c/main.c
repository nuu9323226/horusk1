/****************************************************/

//**main.c

/****************************************************/

#include <stdio.h>

#include <linux/types.h>

#include <stdlib.h>

#include <fcntl.h>

#include <unistd.h>

#include <sys/types.h>

#include <sys/ioctl.h>

#include <errno.h>

#include "delay.h"

 

#define I2C_RETRIES 0x0701

#define I2C_TIMEOUT 0x0702

#define I2C_RDWR 0x0707

 

#include <string.h>

 

#define STORAGE_DEV (0xA8 >> 1) /*Slave Device Address*/

 

struct i2c_msg {

    unsigned short addr;

    unsigned short flags;

    #define I2C_M_TEN 0x0010

    #define I2C_M_RD 0x0001

    unsigned short len;

    unsigned char *buf;

};

 

struct i2c_rdwr_ioctl_data{

    struct i2c_msg *msgs;

    int nmsgs;

};

 

struct i2c_rdwr_ioctl_data storage_data;

struct i2c_msg storage_msg[2];

 

 

int i2c_write(int fd, unsigned char slvAddr, unsigned char index, unsigned char * const data, unsigned char len )

{

        int ret;

        unsigned char *tmp = (unsigned char*) malloc(1024);

 

        if (tmp == NULL) {

            perror("memory allocate error(write)!!");

            return -1;

        }

 

        memcpy(tmp, (unsigned char *)&index, sizeof(unsigned char));

        memcpy((tmp + sizeof(unsigned char)), data, len);

       len += sizeof(unsigned char);

 

        /***write data to storage**/
/*
        storage_data.nmsgs = 1;

        (storage_data.msgs[0]).len = len; // Data length

        (storage_data.msgs[0]).addr = slvAddr;//Device Addr

        (storage_data.msgs[0]).flags = 0; //write

        (storage_data.msgs[0]).buf = tmp;

        ret = ioctl(fd, I2C_RDWR, &storage_data);
*/
        storage_data.nmsgs = 1;

        (storage_data.msgs[0]).len = len;

        (storage_data.msgs[0]).addr = slvAddr;

        (storage_data.msgs[0]).flags = 0;  //Dummy write

        (storage_data.msgs[0]).buf = tmp;

       

        (storage_data.msgs[1]).len = len;

        (storage_data.msgs[1]).addr = slvAddr;

        (storage_data.msgs[1]).flags = 1;

        (storage_data.msgs[1]).buf = data;

        ret = ioctl(fd, I2C_RDWR, &storage_data);

        if (tmp)

          free(tmp);

 

        return (ret < 0) ? -1 : 0;

}

 

int i2c_read(int fd, unsigned char slvAddr, unsigned short index, unsigned char *data, int len)

{

        storage_data.nmsgs = 2;

        (storage_data.msgs[0]).len = sizeof(unsigned short);

        (storage_data.msgs[0]).addr = slvAddr;

        (storage_data.msgs[0]).flags = 0;  //Dummy write

        (storage_data.msgs[0]).buf = (unsigned char *)&index;

       

        (storage_data.msgs[1]).len = len;

        (storage_data.msgs[1]).addr = slvAddr;

        (storage_data.msgs[1]).flags = 1;

        (storage_data.msgs[1]).buf = data;

 

        if(ioctl(fd,I2C_RDWR, &storage_data) < 0){

            perror("ioctl error");

                return -1;

        }

 

        return 0;

}

 

int

i2c_transmit_data(int fd, unsigned char *data_ptr, unsigned short start_word_address ,unsigned short write_data_length)

{

        int i, ret = 0;

        unsigned char write_data_buffer[1024];      

        unsigned short transmit_legth = 16;

 

        memset(write_data_buffer, 0, 1024);

        write_data_length = ((write_data_length > 1024) ? 1024 : write_data_length);

        memcpy(write_data_buffer, data_ptr, write_data_length);

 

        for (i = 0; i < write_data_length; i ++) {

                if ((i % transmit_legth) == 0) {

                        int retry = 5;

                        do {

                                ret = i2c_write(fd, STORAGE_DEV, (start_word_address + i), (write_data_buffer + i), transmit_legth);

                                if(ret == 0) {

                                        break;

                                }

                               mdelay(1);

                        } while (-- retry);

 

                        if (ret) {

                            printf("i2c write %i error!!!\n", i);

                        }

                }

        }

        return ret;

}

 

int

i2c_receive_data(int fd, unsigned char *data_ptr, unsigned short start_word_address ,unsigned short read_data_length)

{

        int i, ret = 0;

        unsigned char read_data_buffer[1024];       

        unsigned short receive_legth = 16;

 

        memset(read_data_buffer, 0x00, 1024);

        read_data_length = ((read_data_length > 1024) ? 1024 : read_data_length);

 

        for (i = 0; i < read_data_length; i ++) {

                if ((i % receive_legth) == 0) {

                        int retry = 5;

                        do {

                                ret = i2c_read(fd, STORAGE_DEV, (start_word_address + i), (read_data_buffer + i), receive_legth);

                                if(ret == 0) {

                                        break;

                                }

                               mdelay(1);

                        } while (-- retry);

                        if(ret == 0) {

#if 0

                                int count;

                                for (count = i; count < (i + receive_legth); count += 16) {

                                        printf("%d. [%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x]\n", count

                                                , read_data_buffer[count + 0]

                                                , read_data_buffer[count + 1]

                                                , read_data_buffer[count + 2]

                                                , read_data_buffer[count + 3]

                                                , read_data_buffer[count + 4]

                                                , read_data_buffer[count + 5]

                                                , read_data_buffer[count + 6]

                                                , read_data_buffer[count + 7]

                                                , read_data_buffer[count + 8]

                                                , read_data_buffer[count + 9]

                                                , read_data_buffer[count + 10]

                                                , read_data_buffer[count + 11]

                                                , read_data_buffer[count + 12]

                                                , read_data_buffer[count + 13]

                                                , read_data_buffer[count + 14]

                                                , read_data_buffer[count + 15]

                                        );

                                }

#endif

                        } else {

                            printf("i2c read %i error!!!\n", i);

                        }

                }

        }

 

        if (ret == 0) {

                memcpy(data_ptr, read_data_buffer, read_data_length);

        }

 

        return ret;

}

 

int str_isxdigit(char *tmp)

{

        int i;

        for (i = 0; i < strlen(tmp); i++) {

                if (!isxdigit(tmp[i]))

                        return -1;

        }

        return 0;

}

 

int str_isdigit(char *tmp)

{

        int i;

        for (i = 0; i < strlen(tmp); i++) {

                if (!isdigit(tmp[i]))

                        return -1;

        }

        return 0;

}

 

int main(int argc, char **argv)

{

 


    int fd = 0, ret = 0;
    int iTemp=0;
    int iRemoteTemp=0;
    char szTemp[4];
    float fTemp=0;
            storage_data.nmsgs = 2;

            storage_data.msgs = storage_msg;





            fd = open("/dev/i2c-0",O_RDWR);

            if(fd < 0){

                    perror("open I2C Dev error!!");

                    exit(1);

            }

            ioctl(fd, I2C_TIMEOUT, 1);/*set timeout value*/

            ioctl(fd, I2C_RETRIES, 2);/*set retry times*/
            unsigned char tmp_buffer[1];

            unsigned char tmp_buffer1[1];

            memset(tmp_buffer, 0x58, 1);

            memset(tmp_buffer1, 0x58, 1);

            i2c_read(fd,0x56,0x10,tmp_buffer,1);
            printf(szTemp,"%d\n",tmp_buffer[0]);
            iTemp = strtol(szTemp,NULL,10);
           // iTemp=iTemp-64;
            //printf("Data ==> %d\n", iTemp);
            //i2c_read(fd,0x56,0x0,tmp_buffer,1);
            //printf("Data ==> %x\n", tmp_buffer[0]);
            //i2c_read(fd,0x56,0x15,tmp_buffer,1);
            //printf("Data ==> %x\n", tmp_buffer[0]);

            i2c_read(fd,0x56,0x11,tmp_buffer,1);
            printf(szTemp,"%d\n",tmp_buffer[0]);
            iRemoteTemp = strtol(szTemp,NULL,10);
           // iRemoteTemp=iRemoteTemp-64;
            //printf("Data ==> %d\n", iTemp);
            i2c_read(fd,0x56,0x12,tmp_buffer,1);
            printf(szTemp,"%d\n",tmp_buffer[0]>>3);
            fTemp = strtol(szTemp,NULL,10);
           // fTemp=fTemp*0.0625+iRemoteTemp;
            //printf("Data ==> %0.4f\n", fTemp);
            //i2c_read(fd,0x56,0x1,tmp_buffer,1);
            //printf("Data ==> %x\n", tmp_buffer[0]);
            i2c_write(fd, 0x56, 0x47, tmp_buffer1, 1 );


    if (fd)
           close(fd);
 //   this->LocalTemp = QString("%1").arg(iTemp);
   // this->RemoteTemp = QString("%1").arg(fTemp);


    //emit finished();

}

 