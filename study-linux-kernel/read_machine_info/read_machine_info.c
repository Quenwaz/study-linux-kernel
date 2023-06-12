#define _GNU_SOURCE
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* GetDiskSerialNum()
{
	char serialnum[32];
	static struct hd_driveid hd;
	int fd;

	if (geteuid() > 0)
	{
		//printf("%s", "错误: 请以root权限运行程序");
		return "";
	}
	//寻找sda和hda设备
	if ((fd = open("/dev/sda", O_RDONLY)) < 0)
	{
		//printf("%s", "错误: 无法找到有效的磁盘存储设备");
		return "";
	}

    unsigned char identity[512];
    int ret = ioctl(fd, HDIO_GET_IDENTITY, identity);
	if (ret != -1)
	{
		sprintf(serialnum, "%s", hd.serial_no);
	}
	else if (errno == -ENOMSG)
	{
		//printf("%s", "错误: 无法识别的磁盘存储设备");
		return "";
	}
	else
	{
		//perror("ERROR: HDIO_GET_IDENTITY");
		//printf("%s", "ERROR: HDIO_GET_IDENTITY");
        perror(strerror(errno));
		return "";
	}
	static char finalNum[32];
	memset(finalNum, 0, 32);
	int k = 0;
	for (; k < 32; k++)
	{
		if (serialnum[k] != ' ')
		{
			break;
		}
	}
	strncpy(finalNum, &serialnum[k], 32 - k);

	//硬盘序列号
	return finalNum;
}

void getSerialNum(){
   const char *p="Serial"; //this key is serical num
   FILE *cmdline = fopen("/proc/cpuinfo", "rb");
   char *arg = 0;
   size_t size = 0;
   //getdeli分割读取字段
   while(getdelim(&arg, &size, 0, cmdline) != -1)
   {
      if(strstr(arg,p) != NULL){
         puts(arg);//put 输出
      }
   }
   free(arg);
   fclose(cmdline);
}


int main(int argc, char const *argv[])
{
    GetDiskSerialNum();
    getSerialNum();
    return 0;
}
