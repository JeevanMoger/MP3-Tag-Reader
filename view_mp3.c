#include <stdio.h>
#include <string.h>
#include "types.h"
#include "view_mp3.h"

char buffer[5];
char data_buffer[100];
int titlePos;
uint size;

/**
 * @brief Determines the type of operation based on the command-line argument.
 *
 * @param argv[] Command-line arguments, where argv[1] is expected to contain an operation flag.
 * @return OperationType Returns one of the predefined operation types:
 *         - `view` if "-v" is passed.
 *         - `edit` if "-e" is passed.
 *         - `help` if "--help" is passed.
 *         - `unsupported` if none of the above matches.
 */
OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1],"-v") == 0) // Check if the argument is "-v", indicating a view operation
        return view;
    else if(strcmp(argv[1],"-e") == 0) // Check if the argument is "-e", indicating an edit operation
        return edit;
    else if(strcmp(argv[1],"--help") == 0) // Check if the argument is "--help", indicating a help request
        return help;
    else
        return unsupported; // If none of the known flags match, return unsupported
}

/**
 * @brief Reads and validates the provided file to check if it contains an ID3 tag.
 *
 * @return Returns `success` if the file is valid and contains an ID3 tag,
 *         otherwise returns `failure` in case of errors.
 */
Status read_and_validate(char *argv[], TagInfo *taginfo)
{
    // Check if the file name argument is provided
    if (argv[2] == NULL)
    {
        printf("ERROR : PLEASE ENTER THE FILE NAME TO VIEW\n");
        printf("INFO : FOR USAGE REFER --help \n");
        return failure;
    }
    else
    {
        // Attempt to open the file in read mode
        taginfo->fdata = fopen(argv[2], "r");
        if (taginfo->fdata != NULL)
        {
            char buff[3]; // Buffer to store the first three bytes of the file

            // Read the first three bytes from the file to check for an ID3 tag
            fread(buff, 1, 3, taginfo->fdata);
            
            // Compare the first three bytes with "ID3" to verify if it’s an MP3 ID3 tag
            if (strcmp(buff, "ID3") == 0)
            {
                printf("INFO : ID3 TAG FOUND\n");
                taginfo->fname = argv[2]; // Store the file name in the structure
            }
            else
            {
                printf("ERROR : ID3 TAG NOT FOUND\n");
                printf("ERROR : PLEASE PROVIDE THE MP3 FILE ONLY\n");
                return failure;
            }
        }
        else
        {
            printf("ERROR : UNABLE TO OPEN THE FILE\n");
            return failure;
        }
        fclose(taginfo->fdata); // Close the file after processing
    }
    return success;
}

/**
 * @brief Extracts metadata information from an MP3 file by reading specific ID3 tags.
 *
 * @return Returns `success` if all required tags are found and extracted, 
 *         otherwise returns `failure` if any critical tag is missing or file cannot be opened.
 */
Status get_info(TagInfo *taginfo)
{
    // Open the MP3 file in read mode
    taginfo->fdata = fopen(taginfo->fname, "r");
    if (taginfo->fdata == NULL)
    {
        printf("ERROR : ERROR OPENING MP3 FILE \n");
        return failure;
    }
    else
    {
        // Move to byte 10 in the file, where metadata tags start
        fseek(taginfo->fdata, 10, SEEK_SET);

        // Read first 4 bytes to check for the "TIT2" tag (Title)
        fread(buffer, 1, 4, taginfo->fdata);
        if (strcmp(buffer, "TIT2") == 0)
        {
            printf("INFO : TIT2 TAG FOUND\n");
            taginfo->title_pos = ftell(taginfo->fdata) - 4;
            printf("INFO : TITLE TAG FOUND AT POSITION %d\n", taginfo->title_pos);

            fread(&size, 1, 4, taginfo->fdata); // Read tag size
            big_to_little(&size); // Convert from big-endian to little-endian
            taginfo->title_tag_size = size;
            fseek(taginfo->fdata, 3, SEEK_CUR); // Skip 3 bytes (flags and encoding)
            fread(taginfo->title_tag, 1, size - 1, taginfo->fdata); // Read title data
        }
        else
        {
            printf("ERROR : TIT2 FLAG NOT FOUND\n");
            return failure;
        }

        // Process "TPE1" tag (Artist)
        fseek(taginfo->fdata, ftell(taginfo->fdata), SEEK_SET);
        fread(buffer, 1, 4, taginfo->fdata);
        if (strcmp(buffer, "TPE1") == 0)
        {
            printf("INFO : TPE TAG FOUND\n");
            taginfo->artist_pos = ftell(taginfo->fdata) - 4;
            printf("INFO : ARTIST TAG FOUND AT POSITION %d\n", taginfo->artist_pos);

            fread(&size, 1, 4, taginfo->fdata);
            big_to_little(&size);
            taginfo->artist_tag_size = size;
            fseek(taginfo->fdata, 3, SEEK_CUR);
            fread(taginfo->artist_tag, 1, size - 1, taginfo->fdata);
        }
        else
        {
            printf("ERROR : TPE1 TAG NOT FOUND\n");
            return failure;
        }

        // Process "TALB" tag (Album)
        fseek(taginfo->fdata, ftell(taginfo->fdata), SEEK_SET);
        fread(buffer, 1, 4, taginfo->fdata);
        if (strcmp(buffer, "TALB") == 0)
        {
            printf("INFO : TALB TAG FOUND\n");
            taginfo->album_pos = ftell(taginfo->fdata) - 4;
            printf("INFO : ALBUM TAG FOUND AT POSITION %d\n", taginfo->album_pos);

            fread(&size, 1, 4, taginfo->fdata);
            big_to_little(&size);
            taginfo->album_tag_size = size;
            fseek(taginfo->fdata, 3, SEEK_CUR);
            memset(data_buffer, '\0', sizeof(data_buffer)); // Clear buffer
            fread(taginfo->album_tag, 1, size - 1, taginfo->fdata);
        }
        else
        {
            printf("ERROR : TALB TAG NOT FOUND\n");
            return failure;
        }

        // Process "TYER" tag (Year)
        fseek(taginfo->fdata, ftell(taginfo->fdata), SEEK_SET);
        fread(buffer, 1, 4, taginfo->fdata);
        if (strcmp(buffer, "TYER") == 0)
        {
            printf("INFO : TYER TAG FOUND\n");
            taginfo->year_pos = ftell(taginfo->fdata) - 4;
            printf("INFO : YEAR TAG FOUND AT POSITION %d\n", taginfo->year_pos);

            fread(&size, 1, 4, taginfo->fdata);
            big_to_little(&size);
            taginfo->year_size = size;
            fseek(taginfo->fdata, 3, SEEK_CUR);
            fread(taginfo->year, 1, size - 1, taginfo->fdata);
        }
        else
        {
            printf("ERROR : TYER TAG NOT FOUND\n");
            return failure;
        }

        // Process "TCON" tag (Content type/Genre)
        fseek(taginfo->fdata, ftell(taginfo->fdata), SEEK_SET);
        fread(buffer, 1, 4, taginfo->fdata);
        if (strcmp(buffer, "TCON") == 0)
        {
            printf("INFO : TCON TAG FOUND\n");
            taginfo->cont_pos = ftell(taginfo->fdata) - 4;
            printf("INFO : CONTENT TAG FOUND AT POSITION %d\n", taginfo->cont_pos);

            fread(&size, 1, 4, taginfo->fdata);
            big_to_little(&size);
            taginfo->content_type_size = size;
            fseek(taginfo->fdata, 3, SEEK_CUR);
            fread(taginfo->content_type, 1, size - 1, taginfo->fdata);
        }
        else
        {
            printf("ERROR : TCON TAG NOT FOUND\n");
            return failure;
        }

        // Process "COMM" tag (Comments)
        fseek(taginfo->fdata, ftell(taginfo->fdata), SEEK_SET);
        fread(buffer, 1, 4, taginfo->fdata);
        if (strcmp(buffer, "COMM") == 0)
        {
            printf("INFO : COMM TAG FOUND\n");
            taginfo->comm_pos = ftell(taginfo->fdata) - 4;
            printf("INFO : COMMENT TAG FOUND AT POSITION %d\n", taginfo->comm_pos);

            fread(&size, 1, 4, taginfo->fdata);
            big_to_little(&size);
            taginfo->comment_size = size;
            fseek(taginfo->fdata, 3, SEEK_CUR);
            memset(data_buffer, '\0', sizeof(data_buffer)); // Clear buffer
            fread(data_buffer, 1, size - 1, taginfo->fdata);

            // Ensure proper formatting of comments
            if (data_buffer[3] == '\0')
                data_buffer[3] = ' ';
            taginfo->comments = data_buffer;
        }
        else
        {
            printf("ERROR : COMM TAG NOT FOUND\n");
            return failure;
        }
    }
    return success;
}
void big_to_little(uint *size)
{
    *size = ((*size >> 24) & 0x000000FF) | ((*size >> 8)  & 0x0000FF00) | ((*size << 8)  & 0x00FF0000) | ((*size << 24) & 0xFF000000);
} 
//print the details
Status display_data(TagInfo *taginfo)
{
    printf("%-10s : %-10s\n","TITLE",taginfo -> title_tag);
    printf("%-10s : %-10s\n","ARTIST",taginfo -> artist_tag);
    printf("%-10s : %-10s\n","ALBUM",taginfo -> album_tag);
    printf("%-10s : %-10s\n","YEAR",taginfo -> year);
    printf("%-10s : %-10s\n","MUSIC",taginfo -> content_type);
    printf("%-10s : %-10s\n","COMMENTS",taginfo -> comments);
    return success;
}