#include "boot_sector.h"

void error_mirror_flags()
{
    printf("ERROR: error with finding or reading FAT\n");
    exit(0);
}

void error_FATs()
{
    printf("ERROR: error with FATs, not equal\n");
}

void error_memory_FAT()
{
    printf("ERROR: cannot allocate memory for FATs\n");
    exit(0);
}

__int1 number_of_FATs(__int2 mirror_flags, __int1 number_FAT_copies)
{
    int current_number_FAT = number_FAT_copies;
    int current_flag = (mirror_flags & 0x0080);
    if (current_flag){
        current_number_FAT = 1; // 7-th bit of flag meams that active is only one FAT, if 7-th bit = zero, all FATs are active
    }
    return current_number_FAT;
}

char **read_FAT(int device_descriptor, __int8 offset_first_FAT, __int2 mirror_flags, __int1 number_FAT_copies, __int8 count_bytes_FAT)
{
    int current_number_FAT = number_FAT_copies;
    int current_flag = (mirror_flags & 0x0080);
    int is_simple_FAT = 0;
    int isFind = 0;
    if (current_flag){
        current_number_FAT = 1; // 7-th bit of flag meams that active is only one FAT, if 7-th bit = zero, all FATs are active
        is_simple_FAT = 1;
        if ((mirror_flags & 0x0001) > 0){
            isFind++;
        }
        if ((mirror_flags & 0x0002) > 0){
            isFind++;
            offset_first_FAT += count_bytes_FAT;
        }
        if ((mirror_flags & 0x0004) > 0){
            isFind++;
            offset_first_FAT += 2*count_bytes_FAT;
        }
        if ((mirror_flags & 0x0008) > 0){
            isFind++;
            offset_first_FAT += 3*count_bytes_FAT;
        }
    }
    if (is_simple_FAT && (isFind == 0 || isFind > 1)){
        error_mirror_flags();
        return NULL;
    }

    char ** FATs = (char**)malloc(current_number_FAT * sizeof(char*));

    if (!FATs){
        error_memory_FAT();
        return NULL;
    }

    for (int i = 0; i < current_number_FAT; i++){
        FATs[i] = (char*)malloc(count_bytes_FAT * sizeof(char));
        if (!FATs[i]){
            error_memory_FAT();
            free(FATs);
            return NULL;
        }
    }

    if (lseek(device_descriptor, offset_first_FAT, 0) == -1L){
        error_mirror_flags();
        return NULL;
    }

    for (int i = 0; i < current_number_FAT; i++){
        if (read(device_descriptor, FATs[i], count_bytes_FAT) == 0){
            error_mirror_flags();
            return NULL;
        }
    }
    return FATs;
}

int check_equal_FAT(char **FATs, __int1 number_of_FATs, __int8 size_of_FAT)
{
    for (int i = 0; i < number_of_FATs - 1; i++){
        for (int j = 0; j < size_of_FAT; j++){
            if (FATs[i][j] != FATs[i+1][j]){
                return 0;
            }
        }

    }
    return 1;
}
