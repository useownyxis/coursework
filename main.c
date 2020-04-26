#include <fcntl.h>	/* open() and O_XXX flags */
#include <sys/stat.h>	/* S_IXXX flags */
#include <sys/types.h>	/* mode_t */
#include <unistd.h>	/* close() */
#include <stdlib.h>
#include <stdio.h>

#include "FSInfo.h"
#include "FAT.h"

int full_information = 0;

void output_full_information(struct BOOT_SECTOR_AC current_boot_sector, struct FSInfo current_FSInfo)
{
    output_boot_sector(current_boot_sector);
    output_FSInfo(current_FSInfo);
}

void output_help()
{
    printf("For using this programm, please inter in folder with this programm.\n");
    printf("Compile it: gcc -Wall -o main main.c\n");
    printf("Then run it: sudo ./main \'Your device path\' -flags\n");
    printf("Some flags:\n");
    printf("   -h -> for output help\n");
    printf("   -f -> for output full information of Your device\n");
    printf("If your defive is normal formating to FAT32, output of this programm will be empty\n");
}

void check_command(int argc, char const *argv[])
{
    for (int i = 1; i < argc; i++){
        if (argv[i][1] == 'h'){
            output_help();
            exit(0);
        }
        if (argv[i][1] == 'f'){
            full_information = 1;
        }
    }
}

int main(int argc, char const *argv[]) {
    check_command(argc, argv);

    const char * device_path = argv[1];

    int current_device_descriptor = open_device(device_path);

    struct BOOT_SECTOR_BC current_boot_sector_BC = read_boot_sector(current_device_descriptor);
    struct BOOT_SECTOR_AC current_boot_sector_AC = converting_boot_sector(current_boot_sector_BC);
    struct FSInfo current_FSInfo = read_FSInfo(current_device_descriptor);

    seek_descriptor(current_device_descriptor, current_boot_sector_AC.backup_boot_sector_location * current_boot_sector_AC.bytes_per_sector);

    if (full_information){
        output_full_information(current_boot_sector_AC, current_FSInfo);
    }

    check_FAT_32(current_boot_sector_AC);

    if (!check_correct_FSInfo(current_FSInfo)){
        output_FSInfo_error();
    }

    struct BOOT_SECTOR_BC backup_boot_sector_BC = read_boot_sector(current_device_descriptor);
    struct FSInfo backup_FSInfo = read_FSInfo(current_device_descriptor);

    int is_normal_boot_sector = check_boot_sector(current_boot_sector_AC);

    if (check_equal_boot_sector(current_boot_sector_BC, backup_boot_sector_BC) != 0){
        warning_boot_sector();
    }

    if (!check_equal_FSInfo(current_FSInfo, backup_FSInfo)){
        warning_FSInfo();
    }






    char **FATs = read_FAT(current_device_descriptor,
                           current_boot_sector_AC.number_reserved_sectors * current_boot_sector_AC.bytes_per_sector,
                           current_boot_sector_AC.mirror_flags,
                           current_boot_sector_AC.number_FAT_copies,
                           current_boot_sector_AC.number_of_sectors_per_FAT * current_boot_sector_AC.bytes_per_sector

        );

    if (!check_equal_FAT(
                            FATs,
                            number_of_FATs(current_boot_sector_AC.mirror_flags, current_boot_sector_AC.number_FAT_copies),
                            current_boot_sector_AC.number_of_sectors_per_FAT * current_boot_sector_AC.bytes_per_sector))
                    {
                            error_FATs();
                    }
    //TODO CHECK SIZE FOR fat32

    return 0;
}
