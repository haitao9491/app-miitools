/*
 * (C) Copyright 2018
 * liye <ye.li@raycores.com>
 *
 * miitool.c - A description goes here.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <linux/mii.h>

static int read_phy_reg(int phy_addr, int phy_reg, int *value)
{
    struct mii_ioctl_data *mii;
    struct ifreq ifr;
    int ret = -1;
    int sockfd = 0;

    memset(&ifr, 0, sizeof(&ifr));
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
    sockfd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (!sockfd) {
//        printf("read_phy_reg sockfd err. \n"); 
        goto err_exit;
    }

//    mii = (struct mii_ioctl_data *)&(ifr.ifr_data);
    mii = (struct mii_ioctl_data *) &(ifr.ifr_ifru);
    mii->phy_id = phy_addr;
    mii->reg_num = (unsigned short)phy_reg;
    ret = ioctl(sockfd, SIOCGMIIREG, &ifr);
    ret = ioctl(sockfd, SIOCGMIIREG, &ifr);
    if (ret < 0) {
//       printf("read_phy_reg ioctl err. \n"); 
        goto err_exit;
    }
    
    if (value != NULL) {
        //printf("read phy addr: 0x%x  reg: 0x%x   value : 0x%x\n\n", mii->phy_id, mii->reg_num, mii->val_out);
        *value = mii->val_out;
    } else {
//        printf("read_phy_reg reg value is null. \n"); 
        goto err_exit;
    }

	close(sockfd);
    return 0;

err_exit:
    close(sockfd);
    return -1;
}

static int write_phy_reg(int phy_addr, int phy_reg, int value)
{
    struct mii_ioctl_data *mii;
    struct ifreq ifr;
    int ret = -1;
    int sockfd = 0;

    memset(&ifr, 0, sizeof(&ifr));
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
    sockfd = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (!sockfd) {
        printf("write_phy_reg socket sockfd err.\n");
        goto err_exit;
    }

    //mii = (struct mii_ioctl_data *)&(ifr.ifr_data);
    mii = (struct mii_ioctl_data *) &(ifr.ifr_ifru);
    mii->phy_id = phy_addr;
    mii->reg_num = (unsigned short)phy_reg;
    mii->val_in = value;
    ret = ioctl(sockfd, SIOCSMIIREG, &ifr);
    if (ret < 0) {
        printf("write_phy_reg ioctl err. \n");
        goto err_exit;
    }

//    printf("wirte phy addr: 0x%x  reg: 0x%x value in: 0x%x out: 0x%x\n", mii->phy_id, mii->reg_num, mii->val_in, mii->val_out);

    close(sockfd);
    return 0;

err_exit:
    close(sockfd);
    return -1;
}

void usage(char *appname)
{
    printf(" %s \n", appname);
    printf("    read  [phyaddr] [regaddr] [regnum] : read phy reg. phyaddr 0-24.\n");
    printf("    write [phyaddr] [regaddr] [regval] : write phy reg. phyaddr 0-24.\n");
    printf("\n");
}

int main(int argc, char **argv)
{
    int regnum = 0;
    int regval = 0;
    int regaddr = 0;
    int phyaddr = 0;
    int i = 0;

    if (argc < 4) {
        usage(argv[0]);
        return -1;
    }

    phyaddr = strtoul(argv[2], NULL, 0);
    if (phyaddr > 26) {
        usage(argv[0]);
        return -1;
    }
    regaddr = strtoul(argv[3], NULL, 0);

    if (!strcmp(argv[1], "read")) {
        if (argv[4]) {
            regnum = atoi(argv[4]);
        } else {
            regnum = 1; 
        }
        for (i = 1; i <= regnum; i++) {
            if (read_phy_reg(phyaddr, regaddr, &regval) != 0) {
                printf("[ERROR]: read phyaddr %d regaddr 0x%x .\n", phyaddr, regaddr);
                continue;
            }
            printf("read : phyaddr[%d] 0x%x = 0x%x.\n", phyaddr, regaddr, regval);
            regaddr++;
        }
    } else if (!strcmp(argv[1], "write")) {
        if (!argv[4]) {
            usage(argv[0]);
            return -1;
        }
        regval = strtoul(argv[4], NULL, 0);
        if (write_phy_reg(phyaddr, regaddr, regval) != 0) {
            printf("[ERROR]: write phyaddr %d regaddr 0x%x regval 0x%x.\n", phyaddr, regaddr, regval);
            return -1;
        }
        printf("write : phyaddr %d regaddr 0x%x regval 0x%x.\n", phyaddr, regaddr, regval);
    } else {
        usage(argv[0]);
        return -1;
    }

    return 0;
}
