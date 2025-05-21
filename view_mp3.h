#ifndef VIEW_MP3_H
#define VIEW_MP3_H

#include "types.h"

typedef struct _TagInfo
{
    FILE *fdata;
    char *fname;
    char frame_Id [4];

    char title_tag[50];
    uint title_tag_size;
    int title_pos;

    char artist_tag[50];
    uint artist_tag_size;
    int artist_pos;

    char album_tag[50];
    uint album_tag_size;
    int album_pos;

    char year[50];
    uint year_size;
    int year_pos;

    char content_type[50];
    uint content_type_size;
    int cont_pos;

    char *comments;
    uint comment_size;
    int comm_pos;

}TagInfo;

OperationType check_operation_type(char *argv[]);
Status read_and_validate(char *argv[],TagInfo *taginfo);
Status get_info(TagInfo *taginfo);
void big_to_little(uint *size);
Status display_data(TagInfo *taginfo);

#endif