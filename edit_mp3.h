#ifndef EDIT_H
#define EDIT_H

#include "types.h"
#include "view_mp3.h" 

typedef struct _TagData
{
    FILE* fdata;
    FILE* ftemp;
    char fname[50];
    char tag_Id [5];
    char tag_data [100];
    uint data_size;
} TagData;

Status read_and_validate_edit(char *argv[], TagData *tagdata);
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);
Status edit_data(TagData *tagdata,TagInfo *taginfo);
#endif